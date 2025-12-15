#include "Player/Player.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Core/Camera.hpp"
#include "Core/Input/IInputController.hpp"
#include "World/World.hpp"

Player::Player(Camera& camera_, IInputController& input_, World& world_, const vec3& initial_pos)
    : camera(camera_), input(input_), collision(world_), physics(collision)
{
    setPosition(initial_pos);
}

void Player::setPosition(const vec3& pos)
{
    physics.setPosition(pos);
    camera.SetPosition(pos + glm::vec3(0, 1.6f, 0));
}

vec3 Player::getPosition() const
{
    return physics.getPosition();
}

void Player::update(float dt)
{
    vec3 move_dir {0.f};

    vec3 forward = camera.GetFront();
    forward.y = 0;
    if (glm::length(forward) > 0.01f) forward = glm::normalize(forward);

    vec3 right = camera.GetRight();
    right.y = 0;
    if (glm::length(right) > 0.01f) right = glm::normalize(right);

    if (input.isKeyPressed(GLFW_KEY_W)) move_dir += forward;
    if (input.isKeyPressed(GLFW_KEY_S)) move_dir -= forward;
    if (input.isKeyPressed(GLFW_KEY_A)) move_dir -= right;
    if (input.isKeyPressed(GLFW_KEY_D)) move_dir += right;

    if (glm::length(move_dir) > 0.01f) {
        move_dir = glm::normalize(move_dir);
        
        float current_speed = MOVE_SPEED;
        if (input.isSprinting()) {
            current_speed *= SPRINT_MLTPL;
        }

        if (!physics.isGrounded()) {
            current_speed *= FREE_FALL_SLOWDONW;
        }
        
        physics.move(move_dir, current_speed * dt);
    }

    if (input.isKeyPressed(GLFW_KEY_SPACE))
    {
        physics.jump(JUMP_FORCE);
    }

    glm::vec2 mouse_delta = input.getMouseDelta();
    camera.processMouseMovement(mouse_delta.x, mouse_delta.y);

    physics.update(dt);

    camera.SetPosition(physics.getPosition() + vec3(0.f, 1.6f, 0.f));
}
