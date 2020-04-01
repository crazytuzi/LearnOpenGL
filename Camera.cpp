#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch): Front(glm::vec3(0.f, 0.f, -1.f)),
                                                                          MovementSpeed(SPEED),
                                                                          MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
	Position = position;

	WorldUp = up;

	Yaw = yaw;

	Pitch = pitch;

	updateCameraVectors();
}

Camera::Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch):
	Front(glm::vec3(0.f, 0.f, -1.f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
{
	Position = glm::vec3(posX, posY, posZ);

	WorldUp = glm::vec3(upX, upY, upZ);

	Yaw = yaw;

	Pitch = pitch;

	updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
	return lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(const Camera_Movement direction, const float deltaTime)
{
	const auto velocity = MovementSpeed * deltaTime;

	if (direction == Camera_Movement::FORWARD)
	{
		Position += Front * velocity;
	}

	if (direction == Camera_Movement::BACKWARD)
	{
		Position -= Front * velocity;
	}

	if (direction == Camera_Movement::LEFT)
	{
		Position -= Right * velocity;
	}

	if (direction == Camera_Movement::RIGHT)
	{
		Position += Right * velocity;
	}
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, const GLboolean constrainPitch)
{
	xoffset *= MouseSensitivity;

	yoffset *= MouseSensitivity;

	Yaw += xoffset;

	Pitch += yoffset;

	/* Make sure that when pitch is out of bounds, screen doesn't get flipped */
	if (constrainPitch)
	{
		if (Pitch > 89.f)
		{
			Pitch = 89.f;
		}

		if (Pitch < -89.f)
		{
			Pitch = -89.f;
		}
	}

	/* Update Front, Right and Up Vectors using the updated Euler angles */
	updateCameraVectors();
}

void Camera::ProcessMouseScroll(const float yoffset)
{
	if (Zoom > 1.f && Zoom <= 45.f)
	{
		Zoom -= yoffset;
	}

	if (Zoom <= 1.f)
	{
		Zoom = 1.f;
	}

	if (Zoom >= 45.f)
	{
		Zoom = 45.f;
	}
}

void Camera::updateCameraVectors()
{
	/* Calculate the new Front vector */
	glm::vec3 front;

	front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));

	front.y = sin(glm::radians(Pitch));

	front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

	Front = normalize(front);

	/* Also re-calculate the Right and Up vector */
	/* Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement. */
	Right = normalize(cross(Front, WorldUp));

	Up = normalize(cross(Right, Front));
}
