#pragma once

namespace rt
{
class Image
{
	size_t width;
	size_t height;

public:
	Image(unsigned int width, unsigned int height) :
		width(width), 
		height(height),
		fov{ 0 },
		fov_tan_half{ 0 },
		foc_len{ 0 },
		cropped_x_start{ 0 },
		cropped_y_start{ 0 },
		cropped_width{ 0 },
		cropped_height{ 0 }{}

	void write_image_to_file(const std::string& file,
		std::vector<glm::vec3>& col);

	unsigned int get_width() const
	{
		return width;
	}

	unsigned int get_height() const
	{
		return height;
	}

private:
	float fov = 0.f;
	float fov_tan_half = 0.f;
	float u = 0.f, v = 0.f;
	
	// distance to view plane
	float foc_len;

	int cropped_x_start;
	int cropped_y_start;
	int cropped_width;
	int cropped_height;
};

}