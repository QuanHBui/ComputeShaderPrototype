/**
 * Camera class
 * My intention is to make a fly camera, but who knows how
 * complicated this gonna get.
 *
 * @author: Quan Bui
 * @version: 02/25/2020
 */

#pragma once

#ifndef LAB471_CAMERA_H
#define LAB471_CAMERA_H

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

class Camera
{
public:
	enum class MovementSet { LEFT, RIGHT, FORWARD, BACKWARD };

	Camera() {}
	Camera(glm::vec3 const &position, glm::vec3 const &front, glm::vec3 const &up, float viewportAspectRatio)
		: position_(position), front_(glm::normalize(front)), up_(glm::normalize(up))
	{
		updateViewMatrix();
		updateProjectionMatrix(viewportAspectRatio);
	}

	~Camera() {}

	glm::vec3 getPosition() const { return position_; }
	glm::vec3 getFront() const { return front_; }
	glm::vec3 getUp() const { return up_; }
	glm::mat4 getViewMatrix() const { return viewMatrix_; }
	glm::mat4 getProjectionMatrix() const { return projectionMatrix_; }
	float getSensitivityX() const { return sensitivityX_; }
	float getSensitivityY() const { return sensitivityY_; }
	float getPitch() const { return pitch_; }
	float getYaw() const { return yaw_; }

	void setPosition(glm::vec3 const &cameraPosition) { position_ = cameraPosition; }
	void setFront(glm::vec3 const &cameraFront) { front_ = cameraFront; }
	void setUp(glm::vec3 const &cameraUp) { up_ = cameraUp; }
	void setSensitivityX(float sensitivityX) { sensitivityX_ = sensitivityX; }
	void setSensitivityY(float sensitivityY) { sensitivityY_ = sensitivityY; }
	void setMovementSpeed(float movementSpeed) { movementSpeed_ = movementSpeed; }
	void lookAtPoint(glm::vec3 const &point) { viewMatrix_ = glm::lookAt(position_, point, up_); }

	void moveView(float, float);
	void zoomFieldOfView(double, float);
	void movePosition(MovementSet const &, float deltaTime = 1.f);

	void updateViewMatrix()
	{
		viewMatrix_ = glm::lookAt(position_, position_ + front_, up_);
	}

	void updateProjectionMatrix(float viewportAspectRatio)
	{
		// perspective(float fov, float aspect, float zNear, float zFar) -- parameter list of perspective
		projectionMatrix_ = glm::perspective(glm::radians(fieldOfView_), viewportAspectRatio, 0.01f, 150.f);
	}

private:
	float yaw_{ -90.f }, pitch_{ 0.f };		// In degrees, yaw is default at -90 degrees to make sure that camera looking at -x by default
	float sensitivityX_{ 0.05f }, sensitivityY_{ 0.05f };
	float movementSpeed_{ 2.5f };
	float fieldOfView_{ 45.f };
	glm::vec3 position_{ 0.f };
	glm::vec3 front_{ 0.f };
	glm::vec3 up_{ 0.f };

	// View and projection matrices that represents the camera. Can be sent straight to vertex shader
	glm::mat4 viewMatrix_{ 0.f };
	glm::mat4 projectionMatrix_{ 0.f };
};

#endif  // LAB471_CAMERA_H
