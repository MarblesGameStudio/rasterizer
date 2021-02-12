#pragma once

#include <string>
#include <vector>

#include <glm/vec2.hpp>

#include "Color.hpp"

class Texture
{
private:
	int width, height, channels;

	std::vector<char> data;
public:
	Texture(std::string filename);
	Color readColor(glm::vec2 uv);
	~Texture();
};

