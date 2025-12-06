#ifndef I_INPUT_CONTROLLER_HPP
#define I_INPUT_CONTROLLER_HPP

#include <glm/glm.hpp>

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

    // system
    virtual bool isKeyPressed(int key) const = 0;
    virtual void update() = 0;
};

#endif // I_INPUT_CONTROLLER_HPP
