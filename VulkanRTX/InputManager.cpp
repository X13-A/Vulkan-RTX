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
		lastMousePos.x = e.x;
		lastMousePos.y = e.y;
	}

	if ((glm::vec3(e.x, e.y, 0) - glm::vec3(lastMousePos.x, lastMousePos.y, 0)).length() > 100)
	{
		lastMousePos.x = e.x;
		lastMousePos.y = e.y;
		return;
	}
	float offsetX = lastMousePos.x - e.x;
	float offsetY = lastMousePos.y - e.y;

	lastMousePos.x = e.x;
	lastMousePos.y = e.y;

	std::cout << "Mouse movement: (" << e.x << ", " << e.y << ")\n";
}

void InputManager::onMouseScroll(const MouseScrollEvent& e)
{
	std::cout << "Mouse scroll: " << e.x << ", " << e.y << std::endl;
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