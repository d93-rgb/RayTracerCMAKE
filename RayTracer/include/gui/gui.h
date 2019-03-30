#pragma once

#include <AntTweakBar.h>

struct Gui
{
	TwBar *bar;
	
	void init()
	{
        if( !glfwInit() )
        {
            std::cerr << "GLFW initialization failed" << std::endl;
            exit(1);
        }

        if ( !(GLFWwindow *win = glfwOpenWindow(
                        640, 480, "Ray Tracer", 
                        nullptr, nullptr) ))
        {
            std::cerr << "Error: GLFW window could not be opened." 
                << std::endl;
            glfwTerminate();
            exit(1);
        }

        glfwEnable()
		TwInit(TW_OPENGL, nullptr);

		TwWindowSize(500, 250);

		bar = TwNewBar("RT TweakBar");

		TwDraw();
	}
};
