#ifndef DEPTH_BUFFER_HPP
#define DEPTH_BUFFER_HPP

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

	 void clear(float value) {
		for (auto& v : data) {
			v = value;
		}
	}

	 void set(int x, int y, float value) {
		auto index = y * width + x;
		data[index] = value;
	}

	 float get(int x, int y) {
		auto index = y * width + x;
		return data[index];
	}
	
};

#endif