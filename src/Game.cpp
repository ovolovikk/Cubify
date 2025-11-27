#include "Game.hpp"
#include <iostream>

Game::Game(int _width, int _height, const std::string& _title)
    : width(_width), height(_height)
{
    init(_title);
    if (window)
        update();
}

Game::~Game()
{
    glfwTerminate();
}

void Game::init(const std::string& title)
{   
    // main initialization
    glewExperimental = true;
    if(!glfwInit())
    {
        std::cout << "glfw init error.\n";
        return;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for MacOS
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Cubify", NULL, NULL);
    if(window == nullptr)
    {
        std::cout << "window init error.\n";
        return;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "glew init error\n";
        return;
    }

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE); // MSAA if avaivable

    // subsystem initialization
    shader = std::make_unique<Shader>();
    if (!shader->load("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl")) {
        std::cerr << "Failed to load shaders" << std::endl;
        return;
    }

    camera = std::make_unique<Camera>(glm::vec3(0.0f, 0.0f, 3.0f));

    texture = std::make_unique<Texture>("textures/grass_atlas.png");

    GLuint sampler;
    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (glewIsSupported("GL_EXT_texture_filter_anisotropic")) {
        GLfloat maxAniso = 0.0f;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        GLfloat targetAniso = (maxAniso >= 4.0f) ? 4.0f : maxAniso;
        glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, targetAniso);
    }
    glBindSampler(0, sampler);

    world = std::make_unique<World>();
    for(int x = 0; x < 3;++x)
    {
        for(int y = 0; y < 3;++y)
        {
            world->addChunk(x, y);
        }
    }
}

void Game::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 projection = camera->GetProjectionMatrix();
    glm::mat4 view = camera->GetViewMatrix();
    
    if (shader) {
        shader->use();
        shader->setMat4("projection", projection);
        shader->setMat4("view", view);
        shader->setInt("u_Atlas", 0);
    }
    
    if (texture) {
        texture->bind(0);
    }

    if (world) {
        world->render(*shader);
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
}

void Game::update()
{
    double lastFrame = 0.0f;
    bool first_mouse = true;
    do {
        double currentFrame = glfwGetTime();
        float deltaTime = float(currentFrame - lastFrame);
        lastFrame = currentFrame;

        camera->ProcessWASDMovement(window);
        if(first_mouse)
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            glfwSetCursorPos(window, width / 2.0, height / 2.0);
            first_mouse = false;
        }
        camera->ProcessMouseMovement(window, deltaTime);
        
        render();

    } while(glfwWindowShouldClose(window) == 0);
}
