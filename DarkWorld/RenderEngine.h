#pragma once

#include <glad/glad.h>


#include "Constants.h"
#include "Shader.h"
#include "ProcGen.h"
#include "ChunkDB.h"

#include <GLFW/glfw3.h>
#include <memory>
#include <iostream>
#include <vector>
#include <algorithm>

/*
Render Engine

- Should only perform rendering.
- Shouldn't be linked to the Chunk Manager.
- Chunk data is passed to the engine via const reference.

*/

class RenderEngine
{
public:
	RenderEngine(float width, float height);
	~RenderEngine();

	bool Init(float width, float height);
	bool CleanUp();
	void Render();

	void UpdateMouse(glm::vec4 mouse_);
	//void UpdateMouseRaycast(glm::ivec3 mouse_voxel_);
	void UpdateLookat(glm::mat4 lookat_);
	void UpdateTime(float time_);
	//void UpdateUniforms();

	void constructVoxelShader();

	void ZoomVoxel(float value);

	std::unique_ptr<VoxelRaytracerShader> m_shader;

private:
	GLuint VAO;
	GLuint VBO;
	GLuint EBO;
	
	GLuint SSBO;
	
	//glm::vec2 resolution;

	glm::vec4 mouse;
	float time = 0.f;
	glm::mat4 lookat;
	glm::ivec3 mouse_voxel;

	float m_vox_w = 20.f;
	float m_vox_h = 10.f;
	
	

	//std::shared_ptr<ChunkDB::ChunkManager> chunkManager;
};

