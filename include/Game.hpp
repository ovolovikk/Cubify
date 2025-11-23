#ifndef GAME_HPP
#define GAME_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <memory>

#include "Shader.hpp"
#include "Camera.hpp"

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
    int width = 1024;
    int height = 768;

    std::unique_ptr<Shader> shader;
    std::unique_ptr<Camera> camera;

    GLuint VAO, VBO;
};


#endif // GAME_HPP