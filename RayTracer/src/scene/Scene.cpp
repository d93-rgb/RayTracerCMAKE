// obj file loader stuff
#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "scene/scene.h"
#include "camera/camera.h"
#include "misc/loader.h"
#include "material/material.h"
#include "shape/shape.h"
#include "shape/quadric/quadrics.h"
#include "light/light.h"
#include "shape/bvh.h"

//#define SHOW_AXIS
//#define LOAD_TEAPOT
// DEBUGGING
// divide the triangle mesh of the teapot to reduce rendering time
constexpr auto TEAPOTSIZE = 1;

namespace rt
{

Scene::Scene() = default;

Scene::~Scene() = default;

void GatheringScene::init()
{
	cam.reset(new Camera());

	float rot_y = glm::radians(0.f);
	float radius[] = { 1, 1.5, 3, 2, 4 , 4, 2, 3, 2 };

	glm::vec3 translation = glm::vec3(0.f, 3.f, 20.f);

	std::vector<glm::vec3> sph_origins = {
		glm::vec3(-10, -2, -5),
		glm::vec3(-9, 21, -22),
		glm::vec3(9, 3, -15),
		glm::vec3(-11, 7, -15),
		glm::vec3(0, 12, -25),
		glm::vec3(5, -2, -11),
		glm::vec3(-6, -3, -4),
		glm::vec3(-8, 4, -2),
		//glm::vec3(0.f, 0.f, 0.f)
	};
	std::unique_ptr<Shape> cube_1[6], cube_2[6], cube_3[6];


	std::vector<std::shared_ptr<Material>> mats = {
		std::make_shared<Material>(glm::vec3(0.02, 0, 0), glm::vec3(0.7, 0, 0), glm::vec3(1.0, 0, 0)),
		std::make_shared<Material>(glm::vec3(0, 0.02, 0), glm::vec3(0, 0.7, 0), glm::vec3(0, 1.0, 0)),
		std::make_shared<Material>(glm::vec3(0.02, 0, 0.02), glm::vec3(0.7, 0, 0.7), glm::vec3(0.7, 0, 0.7)),
		std::make_shared<Material>(glm::vec3(0.013, 0.013, 0.035), glm::vec3(0.3, 0.3, 0.8), glm::vec3(0.7, 0.7, 0.7)),
		std::make_shared<Material>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0, 0, 0)),
		std::make_shared<Material>(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0, 0, 0)),
		std::make_shared<Material>(glm::vec3(0.02, 0.02, 0.f), glm::vec3(0.8, 0.8, 0.0), glm::vec3(0.5, 0.5, 0)),
		std::make_shared<Material>(glm::vec3(0.0, 0.0f, 0.0f), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0f, 0.0f, 0.0f)),
		std::make_shared<Material>(glm::vec3(0, 0.f, 0.f), glm::vec3(1.0, 1.0, 1.0), glm::vec3(0.f, 0.f, 0.f))
	};

	mats[2]->setShininess(10.f);
	mats[3]->setShininess(5.f);

	mats[4]->setReflective(glm::vec3(0.8f));
	mats[5]->setReflective(glm::vec3(1.0f)); // ideal mirror

	// glass sphere
	mats[7]->setTransparent(glm::vec3(1.f));
	mats[7]->setRefractiveIdx(1.5f);

	// material for walls
	auto wall_bot =
		std::make_shared<Material>(glm::vec3(0.02, 0.02, 0.02), glm::vec3(0.4, 0.4, 0.4), glm::vec3(0.0, 0.0, 0.0));
	auto wall_front =
		std::make_shared<Material>(glm::vec3(0.02, 0.02, 0.02), glm::vec3(0.2, 0.2, 0.2), glm::vec3(0.1, 0.1, 0.1));
	auto wall_right =
		std::make_shared<Material>(glm::vec3(0.02, 0.024, 0.016), glm::vec3(0.5, 0.6, 0.4), glm::vec3(0.1, 0.1, 0.1));
	auto wall_left =
		std::make_shared<Material>(glm::vec3(0.02, 0.02, 0.02), glm::vec3(0.2, 0.4, 0.6), glm::vec3(0.1, 0.1, 0.1));

	wall_bot->setReflective(glm::vec3(0.2f));

	for (size_t i = 0; i < sph_origins.size(); ++i)
	{
		sc.emplace_back(std::make_unique<Sphere>(sph_origins[i], radius[i], glm::vec3(0.f), mats[i]));
	}

	//bottom
	sc.emplace_back(std::make_unique<Rectangle>(glm::vec3(-4, 2, -18),
		glm::vec3(150, 0, 0), glm::vec3(0, 150, -150), wall_bot));

	// get pointer to the floor
	Rectangle *floor = dynamic_cast<Rectangle*>(sc.back().get());

	//front
	//sc.emplace_back(std::unique_ptr<Shape>(new Rectangle(glm::vec3(-4, 11, -27),
	//	glm::vec3(50, 0, 0), glm::vec3(0, 70, 0), wall_right)));
	////right
	//sc.emplace_back(std::unique_ptr<Shape>(new Rectangle(glm::vec3(21, 2, -18),
	//	glm::vec3(0, 50, 0), glm::vec3(0, 0, -60), wall_right)));
	////left
	//sc.emplace_back(std::unique_ptr<Shape>(new Rectangle(glm::vec3(-29.f, 2, -18),
	//	glm::vec3(0, 50, 0), glm::vec3(0, 0, 60), wall_left)));

	// cube material
	auto cube_mat_1 =
		std::shared_ptr<Material>(new Material(
			glm::vec3(0.02, 0.0, 0.02),
			glm::vec3(0.1, 0.0, 0.7f),
			glm::vec3(0.6, 0.0, 0.6)));
	auto cube_mat_2 =
		std::shared_ptr<Material>(new Material(
			glm::vec3(0.02, 0.02, 0.0),
			glm::vec3(0.9, 0.2, 0.0f),
			glm::vec3(0.6, 0.0, 0.6)));
	cube_mat_1->setShininess(10.f);
	cube_mat_2->setShininess(10.f);


	glm::vec4 cube_position = floor->getRectPos(13.f, -25.f, 'y');
	glm::vec3 cube_normal = floor->get_normal();

	create_cube(cube_position + 2.f * glm::vec4(cube_normal, 0.f),
		cube_normal,
		glm::vec3(1.f, 0.f, 0.f),
		4.f,
		cube_1,
		cube_mat_1);
	create_cube(glm::vec3(14.f, 8.f, -3.f),
		//glm::rotate(glm::mat4(1), -30.f, glm::vec3(1.f, 0.f, 0.f)) *
		//glm::vec4(0.f, 0.f, 1.f, 1.f),
		cube_normal,
		/*glm::rotate(glm::mat4(1), -30.f, glm::vec3(1.f, 0.f, 0.f)) *
		glm::vec4(1.f, 0.f, 0.f, 1.f),*/
		glm::perp(glm::vec3(1.f, 1.f, 0.f), cube_normal),
		6.f,
		cube_2,
		cube_mat_2);
	/*create_cube(glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(1.f, 0.f, 0.f),
		glm::vec3(0.f, 1.f, 0.f),
		5.f,
		cube_3,
		cube_mat_2);
	*/

	////////////////////////////////
	// NEW CUBE
	////////////////////////////////
	// cube material for new cube class object
	auto new_cube_mat = std::shared_ptr<Material>(new Material(glm::vec3(0.01f, 0.02f, 0.005f),
		glm::vec3(0.2f, 0.6f, 0.1f),
		glm::vec3(0.2f, 0.6f, 0.1f)));

	sc.emplace_back(std::unique_ptr<Shape>(new Cube(glm::vec3(3.f), new_cube_mat)));
	sc.back()->obj_to_world = glm::rotate(glm::scale(glm::translate(
		glm::mat4(1.f),
		glm::vec3(0.f, -1.f, 10.f)),
		glm::vec3(1.25f, 0.5f, 1.f)),
		glm::radians(60.f),
		glm::vec3(1.f, 0.f, 0.f));

	sc.back()->world_to_obj = glm::inverse(sc.back()->obj_to_world);
	////////////////////////////////
	// END
	////////////////////////////////

	for (std::unique_ptr<Shape> &p : cube_1)
	{
		sc.emplace_back(std::move(p));
	}
	for (std::unique_ptr<Shape> &p : cube_2)
	{
		sc.emplace_back(std::move(p));
	}
	/*for (std::unique_ptr<Shape> &p : cube_3)
	{
		sc.emplace_back(std::move(p));
	}*/
	// add lights to the scene
	//lights.emplace_back(std::unique_ptr<Light>(new DistantLight(glm::vec3(-2, -4, -2), glm::vec3(0.8f))));
	//lights.emplace_back(std::unique_ptr<Light>(new DistantLight(glm::vec3(0, 0, -1), glm::vec3(0.8f))));

	lights.emplace_back(std::unique_ptr<Light>(new PointLight(glm::vec3(-2.f, 4.f, -17.f),
		glm::vec3(-2, -4, -2),
		glm::vec3(100.f))));


	cam->setCamToWorld(translation, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
	cam->update();
}

void MixedScene::init()
{
	//glm::vec3 translation = glm::vec3(0.f, sqrtf(2.f), sqrtf(2.f));
	glm::vec3 translation = glm::vec3(0.f, 5.f, 30.f);
	//glm::vec3 look_pos = glm::vec3(0.f, -sqrtf(2.f), -sqrt(2.f));
	glm::vec3 look_pos = glm::vec3(0.f, 0.f, -10.f);
	glm::vec3 cam_up = glm::vec3(0.f, 1.f, 0.f);
	glm::vec4 cube_position;
	glm::vec3 cube_normal;
	//	glm::vec3 p1, p2, p3, tr_normal;

	std::string teapot =
		"C:\\Users\\Dood\\Documents\\ComputerGraphics\\models\\teapot.obj";

	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;

	glm::mat4 teapot_to_world = glm::rotate(
		glm::scale(
			//glm::mat4(1.f),
			glm::translate(glm::mat4(1.f), glm::vec3(-6.f, 0.f, 2.f)),
			glm::vec3(0.9f)),
		glm::radians(30.f),
		glm::vec3(0.f, 1.f, 0.f));

	Rectangle *floor;
	std::unique_ptr<Shape> cube_2[6];

	/////////////////////////////////////
	// Triangle mesh
	/////////////////////////////////////
#ifdef LOAD_TEAPOT
	std::unique_ptr<TriangleMesh> t_pot{ new TriangleMesh() };
	glm::vec3 b_min = glm::vec3(INFINITY), b_max = glm::vec3(-INFINITY);

	extractMesh(teapot, vertices, indices);

	std::shared_ptr<Material> teapot_mat =
		std::shared_ptr<Material>(
			new Material(glm::vec3(0.02f, 0.f, 0.02f),
				glm::vec3(0.4f, 0.f, 0.4f),
				glm::vec3(0.05f, 0.05f, 0.05f)));
	teapot_mat->setShininess(20.f);

	for (size_t i = 0; i < indices.size() / TEAPOTSIZE; i += 3)
	{
		p1 = vertices[indices[i]];
		p2 = vertices[indices[i + 1]];
		p3 = vertices[indices[i + 2]];
		tr_normal = glm::normalize(glm::cross(p2 - p1, p3 - p2));

		// get boundaries of the triangle mesh
		b_min = glm::min(b_min, glm::min(glm::min(p1, p2), p3));
		b_max = glm::max(b_max, glm::max(glm::max(p1, p2), p3));

		t_pot->tr_mesh.push_back(std::unique_ptr<Triangle>(new Triangle(p1,
			p2,
			p3,
			tr_normal,
			teapot_to_world,
			teapot_mat)));
	}
	/////////////////////////////////////
	// Triangle mesh END
	/////////////////////////////////////


	////////////////////////////////
	// BOUNDARY FOR THE TEAPOT
	////////////////////////////////
	t_pot->boundary.reset(new Bounds3(
		teapot_to_world * glm::vec4(b_min, 1.f),
		teapot_to_world * glm::vec4(b_max, 1.f)));

	// put triangle mesh into scene
	sc.emplace_back(std::move(t_pot));
	////////////////////////////////
	// END
	////////////////////////////////
#endif

	// material for walls
	auto wall_bot =
		std::make_shared<Material>(
			glm::vec3(0.02, 0.02, 0.02),
			glm::vec3(0.4, 0.4, 0.4),
			glm::vec3(0.0, 0.0, 0.0));
	wall_bot->setReflective(glm::vec3(0.2f));

	//bottom
	sc.emplace_back(std::make_unique<Rectangle>(glm::vec3(-4, 2, -18),
		glm::vec3(150, 0, 0), glm::vec3(0, 150, -150), wall_bot));

	// get pointer to the floor
	floor = dynamic_cast<Rectangle*>(sc.back().get());
	cube_position = floor->getRectPos(1.5f, -27.f, 'y');
	cube_normal = floor->get_normal();

	auto triangle_mat_1 =
		std::make_shared<Material>(
			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(0.f, 0.f, 0.f));
	auto cube_mat_2 =
		std::make_shared<Material>(
			glm::vec3(0.02, 0.02, 0.0),
			glm::vec3(0.9, 0.2, 0.0f),
			glm::vec3(0.6, 0.0, 0.6));
	cube_mat_2->setShininess(10.f);

	create_cube(cube_position + glm::vec4((3.f * cube_normal), 1.f),
		//glm::rotate(glm::mat4(1), -30.f, glm::vec3(1.f, 0.f, 0.f)) *
		//glm::vec4(0.f, 0.f, 1.f, 1.f),
		cube_normal,
		/*glm::rotate(glm::mat4(1), -30.f, glm::vec3(1.f, 0.f, 0.f)) *
		glm::vec4(1.f, 0.f, 0.f, 1.f),*/
		glm::perp(glm::vec3(1.f, 1.f, 0.f), cube_normal),
		6.f,
		cube_2,
		cube_mat_2);

	for (std::unique_ptr<Shape> &p : cube_2)
	{
		sc.emplace_back(std::move(p));
	}


	/////////////////////////////////////
	// Single Triangle
	/////////////////////////////////////
	triangle_mat_1 = std::make_shared<Material>();
	sc.emplace_back(std::make_unique<Triangle>(glm::vec3(0.f, 0.f, -4.f),
		glm::vec3(4.f, 0.f, -4.f),
		glm::vec3(4.f, 4.f, -4.f),
		glm::vec3(0.f, 0.f, 1.f),
		glm::vec3(0.f, 0.f, 1.f),
		glm::vec3(0.f, 0.f, 1.f),
		glm::vec3(0.f, 0.f, 1.f),
		glm::translate(glm::scale(glm::mat4(1.f), glm::vec3(2.f)),
			glm::vec3(0.f, 0.f, -3.f)),
		triangle_mat_1));
	auto tr_tex = std::make_shared<RGB_TextureTriangle>(
		dynamic_cast<Triangle*>(sc.back().get()));
	triangle_mat_1->setTexture(tr_tex);

	/////////////////////////////////////
	// Sphere
	/////////////////////////////////////
	auto sphere_mat =
		std::make_shared<Material>(
			glm::vec3(0.02, 0.02, 0.02),
			glm::vec3(0.3, 0.3, 0.3f),
			glm::vec3(0.4f, 0.4f, 0.4f));
	sphere_mat->setShininess(10.f);

	/*auto sphere_texture = std::make_shared<CheckerBoardTexture>(
		std::make_shared<SphericalMapping>(glm::vec3(-3.f, 0.f, -7.f)));*/
	auto sphere_texture = std::make_shared<CheckerBoardTexture>(
		/*std::make_shared<PlanarMapping>(
			glm::vec3(-7.f, -4.f, -7.f),
			glm::vec3(4.f, 0.f, 0.f),
			glm::vec3(0.f, 4.f, 0.f)),*/
		std::make_shared<SphericalMapping>(
			glm::vec3(-35.f, 6.f, -3.f)
			),
		glm::vec3(1.f),
		ImageWrap::REPEAT);
	sphere_mat->setTexture(sphere_texture);

	sc.emplace_back(std::make_unique<Sphere>(
		glm::vec3(-35.f, 6.f, -3.f),
		2.f,
		glm::vec3(1.f),
		sphere_mat));

	// red sphere at origin
	//sphere_mat =
	//	std::make_shared<Material>(
	//		glm::vec3(1.f, 0.f, 0.f),
	//		glm::vec3(0.f, 0.f, 0.f),
	//		glm::vec3(0.f, 0.f, 0.f));

	//sc.emplace_back(std::make_unique<Sphere>(
	//	glm::vec3(0.f, 0.f, 0.f),
	//	0.1f,
	//	glm::vec3(1.f),
	//	sphere_mat));

	// purple sphere
	sphere_mat =
		std::make_shared<Material>(
			glm::vec3(0.02, 0.0f, 0.02),
			glm::vec3(0.3, 0.f, 0.4f),
			glm::vec3(0.4f, 0.4f, 0.4f));
	sphere_mat->setShininess(20.f);

	sc.emplace_back(std::make_unique<Sphere>(
		glm::vec3(8.f, 2.f, -3.f),
		1.f,
		glm::vec3(1.f),
		sphere_mat));

	// mirror
	sphere_mat =
		std::make_shared<Material>();
	sphere_mat->setReflective(glm::vec3(1.f));

	sc.emplace_back(std::make_unique<Sphere>(
		glm::vec3(-9.f, 1.f, -10.f),
		3.f,
		glm::vec3(1.f),
		sphere_mat));

	// glass
	sphere_mat =
		std::make_shared<Material>();
	sphere_mat->setTransparent(glm::vec3(1.f));

	sc.emplace_back(std::make_unique<Sphere>(
		glm::vec3(10.f, -10.f, 0.f),
		3.f,
		glm::vec3(1.f),
		sphere_mat));

	/////////////////////////////////////
	// Cylinders
	/////////////////////////////////////
#ifdef SHOW_AXIS
	float cyl_rad = 0.1f;
	auto cylinder_mat_1 =
		std::make_shared<Material>(
			glm::vec3(1.f, 0.f, 0.f),
			glm::vec3(1.f, 0.f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f));

	sc.emplace_back(std::make_unique<Cylinder>(
		glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(1.f, 0.f, 0.f),
		cyl_rad,
		8.f,
		cylinder_mat_1));

	cylinder_mat_1 =
		std::make_shared<Material>(
			glm::vec3(0.f, 1.0f, 0.f),
			glm::vec3(0.f, 1.f, 0.0f),
			glm::vec3(0.0f, 0.0f, 0.0f));
	sc.emplace_back(std::make_unique<Cylinder>(
		glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(0.f, 1.f, 0.f),
		cyl_rad,
		8.f,
		cylinder_mat_1));

	cylinder_mat_1 =
		std::make_shared<Material>(
			glm::vec3(0.f, 0.0f, 1.f),
			glm::vec3(0.f, 0.f, 1.f),
			glm::vec3(0.0f, 0.0f, 0.0f));
	sc.emplace_back(std::make_unique<Cylinder>(
		glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(0.f, 0.f, 1.f),
		cyl_rad,
		8.f,
		cylinder_mat_1));
#endif
	/*dynamic_cast<Cylinder*>(sc.back().get())->worldToObj =
		glm::inverse(
			glm::translate(
			glm::rotate(glm::mat4(1.f), glm::radians(60.f), glm::vec3(1.f, 0.f, 0.f)),
			glm::vec3(5.f, 1.f, 2.f)));*/
			//dynamic_cast<Cylinder*>(sc.back().get())->worldToObj = glm::mat4(1.f);

	////////////////////////////////
	// NEW CUBE
	////////////////////////////////
	cube_position = floor->getRectPos(8.f, 3.f, 'z') +
		1.5f * glm::vec4(cube_normal, 0.f);
	// cube material for new cube class object
	auto new_cube_mat = std::shared_ptr<Material>(new Material(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.2f, 0.6f, 0.1f),
		glm::vec3(0.f, 0.f, 0.f)));
	new_cube_mat->setShininess(40.f);
	auto cube_tex_mapping = std::make_shared<SphericalMapping>(cube_position);

	sc.emplace_back(std::make_unique<Cube>(
		glm::vec3(3.f),
		new_cube_mat));
	auto cube_texture = std::make_shared<RGBCubeTexture>(
		dynamic_cast<Cube*>(sc.back().get()));
	new_cube_mat->setTexture(cube_texture);

	/*sc.back()->obj_to_world = glm::rotate(glm::scale(glm::translate(
		glm::mat4(1.f),
		glm::vec3(cube_position) + glm::vec3(3.f * cube_normal)),
		glm::vec3(1.25f, 0.5f, 1.f)),
		glm::radians(0.f),
		glm::vec3(1.f, 0.f, 0.f));*/

	glm::vec3 tangent_v = glm::normalize(Plane::getTangentVector(cube_normal));

	//objToWorld = glm::lookAt(pos, pos + tangent_v, dir);
	// transform axis of the cylinder to the axis given by dir
	sc.back()->obj_to_world[0] = glm::vec4(glm::cross(cube_normal, tangent_v), 0.f);
	sc.back()->obj_to_world[1] = glm::vec4(cube_normal, 0.f);
	sc.back()->obj_to_world[2] = glm::vec4(tangent_v, 0.f);
	sc.back()->obj_to_world[3] = glm::vec4(0.f, 0.f, 0.f, 1.f);

	sc.back()->world_to_obj = glm::inverse(
		glm::translate(glm::mat4(1.f), (glm::vec3(cube_position) +
			glm::vec3(0.f, 3.f, 0.f))) *
		glm::scale(glm::mat4(1.f), glm::vec3(1.f, 1.f, 1.f)) *
		sc.back()->obj_to_world);

	// TODO: REMOVE AFTER TESTING
	glm::mat4 temp_matrix = sc.back()->obj_to_world;
	cube_position = floor->getRectPos(-3.f, -10.f, 'y') + glm::vec4(cube_normal, 0.f);

	////////////////////////////////
	// UNIT CUBE
	////////////////////////////////
	new_cube_mat = std::make_shared<Material>();

	/*sc.emplace_back(std::make_unique<UnitCube>(
		new_cube_mat));
	cube_texture = std::make_shared<RGBCubeTexture>(
		dynamic_cast<UnitCube*>(sc.back().get()));
	new_cube_mat->setTexture(cube_texture);
*/
/*sc.back()->world_to_obj = glm::inverse(glm::scale(glm::translate(glm::mat4(1.f),
	glm::vec3(3.f, -3.f, 3.f)), glm::vec3(2.f, 1.5, 3.f)));
*/////////////////////////////////
// END UNIT CUBE
////////////////////////////////

	for (int i = 0; i < 6; ++i)
	{
		new_cube_mat = std::make_shared<Material>();

		sc.emplace_back(std::make_unique<UnitCube>(
			new_cube_mat));
		cube_texture = std::make_shared<RGBCubeTexture>(
			dynamic_cast<UnitCube*>(sc.back().get()));
		new_cube_mat->setTexture(cube_texture);

		sc.back()->world_to_obj = glm::inverse(
			glm::translate(
				glm::rotate(glm::mat4(1.f),
					float(i * M_PI * 0.2f), cube_normal),
				glm::vec3(cube_position))
			* temp_matrix
			* glm::rotate(glm::mat4(1.f), float(i * M_PI * 0.5f), glm::vec3(0.f, 1.f, 0.f)));
	}
	/*
	Scaling along arbitrary axis:
	1. Choose rotation axis
	3. Translate the shape to the origin of your coordinate system
	2. Rotate chosen axis onto x-, y- or z-axis of your coordinate system
	 a) Do this by computing the normal of the spanned plane of your chosen axis
		and coordinate system axis
	 b) Rotate around this normal onto your coordinate system axis
	3. Scale along chosen coordinate axis
	4. Rotate back
	5. Translate back to originial position

	*/
	/*sc.back()->world_to_obj = glm::inverse(
		glm::rotate(
			glm::scale(
				glm::mat4(1.f), glm::vec3(4.f, 1.f, 1.f)),
			glm::radians(30.f),
			glm::vec3(1.f, 0.f, 1.f)));*/
			////////////////////////////////
			// END
			////////////////////////////////

			/////////////////////////////////////
			// Lights
			/////////////////////////////////////
	lights.emplace_back(std::make_unique<PointLight>(glm::vec3(-2.f, 20.f, -5.f),
		glm::vec3(-2, -4, -2),
		glm::vec3(110.f)));
	/*lights.emplace_back(std::make_unique<PointLight>(
		cube_position + glm::vec4(0.f, 3.f, -4.f, 0.f),
		glm::vec3(0.f, 0.f, -1.f),
		glm::vec3(30.f)));
*/
/////////////////////////////////////
// Lights END
/////////////////////////////////////

/////////////////////////////////////
// Camera
/////////////////////////////////////
	cam.reset(new Camera());
	/*cam->setCamToWorld(glm::rotate(glm::translate(glm::mat4(1.f), translation),
		rot_x, glm::vec3(0.f, 1.f, 0.f)));*/
	cam->setCamToWorld(translation, look_pos, cam_up);
	cam->update();
	/////////////////////////////////////
	// Camera END
	/////////////////////////////////////

}

void TeapotScene::init()
{
	//camera position
	glm::vec3 translation = glm::vec3(0.f, 2.5f, 25.f);
	glm::vec3 look_pos = glm::vec3(-1.0f, 0.f, -10.f);
	glm::vec3 cam_up = glm::vec3(0.f, 1.f, 0.f);
	glm::vec3 p1, p2, p3, tr_normal;

	std::string teapot =
		"C:\\Users\\Damian\\Documents\\ComputerGraphics\\models\\teapot.obj";
	teapot =
		"C:\\Users\\Damian\\Documents\\ComputerGraphics\\models\\newell_teaset\\newell_teaset\\teapot.obj";
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<unsigned int> indices;

	glm::mat4 teapot_to_world = glm::rotate(
		glm::scale(
			//glm::mat4(1.f),
			glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, 15.f)),
			glm::vec3(0.9f)),
		glm::radians(90.f),
		glm::vec3(0.f, 1.f, 0.f));

	// material for walls
	auto wall_bot =
		std::make_shared<Material>(
			glm::vec3(0.02, 0.02, 0.02),
			glm::vec3(0.4, 0.4, 0.4),
			glm::vec3(0.0, 0.0, 0.0));
	wall_bot->setReflective(glm::vec3(0.1f));
	auto wall_left =
		std::make_shared<Material>(
			glm::vec3(0.02, 0.02, 0.02),
			glm::vec3(0.4, 0.4, 0.4),
			glm::vec3(0.0, 0.0, 0.0));
	wall_left->setReflective(glm::vec3(1.0f));

	//bottom
	sc.emplace_back(std::make_unique<Rectangle>(glm::vec3(-4, 0, -18),
		glm::vec3(150, 0, 0), glm::vec3(0, 0, -150), wall_bot));
	//front
	sc.emplace_back(std::make_unique<Rectangle>(glm::vec3(-4, 0, -18-75),
		glm::vec3(300, 0, 0), glm::vec3(0, 300, 0), wall_bot));
	//back
	sc.emplace_back(std::make_unique<Rectangle>(glm::vec3(-4, 0, -18+75),
		glm::vec3(300, 0, 0), glm::vec3(0, 300, 0), wall_bot));
	//left
	sc.emplace_back(std::make_unique<Rectangle>(glm::vec3(-4, 0, -18),
		glm::vec3(0, 300, 0), glm::vec3(0, 0, -300), wall_left));
	//right
	sc.emplace_back(std::make_unique<Rectangle>(glm::vec3(-4+75, 0, -18),
		glm::vec3(0, 300, 0), glm::vec3(0, 0, -300), wall_bot));

	/////////////////////////////////////
	// Triangle mesh
	/////////////////////////////////////
	glm::vec3 b_min = glm::vec3(INFINITY), b_max = glm::vec3(-INFINITY);

	extractMesh(teapot, vertices, normals, indices);

	std::shared_ptr<Material> teapot_mat =
		std::shared_ptr<Material>(
			new Material(glm::vec3(0.01f, 0.01f, 0.01f),
				glm::vec3(0.1f, 0.1f, 0.1f),
				glm::vec3(0.2f, 0.2f, 0.2f)));
	teapot_mat->setShininess(1.f);
	teapot_mat->setReflective(glm::vec3(1.0f));

	std::vector<std::shared_ptr<Shape>> t_pot_triangles;
	for (size_t i = 0; i < indices.size() / TEAPOTSIZE; i += 3)
	{
		p1 = vertices[indices[i]];
		p2 = vertices[indices[i + 1]];
		p3 = vertices[indices[i + 2]];
		tr_normal = glm::normalize(glm::cross(p2 - p1, p3 - p2));

		// get boundaries of the triangle mesh
		b_min = glm::min(b_min, glm::min(glm::min(p1, p2), p3));
		b_max = glm::max(b_max, glm::max(glm::max(p1, p2), p3));

		t_pot_triangles.push_back(std::make_shared<Triangle>(p1,
			p2,
			p3,
			normals[indices[i]],
			normals[indices[i + 1]],
			normals[indices[i + 2]],
			tr_normal,
			teapot_to_world,
			teapot_mat));
	}
	/////////////////////////////////////
	// Triangle mesh END
	/////////////////////////////////////


	std::unique_ptr<TriangleMesh> t_pot = std::make_unique<TriangleMesh>(
		t_pot_triangles,
		std::make_unique<Bounds3>(
		teapot_to_world * glm::vec4(b_min, 1.f),
		teapot_to_world * glm::vec4(b_max, 1.f)));

	// put triangle mesh into scene
	sc.emplace_back(std::move(t_pot));
	////////////////////////////////
	// END
	////////////////////////////////

	lights.emplace_back(std::make_unique<PointLight>(glm::vec3(5.f, 5.f, 25.f),
		glm::vec3(-2, -4, -2),
		glm::vec3(120.f)));
	
	lights.emplace_back(std::make_unique<PointLight>(glm::vec3(-5.f, 5.f, 25.f),
		glm::vec3(-2, -4, -2),
		glm::vec3(80.f)));

	cam.reset(new Camera());
	/*cam->setCamToWorld(glm::rotate(glm::translate(glm::mat4(1.f), translation),
		rot_x, glm::vec3(0.f, 1.f, 0.f)));*/
	cam->setCamToWorld(translation, look_pos, cam_up);
	cam->update();
}

void SingleTriangleScene::init()
{
	glm::vec3 translation = glm::vec3(0.f, 5.f, 30.f);
	glm::vec3 look_pos = glm::vec3(0.f, 0.f, -10.f);
	glm::vec3 cam_up = glm::vec3(0.f, 1.f, 0.f);

	auto triangle_mat_1 =
		std::make_shared<Material>(
			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(0.f, 0.f, 0.f),
			glm::vec3(0.f, 0.f, 0.f));

	// single triangle
	triangle_mat_1 = std::make_shared<Material>();
	sc.emplace_back(std::make_unique<Triangle>(glm::vec3(0.f, 0.f, -4.f),
		glm::vec3(4.f, 0.f, -4.f),
		glm::vec3(4.f, 4.f, -4.f),
		glm::vec3(0.f, 0.f, 1.f),
		glm::vec3(0.f, 0.f, 1.f),
		glm::vec3(0.f, 0.f, 1.f),
		glm::vec3(0.f, 0.f, 1.f),
		glm::translate(glm::scale(glm::mat4(1.f), glm::vec3(2.f)),
			glm::vec3(0.f, 0.f, 5.f)),
		triangle_mat_1));
	auto tr_tex = std::make_shared<RGB_TextureTriangle>(
		dynamic_cast<Triangle*>(sc.back().get()));
	triangle_mat_1->setTexture(tr_tex);

	lights.emplace_back(std::make_unique<PointLight>(glm::vec3(0.f, 0.f, 30.f),
		glm::vec3(0, 0, -1),
		glm::vec3(110.f)));

	cam.reset(new Camera());
	/*cam->setCamToWorld(glm::rotate(glm::translate(glm::mat4(1.f), translation),
		rot_x, glm::vec3(0.f, 1.f, 0.f)));*/
	cam->setCamToWorld(translation, look_pos, cam_up);
	cam->update();
}

}