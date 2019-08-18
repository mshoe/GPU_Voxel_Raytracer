#pragma once

#include <glad/glad.h>

#include "Constants.h"
#include "RenderEngine.h"
#include "Camera.h"
#include "ChunkDB.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


#include <GLFW/glfw3.h>
#include <memory>
#include <iostream>
#include <chrono>

/*

Main engine should be the one engine that links all the other engines together.

e.g. Render engine wants to know which chunks to render? 
Main engine passes those chunks from the Chunk Manager to the Rendering Engine

*/


class MainEngine
{
public:
	MainEngine();
	~MainEngine();

	void Loop();

private:

	bool Init();
	bool CleanUp();

	bool InitShaderPipeline(const ChunkDB::Chunk &chunk);

	void processInput(GLFWwindow *window, float dt);
	void Update(float lag);
	void Render();

	//void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

	void UpdateMouse();
	void UpdateLookat();


	bool m_shouldClose = false;
	bool m_cameraImgui = true;

	std::unique_ptr<Camera> m_camera;

	std::unique_ptr<RenderEngine> m_renderEngine;
	
	std::unique_ptr<ComputeShader> m_cShader;
	GLuint voxel_select_ssbo;
	GLsizeiptr voxel_select_ssbo_length;
	GLuint voxel_grid_ssbo;

	glm::uvec4 m_hoverVoxel;


	ChunkDB::Chunk m_chunk;
	//std::shared_ptr<ChunkDB::ChunkManager> chunkManager;

	GLFWwindow *m_window;

	std::string m_seed = "yuniyah";

	
};

