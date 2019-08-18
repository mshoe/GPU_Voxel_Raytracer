#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "MainEngine.h"


int main(int argc, char* argv) {

	

	MainEngine *mainEngine = new MainEngine();
	

	mainEngine->Loop();

	delete mainEngine;
	
	return 0;
}