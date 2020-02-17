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


#ifdef NOGLFW
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int main(int argc, const char** argv)
{
	size_t img_w = 800;
	size_t img_h = 600;
	std::string dest = "";

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
			else
			{
				printf("Error: USAGE\n");
				exit(1);
			}
		}
	}
// launch rendering
	Renderer renderer(img_w, img_h, std::string("picture.ppm"));
	renderer.run(RenderMode::THREADS);

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
		img_w,
		img_h
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
	size_t buffer_size = img_w * img_h * 4;
	std::vector<unsigned char> color_mem(buffer_size, 0);
	BITMAPINFO bmi;
	HDC hdc = GetDC(hwnd);

	const auto & colors = renderer.get_colors();
	for (size_t i = 0, j = 0; i < buffer_size; i += 4, ++j)
	{
		// prevent sign extension by casting to unsigned char
		color_mem[i + 2]	= (unsigned char)round((colors)[j].x);
		color_mem[i + 1]	= (unsigned char)round((colors)[j].y);
		color_mem[i]		= (unsigned char)round((colors)[j].z);
		//color_mem[i + 3]	= 0;
	}

	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = static_cast<long>(img_w);
	bmi.bmiHeader.biHeight = -int(img_h); // top-down image requires negative height
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	ShowWindow(hwnd, SW_SHOW);
	
	// Run the message loop.
	while(!RT_EXIT_PROGRAM)
	{
		// render into window
		StretchDIBits(hdc, 0, 0, static_cast<int>(img_w), 
			static_cast<int>(img_h), 0, 0, 
			static_cast<int>(img_w), 
			static_cast<int>(img_h), 
			color_mem.data(), 
			&bmi,
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

int main(int argc, char* argv[])
{

}

#endif