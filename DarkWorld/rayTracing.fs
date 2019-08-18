#version 450 core





uniform vec2 iResolution; // viewport resolution (in pixels)
uniform vec4 iMouse; // Mouse pixel coords. xy: current, zw: click
uniform float iTime; // shader playback time (in seconds)
uniform mat4 iCamera;



uniform float VOX_W = 20.0;
uniform float VOX_H = 10.0;

const int CHUNK_W = 32;
const int CHUNK_H = 32;
const int CHUNK_L = 32;

const vec3 VEC_NULL = vec3(0.0, 0.0, 0.0);
const vec3 VEC_POS_X = vec3(1.0, 0.0, 0.0);
const vec3 VEC_NEG_X = vec3(-1.0, 0.0, 0.0);
const vec3 VEC_POS_Y = vec3(0.0, 1.0, 0.0);
const vec3 VEC_NEG_Y = vec3(0.0, -1.0, 0.0);
const vec3 VEC_POS_Z = vec3(0.0, 0.0, 1.0);
const vec3 VEC_NEG_Z = vec3(0.0, 0.0, -1.0);

const vec3 camera_from = iCamera[3].xyz;
const vec3 camera_dir = -iCamera[2].xyz;
const vec3 camera_right = iCamera[0].xyz;
const vec3 camera_up = iCamera[1].xyz;


// CHUNK DATA
layout(std430, binding = 3) buffer chunk_data
{
    readonly uint data_SSBO[];//[CHUNK_W * CHUNK_H * CHUNK_W];
};

/** Simple World **/
uint SimpleWorld(uint i, uint j, uint k) {
	if (0 <= i && i < CHUNK_L &&
		0 <= j && j < CHUNK_H &&
		0 <= k && k < CHUNK_W) {
		return data_SSBO[i * CHUNK_W * CHUNK_H + j * CHUNK_W + k];
	}
	return 0;
}

float Map(vec3 p) {
	return float(SimpleWorld(int(floor(p.x + 0.5)), int(floor(p.y + 0.5)), int(floor(p.z + 0.5))));
}


float PlaneIntersect(float s, float v, float r) {
   // intersection between line and a plane
   return (r - s) / v;
}

void VoxelPlaneIntersect(float s, float v, float r0, float r1,
                           out float t) {
   // This function returns whether the ray hits plane r0, r1, or neither
   // r0 < r1
   // If the ray described by P(t) = S + tV hits r0 or r1, 
   // return the parametric value in t

	if (v == 0.0) {
		t = -1.0;
		return;
	}
	else if (v > 0.0) {
		if (s < r0)
			t = PlaneIntersect(s, v, r0);
		else if (s < r1) // in between the two faces
			t = PlaneIntersect(s, v, r1);
		else
			t = -1.0;
		return;
	}
	else if (v < 0.0) {
		if (s > r1)
			t = PlaneIntersect(s, v, r1);
		else if (s > r0) // in between the two faces
			t = PlaneIntersect(s, v, r1);
		else
			t = -1.0;
		return;
	}
}

void BoxIntersect(	vec3 rs, vec3 rv,
					float rx0, float ry0, float rz0,
					float rx1, float ry1, float rz1,
					out float t, out vec3 n_dir, 
					out vec2 uv, out vec3 u_dir, out vec3 v_dir) {

	bool pos_neg_face;

	VoxelPlaneIntersect(rs.x, rv.x, rx0, rx1, t);
	if (t > 0.0) {
		vec3 p = rs + t * rv;
		if ( ry0 <= p.y && p.y <= ry1 && rz0 <= p.z && p.z <= rz1 ) {
			n_dir = (rv.x > 0) ? VEC_NEG_X : VEC_POS_X;
			u_dir = VEC_POS_Z;
			v_dir = VEC_POS_Y;

			uv.x = (p.z - rz0) / (rz1 - rz0);
			uv.y = (p.y - ry0) / (ry1 - ry0);
			return;
		}
	}

	VoxelPlaneIntersect(rs.y, rv.y, ry0, ry1, t);
	if (t > 0.0) {
		vec3 p = rs + t * rv;
		if ( rx0 <= p.x && p.x <= rx1 && rz0 <= p.z && p.z <= rz1 ) {
			n_dir = (rv.y > 0) ? VEC_NEG_Y : VEC_POS_Y;

			u_dir = VEC_POS_X;
			v_dir = VEC_POS_Z;

			uv.x = (p.x - rx0) / (rx1 - rx0);
			uv.y = (p.z - rz0) / (rz1 - rz0);
			return;
		}
	}

	VoxelPlaneIntersect(rs.z, rv.z, rz0, rz1, t);
	if (t > 0.0) {
		vec3 p = rs + t * rv;
		if ( rx0 <= p.x && p.x <= rx1 && ry0 <= p.y && p.y <= ry1 ) {
			n_dir = (rv.z > 0) ? VEC_NEG_Z : VEC_POS_Z;

			u_dir = VEC_POS_X;
			v_dir = VEC_POS_Y;

			uv.x = (p.x - rx0) / (rx1 - rx0);
			uv.y = (p.y - ry0) / (ry1 - ry0);
			return;
		}
	}

	t = -1.0;
	n_dir = VEC_NULL;
	uv.xy = vec2(-1.0, -1.0);
	u_dir = VEC_NULL;
	v_dir = VEC_NULL;
	return;
}

void VoxelIntersect(vec3 rs, vec3 rv, 
			float i, float j, float k, 
			float width, float height,
          	out float t, out vec3 n_dir, out vec2 uv, out vec3 u_dir, out vec3 v_dir) {
   // rs is ray starting point
   // rv is ray direction
   // This function computes the intersection between a ray and a voxel

	float rx0 = i*width + 0.0;
	float ry0 = j*height + 0.0;
	float rz0 = k*width + 0.0;

	float rx1 = (i + 1)*width;
	float ry1 = (j + 1)*height;
	float rz1 = (k + 1)*width;

	BoxIntersect(rs, rv, rx0, ry0, rz0, rx1, ry1, rz1,
				t, n_dir, uv, u_dir, v_dir);

	return;
}

void ConstructRay(vec2 frag_coord,
                   out vec3 rs, out vec3 rv) {
   // constructs a ray described by the equation P(t) = S + tV
   vec2 st = frag_coord;// / iResolution * 100.0;

   rs = camera_from;
   rs += (-iResolution.x / 2.0 + st.x) * camera_right;
   rs += (-iResolution.y / 2.0 + st.y) * camera_up;
   rv = camera_dir;
}

void FindStartingVoxel(vec3 rs, vec3 rv,
								out vec3 rs_onvoxel, 
                        out int i, out int j, out int k,
                        out float t) {
	// finds the voxel that the ray hits first.
	// Voxels exist in a 3D grid of chunk_w * chunk_h * chunk_l voxels.
	// Each voxel is vox_w * vox_w * vox_h volume.

	vec3 u_dir, v_dir, n_dir;
	vec2 uv;
	
	// First intersect with the entire 3D grid
	BoxIntersect(rs, rv, 
					0.0, 0.0, 0.0, 
					VOX_W * float(CHUNK_L), VOX_H * float(CHUNK_H), VOX_W * float(CHUNK_W),
					t, n_dir, uv, u_dir, v_dir);

	if (t > 0.0) {
		rs_onvoxel = rs + t * rv;
		// for numerical robustness
		
		i = clamp(int(floor(rs_onvoxel.x / VOX_W)), 0, CHUNK_L - 1);
		j = clamp(int(floor(rs_onvoxel.y / VOX_H)), 0, CHUNK_H - 1);
		k = clamp(int(floor(rs_onvoxel.z / VOX_W)), 0, CHUNK_W - 1);
		
	}
}

void RayTrace(vec3 rs, vec3 rv, 
               inout int i, inout int j, inout int k, 
               out float t, out vec3 n_dir, out uint blockID) {

	// THE RAY TRACING ALGORITHM!!!

	// Given rs and rv, and starting voxels i j k, this function will raytrace

	// find whether to increment or decrement the voxel index as we travel along the ray
	
	int di = (rv.x > 0.0) ? 1 : -1;
	int dj = (rv.y > 0.0) ? 1 : -1;
	int dk = (rv.z > 0.0) ? 1 : -1;

	// find the deltas (distance along ray to get to next voxel per dimension)
	float dx = (rv.x != 0.0) ? VOX_W / abs(rv.x) : VOX_W * CHUNK_W;
	float dy = (rv.y != 0.0) ? VOX_H / abs(rv.y) : VOX_H * CHUNK_H;
	float dz = (rv.z != 0.0) ? VOX_W / abs(rv.z) : VOX_W * CHUNK_W;

	dx = VOX_W / abs(rv.x);
	dy = VOX_H / abs(rv.y);
	dz = VOX_W / abs(rv.z);
	
	float tx, ty, tz;

	float sCellx = (di == 1) ? VOX_W * float(i+1) - rs.x : rs.x - VOX_W * float(i);
	float sCelly = (dj == 1) ? VOX_H * float(j+1) - rs.y : rs.y - VOX_H * float(j);
	float sCellz = (dk == 1) ? VOX_W * float(k+1) - rs.z : rs.z - VOX_W * float(k);

	tx = sCellx / abs(rv.x);
	ty = sCelly / abs(rv.y);
	tz = sCellz / abs(rv.z);

/*
	while (	0 <= i && i < CHUNK_W &&
	      	0 <= j && j < CHUNK_H &&
	      	0 <= k && k < CHUNK_W) {*/
	while ( 0 <= j && j < CHUNK_H) {
		// terminating condition
		blockID = SimpleWorld(i, j, k);

		if (blockID != 0) {
			t = min(tx, min(ty, tz));
			n_dir = VEC_NULL;
			return;
		}
		
		if (tx < ty) {
			if (tx < tz) {
				tx += dx;
				i += di;
			} else {
				tz += dz;
				k += dk;
			}
		}
		else {
			if (ty < tz) {
				ty += dy;
				j += dj;
			} else {
				tz += dz;
				k += dk;
			}
		}
	}

	t = min(tx, min(ty, tz));
	blockID = 0;
	n_dir = VEC_NULL;
	return;
}