#pragma once
#include "rt.h"

namespace rt
{

class PhongIntegrator : public Integrator
{
public:

	virtual glm::vec3 Li();

private:

};

} // namespace rt