#include "camera/camera.h"

namespace rt
{

Camera::~Camera() {}

// This function provides a look-at transformation.
// It moves the camera to the desired position and orientates its viewing direction to the
// given gaze point.
/*
	eyePosition: the position of the camera in world space
	gazePoint: the point the camera is looking at
	upVector: the up vector of the camera for specifying orientation
*/
void Camera::setCamToWorld(glm::vec3 eyePosition, glm::vec3 gazePoint, glm::vec3 upVector)
{
	// z-axis points in the opposite direction of the view vector, by convention
	// the viewDir therefore must point in the opposite direction
	assert(eyePosition != gazePoint);
	glm::vec3 viewDir = glm::normalize(eyePosition - gazePoint);
	glm::vec3 crossVec = glm::cross(glm::normalize(upVector), viewDir);
	glm::vec3 newUpVec = glm::cross(viewDir, crossVec);

	camToWorld[0] = glm::vec4(crossVec, 0.f);
	camToWorld[1] = glm::vec4(newUpVec, 0.f);
	camToWorld[2] = glm::vec4(viewDir, 0.f);
	camToWorld[3] = glm::vec4(eyePosition, 1.f);

}

void Camera::update()
{
	origin = camToWorld * origin;
	up = camToWorld * up;
	right = camToWorld * right;
	front = camToWorld * front;
}
}