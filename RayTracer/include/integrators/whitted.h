#pragma once
#include "core/rt.h"
#include "integrators/integrator.h"

namespace rt
{

class WhittedIntegrator : public Integrator
{
public:
	RGB_Color Li(const Ray& ray, const Scene& scene, int depth);
private:

};

} // namespace rt