#include "Texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Texture::Texture(std::string filename) {
	auto imageData = stbi_load(filename.data(), &width, &height, &channels, STBI_rgb);

	data = std::vector<char>(imageData, imageData + width * height * channels);

}

__forceinline int clamp(int value, int min, int max) {
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

Color Texture::readColor(glm::vec2 uv) {
	int x = clamp(uv.x * (width - 1), 0, width - 1);
	int y = clamp(uv.y * (height - 1), 0, height - 1);


	Color c;
	auto s = y * width * channels + x * channels;
	c.r = data[s];
	c.g = data[s + 1];
	c.b = data[s + 2];
	c.a = 255;
	return c;
}

Texture::~Texture() {

}
