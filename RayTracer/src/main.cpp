#include <chrono>
#include <set>

// OpenGL loader
#include <GL/gl3w.h>

#include <GLFW/glfw3.h>

#include "core/rt.h"
#include "core/renderer.h"
#include "samplers/sampler2D.h"
#include "scene/scene.h"
#include "camera/camera.h"
#include "core/utility.h"

//threading
#include "image/image.h"
#include "threads/dispatcher.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// use for debugging
#define DEBUG
//#define BLACK_COLOR_ARRAY_FOR_DEBUGGING

using namespace rt;

bool RT_EXIT_PROGRAM = false;


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

void center_glfw_window(GLFWwindow* window, GLFWmonitor* monitor)
{
	int error_code;
	const char* error_description;
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	if (!mode)
	{
		error_code = glfwGetError(&error_description);

		std::cout << error_description << std::endl;
		glfwTerminate();
		std::exit(1);
	}

	// center window
	int monitorX, monitorY;
	glfwGetMonitorPos(monitor, &monitorX, &monitorY);

	int windowWidth, windowHeight;
	glfwGetWindowSize(window, &windowWidth, &windowHeight);

	glfwSetWindowPos(window,
		monitorX + (mode->width - windowWidth) / 2,
		monitorY + (mode->height - windowHeight) / 2);

}

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
	int screen_width = 800;
	int screen_height = 600;

	int error_code;
	const char* error_description;

	if (glfwInit() == GLFW_FALSE)
	{
		error_code = glfwGetError(&error_description);

		std::cout << error_description << std::endl;
		std::exit(1);
	}

	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(
		screen_width,
		screen_height,
		"RayTracer",
		nullptr,
		nullptr);

	if (window == nullptr) {
		error_code = glfwGetError(&error_description);

		std::cout << error_description << std::endl;
		glfwTerminate();
		std::exit(1);
	}
	
	center_glfw_window(window, glfwGetPrimaryMonitor());

	glfwMakeContextCurrent(window);
	

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glfwSetKeyCallback(window, key_callback);

	if (gl3wInit()) {
		std::cout << "failed to initialize OpenGL" << std::endl;
		std::exit(1);
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGuiStyle* imgui_style = &ImGui::GetStyle();
	imgui_style->WindowPadding = ImVec2(0, 0);
	imgui_style->WindowRounding = 0.0f;
	imgui_style->FrameRounding = 0.0f;
	imgui_style->DisplaySafeAreaPadding = ImVec2(0, 0);
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	size_t render_w = 800;
	size_t render_h = 600;
	glViewport(0, 0, render_w, render_h);
	
	Renderer renderer(render_w, render_h, std::string("picture.ppm"));
	renderer.run(RenderMode::THREADS);

	int idx = 0;
	std::unique_ptr<char[]> img_data(new char[render_w * render_h * 3]);
	for (const auto& c : renderer.get_colors())
	{
		for (int i = 0; i < (int)c.length(); ++i)
		{
			img_data[idx++] = static_cast<unsigned char>(std::roundf(c[i]));
		}
	}
	
	// Create an OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Upload pixels into texture
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, render_w, render_h, 0, GL_RGB, 
		GL_UNSIGNED_BYTE, 
		img_data.get());

	glfwShowWindow(window);

	while (!glfwWindowShouldClose(window)) {
		glfwWaitEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::Begin("OpenGL Texture Test", 
			0, 
			ImGuiWindowFlags_NoResize | 
			ImGuiWindowFlags_NoMove | 
			ImGuiWindowFlags_NoCollapse | 
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoTitleBar | 
			ImGuiWindowFlags_NoScrollbar | 
			ImGuiWindowFlags_NoScrollWithMouse);
		//ImGui::Text("size = %d x %d", render_w, render_h);
		ImGui::Image((void*)image_texture, ImVec2(render_w, render_h));
		ImGui::End();

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

#endif