#pragma once

namespace rt
{
class Image
{
public:
	Image(size_t width, size_t height, const std::string& file_name) :
		width(width), 
		height(height),
		fov{ 0 },
		fov_tan_half{ 0 },
		foc_len{ 0 },
		cropped_x_start{ 0 },
		cropped_y_start{ 0 },
		cropped_width{ 0 },
		cropped_height{ 0 },
		file_name(file_name)
	{}

	void write_image_to_file(std::vector<glm::vec3>& col);

	size_t get_width() const
	{
		return width;
	}

	size_t get_height() const
	{
		return height;
	}

	std::string get_file_name() const
	{
		return file_name;
	}

	void append_to_file_name(const std::string& suffix);

private:
	size_t width;
	size_t height;

	float fov = 0.f;
	float fov_tan_half = 0.f;
	float u = 0.f, v = 0.f;
	
	// distance to view plane
	float foc_len;

	int cropped_x_start;
	int cropped_y_start;
	int cropped_width;
	int cropped_height;

	std::string file_name;
};

}