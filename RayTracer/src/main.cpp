#include <chrono>
#include <set>

#include "core/rt.h"
#include "core/renderer.h"
#include "samplers/sampler2D.h"
#include "scene/scene.h"
#include "camera/camera.h"
#include "core/utility.h"

//threading
#include "image/image.h"
#include "threads/dispatcher.h"

// use for debugging
#define DEBUG
#define GAMMA_CORRECTION
#define RENDER_SCENE
//#define NO_THREADS
//#define OPEN_WITH_GIMP
//#define BLACK_COLOR_ARRAY_FOR_DEBUGGING

using namespace rt;

constexpr auto SPP = 1;
constexpr auto GRID_DIM = 3;

constexpr unsigned int WIDTH = 480;
constexpr unsigned int HEIGHT = 270;

constexpr auto NUM_THREADS = 4;

bool EXIT_PROGRAM = false;

int MAX_DEPTH = 4;

//std::vector<float> debug_vec;
void helper_fun(std::vector<glm::vec3>* colors, const std::string& file);
std::vector<glm::vec3> render(unsigned int& width, unsigned int& height);
std::vector<glm::vec3> render_with_threads(unsigned int& width, unsigned int& height);

// for creating color gradients
std::vector<glm::vec3> render_gradient(unsigned int& width_img, const unsigned int& width_stripe,
	unsigned int& height);


void write_file(const std::string& file, std::vector<glm::vec3>& col, unsigned int width, unsigned int height);
void get_color(std::vector<glm::vec3>& col,
	const Scene& sc,
	StratifiedSampler2D& sampler,
	unsigned int array_size,
	const glm::vec2* samplingArray,
	float inv_grid_dim,
	float inv_spp,
	float fov_tan,
	float d,
	int x,
	int y,
	int x1,
	int y1);

/*
	Short helper function
*/
void helper_fun(std::vector<glm::vec3>* colors, std::string& file) 
{
	unsigned int width, height;

#ifdef RENDER_SCENE
#ifdef NO_THREADS
	*colors = render(width, height);
#else
	*colors = render_with_threads(width, height);
#endif
#else
	*colors = render_gradient(width, 10, height);
#endif

	if (file.empty())
	{
		char buf[200];
		GET_PWD(buf, 200);
		std::string file_name = "picture.ppm";
		std::cout << buf << std::endl;
		std::string fn = buf;


		LOG(INFO) << "Image will be written to \"" <<
			fn.substr(0, fn.find_last_of("\\/")).append(OS_SLASH).append(file_name);
		write_file(file_name, *colors, width, height);
	}
	else
	{
		file.append(".ppm");
		write_file(file, *colors, width, height);
	}
}

std::vector<glm::vec3> render_gradient(unsigned int& width_img, const unsigned int& width_stripe,
	unsigned int& height)
{
	// do not let RAM explode, limit maximum stripe width
	assert(width_stripe < 4e3);

	// set image width and height
	width_img = 256 * width_stripe;
	height = HEIGHT;

	std::vector<glm::vec3> color{ 256 * width_stripe * height, glm::vec3(0) };

	for (int k = 0; k < 256; ++k)
	{
		for (unsigned int i = 0; i < height; ++i)
		{
			for (unsigned int j = 0; j < width_stripe; ++j)
			{
				color[i * width_img + j + k * width_stripe] = glm::vec3(float(k) / 255.0f);
			}
		}
	}
	return color;
}


/*
	Starts rendering a scene and returns the color vector.
*/
std::vector<glm::vec3> render(unsigned int& width, unsigned int& height)
{
	constexpr float fov = glm::radians(90.f);
	float fov_tan = tan(fov / 2);
	float u = 0.f, v = 0.f;
	// distance to view plane
	float d = 1.f;
	float inv_spp;
	float inv_grid_dim = 1.f / (GRID_DIM * GRID_DIM);

	float crop_min_x = 0.f, crop_max_x = 1.f;
	float crop_min_y = 0.f, crop_max_y = 1.f;

	assert(crop_min_x <= crop_max_x && crop_min_y <= crop_max_y);

	unsigned int cropped_width[2];
	unsigned int cropped_height[2];

	crop(crop_min_x, crop_max_x, WIDTH, cropped_width);
	crop(crop_min_y, crop_max_y, HEIGHT, cropped_height);

	width = cropped_width[1] - cropped_width[0];
	height = cropped_height[1] - cropped_height[0];

	LOG(INFO) << "Image width = " << WIDTH << "; Image height = " << HEIGHT;
	LOG(INFO) << "Cropped width = " << width << "; Cropped height = " << height;

	std::vector<glm::vec3> col{ width * height, glm::vec3(0.f) };

	StratifiedSampler2D sampler{ width, height, GRID_DIM };
	unsigned int array_size = GRID_DIM * GRID_DIM;
	const glm::vec2* samplingArray;
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
		pbrt::ProgressReporter reporter(HEIGHT, "Rendering:");
		/***************************************/
		// LOOPING OVER PIXELS
		/***************************************/
		// dynamic schedule for proper I/O progress update
//#pragma omp parallel for schedule(dynamic, 1)
		for (unsigned int y = cropped_height[0]; y < cropped_height[1]; ++y)
		{
			//fprintf(stderr, "\rRendering %5.2f%%", 100.*y / (HEIGHT - 1));
			reporter.Update();
			for (unsigned int x = cropped_width[0]; x < cropped_width[1]; ++x)
			{
				//TODO: NOT threadsafe
				samplingArray = sampler.get2DArray();

				// hackery needed for omp pragma
				// the index i will be distributed among all threads
				// by omp automatically
				unsigned int i = (y - cropped_height[0]) * width + (x - cropped_width[0]);
				for (unsigned int idx = 0; idx < array_size; ++idx)
				{
					SurfaceInteraction isect;

					// map pixel coordinates to[-1, 1]x[-1, 1]
					float u = (2.f * (x + samplingArray[idx].x) - WIDTH) / HEIGHT * fov_tan;
					float v = (-2.f * (y + samplingArray[idx].y) + HEIGHT) / HEIGHT * fov_tan;

					// this can not be split up and needs to be in one line, otherwise
					// omp will not take the
					/*col[i] += clamp(shoot_recursively(sc, sc.cam->getPrimaryRay(u, v, d), &isect, 0))
						* inv_grid_dim;*/
					col[i] = glm::normalize(sc.cam->getPrimaryRay(u, v, d).rd);
				}
			}
		}
		reporter.Done();
	}
	//enclose with braces for destructor of ProgressReporter at the end of rendering
//	{
		/***************************************/
		// START PROGRESSREPORTER
		/***************************************/
//		pbrt::ProgressReporter reporter(HEIGHT, "Rendering:");
//		/***************************************/
//		// LOOPING OVER PIXELS
//		/***************************************/
//		std::random_device rd;
//		std::default_random_engine eng(rd());
//		std::uniform_real_distribution<> dist(0, 1);
//		// dynamic schedule for proper I/O progress update
//#pragma omp parallel for schedule(dynamic, 1)
//		for (size_t y = cropped_height[0]; y < cropped_height[1]; ++y)
//		{
//			//fprintf(stderr, "\rRendering %5.2f%%", 100.*y / (HEIGHT - 1));
//			reporter.Update();
//			for (size_t x = cropped_width[0]; x < cropped_width[1]; ++x)
//			{
//				for (int m = 0; m < GRID_DIM; ++m)
//				{
//					for (int n = 0; n < GRID_DIM; ++n)
//					{
//						// hackery needed for omp pragma
//						// the index i will be distributed among all threads
//						// by omp automatically
//						for (size_t k = 0,
//							i = (y - cropped_height[0]) * width + x - cropped_width[0];
//							k < SPP; ++k)
//						{
//							SurfaceInteraction isect;
//
//							// stratified sampling
//							float u_rnd = float(dist(eng));
//							float v_rnd = float(dist(eng));
//							// map pixel coordinates to[-1, 1]x[-1, 1]
//							float u = (2.f * (x + (m + u_rnd) / GRID_DIM) - WIDTH) / HEIGHT * fov_tan;
//							float v = (-2.f * (y + (n + v_rnd) / GRID_DIM) + HEIGHT) / HEIGHT * fov_tan;
//
//							// this can not be split up and needs to be in one line, otherwise
//							// omp will not take the average
//							col[i] += clamp(shoot_recursively(sc, sc.cam->getPrimaryRay(u, v, d), &isect, 0))
//								* inv_spp * inv_grid_dim;
//						}
//					}
//				}
//			}
//		}
//		reporter.Done();
//	}

	//#pragma omp parallel for
		//	for (int i = 0; i < 10; ++i)
		//	{
		//		std::this_thread::sleep_for(std::chrono::seconds(1));
		//		std::cout << " thread: " << omp_get_thread_num() << std::endl;
		//	}
	return col;
}

std::vector<glm::vec3> render_with_threads(unsigned int& width, unsigned int& height)
{
	constexpr float fov = glm::radians(30.f);
	float fov_tan = tan(fov / 2);
	float u = 0.f, v = 0.f;
	// distance to view plane
	float foc_len = 0.5f * 1.0f / fov_tan;
	float inv_spp;
	float inv_grid_dim = 1.f / (GRID_DIM * GRID_DIM);

	float crop_min_x = 0.f, crop_max_x = 1.f;
	float crop_min_y = 0.f, crop_max_y = 1.f;

	assert(crop_min_x <= crop_max_x && crop_min_y <= crop_max_y);

	unsigned int cropped_width[2];
	unsigned int cropped_height[2];

	crop(crop_min_x, crop_max_x, WIDTH, cropped_width);
	crop(crop_min_y, crop_max_y, HEIGHT, cropped_height);

	width = cropped_width[1] - cropped_width[0];
	height = cropped_height[1] - cropped_height[0];

	LOG(INFO) << "Image width = " << WIDTH << "; Image height = " << HEIGHT;
	LOG(INFO) << "Cropped width = " << width << "; Cropped height = " << height;

	std::vector<glm::vec3> col{ width * height, glm::vec3(0.f) };

#ifdef BLACK_COLOR_ARRAY_FOR_DEBUGGING
	return col;
#endif

	StratifiedSampler2D sampler{ width, height, GRID_DIM };
	unsigned int array_size = GRID_DIM * GRID_DIM;
	const glm::vec2* samplingArray;
	inv_spp = 1.0f / SPP;
	/***************************************/
	// CREATING SCENE
	/***************************************/
	//GatheringScene sc;
	MixedScene sc;
	//	// enclose with braces for destructor of ProgressReporter at the end of rendering
	{
		rt::Image img(WIDTH, HEIGHT);
		Slice slice(img, 16, 16);
		std::mutex pairs_mutex;
		std::vector<std::thread> threads_v;
		/***************************************/
		// START PROGRESSREPORTER
		/***************************************/
		pbrt::ProgressReporter reporter(slice.dx * slice.dy, "Rendering:");
		/***************************************/
		// LOOPING OVER PIXELS
		/***************************************/
			//fprintf(stderr, "\rRendering %5.2f%%", 100.*y / (HEIGHT - 1));

		for (int i = 0; i < NUM_THREADS; ++i)
		{
			/*threads_v.push_back(std::thread(work,
				std::ref(slice),
				std::ref(pairs_mutex),
				std::ref(col),
				std::ref(sc),
				std::ref(sampler),
				std::ref(reporter),
				array_size,
				std::ref(samplingArray),
				inv_grid_dim,
				inv_spp,
				fov_tan,
				foc_len,
				std::ref(get_color)));*/

			threads_v.push_back(std::thread([&]() {
				int idx = 0;
				unsigned int h_step;
				unsigned int w_step;

				while (idx != -1)
				{
					// try to access the next free raster
					// TODO: Change locking with mutex guards!
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
							//TODO: NOT threadsafe
							samplingArray = sampler.get2DArray();

							for (unsigned int n = 0; n < array_size; ++n)
							{
								SurfaceInteraction isect;

								// map pixel coordinates to[-1, 1]x[-1, 1]
								float u = (2.f * (slice.pairs[idx].first + j + samplingArray[n].x) - WIDTH) / HEIGHT;
								float v = (-2.f * (slice.pairs[idx].second + i + samplingArray[n].y) + HEIGHT) / HEIGHT;

								/*float u = (x + samplingArray[idx].x) - WIDTH * 0.5f;
								float v = -((y + samplingArray[idx].y) - HEIGHT * 0.5f);
						*/
								col[(slice.pairs[idx].second + i) * slice.img_width + slice.pairs[idx].first + j] +=
									clamp(shoot_recursively(
										sc, sc.cam->getPrimaryRay(u, v, foc_len), &isect, 0)) *
									inv_grid_dim * inv_spp;
								//col[x + y] = glm::normalize(sc.cam->getPrimaryRay(u, v, d).rd);
							}
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
	//enclose with braces for destructor of ProgressReporter at the end of rendering
//	{
		/***************************************/
		// START PROGRESSREPORTER
		/***************************************/
//		pbrt::ProgressReporter reporter(HEIGHT, "Rendering:");
//		/***************************************/
//		// LOOPING OVER PIXELS
//		/***************************************/
//		std::random_device rd;
//		std::default_random_engine eng(rd());
//		std::uniform_real_distribution<> dist(0, 1);
//		// dynamic schedule for proper I/O progress update
//#pragma omp parallel for schedule(dynamic, 1)
//		for (size_t y = cropped_height[0]; y < cropped_height[1]; ++y)
//		{
//			//fprintf(stderr, "\rRendering %5.2f%%", 100.*y / (HEIGHT - 1));
//			reporter.Update();
//			for (size_t x = cropped_width[0]; x < cropped_width[1]; ++x)
//			{
//				for (int m = 0; m < GRID_DIM; ++m)
//				{
//					for (int n = 0; n < GRID_DIM; ++n)
//					{
//						// hackery needed for omp pragma
//						// the index i will be distributed among all threads
//						// by omp automatically
//						for (size_t k = 0,
//							i = (y - cropped_height[0]) * width + x - cropped_width[0];
//							k < SPP; ++k)
//						{
//							SurfaceInteraction isect;
//
//							// stratified sampling
//							float u_rnd = float(dist(eng));
//							float v_rnd = float(dist(eng));
//							// map pixel coordinates to[-1, 1]x[-1, 1]
//							float u = (2.f * (x + (m + u_rnd) / GRID_DIM) - WIDTH) / HEIGHT * fov_tan;
//							float v = (-2.f * (y + (n + v_rnd) / GRID_DIM) + HEIGHT) / HEIGHT * fov_tan;
//
//							// this can not be split up and needs to be in one line, otherwise
//							// omp will not take the average
//							col[i] += clamp(shoot_recursively(sc, sc.cam->getPrimaryRay(u, v, d), &isect, 0))
//								* inv_spp * inv_grid_dim;
//						}
//					}
//				}
//			}
//		}
//		reporter.Done();
//	}

	//#pragma omp parallel for
		//	for (int i = 0; i < 10; ++i)
		//	{
		//		std::this_thread::sleep_for(std::chrono::seconds(1));
		//		std::cout << " thread: " << omp_get_thread_num() << std::endl;
		//	}
	return col;
}


#ifdef DEBUG
//for debugging
std::set<int> bin;
#endif

void get_color(std::vector<glm::vec3>& col,
	const Scene& sc,
	StratifiedSampler2D& sampler,
	unsigned int array_size,
	const glm::vec2* samplingArray,
	float inv_grid_dim,
	float inv_spp,
	float fov_tan,
	float d,
	int x,
	int y,
	int x1,
	int y1)
{
	//TODO: NOT threadsafe
	samplingArray = sampler.get2DArray();

	// hackery needed for omp pragma
	// the index i will be distributed among all threads
	// by omp automatically
	//unsigned int i = (y - cropped_height[0]) * width + (x - cropped_width[0]);
#ifdef DEBUG
	assert(bin.find(x1 + y1) == bin.end());
	bin.insert(x1 + y1);

	if (x1 + y1 >= col.size())
	{
		printf("Error: index out of range: x1+y1 = %d > %zu\n", x * y + y, col.size());
		exit(1);
	}
#endif

	for (unsigned int idx = 0; idx < array_size; ++idx)
	{
		SurfaceInteraction isect;

		// map pixel coordinates to[-1, 1]x[-1, 1]
		float u = (2.f * (x + samplingArray[idx].x) - WIDTH) / HEIGHT;
		float v = (-2.f * (y + samplingArray[idx].y) + HEIGHT) / HEIGHT;

		/*float u = (x + samplingArray[idx].x) - WIDTH * 0.5f;
		float v = -((y + samplingArray[idx].y) - HEIGHT * 0.5f);
*/
// this can not be split up and needs to be in one line, otherwise
// omp will not take the
		col[x1 + y1] += clamp(shoot_recursively(sc, sc.cam->getPrimaryRay(u, v, d), &isect, 0))
			* inv_grid_dim;
		//col[x + y] = glm::normalize(sc.cam->getPrimaryRay(u, v, d).rd);
	}
}


void write_file(const std::string& file,
	std::vector<glm::vec3>& col, unsigned int width, unsigned int height)
{
#ifdef DEBUG
	//assert(bin.size() == col.size());
#endif
	static int i_debug = 0;
	std::ofstream ofs;

	/***************************************/
	// WRITING TO IMAGE FILE
	/***************************************/
	ofs.open(file, std::ios::binary);

	if (ofs.fail())
	{
		char err_str[75] = { '\0' };
		std::cout << "Error: Image could not be saved to \"" << file << "\"."
			<< std::endl;
		// print related error message
		GET_STRERR(errno, err_str, 75);
		std::cout << err_str;
		exit(1);
	}

	LOG(INFO) << "Writing image to \"" << file << "\"";

	// don't use \n as ending white space, because of Windows
	ofs << "P6 " << width << " " << height << " 255 ";

	// write to image file
	for (size_t i = 0; i < col.size(); ++i)
	{
#ifdef GAMMA_CORRECTION
		// gamma correction and mapping to [0;255]
		col[i] = glm::pow(glm::min(glm::vec3(1), col[i]),
			glm::vec3(1 / 2.2f)) * 255.f;
#else
		col[i] = glm::min(glm::vec3(1), col[i]) * 255.f;
#endif

#ifdef DEBUG
		i_debug = (++i_debug) % 3000;
#endif

		// prevent sign extension by casting to unsigned int
		unsigned char r = (unsigned int)round(col[i].x);
		unsigned char g = (unsigned int)round(col[i].y);
		unsigned char b = (unsigned int)round(col[i].z);

		ofs << r << g << b;
	}

	ofs.close();

	LOG(INFO) << "Writing image to \"" << file << "\" finished.";
}

#ifdef WIN32
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int main(int argc, const char** argv)
{
	bool owg = false;
	std::string dest = "";

	std::unique_ptr<std::vector<glm::vec3>> colors(new std::vector<glm::vec3>);

	// Windows stuff
	HINSTANCE hInstance = GetModuleHandle(0);
	const wchar_t CLASS_NAME[] = L"Sample Window Class";
	//std::cout << "OpenMP max threads:" << omp_get_max_threads() << std::endl;

	if (argc > 1)
	{
		for (int i = 0; i < argc; ++i)
		{
			printf("argv[%i] = %s\n", i, argv[i]);
		}
		int pos = 1;
		while (pos < argc)
		{
			if (!strcmp(argv[pos], "--destination")
				|| !strcmp(argv[pos], "-d"))
			{
				if (++pos == argc)
				{
					printf("Error: USAGE\n");
					exit(1);
				}
				printf("argc = %i, pos == %i\n", argc, pos);
				dest = argv[pos++];
			}
			else if (!strcmp(argv[pos], "--open_with_gimp")
				|| !strcmp(argv[pos], "-owg"))
			{
				++pos;
				owg = true;
			}
			else
			{
				printf("Error: USAGE\n");
				exit(1);
			}
		}
	}
// launch rendering
	helper_fun(colors.get(), dest);

	// create window and draw in it
	WNDCLASS window_class = {};
	window_class.style = CS_VREDRAW | CS_HREDRAW;
	window_class.lpszClassName = CLASS_NAME;
	window_class.lpfnWndProc = WindowProc;

	if (!RegisterClass(&window_class))                                    
	{
		MessageBox(NULL, L"Failed To Register the window class.", L"ERROR", MB_OK | MB_ICONEXCLAMATION);
		exit(1);                                           
	}

	RECT client_rect = {
		0,
		0,
		WIDTH,
		HEIGHT
	};

	if (!AdjustWindowRect(&client_rect, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, false))
	{
		std::cout << "ERROR: Adjusting window rectangle failed." << std::endl;
	}

	// disable automatic rescaling 
	SetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
	
	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,     // Window class
		L"Raytracer",    // Window text
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,    // prevent resizing
		CW_USEDEFAULT, CW_USEDEFAULT, 
		client_rect.right - client_rect.left, 
		client_rect.bottom - client_rect.top,
		NULL,       // Parent window    
		NULL,       // Menu
		hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		printf("ERROR: hwnd == NULL\n");
		return 0;
	}

	// data of a single pixel is as big as an unsinged int (= 4 bytes)
	size_t buffer_size = WIDTH * HEIGHT * 4;
	std::vector<unsigned char> color_mem(buffer_size, 0);
	BITMAPINFO bmi;
	HDC hdc = GetDC(hwnd);

	for (size_t i = 0, j = 0; i < buffer_size; i += 4, ++j)
	{
		// prevent sign extension by casting to unsigned char
		color_mem[i + 2]	= (unsigned char)round((*colors)[j].x);
		color_mem[i + 1]	= (unsigned char)round((*colors)[j].y);
		color_mem[i]		= (unsigned char)round((*colors)[j].z);
		//color_mem[i + 3]	= 0;
	}

	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = WIDTH;
	bmi.bmiHeader.biHeight = -int(HEIGHT); // top-down image requires negative height
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	ShowWindow(hwnd, SW_SHOW);
	
	// Run the message loop.
	while(!EXIT_PROGRAM)
	{
		// render into window
		StretchDIBits(hdc, 0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, color_mem.data(), &bmi,
			DIB_RGB_COLORS, SRCCOPY);

		MSG msg = { };
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	ReleaseDC(hwnd, hdc);

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		EXIT_PROGRAM = true;
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		//FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(hwnd, &ps);
	}
	return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#else
int main(int argc, const char** argv)
{
	// open image with gimp
	auto owg = false;
	std::string dest = "";
	//std::cout << "OpenMP max threads:" << omp_get_max_threads() << std::endl;

	if (argc > 1)
	{
		for (int i = 0; i < argc; ++i)
		{
			printf("argv[%i] = %s\n", i, argv[i]);
		}
		int pos = 1;
		while (pos < argc)
		{
			if (!strcmp(argv[pos], "--destination")
				|| !strcmp(argv[pos], "-d"))
			{
				if (++pos == argc)
				{
					printf("Error: USAGE\n");
					exit(1);
				}
				printf("argc = %i, pos == %i\n", argc, pos);
				dest = argv[pos++];
			}
			else if (!strcmp(argv[pos], "--open_with_gimp")
				|| !strcmp(argv[pos], "-owg"))
			{
				++pos;
				owg = true;
			}
			else
			{
				printf("Error: USAGE\n");
				exit(1);
			}
		}
	}
	//google::InitGoogleLogging(argv[0]);
	//for (int i = 0; i < 3; ++i)
/*
	Gui g = Gui();
	g.init();
*/

/*
ofs.open("debug.txt");
for (float f : debug_vec)
{
	ofs << f << std::endl;
}
ofs.close();
*/
//getchar();

// launch rendering
	helper_fun(dest);

	if (owg)
	{
	}
#ifdef OPEN_WITH_GIMP
	// OPEN FILE IN GIMP
	std::string gimp_path = "C:\\Program Files\\GIMP 2\\bin\\gimp-2.10.exe";
	std::string image_path = dest;
	std::string szCmdline = gimp_path + " " + image_path;

	LOG(INFO) << "Opening image with " << gimp_path;

	// additional information
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// start the program up
	if (!CreateProcess(nullptr,   // the path
		&szCmdline[0],        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory
		&si,            // Pointer to STARTUPINFO structure
		&pi))            // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	{
		printf("CreateProcess failed (%d).\n%s\n",
			GetLastError(), szCmdline.c_str());
	}
	// Close process and thread handles.
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
#endif
	//MessageBox(nullptr, TEXT("Done."), TEXT("Notification"), MB_OK);
	return 0;
}
#endif