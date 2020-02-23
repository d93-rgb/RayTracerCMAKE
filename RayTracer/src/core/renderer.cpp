#include "core/renderer.h"
#include "shape/ray.h"
#include "scene/scene.h"
#include "camera/camera.h"
#include "shape/shape.h"
#include "light/light.h"
#include "image/image.h"
#include "samplers/sampler2D.h"
#include "threads/dispatcher.h"

namespace rt
{
//#define DEBUG_NORMALS

Renderer::Renderer(size_t w, size_t h,
	const std::string& file,
	size_t max_depth) :
	MAX_DEPTH(max_depth),
	img(new Image(w, h, file)),
	SPP(1),
	GRID_DIM(3),
	NUM_THREADS(4)
{}

/*
	Calculate the normalized reflection vector.
	dir	: the incident ray
	N	: the normalized normal vector of a surface
*/
glm::dvec3 Renderer::reflect(glm::dvec3 dir, glm::dvec3 N)
{
	return glm::normalize(dir - 2 * glm::dot(N, dir) * N);
}

/*
	Calculate the normalized refracted vector.
	V	: the view direction
	N	: the normalized normal vector of a surface
*/
bool Renderer::refract(glm::dvec3 V, glm::dvec3 N, double refr_idx, glm::dvec3* refracted)
{
	double cos_alpha = glm::dot(-V, N);

	// TODO: calculate refracted ray when coming from a medium other than air
	// refractive index of air = 1.f
	double eta = 1.0 / refr_idx;

	if (cos_alpha < 0.f)
	{
		eta = 1.0 / eta;
		cos_alpha *= -1;
		N = -N;
	}

	double radicand = 1.0 - eta * eta * (1.0 - cos_alpha * cos_alpha);

	// check for total internal reflection
	if (radicand < 0.0)
	{
		*refracted = glm::dvec3(0.0);
		return false;
	}

	*refracted = glm::normalize(eta * V - (eta * cos_alpha + sqrt(radicand)) * N);
	return true;
}

/*
	Compute the fresnel term, that is, the factor for reflective contribution
	rel_eta: the relative refractive coefficient
	c: the cosine of the angle between incident and normal ray
*/
double Renderer::fresnel(double rel_eta, double c)
{

	if (c < 0.0)
	{
		c = -c;
		rel_eta = 1.0 / rel_eta;
	}
	// using Schlick's approximation
	double r0 = (rel_eta - 1.0) / (rel_eta + 1.0);
	r0 = r0 * r0;

	c = 1.0 - c;

	return r0 + (1.0 - r0) * pow(c, 5);
}

glm::dvec3 Renderer::handle_reflection(const Scene& s,
	const Ray& ray,
	const glm::dvec3& isect_p,
	SurfaceInteraction* isect,
	int depth)
{
	glm::dvec3 reflected = reflect(ray.rd, isect->normal);

	return shoot_recursively(s,
		Ray(isect_p + shadowEpsilon * reflected, reflected),
		isect,
		++depth);
}

glm::dvec3 Renderer::handle_transmission(const Scene& s,
	const Ray& ray,
	const glm::dvec3& isect_p,
	SurfaceInteraction* isect,
	int depth)
{
	glm::dvec3 reflected, refracted;
	double f;

	reflected = reflect(ray.rd, isect->normal);

	// check for total internal reflection
	if (!refract(ray.rd, isect->normal, isect->mat->getRefractiveIdx(), &refracted))
	{
		//reflected = glm::normalize(reflect(ray.rd, (*o)->get_normal(isect_p)));
		return shoot_recursively(s,
			Ray(isect_p + shadowEpsilon * reflected, reflected),
			isect,
			++depth);
	}

	f = fresnel(1.f / isect->mat->getRefractiveIdx(),
		glm::dot(-ray.rd, isect->normal));
	++depth;

	return f * shoot_recursively(s,
		Ray(isect_p + shadowEpsilon * reflected, reflected),
		isect,
		depth) +
		(1.f - f) * shoot_recursively(s,
			Ray(isect_p + shadowEpsilon * refracted, refracted),
			isect,
			depth);
}

/*
	Shoot next ray and obtain the next intersection point.
	Returns the distance to the hit surface and saves hit object
	in the given pointer 'o'.
	s: the scene with its objects
	ray: the next ray to trace
	o: the object that was hit
*/
double Renderer::shoot_ray(const Scene& s, const Ray& ray, SurfaceInteraction* isect)
{
	double t_int = INFINITY;
	double tmp = INFINITY;

	// get nearest intersection point
	for (auto& objs : s.get_scene())
	{
		tmp = objs->intersect(ray, isect);

		//if (tmp >= 0 && t_int > tmp)
		//{
		//	t_int = tmp;
		//	// update intersection data with the properties of the closer object
		//	//col[i] = sphs.color * glm::max(0.f, glm::dot(-rd, sphs.get_normal(inters_p)));
		//	//std::cout << col[i].x << " " << col[i].y << " " << col[i].z << std::endl;
		//}
	}
	return ray.tNearest;
}

glm::dvec3 Renderer::shoot_recursively(const Scene& s,
	const Ray& ray,
	SurfaceInteraction* isect,
	int depth)
{
	if (depth == MAX_DEPTH)
	{
		return glm::dvec3(0);
	}

	double distance;
	glm::dvec3 contribution = glm::dvec3(0);
	glm::dvec3 isect_p;

	distance = shoot_ray(s, ray, isect);

	// check for no intersection
	if (distance < 0 || distance == INFINITY)
	{
		return glm::dvec3(0.0f);
	}

	isect_p = ray.ro + distance * ray.rd;

	//if ((glm::length((*o)->mat->ambient) > 0) || (glm::length((*o)->mat->specular) > 0))
		// accumulate all light contribution

	// map direction of normals to a color for debugging
#ifdef DEBUG_NORMALS
	return contribution = (glm::dvec3(1.f) + isect->normal) * 0.5f;
#endif

	for (auto& l : s.lights)
	{
		contribution += l->phong_shade(s,
			ray/*Ray(ray.ro + shadowEpsilon * ray.rd, ray.rd)*/,
			isect_p,
			*isect);
	}

	if (glm::length(isect->mat->getReflective()) > 0)
	{
		glm::dvec3 reflective = isect->mat->getReflective();
		contribution += reflective * handle_reflection(s, ray, isect_p, isect, depth);
	}

	if (glm::length(isect->mat->getTransparent()) > 0)
	{
		glm::dvec3 transparent = isect->mat->getTransparent();
		contribution += transparent * handle_transmission(s, ray, isect_p, isect, depth);
	}

	return contribution;
}

void Renderer::render_gradient(
	size_t& width_img,
	const size_t& width_stripe,
	size_t& height)
{
	// do not let RAM explode, limit maximum stripe width
	assert(width_stripe < 4e3);

	// set image width and height
	width_img = 256 * width_stripe;
	height = img->get_height();

	img->resize_color_array(256 * width_stripe, height);
	img->colors.resize(256 * width_stripe * height);

	for (int k = 0; k < 256; ++k)
	{
		for (unsigned int i = 0; i < height; ++i)
		{
			for (unsigned int j = 0; j < width_stripe; ++j)
			{
				img->colors[i * width_img + j + k * width_stripe] = glm::dvec3(double(k) / 255.0f);
			}
		}
	}
}

/*
	Starts rendering a scene and returns the color vector.
*/
void Renderer::render(
	size_t& width,
	size_t& height)
{
	constexpr double fov = glm::radians(90.f);
	double fov_tan = tan(fov / 2);
	double u = 0.f, v = 0.f;
	// distance to view plane
	double d = 1.f;
	double inv_spp;
	double inv_grid_dim = 1.f / (GRID_DIM * GRID_DIM);

	double crop_min_x = 0.f, crop_max_x = 1.f;
	double crop_min_y = 0.f, crop_max_y = 1.f;

	assert(crop_min_x <= crop_max_x && crop_min_y <= crop_max_y);

	size_t cropped_width[2];
	size_t cropped_height[2];

	crop(crop_min_x, crop_max_x, img->get_width(), cropped_width);
	crop(crop_min_y, crop_max_y, img->get_height(), cropped_height);

	width = cropped_width[1] - cropped_width[0];
	height = cropped_height[1] - cropped_height[0];

	LOG(INFO) << "Image width = " << img->get_width() << "; Image height = " << img->get_height();
	LOG(INFO) << "Cropped width = " << width << "; Cropped height = " << height;

	StratifiedSampler2D sampler{ width, height, GRID_DIM };
	size_t array_size = GRID_DIM * GRID_DIM;
	const glm::dvec2* samplingArray;
	inv_spp = 1.f; // sampler.samplesPerPixel;
	/***************************************/
	// CREATING SCENE
	/***************************************/
	//GatheringScene sc;
	MixedScene sc;
	//	// enclose with braces for destructor of ProgressReporter at the end of rendering
	{
		/***************************************/
		// START PROGRESSREPORTER
		/***************************************/
		pbrt::ProgressReporter reporter(img->get_height(), "Rendering:");
		/***************************************/
		// LOOPING OVER PIXELS
		/***************************************/
		// dynamic schedule for proper I/O progress update
//#pragma omp parallel for schedule(dynamic, 1)
		for (size_t y = cropped_height[0]; y < cropped_height[1]; ++y)
		{
			//fprintf(stderr, "\rRendering %5.2f%%", 100.*y / (HEIGHT - 1));
			reporter.Update();
			for (size_t x = cropped_width[0]; x < cropped_width[1]; ++x)
			{
				//TODO: NOT threadsafe
				samplingArray = sampler.get2DArray();

				// hackery needed for omp pragma
				// the index i will be distributed among all threads
				// by omp automatically
				size_t i = (y - cropped_height[0]) * width + (x - cropped_width[0]);
				for (size_t idx = 0; idx < array_size; ++idx)
				{
					SurfaceInteraction isect;

					// map pixel coordinates to[-1, 1]x[-1, 1]
					double u = (2.f * (x + samplingArray[idx].x) - img->get_width()) / img->get_height() * fov_tan;
					double v = (-2.f * (y + samplingArray[idx].y) + img->get_height()) / img->get_height() * fov_tan;

					// this can not be split up and needs to be in one line, otherwise
					// omp will not take the
					/*col[i] += clamp(shoot_recursively(sc, sc.cam->getPrimaryRay(u, v, d), &isect, 0))
						* inv_grid_dim;*/
					img->colors[i] = glm::normalize(sc.cam->getPrimaryRay(u, v, d).rd);
				}
			}
		}
		reporter.Done();
	}
}

void Renderer::render_with_threads(
	size_t& width,
	size_t& height)
{
	constexpr double fov = glm::radians(30.0);
	double fov_tan = tan(fov / 2);
	double u = 0.0, v = 0.0;
	// distance to view plane
	double foc_len = 0.5 * 1.0 / fov_tan;
	double inv_spp;
	double inv_grid_dim = 1.f / (GRID_DIM * GRID_DIM);

	double crop_min_x = 0.0, crop_max_x = 1.0;
	double crop_min_y = 0.0, crop_max_y = 1.0;

	assert(crop_min_x <= crop_max_x && crop_min_y <= crop_max_y);

	size_t cropped_width[2];
	size_t cropped_height[2];

	crop(crop_min_x, crop_max_x, img->get_width(), cropped_width);
	crop(crop_min_y, crop_max_y, img->get_height(), cropped_height);

	width = cropped_width[1] - cropped_width[0];
	height = cropped_height[1] - cropped_height[0];

	LOG(INFO) << "Image width = " << img->get_width() << "; Image height = " << img->get_height();
	LOG(INFO) << "Cropped width = " << width << "; Cropped height = " << height;

#ifdef BLACK_COLOR_ARRAY_FOR_DEBUGGING
	return col;
#endif

	StratifiedSampler2D sampler{ width, height, GRID_DIM };
	size_t array_size = GRID_DIM * GRID_DIM;
	const glm::dvec2* samplingArray;
	inv_spp = 1.0 / SPP;

	std::unique_ptr<Scene> sc = std::make_unique<TeapotScene>();

	// enclose with braces for destructor of ProgressReporter at the end of rendering
	{
		Slice slice(*img, 16, 16);
		std::mutex pairs_mutex;
		std::mutex sampler_mutex;
		std::vector<std::thread> threads_v;

		// launch progress reporter
		pbrt::ProgressReporter reporter(slice.dx * slice.dy, "Rendering:");

		// start rendering with threads
		for (int i = 0; i < NUM_THREADS; ++i)
		{
			threads_v.push_back(std::thread([&]() {
				int idx = 0;
				unsigned int h_step;
				unsigned int w_step;

				while (idx != -1)
				{
					// try to access the next free image raster
					pairs_mutex.lock();
					idx = slice.get_index();
					pairs_mutex.unlock();

					if (idx < 0)
					{
						break;
					}

					assert(idx < slice.get_length());

					// get step range
					w_step = std::min(slice.w_step, slice.img_width - slice.pairs[idx].first);
					h_step = std::min(slice.h_step, slice.img_height - slice.pairs[idx].second);

					for (unsigned int i = 0; i < h_step; ++i)
					{
						for (unsigned int j = 0; j < w_step; ++j)
						{
							sampler_mutex.lock();
							samplingArray = sampler.get2DArray();
							sampler_mutex.unlock();

							for (int s = 0; s < SPP; ++s)
							{
								for (size_t n = 0; n < array_size; ++n)
								{
									SurfaceInteraction isect;

									// map pixel coordinates to[-1, 1]x[-1, 1]
									double u = (2.0 * (slice.pairs[idx].first + j + samplingArray[n].x) - img->get_width()) / img->get_height();
									double v = (-2.0 * (slice.pairs[idx].second + i + samplingArray[n].y) + img->get_height()) / img->get_height();

									/*double u = (x + samplingArray[idx].x) - WIDTH * 0.5f;
									double v = -((y + samplingArray[idx].y) - HEIGHT * 0.5f);
							*/
									img->colors[(slice.pairs[idx].second + i) * slice.img_width + slice.pairs[idx].first + j] +=
										clamp(shoot_recursively(
											*sc, sc->cam->getPrimaryRay(u, v, foc_len), &isect, 0));
								}
							}
							img->colors[(slice.pairs[idx].second + i) * slice.img_width + slice.pairs[idx].first + j] *=
								inv_grid_dim * inv_spp;
						}
					}
					reporter.Update();
				}
				}));
		}

		for (int i = 0; i < NUM_THREADS; ++i)
		{
			threads_v[i].join();
		}

		reporter.Done();
	}
}

/*
	Short helper function
*/
void Renderer::run(RenderMode mode)
{
	size_t width, height;

	if (mode == RenderMode::NO_THREADS)
	{
		render(width, height);
	}
	else if (mode == RenderMode::THREADS)
	{
		render_with_threads(width, height);
	}
	else if (mode == RenderMode::GRADIENT)
	{
		render_gradient(width, 10, height);
	}
	else
	{
		render_with_threads(width, height);
	}

	if (img->get_file_name().empty())
	{
		char buf[200];
		GET_PWD(buf, 200);
		std::cout << buf << std::endl;
		std::string fn = buf;


		LOG(INFO) << "Image will be written to \"" <<
			fn.substr(0, fn.find_last_of("\\/")).append(OS_SLASH).append(
				img->get_file_name());
		img->write_image_to_file();
	}
	else
	{
		//img->append_to_file_name(".ppm");
		img->write_image_to_file();
	}
}

std::vector<glm::dvec3> Renderer::get_colors() const
{
	return img->colors;
}

glm::u64vec2 Renderer::get_image_dim() const
{
	return glm::u64vec2(img->get_width(), img->get_height());
}



} // namespace rt
