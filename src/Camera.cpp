#include "Camera.h"

#include <iostream>

/**
 *  Nothing fancy here just to move the view of camera.
 *  Should include: yaw, pitch. TODO: roll
 */
void Camera::moveView(float deltaX, float deltaY)
{
	yaw_   += deltaX * sensitivityX_;
	pitch_ += deltaY * sensitivityY_;

	// Constrain pitch so there's no weird effect on the view when
	//  look all the way up or all the way down.
	if (pitch_ > 89.0f)
		pitch_ = 89.0f;
	if (pitch_ < -89.0f)
		pitch_ = -89.0f;

	// Calculate the front vector, a.k.a where the camera is looking at
	glm::vec3 frontVector;
	frontVector.x = cosf(glm::radians(pitch_)) * cosf(glm::radians(yaw_));
	frontVector.y = sinf(glm::radians(pitch_));
	frontVector.z = cosf(glm::radians(pitch_)) * sinf(glm::radians(yaw_));

	// This one updates the front vector but not in the way that I want
	front_ = normalize(frontVector);
	updateViewMatrix();
}

void Camera::movePosition(MovementSet const &movementDirection, float deltaTime)
{
	switch (movementDirection)
	{
	case MovementSet::LEFT:
		position_ -= glm::normalize(glm::cross(front_, up_)) * movementSpeed_ * deltaTime;
		break;
	case MovementSet::RIGHT:
		position_ += glm::normalize(glm::cross(front_, up_)) * movementSpeed_ * deltaTime;
		break;
	case MovementSet::FORWARD:
		position_ += front_ * movementSpeed_ * deltaTime;
		break;
	case MovementSet::BACKWARD:
		position_ -= front_ * movementSpeed_ * deltaTime;
		break;
	}

	updateViewMatrix();
}

void Camera::zoomFieldOfView(double deltaY, float viewportAspectRatio)
{
	if (fieldOfView_ >= 1.f && fieldOfView_ <= 45.f)
		fieldOfView_ -= static_cast<float>(deltaY);
	if (fieldOfView_ <= 1.f)
		fieldOfView_  = 1.f;
	if (fieldOfView_ >= 45.f)
		fieldOfView_  = 45.f;

	updateProjectionMatrix(viewportAspectRatio);
}
