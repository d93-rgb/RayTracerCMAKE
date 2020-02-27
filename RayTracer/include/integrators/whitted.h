#pragma once
#include "core/rt.h"
#include "integrators/integrator.h"

namespace rt
{

class WhittedIntegrator : public Integrator
{
public:
	glm::dvec3 Li(const Ray& ray, const Scene& scene, int depth);
private:

};

} // namespace rt