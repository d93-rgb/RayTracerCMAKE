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
//#define BLACK_COLOR_ARRAY_FOR_DEBUGGING

using namespace rt;

bool RT_EXIT_PROGRAM = false;


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
	Renderer renderer;
	renderer.run();

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
	while(!RT_EXIT_PROGRAM)
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

LRESULT CALLBACK WindowProc(
	HWND hwnd, 
	UINT uMsg, 
	WPARAM wParam, 
	LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		RT_EXIT_PROGRAM = true;
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