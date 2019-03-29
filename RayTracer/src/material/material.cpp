#include "material/material.h"

namespace rt
{
glm::vec3 Material::getAmbient(glm::vec3 pos)
{
	if (tex)
	{
		return 0.001f * tex->getTexel(pos);
	}
	return ambient;
}

glm::vec3 Material::getDiffuse(glm::vec3 pos)
{
	if (tex)
	{
		return tex->getTexel(pos);
	}
	return diffuse;
}

} // namespace rt