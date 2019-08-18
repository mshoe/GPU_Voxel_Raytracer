#include "MainEngine.h"

void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);
std::string vec3_to_string(glm::vec3 vec);
void APIENTRY
DebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}



MainEngine::MainEngine()
{
	bool res = Init();
	if (!res) {
		std::cout << "Init Error" << std::endl;
	}
}


MainEngine::~MainEngine()
{
	CleanUp();
}

bool MainEngine::Init()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	m_window = glfwCreateWindow(SRC_WIDTH, SRC_HEIGHT, "Dark World", NULL, NULL);
	if (m_window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(m_window);
	glfwSetFramebufferSizeCallback(m_window, FrameBufferSizeCallback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(DebugOutput, NULL);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	glfwSetWindowShouldClose(m_window, false);

	

	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplGlfw_InitForOpenGL(m_window, true);
	ImGui_ImplOpenGL3_Init("#version 450");

	// Setup style
	ImGui::StyleColorsDark();

	// create game variables

	// Initialize camera
	m_camera = std::make_unique<Camera>(glm::vec3(1000.f, 730.f, 1000.f), glm::vec3(0.f, -100.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	

	// Initialize render engine
	
	m_renderEngine = std::make_unique<RenderEngine>(float(SRC_WIDTH), float(SRC_HEIGHT));
	
	

	// ProcGen the chunk
	
	ProcGen::simple_noise(m_seed, m_chunk);

	InitShaderPipeline(m_chunk);

	

	UpdateLookat();

	
	
	return true;
}

bool MainEngine::CleanUp()
{
	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(m_window);
	glfwTerminate();
	return true;
}

bool MainEngine::InitShaderPipeline(const ChunkDB::Chunk &chunk)
{

	// 1. Make the ssbo for the voxel grid
	glCreateBuffers(1, &voxel_grid_ssbo);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxel_grid_ssbo);
	glNamedBufferStorage(voxel_grid_ssbo, sizeof(GLuint)*chunk.data.size(), &(chunk.data.front()), GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, voxel_grid_ssbo);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind

	// 2. Make the compute shader for selecting voxels
	m_cShader = std::make_unique<ComputeShader>(std::vector<std::string> { "voxelSelect.comp", "rayTracing.fs" });
	//m_cShader = std::make_unique<ComputeShader>(std::vector<std::string> { "computeShaderTest.comp" });

	// 3. Make the ssbo for storing which voxel is selected
	glCreateBuffers(1, &voxel_select_ssbo);
	std::vector<GLuint> init_select = { 0, 0, 0, 0 };
	voxel_select_ssbo_length = sizeof(init_select);
	glNamedBufferStorage(voxel_select_ssbo, voxel_select_ssbo_length, &(init_select.front()), GL_DYNAMIC_STORAGE_BIT | GL_MAP_READ_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, voxel_select_ssbo);
	
	// 4. Make the shader program for rendering the voxels
	m_renderEngine->constructVoxelShader();

	
	return true;
}


// INPUT MANAGER
void MainEngine::processInput(GLFWwindow *window, float dt)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		m_camera->rotateAroundToYAxis(-TURN_SPEED * dt);
	else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		m_camera->rotateAroundToYAxis(TURN_SPEED * dt);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		m_camera->moveHorizontal(MOVE_SPEED * dt);
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		m_camera->moveHorizontal(-MOVE_SPEED * dt);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		m_camera->moveForward(MOVE_SPEED * dt);
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		m_camera->moveForward(-MOVE_SPEED * dt);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		m_camera->moveVertical(MOVE_SPEED * dt);
	else if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS)
		m_camera->moveVertical(-MOVE_SPEED * dt);

	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		m_renderEngine->ZoomVoxel(ZOOM_SPEED * dt);
	else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		m_renderEngine->ZoomVoxel(-ZOOM_SPEED * dt);

}





void MainEngine::Loop()
{
	using namespace ChunkDB;
	using namespace std::chrono;
	steady_clock::time_point t1 = steady_clock::now();
	float lag = 0.f;

	
	while (!glfwWindowShouldClose(m_window))
	{
		steady_clock::time_point t2 = steady_clock::now();
		float dt = duration_cast<duration<float>>(t2 - t1).count();
		t1 = t2;
		lag += dt;

		while (lag >= S_PER_UPDATE) {
			processInput(m_window, lag);
			Update(lag);
			lag -= S_PER_UPDATE;
		}

		double xpos, ypos;
		glfwGetCursorPos(m_window, &xpos, &ypos);
		glm::vec4 mouse(float(xpos), SRC_HEIGHT - float(ypos), 0.0f, 0.0f);
		glm::mat4 camera(m_camera->lookat());

		// Set Uniforms
		m_renderEngine->m_shader->Use();
		m_renderEngine->m_shader->SetVec("iMouse", mouse);
		m_renderEngine->m_shader->SetMat("iCamera", camera);
		m_renderEngine->m_shader->SetVec("iResolution", glm::ivec2(SRC_WIDTH, SRC_HEIGHT));
		m_renderEngine->m_shader->SetFloat("VOX_W", 20.f);
		m_renderEngine->m_shader->SetFloat("VOX_H", 10.f);

		m_cShader->Use();
		m_cShader->SetVec("iMouse", mouse);
		m_cShader->SetMat("iCamera", camera);
		m_cShader->SetVec("iResolution", glm::ivec2(SRC_WIDTH, SRC_HEIGHT));
		m_cShader->SetFloat("VOX_W", 20.f);
		m_cShader->SetFloat("VOX_H", 10.f);

		

		glDispatchCompute(1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT); // let compute shader finish running
		void *ptr = glMapNamedBuffer(voxel_select_ssbo, GL_READ_ONLY);
		//GLuint test = (GLuint)(*ptr);

		GLuint* data = static_cast<GLuint*>(ptr);
		std::vector<GLuint> selectedVoxel(data, data + 4);
		//std::cout << selectedVoxel[0] << ", " << selectedVoxel[1] << ", " << selectedVoxel[2] << ", " << selectedVoxel[3] << std::endl;
		glUnmapNamedBuffer(voxel_select_ssbo);

		//std::vector<GLuint> selectedVoxel = { 1, 1, 1, 1 };
		m_hoverVoxel.x = selectedVoxel[0];
		m_hoverVoxel.y = selectedVoxel[1];
		m_hoverVoxel.z = selectedVoxel[2];
		m_hoverVoxel.w = selectedVoxel[3];

		if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			std::cout << "pressed" << std::endl;
			unsigned int i, j, k;
			i = m_hoverVoxel.x;
			j = m_hoverVoxel.y;
			k = m_hoverVoxel.z;
			m_chunk.data[i * CHUNK_W * CHUNK_H + j * CHUNK_W + k] = 0;
			glNamedBufferSubData(voxel_grid_ssbo, 0, sizeof(GLuint)*m_chunk.data.size(), &(m_chunk.data.front()));
		}
		m_renderEngine->m_shader->Use();
		m_renderEngine->m_shader->SetVec("iHoverVoxel", m_hoverVoxel);
		Render();
		
		
		glfwPollEvents();
	}
	
}

void MainEngine::Render()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	m_renderEngine->Render();

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	if (m_cameraImgui) {
		ImGui::Begin("Camera Vectors", &m_cameraImgui);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text(vec3_to_string(m_camera->right()).c_str());
		ImGui::Text(vec3_to_string(m_camera->up()).c_str());
		ImGui::Text(vec3_to_string(-m_camera->forward()).c_str());
		ImGui::Text(vec3_to_string(m_camera->from()).c_str());
		ImGui::Text(vec3_to_string(m_camera->to()).c_str());
		//static std::vector<char> = "mxu is the coolest";
		//ImGui::InputText("seed", seed_str., IM_ARRAYSIZE(seed_str));
		if (ImGui::Button("Close Me"))
			m_cameraImgui = false;
		if (ImGui::Button("Flat")) {
			ProcGen::basic_init(m_chunk);
			//m_renderEngine->constructVoxelShader(m_chunk);
			InitShaderPipeline(m_chunk);
		}
		if (ImGui::Button("Simple Noise")) {
			ProcGen::simple_noise(m_seed, m_chunk);
			//m_renderEngine->constructVoxelShader(m_chunk);
			InitShaderPipeline(m_chunk);
		}
		if (ImGui::Button("Cellular Automata Cave")) {
			ProcGen::cellular_cave(m_seed, m_chunk);
			//m_renderEngine->constructVoxelShader(m_chunk);
			InitShaderPipeline(m_chunk);
		}
		ImGui::End();

		//ImGui::ShowDemoWindow();
	}
	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(m_window);
}

void MainEngine::Update(float lag)
{
	
	//UpdateMouse();
	//UpdateLookat();
}

void MainEngine::UpdateMouse()
{
	double xpos, ypos;
	glfwGetCursorPos(m_window, &xpos, &ypos);
	//m_renderEngine->UpdateMouse(float(xpos), float(ypos), 0.0f, 0.0f);
}

void MainEngine::UpdateLookat()
{
	
	m_renderEngine->UpdateLookat(m_camera->lookat());
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void FrameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	//renderEngine->UpdateResolution(width, height);
}

// Helper function for imgui
std::string vec3_to_string(glm::vec3 vec) {
	return std::string("(") +
		std::to_string(vec.x) + std::string(", ") +
		std::to_string(vec.y) + std::string(", ") +
		std::to_string(vec.z) + std::string(")");
}

