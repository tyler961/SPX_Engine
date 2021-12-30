#include "Camera.h"
#include "../Events/Event.h"
#include <GLFW/glfw3.h>



Camera::Camera() {
	mCameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
	mUp = glm::vec3(0.0f, -1.0f, 0.0f);
	mCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);

	mMoveRight = false;
	mMoveLeft = false;
	mMoveUp = false;
	mMoveDown = false;
	mCameraSpeed = 0.02f;
}

Camera::~Camera() {}

void Camera::handleEvents(std::vector<Event*> events) {
	for (size_t i = 0; i < events.size(); i++) {
		if (events.at(i)->getType() == EventType::KeyPressed) {
			if (events.at(i)->getKeycode() == GLFW_KEY_D) {
				mMoveRight = true;
				events.at(i)->setIsHandled(true);
			}
			if (events.at(i)->getKeycode() == GLFW_KEY_A) {
				mMoveLeft = true;
				events.at(i)->setIsHandled(true);
			}
			if (events.at(i)->getKeycode() == GLFW_KEY_W) {
				mMoveUp = true;
				events.at(i)->setIsHandled(true);
			}
			if (events.at(i)->getKeycode() == GLFW_KEY_S) {
				mMoveDown = true;
				events.at(i)->setIsHandled(true);
			}
		}

		if (events.at(i)->getType() == EventType::KeyReleased) {
			if (events.at(i)->getKeycode() == GLFW_KEY_D) {
				mMoveRight = false;
				events.at(i)->setIsHandled(true);
			}
			if (events.at(i)->getKeycode() == GLFW_KEY_A) {
				mMoveLeft = false;
				events.at(i)->setIsHandled(true);
			}
			if (events.at(i)->getKeycode() == GLFW_KEY_W) {
				mMoveUp = false;
				events.at(i)->setIsHandled(true);
			}
			if (events.at(i)->getKeycode() == GLFW_KEY_S) {
				mMoveDown = false;
				events.at(i)->setIsHandled(true);
			}
		}
	}
}

void Camera::updateCamera()
{
	if (mMoveRight) 
		mCameraPos += glm::vec3(-1.0f, 0.0f, 0.0f) * mCameraSpeed;

	if (mMoveLeft)
		mCameraPos += glm::vec3(1.0f, 0.0f, 0.0f) * mCameraSpeed;

	if (mMoveUp)
		mCameraPos += glm::vec3(0.0f, -1.0f, 0.0f) * mCameraSpeed;

	if (mMoveDown)
		mCameraPos += glm::vec3(0.0f, 1.0f, 0.0f) * mCameraSpeed;

	//mView = glm::lookAt(mCameraPos, mCameraPos + mCameraFront, mUp); (Creates an orthocamera. Only changes x y planes)
	mView = glm::lookAt(mCameraPos, glm::vec3(0.0f, 0.0f, 0.0f), mUp);
}

glm::mat4 Camera::getViewMatrix() {
	return mView;
}