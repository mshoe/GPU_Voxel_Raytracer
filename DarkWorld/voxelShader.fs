/*
 * Author: Mshoe
 */

#version 450 core

uniform vec2 iResolution; // viewport resolution (in pixels)
uniform vec4 iMouse; // Mouse pixel coords. xy: current, zw: click
uniform float iTime; // shader playback time (in seconds)

uniform mat4 iCamera;

uniform uvec4 iHoverVoxel;

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

#define DEBUG 0

uint SimpleWorld(int i, int j, int k);

float Map(vec3 p);



float PlaneIntersect(float s, float v, float r);

void VoxelPlaneIntersect(float s, float v, float r0, float r1,
                           out float t);

void BoxIntersect(	vec3 rs, vec3 rv,
					float rx0, float ry0, float rz0,
					float rx1, float ry1, float rz1,
					out float t, out vec3 n_dir, 
					out vec2 uv, out vec3 u_dir, out vec3 v_dir);

void VoxelIntersect(vec3 rs, vec3 rv, 
			float i, float j, float k, 
			float width, float height,
          	out float t, out vec3 n_dir, out vec2 uv, out vec3 u_dir, out vec3 v_dir);

void ConstructRay(vec2 frag_coord,
                   out vec3 rs, out vec3 rv); 

void FindStartingVoxel(vec3 rs, vec3 rv,
								out vec3 rs_onvoxel, 
                        out int i, out int j, out int k,
                        out float t);

void RayTrace(vec3 rs, vec3 rv, 
               inout int i, inout int j, inout int k, 
               out float t, out vec3 n_dir, out uint blockID);

void GetEdgeVectors( 	vec3 n_dir, vec3 u_dir, vec3 v_dir, vec3 pos, 
						out vec4 va, out vec4 vc) {

	vec3 vax = pos - v_dir;
	vec3 vay = pos + v_dir;
	vec3 vaz = pos - u_dir;
	vec3 vaw = pos + u_dir;

	va = vec4(Map(vax), Map(vay), Map(vaz), Map(vaw));

	vec3 vcx = pos + n_dir - v_dir;
	vec3 vcy = pos + n_dir + v_dir;
	vec3 vcz = pos + n_dir - u_dir;
	vec3 vcw = pos + n_dir + u_dir;

	vc = vec4(Map(vcx), Map(vcy), Map(vcz), Map(vcw));
}

void GetCornerVectors( 	vec3 n_dir, vec3 u_dir, vec3 v_dir, vec3 pos, 
							out vec4 vb, out vec4 vd) {
	vec3 vbx = pos - v_dir - u_dir;
	vec3 vby = pos + v_dir - u_dir;
	vec3 vbz = pos + v_dir + u_dir;
	vec3 vbw = pos - v_dir + u_dir;

	vb = vec4(Map(vbx), Map(vby), Map(vbz), Map(vbw));

	vec3 vdx = pos + n_dir - v_dir - u_dir;
	vec3 vdy = pos + n_dir + v_dir - u_dir;
	vec3 vdz = pos + n_dir + v_dir + u_dir;
	vec3 vdw = pos + n_dir - v_dir + u_dir;

	vd = vec4(Map(vdx), Map(vdy), Map(vdz), Map(vdw));
}

void FindSurroundingVoxels(	vec3 n_dir, vec3 u_dir, vec3 v_dir, int i, int j, int k,
								out vec4 va, out vec4 vb, out vec4 vc, out vec4 vd) {
	// first find every voxel surrounding the i,j,k voxel
	
	vec3 pos = vec3(float(i), float(j), float(k));

	GetEdgeVectors(n_dir, u_dir, v_dir, pos, va, vc);
	GetCornerVectors(n_dir, u_dir, v_dir, pos, vb, vd);
}

float MaxComp( in vec4 v ) {
	return max( max(v.x, v.y), max(v.z, v.w) );
}

float IsEdge( in vec2 uv, vec4 va, vec4 vb, vec4 vc, vec4 vd)
{
	vec2 st = 1.0 - uv;

	// sides
	vec4 wb = smoothstep( 0.85, 0.95, vec4(	st.y,
											uv.y,
											st.x,
											uv.x) ) * (1.0 - va + va * vc);

	
	// corners
	vec4 wc = smoothstep( 0.85, 0.95, vec4( st.x * st.y,
											st.x * uv.y,
											uv.x * uv.y,
											uv.x * st.y) ) * (1.0 - vb + vd * vb);
	
	return MaxComp( max(wb, wc) );
}

float SkyLight(float y_pos) {
	return y_pos / (float(CHUNK_H) * VOX_H);
}

float CalcOcc(	vec2 uv,
				vec4 va, vec4 vb, vec4 vc, vec4 vd) {
	vec2 st = 1.0 - uv;

	// edges
	vec4 wa = vec4( st.y, uv.y, st.x, uv.x) * vc;

	// corners
	vec4 wb = vec4( st.x * st.y,
					st.x * uv.y,
					uv.x * uv.y,
					uv.x * st.y) * vd * (1.0 - vc.xzyw) * (1.0 - vc.zywx);

	//wb = vec4(0.0);
	return 	wa.x + wa.y + wa.z + wa.w + 
			wb.x + wb.y + wb.z + wb.w;
}

/*
bool HoverVoxel(	vec2 mouse, vec2 fc,
					int st_i, int st_j, int st_k) {
	// create a glow around the mouse cursor

	float x = mouse.x;
	float y = mouse.y;

	// check if the mouse is near the frag coordinate
	if ((step(x - VOX_W * 2.0, fc.x) -
		step(x + VOX_W * 2.0, fc.x)) *
		(step(y - VOX_H * 2.0, fc.y) -
		step(y + VOX_H * 2.0, fc.y)) < 0.99) {
		return false;
	}

	vec3 rs, rv, rs_onvoxel;
	construct_ray(mouse.xy, rs, rv);

	float t;
	int i, j, k;
	vec3 n_dir;
	
	find_starting_voxel(rs, rv, rs_onvoxel, i, j, k, t);
	if (t > 0.0) {
		uint blockID;
		ray_trace(rs_onvoxel, rv, i, j, k, t, n_dir, blockID);

		if (st_i == i && st_j == j && st_k == k && blockID != 0) {
			
			return true;
		}
	}

	return false;
}*/

void main()
{

	vec3 debugColor = vec3(0.0, 0.0, 0.0);
	vec3 color = vec3(0.0, 0.0, 0.0);
	vec3 hover_color = vec3(0.0);

	vec2 st = gl_FragCoord.xy / iResolution;
	float time = iTime;
	//float mouse = mousePos(iMouse.xy / iResolution, st);


	vec3 rs, rv; // the vectors describing the ray P(t) = S + tV

	// first construct the ray (rs, rv)
	ConstructRay(gl_FragCoord.xy, rs, rv);

	// then find the voxel in the voxel grid the ray starts at
	float t;
	int i, j, k;
	vec3 rs_onvoxel;
	FindStartingVoxel(rs, rv, rs_onvoxel, i, j, k, t);
	
#if DEBUG
	if (t > 0.0) {
		debugColor.x = 1;

	}
#endif

	uint blockID = 0;
	vec3 n_dir; // direction of normal vector
	if (t > 0.0) {
		RayTrace(rs_onvoxel, rv, i, j, k, t, n_dir, blockID);
	}

	if (blockID != 0) {
		t = 0.0;
		vec3 u_dir, v_dir; // direction of u and v vectors (perpendicular to normal)
		vec2 uv;
		VoxelIntersect(rs, rv, i, j, k, VOX_W, VOX_H, t, n_dir, uv, u_dir, v_dir);
		vec4 va, vb, vc, vd;
		FindSurroundingVoxels(n_dir, u_dir, v_dir, i, j, k, va, vb, vc, vd);

		color.x = 1.0;
		vec4 v0 = vec4(0.0);
		//color.y = isEdge(uv, v0, v0, v0, v0);
		float edge = IsEdge(uv, va, vb, vc, vd);

		color.y = edge;
		
		
		float light = SkyLight((rs + t*rv).y);
		light *= 1.0 - 0.5 * CalcOcc(uv, va, vb, vc, vd);
		color *= light;

		// DEBUG CODE
		//color = vec3(1.0, 0.0, 0.0);

		if (iHoverVoxel.xyz == uvec3(i, j, k)) {
			hover_color.y = 1.0;
		}

		// for getting the voxel the mouse is hovering over
		/*
		if (hover_voxel(iMouse.xy, gl_FragCoord.xy,
				i, j, k)) {
			hover_color.y = 1.0;
		}*/
	}



	
	
	gl_FragColor = vec4(color + hover_color, 1.0);
#if DEBUG
	gl_FragColor = vec4(debugColor, 1.0);
	//gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
#endif
	//gl_FragColor = vec4(camera_up, 1.0);

}