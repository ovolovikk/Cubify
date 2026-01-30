#ifndef I_INPUT_CONTROLLER_HPP
#define I_INPUT_CONTROLLER_HPP

#include <glm/glm.hpp>

// Basic Interface for managing user input
class IInputController
{
public:
    virtual ~IInputController() = default;

    // movement
    virtual bool isMovingForward() const = 0;
    virtual bool isMovingBackward() const = 0;
    virtual bool isMovingLeft() const = 0;
    virtual bool isMovingRight() const = 0;
    virtual bool isMovingUp() const = 0;
    virtual bool isMovingDown() const = 0;
    virtual bool isSprinting() const = 0;

    // mouse
    virtual glm::vec2 getMouseDelta() const = 0;
    virtual bool isMouseButtonPressed(int button) const = 0;
    virtual bool wasMouseButtonJustPressed(int button) const = 0;
    virtual bool wasMouseButtonJustReleased(int button) const = 0;

    // cursor control
    virtual void setCursorEnabled(bool enabled) = 0;
    virtual bool isCursorEnabled() const = 0;
    
    // keyboard
    virtual bool isKeyPressed(int key) const = 0;
    virtual bool wasKeyJustPressed(int key) const = 0;
    virtual bool wasKeyJustReleased(int key) const = 0;
     
    virtual void update() = 0;
};

#endif // I_INPUT_CONTROLLER_HPP
