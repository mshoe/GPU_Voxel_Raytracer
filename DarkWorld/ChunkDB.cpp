#include "ChunkDB.h"


namespace ChunkDB {

	/*static int c_GetChunkCallback(void *param, int argc, char **argv, char **colName)
	{
		
		ChunkManager *cm = reinterpret_cast<ChunkManager*>(param);
		return cm->GetChunkCallback(argc, argv, colName);
	}*/

	Chunk::Chunk()
	{
		
	}

	Chunk::Chunk(std::string chunk_str)
	{
		data.resize(CHUNK_L * CHUNK_H * CHUNK_W);

		std::string num;
		int x = 0;
		for (int i = 0; i < CHUNK_L; i++) {
			for (int j = 0; j < CHUNK_H; j++) {
				for (int k = 0; k < CHUNK_W; k++) {


					while (chunk_str[x] != ',') {
						num += chunk_str[x];
						x++;
					}
					x++;
					GLuint id = std::stoi(num);
					num = "";
					data[i * CHUNK_W * CHUNK_H + j * CHUNK_W + k] = id;
				}
			}
		}
	}

	Chunk::~Chunk()
	{

	}

	std::vector<GLuint> Chunk::getData()
	{
		return data;
	}

	std::string Chunk::dataToString()
	{
		std::string save_str = "";
		for (int i = 0; i < data.size(); i++) {
			save_str += std::to_string(data[i]) + ",";
		}
		return save_str;
	}

//	ChunkManager::ChunkManager(std::string save_file_)
//	{
//		Init(save_file_);
//	}
//
//
//	ChunkManager::~ChunkManager()
//	{
//		Cleanup();
//	}
//
//	void ChunkManager::Init(std::string save_file_)
//	{
//		save_file = save_file_;
//		int rc = sqlite3_open(save_file.c_str(), &pDB);
//		if (rc != SQLITE_OK) {
//			std::cout << "Load unsuccessful" << std::endl;
//		}
//
//		std::string createTableQuery = "create table if not exists " + 
//			mainTableName +
//			"(pos text primary key, data text)";
//
//		bool result = ExecQuery(createTableQuery, nullptr, nullptr);
//
//	}
//
//
//	void ChunkManager::Cleanup()
//	{
//		sqlite3_close(pDB);
//	}
//
//	void ChunkManager::SaveChunk(Chunk chunk)
//	{
//		std::string pos_str = "'" + std::to_string(chunk.pos.x) + "," + std::to_string(chunk.pos.y) + "'";
//		std::string chunk_str = "'" + chunk.dataToString() + "'";
//		std::string insertQuery = "insert into " + mainTableName +
//			" (pos, data) " +
//			" values " +
//			" (" + pos_str + "," + chunk_str + ")" +
//			" on conflict (pos) do update set data = excluded.data;";
//		ExecQuery(insertQuery, nullptr, nullptr);
//	}
//
//	bool ChunkManager::ExecQuery(std::string sql, int(*callback)(void *, int, char **, char **), void *data)
//	{
//		char *errMsg;
//		int rc = sqlite3_exec(pDB, sql.c_str(), callback, data, &errMsg);
//#ifdef CHUNK_DB_DEBUG
//		//std::cout << sql << std::endl;
//#endif
//		if (rc != SQLITE_OK) {
//#ifdef CHUNK_DB_DEBUG
//			std::cout << "SQL Error: " << errMsg << std::endl;
//#endif
//			sqlite3_free(errMsg);
//			return false;
//		}
//#ifdef CHUNK_DB_DEBUG
//		std::cout << "Query executed sucessfully" << std::endl;
//#endif
//		return true;
//	}
//
//	Chunk ChunkManager::GetChunk(glm::ivec2 pos)
//	{
//		std::string pos_str = "'" + std::to_string(pos.x) + "," + std::to_string(pos.y) + "'";
//		std::string selectQuery = "select data from " + mainTableName +
//			" where pos = " + pos_str + ";";
//
//		bool result = ExecQuery(selectQuery, c_GetChunkCallback, this);
//		return got_chunk;
//	}
//
//	int ChunkManager::GetChunkCallback(int argc, char ** argv, char ** colName)
//	{
//		std::string chunk_str = argv[0];
//		got_chunk = Chunk(chunk_str);
//		return 0;
//	}

}