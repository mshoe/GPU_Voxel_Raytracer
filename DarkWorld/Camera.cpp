#include "Camera.h"




Camera::Camera(glm::vec3 from_, glm::vec3 to_, glm::vec3 world_up_)
	: from(from_)
	, to(to_)
	, world_up(world_up_)
{
	createCameraVectors();
	createLookat();
	
}

Camera::~Camera()
{
	
}

void Camera::createCameraVectors()
{
	// forward
	forward = glm::normalize(from - to);

	// right
	right = glm::cross(world_up, forward);

	// up
	up = glm::cross(forward, right);

	// world_forward
	world_forward = glm::cross(world_up, right);
}

void Camera::createLookat()
{
	lookat[0] = glm::vec4(right, 0.0f);
	lookat[1] = glm::vec4(up, 0.0f);
	lookat[2] = glm::vec4(forward, 0.0f);
	lookat[3] = glm::vec4(from, 1.0f);
}

void Camera::moveHorizontal(float move)
{
	from += right * move;
	to += right * move;
	createLookat();
}

void Camera::moveVertical(float move)
{
	from += world_up * move;
	to += world_up * move;
	createLookat();
}

void Camera::moveForward(float move)
{
	from += world_forward * move;
	to += world_forward * move;
	createLookat();
}

void Camera::rotateAroundToYAxis(float angle)
{
	angle = glm::radians(angle);
	glm::mat3 rotation;
	rotation[0] = glm::vec3(glm::cos(angle), 0.0f, glm::sin(angle));
	rotation[1] = glm::vec3(0.0f, 1.0f, 0.0f);
	rotation[2] = glm::vec3(-glm::sin(angle), 0.0f, glm::cos(angle));

	from -= to;
	from = rotation * from;
	from += to;

	createCameraVectors();
	createLookat();
}