#pragma once

#include "../pch.h"


class Camera
{
public:
	Camera();
	~Camera();

	void updateCamera();


	glm::vec4 mCameraTransformMatrix;
};