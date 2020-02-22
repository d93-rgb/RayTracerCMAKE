#include "texture/texture.h"

namespace rt
{

glm::dvec3 RGB_TextureTriangle::getTexel(const glm::dvec3 &pos) const
{
	if (tr)
		// calculate barycentric coordinates
		return 10.0 * (tr->m_inv * pos);
	else
	{
		VLOG(1) << "RGB_TextureTriangle Triangle class member is empty";
		return glm::dvec3(0.0);
	}
}

glm::dvec3 RGBCubeTexture::getTexel(const glm::dvec3 &pos) const
{
	if (cube)
		// calculate barycentric coordinates
		return 10.0 * ((cube->world_to_obj * glm::dvec4(pos, 1.0) 
			+ glm::dvec4(cube->boundaries, 0.0)) / glm::dvec4(cube->boundaries, 1.0) * 0.5);
	else if (unitcube)
	{
		return 100.0 * ((unitcube->world_to_obj * glm::dvec4(pos, 1.0)
			+ glm::dvec4(0.5, 0.5, 0.5, 0.0)));
	}
	else
	{
		VLOG(1) << "RGBCubeTexture Cube/UnitCube class member is empty";
		return glm::dvec3(0.0);
	}
}

}