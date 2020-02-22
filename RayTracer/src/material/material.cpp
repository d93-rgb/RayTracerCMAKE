#include "material/material.h"

namespace rt
{
glm::dvec3 Material::getAmbient(glm::dvec3 pos)
{
	if (tex)
	{
		return 0.001f * tex->getTexel(pos);
	}
	return ambient;
}

glm::dvec3 Material::getDiffuse(glm::dvec3 pos)
{
	if (tex)
	{
		return tex->getTexel(pos);
	}
	return diffuse;
}

} // namespace rt