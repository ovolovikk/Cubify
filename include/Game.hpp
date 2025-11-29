#ifndef GAME_HPP
#define GAME_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>

#include "Shader.hpp"
#include "Camera.hpp"
#include "World.hpp"
#include "Texture.hpp"

class Game
{
public:
    Game(int _width, int _height, const std::string& _title);
    ~Game();

    void init(const std::string& title);

    void update();
private:
    void render();
    void processInput(float deltaTime);

    GLFWwindow* window;
    int width;
    int height;

    std::unique_ptr<Shader> shader;
    std::unique_ptr<Camera> camera;
    std::unique_ptr<World> world;
    std::unique_ptr<Texture> texture_atlas;

};


#endif // GAME_HPP
