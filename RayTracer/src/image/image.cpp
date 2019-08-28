#include <core/rt.h>
#include "..\..\include\image\image.h"

void rt::Image::write_image_to_file(const std::string& file, std::vector<glm::vec3>& col)
{
#ifdef DEBUG
	//assert(bin.size() == col.size());
#endif
	static int i_debug = 0;
	std::ofstream ofs;

	/***************************************/
	// WRITING TO IMAGE FILE
	/***************************************/
	ofs.open(file, std::ios::binary);

	if (ofs.fail())
	{
		char err_str[75] = { '\0' };
		std::cout << "Error: Image could not be saved to \"" << file << "\"."
			<< std::endl;
		// print related error message
		GET_STRERR(errno, err_str, 75);
		std::cout << err_str;
		exit(1);
	}

	LOG(INFO) << "Writing image to \"" << file << "\"";

	// don't use \n as ending white space, because of Windows
	ofs << "P6 " << width << " " << height << " 255 ";

	// write to image file
	for (size_t i = 0; i < col.size(); ++i)
	{
#ifdef GAMMA_CORRECTION
		// gamma correction and mapping to [0;255]
		col[i] = glm::pow(glm::min(glm::vec3(1), col[i]),
			glm::vec3(1 / 2.2f)) * 255.f;
#else
		col[i] = glm::min(glm::vec3(1), col[i]) * 255.f;
#endif

#ifdef DEBUG
		i_debug = (++i_debug) % 3000;
#endif

		// prevent sign extension by casting to unsigned int
		unsigned char r = (unsigned int)round(col[i].x);
		unsigned char g = (unsigned int)round(col[i].y);
		unsigned char b = (unsigned int)round(col[i].z);

		ofs << r << g << b;
	}

	ofs.close();

	LOG(INFO) << "Writing image to \"" << file << "\" finished.";

}
