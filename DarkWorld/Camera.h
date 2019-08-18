#pragma once
#include <glm/glm.hpp>

class Camera
{
public:
	Camera(glm::vec3 from_, glm::vec3 to_, glm::vec3 world_up_) : m_from(from_)
		, m_to(to_)
		, m_world_up(world_up_)
	{
		CreateCameraVectors();
		CreateLookat();

	}
	~Camera() {

	}

	void CreateCameraVectors() {
		m_forward = glm::normalize(m_from - m_to);
		m_right = glm::cross(m_world_up, m_forward);
		m_up = glm::cross(m_forward, m_right);
		m_world_forward = glm::cross(m_world_up, m_right);
	}

	void CreateLookat() {
		m_lookat[0] = glm::vec4(m_right, 0.0f);
		m_lookat[1] = glm::vec4(m_up, 0.0f);
		m_lookat[2] = glm::vec4(m_forward, 0.0f);
		m_lookat[3] = glm::vec4(m_from, 1.0f);
	}

	void moveHorizontal(float move) {
		m_from += m_right * move;
		m_to += m_right * move;
		CreateLookat();
	}
	void moveVertical(float move) {
		m_from += m_world_up * move;
		m_to += m_world_up * move;
		CreateLookat();
	}
	void moveForward(float move) {
		m_from += m_world_forward * move;
		m_to += m_world_forward * move;
		CreateLookat();
	}
	void rotateAroundToYAxis(float angle) {
		angle = glm::radians(angle);
		glm::mat3 rotation;
		rotation[0] = glm::vec3(glm::cos(angle), 0.0f, glm::sin(angle));
		rotation[1] = glm::vec3(0.0f, 1.0f, 0.0f);
		rotation[2] = glm::vec3(-glm::sin(angle), 0.0f, glm::cos(angle));

		m_from -= m_to;
		m_from = rotation * m_from;
		m_from += m_to;

		CreateCameraVectors();
		CreateLookat();
	}

	glm::mat4 lookat() const { return m_lookat; }
	glm::vec3 right() const { return m_right; }
	glm::vec3 up() const { return m_up; }
	glm::vec3 forward() const { return m_forward; }
	glm::vec3 from() const { return m_from; }
	glm::vec3 to() const { return m_to; }
	glm::vec3 worldUp() const { return m_world_up; }
	glm::vec3 worldForward() const { return m_world_forward; }

private:

	glm::mat4 m_lookat;
	
	
	glm::vec3 m_up;
	glm::vec3 m_forward;
	glm::vec3 m_right;
	glm::vec3 m_from;
	glm::vec3 m_to;
	glm::vec3 m_world_up;
	glm::vec3 m_world_forward;
};

