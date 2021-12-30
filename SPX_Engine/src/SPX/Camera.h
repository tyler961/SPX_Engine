#pragma once

#include "../pch.h"

class Event;

class Camera
{
public:
	Camera();
	~Camera();

	void updateCamera();
	void handleEvents(std::vector<Event*> events);
	glm::mat4 getViewMatrix();


	glm::vec4 mCameraTransformMatrix;
	
	// This represents the camera's position in world space.
	glm::vec3 mCameraPos;
	// This is the camera's target, it will start at the origin of the scene (0.0f, 0.0f, 0.0f);
	glm::vec3 mCameraTarget;
	// Pointing at the reverse direction of what the camera is targeting. It is the cameraPos - cameraTarget
	glm::vec3 mCameraDirection;
	// Right axis that represents the positive x-axis of the camera space.
	// To get this, first I specify the up vector that points upwards (in world space) then do a cross product on up and direction
	// vector. This produces a vector perpendicular to both vectors. This vector points in the positive x axis direction.
	// If I switch the order of the cross product, I would get the negative x-axis.
	glm::vec3 mUp;
	glm::vec3 mCameraRight;
	// Now that I have x and the z axis, retreiving the vector that points to the camera's positive y axis is the cross product
	// of the right and direction vector.
	glm::vec3 mCameraUp;

	glm::vec3 mCameraFront;

	// THE PROCESS ABOVE IS CALLED THE Gram-Schmidt process. I can now create a LookAt matrix.
	glm::mat4 mView;

private:
	bool mMoveRight;
	bool mMoveLeft;
	bool mMoveUp;
	bool mMoveDown;
	float mCameraSpeed;
};