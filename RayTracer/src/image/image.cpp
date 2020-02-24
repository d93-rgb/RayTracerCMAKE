#include <core/rt.h>
#include "image/image.h"

namespace rt
{

#define GAMMA_CORRECTION

void Image::write_image_to_file()
{
#ifdef DEBUG
	//assert(bin.size() == col.size());
#endif
	static int i_debug = 0;
	std::ofstream ofs;

	/***************************************/
	// WRITING TO IMAGE FILE
	/***************************************/
	ofs.open(file_name, std::ios::binary);

	if (ofs.fail())
	{
		char err_str[75] = { '\0' };
		std::cout << "Error: Image could not be saved to \"" << file_name << "\"."
			<< std::endl;
		// print related error message
		GET_STRERR(errno, err_str, 75);
		std::cout << err_str;
		exit(1);
	}

	LOG(INFO) << "Writing image to \"" << file_name << "\"";

	// don't use \n as ending white space, because of Windows
	ofs << "P6 " << width << " " << height << " 255 ";

	// write to image file
	for (size_t i = 0; i < colors.size(); ++i)
	{
#ifdef GAMMA_CORRECTION
		// gamma correction and mapping to [0;255]
		colors[i] = glm::pow(glm::min(glm::dvec3(1), colors[i]),
			glm::dvec3(1 / 2.2f)) * 255.0;
#else
		colors[i] = glm::min(glm::dvec3(1), col[i]) * 255.f;
#endif

#ifdef DEBUG
		i_debug = (++i_debug) % 3000;
#endif

		// prevent sign extension by casting to unsigned int
		unsigned char r = (unsigned int)round(colors[i].x);
		unsigned char g = (unsigned int)round(colors[i].y);
		unsigned char b = (unsigned int)round(colors[i].z);

		ofs << r << g << b;
	}

	ofs.close();

	LOG(INFO) << "Writing image to \"" << file_name << "\" finished.";
}

void Image::resize_color_array(size_t new_width, size_t new_height)
{
	colors.resize(new_width * new_height);
	width = new_width;
	height = new_height;
}

void Image::append_to_file_name(const std::string& suffix)
{
	file_name.append(suffix);
}

} // namespace rt
