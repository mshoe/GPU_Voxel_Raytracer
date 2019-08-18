#include "RenderEngine.h"



RenderEngine::RenderEngine(float width, float height)
{
	bool result = Init(width, height);
	if (!result)
		std::cout << "Failed to initialize render engine" << std::endl;

}

RenderEngine::~RenderEngine()
{
	bool result = CleanUp();
	if (!result)
		std::cout << "Failed to clean up render engine" << std::endl;
}

bool RenderEngine::Init(float width, float height)
{
	//chunkManager = chunkManager_;
	time = 0.f;

	float vertices[] = {
		1.f,  1.f, // top right
		1.f, -1.f, // bottom right
		-1.f, -1.f, // bottom left
		-1.f,  1.f, // top left 
	};
	unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};


	// ..:: Initialization code :: ..
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glCreateVertexArrays(1, &VAO);
	glCreateBuffers(1, &VBO);
	glCreateBuffers(1, &EBO);

	// 1. bind Vertex Array Object
	glBindVertexArray(VAO);

	// 2. copy our vertices array in a vertex buffer for OpenGL to use
	glNamedBufferStorage(VBO, sizeof(vertices), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glNamedBufferSubData(VBO, 0, sizeof(vertices), vertices);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// 3. copy our index array in a element buffer for OpenGL to use
	glNamedBufferStorage(EBO, sizeof(indices), NULL, GL_DYNAMIC_STORAGE_BIT);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glNamedBufferSubData(EBO, 0, sizeof(indices), indices);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// 4. then set the vertex attributes pointers
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// temporary proc gen chunk data code

	//std::cout << chunk.toString() << std::endl;
	//chunkManager->SaveChunk(ProcGen::simple_noise("yghyuh"));


	return true;
}


void RenderEngine::constructVoxelShader()
{
	m_shader = std::make_unique<VoxelRaytracerShader>("voxelShader.vs", std::vector<std::string>{"voxelShader.fs", "rayTracing.fs"});

	m_shader->Use();
	m_shader->SetFloat("iTime", time);
	m_shader->SetFloat("VOX_W", m_vox_w);
	m_shader->SetFloat("VOX_H", m_vox_h);
	//shader->setInt("CHUNK_W", ChunkDB::CHUNK_W);
	//shader->setInt("CHUNK_H", ChunkDB::CHUNK_H);


	

	//glCreateBuffers(1, &SSBO);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
	//glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint)*chunk.data.size(), &(chunk.data.front()), GL_STATIC_DRAW);
	//glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, SSBO);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
}

void RenderEngine::ZoomVoxel(float value)
{
	m_vox_w += value;
	m_vox_w = glm::min(m_vox_w, 40.f);
	m_vox_w = glm::max(m_vox_w, 5.f);

	m_vox_h = m_vox_w / 2.f;

}


bool RenderEngine::CleanUp()
{
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	return true;
}

void RenderEngine::Render()
{


	m_shader->Use();
	//UpdateUniforms();
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	
}

void RenderEngine::UpdateMouse(glm::vec4 mouse_)
{
	mouse = mouse_;
}

void RenderEngine::UpdateLookat(glm::mat4 lookat_)
{
	lookat = lookat_;
}


void RenderEngine::UpdateTime(float time_)
{
	time = time_;
}

//void RenderEngine::UpdateUniforms()
//{
//	m_shader->SetVec("iResolution", glm::ivec2(SRC_WIDTH, SRC_HEIGHT));
//	m_shader->SetVec("iMouse", mouse);
//	m_shader->SetMat("iCamera", lookat);
//	m_shader->SetFloat("iTime", time);
//	m_shader->SetFloat("VOX_W", vox_w);
//	m_shader->SetFloat("VOX_H", vox_h);
//}
