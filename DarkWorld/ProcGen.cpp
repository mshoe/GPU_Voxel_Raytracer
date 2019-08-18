#include "ProcGen.h"

using namespace ChunkDB;
namespace ProcGen {


	void basic_init(Chunk &chunk)
	{
		chunk = Chunk();
		chunk.pos = glm::ivec2(0, 0);
		chunk.data = std::vector<GLuint>(CHUNK_L * CHUNK_H * CHUNK_W, 1);
		//std::fill(chunk.data.begin(), chunk.data.end(), 1);
	}

	void simple_noise(const std::string &seed_str, Chunk &chunk)
	{
		chunk = Chunk();
		chunk.pos = glm::ivec2(0, 0);
		chunk.data = std::vector<GLuint>(CHUNK_L * CHUNK_H * CHUNK_W, 0);

		//std::string seed_str = "TESTING_MXU";
		std::seed_seq seed(seed_str.begin(), seed_str.end());
		std::minstd_rand0 generator(seed);
		

		// First create array with random values
		int dv = 8;
		int vw = CHUNK_W / dv;
		int vl = CHUNK_L / dv;
		chunk.hmap.resize(vl);
		for (int i = 0; i < vl; i++) {
			chunk.hmap[i].resize(vw);
			for (int k = 0; k < vw; k++) {
				chunk.hmap[i][k] =  generator() % (CHUNK_H - 1);
			}
		}

		chunk.hcorners.x = chunk.hmap[0][0];
		chunk.hcorners.y = chunk.hmap[vw - 1][0];
		chunk.hcorners.z = chunk.hmap[vw - 1][vw - 1];
		chunk.hcorners.w = chunk.hmap[0][vw - 1];


		for (int i = 0; i < CHUNK_L; i++) {
			for (int k = 0; k < CHUNK_W; k++) {
				int voxel_height = simple_noise_eval(glm::ivec2(i, k), chunk.hmap, dv, vl, vw);
				for (int j = voxel_height; j >= 0; j--) {
					//int j = voxel_height;
					chunk.data[i*CHUNK_W*CHUNK_H + j * CHUNK_W + k] = 1;
				}
			}
		}
	}

	int simple_noise_eval(glm::ivec2 pos, std::vector<std::vector<float>> &hmap, int dv, int vl, int vw)
	{
		int i1 = int(glm::floor(pos.x / dv));
		int k1 = int(glm::floor(pos.y / dv));
		int i2 = (i1 + 1) % vl;
		int k2 = (k1 + 1) % vw;
		float tx = glm::smoothstep(float(i1 * dv), float((i1 + 1.f) * dv), float(pos.x));
		float rx0 = glm::mix(hmap[i1][k1], hmap[i2][k1], tx);
		float rx1 = glm::mix(hmap[i1][k2], hmap[i2][k2], tx);

		float tz = glm::smoothstep(float(k1 * dv), float((k1 + 1.f) * dv), float(pos.y));
		float height = glm::mix(rx0, rx1, tz);
		int voxel_height = glm::clamp(int(glm::round(height)), 0, CHUNK_H - 1);
		return voxel_height;
	}

	void random_boolean_2d(const std::string & seed_str, std::vector<std::vector<bool>>& ret_val)
	{
		std::seed_seq seed(seed_str.begin(), seed_str.end());
		std::minstd_rand0 generator(seed);

		ret_val.resize(CHUNK_L);
		for (int i = 0; i < CHUNK_L; i++) {
			ret_val[i].resize(CHUNK_W);
			for (int k = 0; k < CHUNK_W; k++) {
				int random_01 = generator() % 100;
				if (random_01 < 45) {
					ret_val[i][k] = false;
				}
				else {
					ret_val[i][k] = true;
				}
			}
		}
	}

	void cellular_cave(const std::string & seed_str, Chunk & chunk)
	{
		// use cellular automata to generate a cave
		std::vector<std::vector<bool>> cave_array;
		random_boolean_2d(seed_str, cave_array);

		chunk = Chunk();
		chunk.data = std::vector<GLuint>(CHUNK_L * CHUNK_H * CHUNK_W, 0);
		
		int j = CHUNK_H / 2;

		for (int i = 0; i < CHUNK_L; i++) {
			for (int k = 0; k < CHUNK_W; k++) {
				chunk.data[i*CHUNK_W*CHUNK_H + j * CHUNK_W + k] = (cave_array[i][k]) ? 1 : 0;
			}
		}


	}

	

}