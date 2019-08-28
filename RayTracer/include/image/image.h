#pragma once

namespace rt
{
class Image
{
	unsigned int width;
	unsigned int height;

public:
	Image(unsigned int width, unsigned int height) :
		width(width), height(height) {}

	void write_image_to_file(const std::string& file,
		std::vector<glm::vec3>& col);

	unsigned int get_width()
	{
		return width;
	}

	unsigned int get_height()
	{
		return height;
	}
};

}