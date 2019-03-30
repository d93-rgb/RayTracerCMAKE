#include <chrono>

#include "core/rt.h"
#include "core/renderer.h"
#include "scene/scene.h"
#include "camera/camera.h"
#include "core/utility.h"

#if defined(_WIN32)
#define GET_STRERR(ERROR_NUM, BUF, LEN) strerror_s(BUF, ERROR_NUM);
#define GET_PWD(BUF, LEN) GetModuleFileNameA(nullptr, BUF, LEN)
constexpr auto OS_SLASH = "\\";
#else
#define GET_STRERR(ERROR_NUM, BUF, LEN)	strerror_r(ERROR_NUM, BUF, LEN);
#define GET_PWD(BUF, LEN) readlink("/proc/self/exe", BUF, LEN)
constexpr auto OS_SLASH = "/";
#endif

// use for debugging
#undef DEBUG
//#define OPEN_WITH_GIMP

using namespace rt;

constexpr auto SPP = 1;
constexpr auto GRID_DIM = 1;

constexpr auto WIDTH = 533;
constexpr auto HEIGHT = 400;

int MAX_DEPTH = 4;

//std::vector<float> debug_vec;
void helper_fun(const std::string &file);
std::vector<glm::vec3> render(unsigned int &width, unsigned int &height);
void write_file(const std::string &file, std::vector<glm::vec3> &col, int width, int height);

std::ostream &operator<<(std::ostream &os, glm::vec3 v)
{
	os << "(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
	return os;
}

/*
	Short helper function
*/
void helper_fun(std::string &file)
{
	unsigned int width, height;
	std::vector<glm::vec3> &&colors = render(width, height);

	if (file.empty())
	{
        char buf[200];
        GET_PWD(buf, 200);
        std::cout << buf << std::endl;
        std::string fn = buf;


		LOG(INFO) << "Image will be written to \"" <<
            fn.substr(0, fn.find_last_of("\\/")).append(OS_SLASH).append("picture.ppm");
		write_file("picture.ppm", colors, width, height);
	}
	else
	{
		file.append(".ppm");
		write_file(file, colors, width, height);
	}
}

/*
	Starts rendering a scene and returns the color vector.
*/
std::vector<glm::vec3> render(unsigned int &width, unsigned int &height)
{
	int i = 0;
	float fov = glm::radians(55.f);
	float fov_tan = tan(fov / 2);
	float u = 0.f, v = 0.f;
	// distance to view plane
	float d = 1.f;
	float inv_spp = 1.f / SPP;
	float inv_grid_dim = 1.f / (GRID_DIM * GRID_DIM);

	float crop_min_x = 0.f, crop_max_x = 1.f;
	float crop_min_y = 0.f, crop_max_y = 1.f;

	assert(crop_min_x <= crop_max_x && crop_min_y <= crop_max_y);

	int cropped_width[2];
	int cropped_height[2];

	crop(crop_min_x, crop_max_x, WIDTH, cropped_width);
	crop(crop_min_y, crop_max_y, HEIGHT, cropped_height);

	width = cropped_width[1] - cropped_width[0];
	height = cropped_height[1] - cropped_height[0];

	LOG(INFO) << "Image width = " << WIDTH << "; Image height = " << HEIGHT;
	LOG(INFO) << "Cropped width = " << width << "; Cropped height = " << height;

	std::vector<glm::vec3> col{ width * height, glm::vec3(0.f) };

	/***************************************/
	// CREATING SCENE
	/***************************************/
	//GatheringScene sc;
	SingleCubeScene sc;
	// enclose with braces for destructor of ProgressReporter at the end of rendering
	{
		/***************************************/
		// START PROGRESSREPORTER
		/***************************************/
		pbrt::ProgressReporter reporter(HEIGHT, "Rendering:");
		/***************************************/
		// LOOPING OVER PIXELS
		/***************************************/
		std::random_device rd;
		std::default_random_engine eng(rd());
		std::uniform_real_distribution<> dist(0, 1);
		// dynamic schedule for proper I/O progress update
#pragma omp parallel for schedule(dynamic, 1)
		for (int y = cropped_height[0]; y < cropped_height[1]; ++y)
		{
			//fprintf(stderr, "\rRendering %5.2f%%", 100.*y / (HEIGHT - 1));
			reporter.Update();
			for (int x = cropped_width[0]; x < cropped_width[1]; ++x)
			{
				for (int m = 0; m < GRID_DIM; ++m)
				{
					for (int n = 0; n < GRID_DIM; ++n)
					{
						// hackery needed for omp pragma
						// the index i will be distributed among all threads
						// by omp automatically
						for (int k = 0,
							i = (y - cropped_height[0]) * width + x - cropped_width[0];
							k < SPP; ++k)
						{
							SurfaceInteraction isect;

							// stratified sampling
							float u_rnd = float(dist(eng));
							float v_rnd = float(dist(eng));
							// map pixel coordinates to[-1, 1]x[-1, 1]
							float u = (2.f * (x + (m + u_rnd) / GRID_DIM) - WIDTH) / HEIGHT * fov_tan;
							float v = (-2.f * (y + (n + v_rnd) / GRID_DIM) + HEIGHT) / HEIGHT * fov_tan;

							// this can not be split up and needs to be in one line, otherwise
							// omp will not take the average
							col[i] += clamp(shoot_recursively(sc, sc.cam->getPrimaryRay(u, v, d), &isect, 0))
								* inv_spp * inv_grid_dim;
						}
					}
				}
			}
		}
		reporter.Done();
	}
	//#pragma omp parallel for
	//	for (int i = 0; i < 10; ++i)
	//	{
	//		std::this_thread::sleep_for(std::chrono::seconds(1));
	//		std::cout << " thread: " << omp_get_thread_num() << std::endl;
	//	}
	return col;
}

void write_file(const std::string &file,
	std::vector<glm::vec3> &col, int width, int height)
{
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
		// gamma correction and mapping to [0;255]
		col[i] = glm::pow(glm::min(glm::vec3(1), col[i]),
			glm::vec3(1 / 2.2f)) * 255.f;

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

int main(int argc, const char **argv)
{
	// open image with gimp
	auto owg = false;
	std::string dest = "";

	if (argc > 1)
	{
		for (int i = 0; i < argc; ++i)
		{
			printf("argv[%i] = %s\n", i, argv[i]);
		}
		int pos = 1;
		while (pos < argc)
		{
			if (!strcmp(argv[pos], "--destination") || !strcmp(argv[pos], "-d"))
			{
				if (++pos == argc)
				{
					printf("Error: USAGE\n");
					exit(1);
				}
				printf("argc = %i, pos == %i\n", argc, pos);
				dest = argv[pos++];
			}
			else if (!strcmp(argv[pos], "--open_with_gimp") || !strcmp(argv[pos], "-owg"))
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
