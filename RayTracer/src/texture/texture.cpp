#include "texture/texture.h"

namespace rt
{

glm::vec3 RGB_TextureTriangle::getTexel(const glm::vec3 &pos) const
{
	if (tr)
		// calculate barycentric coordinates
		return 10.f * (tr->m_inv * pos);
	else
	{
		VLOG(1) << "RGB_TextureTriangle Triangle class member is empty";
		return glm::vec3(0.f);
	}
}

glm::vec3 RGBCubeTexture::getTexel(const glm::vec3 &pos) const
{
	if (cube)
		// calculate barycentric coordinates
		return 10.f * ((cube->world_to_obj * glm::vec4(pos, 1.f) 
			+ glm::vec4(cube->boundaries, 0.f)) / glm::vec4(cube->boundaries, 1.f) * 0.5f);
	else if (unitcube)
	{
		return 100.f * ((unitcube->world_to_obj * glm::vec4(pos, 1.f)
			+ glm::vec4(0.5f, 0.5f, 0.5f, 0.f)));
	}
	else
	{
		VLOG(1) << "RGBCubeTexture Cube/UnitCube class member is empty";
		return glm::vec3(0.f);
	}
}

}