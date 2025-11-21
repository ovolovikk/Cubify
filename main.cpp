#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Shader.hpp"
#include "Camera.hpp"

int main()
{
    std::cout << "Cubify!" << std::endl;

    glewExperimental = true;
    if(!glfwInit())
    {
        std::cout << "glfw init error.\n";
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    window = glfwCreateWindow(1024, 768, "Cubify", NULL, NULL);
    if(window == nullptr)
    {
        std::cout << "window init error.\n";
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "glew init error\n";
        return -1;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double lastX = 512.f, lastY = 384.f;
    bool firstMouse = true;
    
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    Shader* myShader = nullptr;
    try {
        myShader = new Shader("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        std::cerr << "Press Enter to exit..." << std::endl;
        std::cin.get();
        return -1;
    }

    Camera camera(vec3(0.0f, 0.0f, 3.0f));
    glm::mat4 model = glm::mat4(1.f);

    float vertices[] = {
        -0.5f, -0.5f, 0.0f, // left  
         0.5f, -0.5f, 0.0f, // right 
         0.0f,  0.5f, 0.0f  // top   
    }; 

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    do {
        float cameraSpeed = 0.05f;
        if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.MoveForward(cameraSpeed);
        if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.MoveLeft(cameraSpeed);
        if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.MoveBackward(cameraSpeed);
        if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.MoveRight(cameraSpeed);
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.MoveUp(cameraSpeed);
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            camera.MoveDown(cameraSpeed);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = camera.GetProjectionMatrix();
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 MVP = projection * view * model;
        if (myShader) {
        myShader->use();
        myShader->setMat4("MVP", MVP);
        }
        
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    } while(glfwWindowShouldClose(window) == 0);

    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteBuffers(1, &vertexbuffer);

    delete myShader;

    glfwTerminate();
    return 0;
}
