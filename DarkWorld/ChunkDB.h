#pragma once
#include "Constants.h"
#include "sqlite3.h"

#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <iostream>

#define CHUNK_DB_DEBUG 1

namespace ChunkDB {


	class Chunk;
	class ChunkManager;


	class Chunk
	{
	public:
		Chunk();
		Chunk(std::string chunk_str);
		~Chunk();

		std::vector<GLuint> getData();

		std::vector<GLuint> data;
		glm::ivec2 pos; // x,z center position of chunk
		std::string dataToString();

		std::vector<std::vector<float>> hmap;
		glm::vec4 hcorners;
	};


	//enum ChunkDBCodes {
	//	SELECT_MISS,
	//	SELECT_HIT
	//};

	//class ChunkManager
	//{	
	//public:
	//	ChunkManager(std::string save_file_);
	//	~ChunkManager();

	//	void Init(std::string save_file_);
	//	void Cleanup();

	//	void SaveChunk(Chunk chunk);

	//	bool ExecQuery(std::string sql, int(*callback)(void*,int,char**,char**), void*data);
	//	Chunk GetChunk(glm::ivec2 pos);
	//	

	//	//std::unordered_map<glm::ivec2, Chunk> loadedChunks;
	//	
	//	int GetChunkCallback(int argc, char **argv, char **colName);
	//private:
	//	std::string save_file;
	//	std::string dbName = "Chunk DB";
	//	std::string mainTableName = "Chunks";
	//	sqlite3* pDB;

	//	Chunk got_chunk;
	//};

}