#pragma once
#include "Camera.hpp"
#include "InputManager.hpp"
#include "AllEvents.hpp"

class CameraControls
{
protected:
	Camera& camera;

public:
	CameraControls(Camera& camera) : camera(camera)
	{

	}

	virtual void handleMouseOffset(const MouseOffsetEvent& e)
	{

	}

	virtual void handleMouseScroll(const MouseScrollEvent& e)
	{

	}

	virtual void update(InputManager& inputManager)
	{

	}
};
