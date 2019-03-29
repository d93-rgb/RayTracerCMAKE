#pragma once

#include <AntTweakBar.h>

struct Gui
{
	TwBar *bar;
	
	void init()
	{
		TwInit(TW_OPENGL, nullptr);

		TwWindowSize(500, 250);

		bar = TwNewBar("MyBar");

		TwDraw();
	}
};