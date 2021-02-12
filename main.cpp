// MyRasterizer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <string>

#include <sdl/SDL.h>
#include <glm/vec2.hpp>

#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi

#include <glm/gtx/quaternion.hpp>


#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


#define PERSPECTIVE_CORRECT
//#define DEBUG_DEPTH

#include "Camera.hpp"
#include "Texture.h"
#include "Color.hpp"
#include "DepthBuffer.h"

using namespace std;

SDL_Window* window;
SDL_Surface* framebuffer;
Texture* woodTexture;
Texture* roomTexture;
DepthBuffer* depthBuffer;




void begin() {
	if (SDL_MUSTLOCK(framebuffer))
		SDL_LockSurface(framebuffer);
}

void end() {
	if (SDL_MUSTLOCK(framebuffer))
		SDL_UnlockSurface(framebuffer);
}

__forceinline void putPixel(int x, int y, Color color) {
	if (x >= 0 && x < framebuffer->w && y >= 0 && y < framebuffer->h) {
		((char*)framebuffer->pixels)[y * framebuffer->pitch + x * 4 + 0] = color.b;
		((char*)framebuffer->pixels)[y * framebuffer->pitch + x * 4 + 1] = color.g;
		((char*)framebuffer->pixels)[y * framebuffer->pitch + x * 4 + 2] = color.r;
		((char*)framebuffer->pixels)[y * framebuffer->pitch + x * 4 + 3] = color.a;
	}
}

void clear(Color clearColor) {

	for (int i = 0; i < framebuffer->h; i++) {
		for (int j = 0; j < framebuffer->w; j++) {
			putPixel(j, i, clearColor);
		}
	}
}

void present() {
	SDL_UpdateWindowSurface(window);
}


void bresenham(int x0, int y0, int x1, int y1, Color color) {
	int dx = abs(x1 - x0);
	int sx = x0 < x1 ? 1 : -1;
	int dy = -abs(y1 - y0);
	int sy = y0 < y1 ? 1 : -1;
	int err = dx + dy;  /* error value e_xy */

	while (true) {   /* loop */
		putPixel(x0, y0, color);
		if (x0 == x1 && y0 == y1)
			break;
		int e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y0 += sy;

		}
	}
}

struct Vertex {
public:
	glm::highp_vec3 position;
	glm::highp_vec2 uv;
	glm::highp_vec3 normal;
};

struct Triangle {
	Vertex vertices[3];
};

struct Mesh {
	vector<Vertex> vertices;
};

void loadScene(std::string inputfile, std::vector<Mesh>& meshes) {

	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "./"; // Path to material files

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(inputfile, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}
		exit(1);
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++) {
		Mesh mesh;

		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			int fv = shapes[s].mesh.num_face_vertices[f];

			// Loop over vertices in the face.
			Triangle tri;

			for (size_t v = 0; v < fv; v++) {


				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
				tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
				tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
				tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
				tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
				tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
				tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
				tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];
				// Optional: vertex colors
				// tinyobj::real_t red = attrib.colors[3*idx.vertex_index+0];
				// tinyobj::real_t green = attrib.colors[3*idx.vertex_index+1];
				// tinyobj::real_t blue = attrib.colors[3*idx.vertex_index+2];

				Vertex xxx;

				xxx.position = glm::highp_vec3(vx, vy, vz);
				xxx.normal = glm::highp_vec3(nx, ny, nz);
				xxx.uv = glm::highp_vec2(tx, ty);

				mesh.vertices.push_back(xxx);
			}


			index_offset += fv;

			// per-face material
			shapes[s].mesh.material_ids[f];

		}

		meshes.push_back(mesh);
	}
}


enum struct PolygonMode {
	Fill,
	Line
};

enum struct CullingMode {
	BackFace,
	Disabled
};


PolygonMode polygonMode = PolygonMode::Fill;
CullingMode cullingMode = CullingMode::Disabled;



__forceinline int edgeFunction(glm::ivec2 v1, glm::ivec2 v2, glm::ivec2 v3) {
	return (v3.x - v1.x) * (v2.y - v1.y) - (v3.y - v1.y) * (v2.x - v1.x);
}

/*
Use baricentric coordinates to draw a triangle and fill in
*/
void bariTriangle(
	const glm::vec4& v1, const glm::vec4& v2, const glm::vec4& v3,
	const glm::vec2& uv1, const glm::vec2& uv2, const glm::vec2& uv3,
	float q1, float q2, float q3,
	float z1, float z2, float z3,
	std::function<Color(glm::fvec2, float depth)> fragmentShader) {


	if (polygonMode == PolygonMode::Fill) {


		auto iv1 = glm::ivec2(v1.x * framebuffer->w, v1.y * framebuffer->h);
		auto iv2 = glm::ivec2(v2.x * framebuffer->w, v2.y * framebuffer->h);
		auto iv3 = glm::ivec2(v3.x * framebuffer->w, v3.y * framebuffer->h);

		int area = edgeFunction(iv1, iv2, iv3);
		float areaInverse = 1.0f / area;

		int minX = min(min(iv1.x, iv2.x), iv3.x);
		int minY = min(min(iv1.y, iv2.y), iv3.y);
		int maxX = max(max(iv1.x, iv2.x), iv3.x);
		int maxY = max(max(iv1.y, iv2.y), iv3.y);

		for (int x = minX; x < maxX; x++) {
			for (int y = minY; y < maxY; y++) {

				glm::ivec2 p = glm::ivec2(x, y);


				auto c1 = edgeFunction(iv2, iv3, p);
				auto c2 = edgeFunction(iv3, iv1, p);
				auto c3 = edgeFunction(iv1, iv2, p);

				//Now texture it with uv we found

				//The value of lambda multiplies in baricenteric coordinates

				auto t1 = c1 * areaInverse;
				auto t2 = c2 * areaInverse;

				//we can calculate t3 by this formula t3 = 1- t1 - t2
				//so instead of t3 = c3 / area , I wrote this line of code
				//cause we know t1+t2+t3 = 1 according to baricentric coordinates formula
				//so we can reduce a division which improves performance
				auto t3 = 1.f - t1 - t2;

				if (t1 >= 0 && t2 >= 0 && t3 >= 0) {
					
#ifndef PERSPECTIVE_CORRECT
					//Interpolate uv values to find the pixel uv using baricenteric coordinates				
					//affine
					float u = uv1.x * t1 + uv2.x * t2 + uv3.x * t3;
					float v = uv1.y * t1 + uv2.y * t2 + uv3.y * t3;

					//Interpolates depth value
					float depth = z1 * t1 + z2 * t2 + z3 * t3;
#else

					//perspective correctenss				

					auto ap = q1 * t1;
					auto bp = q2 * t2;
					auto cp = q3 * t3;

					auto inv = 1.f / (ap + bp + cp);

					//Interpolate uv values to find the pixel uv using baricenteric coordinates
					float u = (ap * uv1.x + bp * uv2.x + cp * uv3.x) * inv;
					float v = (ap * uv1.y + bp * uv2.y + cp * uv3.y) * inv;

					//Interpolates depth value
					float depth = (ap * z1 + bp * z2 + cp * z3) * inv;
#endif			



#ifndef DEBUG_DEPTH
					if (depthBuffer->get(x, y) > depth) {
						depthBuffer->set(x, y, depth);
						putPixel(x, y, fragmentShader(glm::fvec2(u, 1 - v), depth));
					}

#else

					int colorized_depth = (depth * 255);

					Color c;
					c.a = 255;
					c.r = c.g = c.b = colorized_depth;
					if (depthBuffer->get(x, y) > depth) {
						depthBuffer->set(x, y, depth);
						putPixel(x, y, c);
					}

#endif						

				}

			}
		}
	}
	else {
		Color c = { 255,255,255,255 };
		bresenham(v1.x * framebuffer->w, v1.y * framebuffer->h, v2.x * framebuffer->w, v2.y * framebuffer->h, c);
		bresenham(v2.x * framebuffer->w, v2.y * framebuffer->h, v3.x * framebuffer->w, v3.y * framebuffer->h, c);
		bresenham(v3.x * framebuffer->w, v3.y * framebuffer->h, v1.x * framebuffer->w, v1.y * framebuffer->h, c);
	}
}

void rasterize(std::vector<Vertex> vertices, Camera& camera, std::function<Color(glm::fvec2, float depth)> fragmentShader) {

	//Checks Back-Face Culling
	for (auto it = vertices.begin(); it != vertices.end();) {
		auto& r1 = *it++;
		auto& r2 = *it++;
		auto& r3 = *it++;

		auto& p1 = r1.position;
		auto& p2 = r2.position;
		auto& p3 = r3.position;

		//Backface culling		
		if (cullingMode == CullingMode::BackFace) {
			auto c = glm::cross((p2 - p1), (p3 - p1));
			if (glm::dot(camera.dir(), c) < 0) {
				continue;
			}
		}
		


		auto v1 = camera.vp() * glm::highp_vec4(p1, 1.0f);
		auto v2 = camera.vp() * glm::highp_vec4(p2, 1.0f);
		auto v3 = camera.vp() * glm::highp_vec4(p3, 1.0f);

		float w1Inv = 1.f / v1.w;
		float w2Inv = 1.f / v2.w;
		float w3Inv = 1.f / v3.w;


		// Used for perspective correct interpolation
		float q1 = w1Inv;
		float q2 = w2Inv;
		float q3 = w3Inv;

		v1 *= w1Inv;
		v2 *= w2Inv;
		v3 *= w3Inv;

		//Transform vertex coordinate to clip space
		v1 = (v1 + 1.0f) * 0.5f;
		v2 = (v2 + 1.0f) * 0.5f;
		v3 = (v3 + 1.0f) * 0.5f;

		

		float z1 = ((camera.v() * glm::vec4(p1, 1.f)).z) * camera.cameraLengthInverse();
		float z2 = ((camera.v() * glm::vec4(p2, 1.f)).z) * camera.cameraLengthInverse();
		float z3 = ((camera.v() * glm::vec4(p3, 1.f)).z) * camera.cameraLengthInverse();

		//std::cout << z1 << std::endl;

		//Draw a triangle using baricentric coordinate
		bariTriangle(
			v1, v2, v3,
			r1.uv, r2.uv, r3.uv,
			q1, q2, q3,
			z1, z2, z3,
			fragmentShader);

	}


}


int main(int argc, char* argv[])
{

	bool paused = false;

	auto ticks = SDL_GetTicks();


	Color clearColor = { 0,0,0,255 };
	Color pixelColor = { 255,255,255,255 };

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {

	}

	window = SDL_CreateWindow("Rasterizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, 0);
	SDL_Event ev;


	framebuffer = SDL_GetWindowSurface(window);

	depthBuffer = new DepthBuffer(framebuffer->w, framebuffer->h, 1.f);

	Camera camera(39.6f, framebuffer->w / (float)framebuffer->h, 0.1f, 10.0f);
	camera.moveTo(4.f, 4.f, -4.f);


	std::vector<Mesh> meshes;
	loadScene("viking-room.obj", meshes);
	std::vector<char> textureData;
	//woodTexture = new Texture(std::string("wood.jpg"));
	roomTexture = new Texture(std::string("texture.png"));

	do {
		if (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				break;
			if (ev.type == SDL_KEYDOWN) {

				if (ev.key.keysym.sym == SDLK_w) {
					if (polygonMode == PolygonMode::Fill)
						polygonMode = PolygonMode::Line;
					else
						polygonMode = PolygonMode::Fill;
				}

				if (ev.key.keysym.sym == SDLK_SPACE) {
					paused = !paused;
				}

			}


		}
		else {



			float deltaTime = (SDL_GetTicks() - ticks) * 0.001;

			if (!paused) {
				camera.rotateAroundTarget(deltaTime * 60);

			}
			ticks = SDL_GetTicks();

			auto renderPerformance = SDL_GetTicks();

			begin();
			clear(clearColor);
			depthBuffer->clear(10000.f);

			auto vp = camera.vp();

			for (const auto& mesh : meshes) {
				rasterize(mesh.vertices, camera, [](glm::fvec2 uv, float depth) {
					return roomTexture->readColor(uv);
					});
			}

			end();

			renderPerformance = SDL_GetTicks() - renderPerformance;
			std::cout << "render perf:" << renderPerformance << std::endl;

			auto persentPerformance = SDL_GetTicks();			
			present();
			persentPerformance = SDL_GetTicks() - persentPerformance;
			std::cout << "present perf:" << persentPerformance << std::endl;
		}
	} while (true);

	return 0;
}
