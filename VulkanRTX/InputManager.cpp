#include "InputManager.hpp"
#include <iostream>
#include "GLM_defines.hpp"
#include "EventManager.hpp"

InputManager::InputManager()
{
	lastMousePos = glm::vec2(0, 0);
}

void InputManager::init()
{
	keysPressed.resize(static_cast<size_t>(KeyboardKey::KeysCount), false);
	EventManager::get().sink<MouseMoveEvent>().connect<&InputManager::onMouseMove>(this);
	EventManager::get().sink<MouseScrollEvent>().connect<&InputManager::onMouseScroll>(this);
}

void InputManager::retrieveInputs(GLFWwindow* window)
{
	for (const std::pair<int, KeyboardKey> pair : keyMap)
	{
		if (pair.second == KeyboardKey::KeysCount) break;
		bool keyPressed = glfwGetKey(window, pair.first) == GLFW_PRESS;
		keysPressed.at(static_cast<size_t>(pair.second)) = keyPressed;
	}
}

void InputManager::onMouseMove(const MouseMoveEvent& e) 
{
	if (firstMouseInput)
	{
		firstMouseInput = false;
		lastMousePos.x = e.xPos;
		lastMousePos.y = e.yPos;
	}

	if ((glm::vec3(e.xPos, e.yPos, 0) - glm::vec3(lastMousePos.x, lastMousePos.y, 0)).length() > 100)
	{
		lastMousePos.x = e.xPos;
		lastMousePos.y = e.yPos;
		return;
	}
	float offsetX = lastMousePos.x - e.xPos;
	float offsetY = lastMousePos.y - e.yPos;

	EventManager::get().trigger(MouseOffsetEvent{ offsetX, offsetY });
	
	lastMousePos.x = e.xPos;
	lastMousePos.y = e.yPos;
}

void InputManager::onMouseScroll(const MouseScrollEvent& e)
{
	std::cout << "Mouse scroll: " << e.xOffset << ", " << e.yOffset << std::endl;
}

bool InputManager::isKeyPressed(KeyboardKey key) const
{
	return keysPressed.at(static_cast<size_t>(key));
}

void InputManager::update(GLFWwindow* window)
{
	retrieveInputs(window);
}

void InputManager::cleanup()
{
	EventManager::get().sink<MouseMoveEvent>().disconnect<&InputManager::onMouseMove>(this);
	EventManager::get().sink<MouseScrollEvent>().disconnect<&InputManager::onMouseScroll>(this);
}