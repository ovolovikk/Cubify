#ifndef GLFW_INPUT_CONTROLLER_HPP
#define GLFW_INPUT_CONTROLLER_HPP

#include "Core/Input/IInputController.hpp"

struct GLFWwindow;

// Using GLFW to manage user input
class GLFWInputController : public IInputController {
public:
  GLFWInputController(GLFWwindow *window);

  bool isMovingForward() const override;
  bool isMovingBackward() const override;
  bool isMovingLeft() const override;
  bool isMovingRight() const override;
  bool isMovingUp() const override;
  bool isMovingDown() const override;
  bool isSprinting() const override;

  glm::vec2 getMouseDelta() const override;
  float getScrollDelta() override;
  bool isMouseButtonPressed(int button) const override;
  bool wasMouseButtonJustPressed(int button) const override;
  bool wasMouseButtonJustReleased(int button) const override;

  bool isKeyPressed(int key) const override;
  bool wasKeyJustPressed(int key) const override;
  bool wasKeyJustReleased(int key) const override;

  void update() override;

  void setCursorEnabled(bool enabled) override;
  bool isCursorEnabled() const override;

  void accumulateScroll(float y);

private:
  GLFWwindow *window = nullptr;
  double last_x = 0, last_y = 0;
  double delta_x = 0, delta_y = 0;
  float scroll_delta = 0;
  bool first_mouse = true;
  bool cursor_enabled = false;

  std::unordered_map<int, bool> m_prevKeyState;
  std::unordered_map<int, bool> m_currKeyState;
  std::unordered_map<int, bool> m_prevMouseState;
  std::unordered_map<int, bool> m_currMouseState;
};

#endif // GLFW_INPUT_CONTROLLER
