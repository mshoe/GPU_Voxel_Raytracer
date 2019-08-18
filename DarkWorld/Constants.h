#pragma once
const int SRC_WIDTH = 1280;
const int SRC_HEIGHT = 800;

const float MOVE_SPEED = 1000.f;
const float TURN_SPEED = 45.f;
const float ZOOM_SPEED = 10.f;

const float S_PER_UPDATE = 0.016667f;

namespace ChunkDB {
	const int CHUNK_W = 32;
	const int CHUNK_H = 32;
	const int CHUNK_L = 32;

	const float VOX_W = 20.f;
	const float VOX_H = 20.f;
}