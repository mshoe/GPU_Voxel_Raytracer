#pragma once

#include "ChunkDB.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <GLFW/glfw3.h>
#include <string>
#include <random>
#include <glm/glm.hpp>



namespace ProcGen {

	
	
	void basic_init(ChunkDB::Chunk &chunk);
	void simple_noise(const std::string &seed_str, ChunkDB::Chunk &chunk);
	int simple_noise_eval(glm::ivec2 pos, std::vector<std::vector<float>> &hmap, int dv, int vl, int vw);

	// create an array of random true or false values
	void random_boolean_2d(const std::string &seed_str, std::vector<std::vector<bool>> &ret_val);

	void cellular_cave(const std::string &seed_str, ChunkDB::Chunk &chunk);



}