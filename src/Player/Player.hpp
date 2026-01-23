#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <glm/vec3.hpp>

#include "Player/PlayerCollision.hpp"
#include "Player/PlayerPhysics.hpp"

using glm::vec3;

class Camera;
class IInputController;
class World;

class Player
{
public:
    Player(Camera& camera, IInputController& input, World& word, const vec3& initial_pos);

    Player(const Player&) = delete;
    Player& operator=(const Player&) = delete;

    void update(float dt);
    void setPosition(const vec3& pos);
    vec3 getPosition() const;

private:
    Camera& camera;
    IInputController& input;
    
    PlayerCollision collision;
    PlayerPhysics physics;
};

#endif // PLAYER_HPP
