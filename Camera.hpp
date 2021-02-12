#pragma once

#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi

class Camera {
private:
	glm::highp_vec3 position;
	glm::highp_vec3 target;
	glm::highp_vec3 up;
	glm::mat4 projection;
	glm::mat4 view;
	float fov;
	float aspectRatio;
	float nearClip;
	float farClip;

public:
	Camera(){}
	Camera(float _fov, float _aspectRatio, float _nearClip, float _farClip) {
		position = glm::highp_vec3(2.0f, 2.0f, 0.0f);
		target = glm::highp_vec3(0.0f, 0.0f, 0.0f);
		up = glm::highp_vec3(0.0f, 1.0f, 0.0f);
		fov = _fov;
		aspectRatio = _aspectRatio;
		nearClip = _nearClip;
		farClip = _farClip;
		projection = glm::mat4(1.0f);
		view = glm::mat4(1.0f);
	}

	void moveTo(float x, float y, float z) {
		position = glm::highp_vec3(x, y, z);
	}

	void lookAt(float x, float y, float z) {
		target = glm::highp_vec3(x, y, z);
	}

	/*
		Angle is in degrees
	*/
	void rotateAroundTarget(float angle) {
		glm::fquat q = glm::angleAxis(glm::radians(angle), glm::normalize(up));
		position = target + q * position;
	}

	glm::vec3 dir() {
		return (position - target);
	}

	glm::mat4 v() {
		view = glm::lookAtLH(position, target, up);
		return view;
	}

	glm::mat4 p() {
		projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);		
		return projection;
	}

	glm::mat4 vp() {
		projection = glm::perspective(glm::radians(fov), aspectRatio, nearClip, farClip);
		view = glm::lookAtLH(position, target, up);
		return projection * view;
	}

	float far() {
		return farClip;
	}


	float near() {
		return nearClip;
	}
};