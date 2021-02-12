#pragma once

#include <vector>

class DepthBuffer {
private:
	int width, height;

	std::vector<float> data;

	
public:
	DepthBuffer(int _width, int _height,float initialValue)
	{
		width = _width;
		height = _height;
		auto length = width * height;
		data = std::vector<float>(length);
		data.resize(length);
		for (auto& v : data) {
			v = initialValue;
		}
	}

	__forceinline void clear(float value) {
		for (auto& v : data) {
			v = value;
		}
	}

	__forceinline void set(int x, int y, float value) {
		auto index = y * width + x;
		data[index] = value;
	}

	__forceinline float get(int x, int y) {
		auto index = y * width + x;
		return data[index];
	}
	
};