#pragma once

namespace rt
{
class Image
{
public:
	Image(size_t width, size_t height, const std::string& file_name = "picture.ppm") :
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

	void write_image_to_file(std::vector<glm::dvec3>& col);

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

	double fov = 0.f;
	double fov_tan_half = 0.f;
	double u = 0.f, v = 0.f;
	
	// distance to view plane
	double foc_len;

	int cropped_x_start;
	int cropped_y_start;
	int cropped_width;
	int cropped_height;

	std::string file_name;
};

}