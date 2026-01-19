#ifndef DEBUG_UI_HPP
#define DEBUG_UI_HPP

struct GLFWwindow;
class Camera;
class World;

class DebugUI
{
public:
    DebugUI(GLFWwindow* window);
    ~DebugUI();

    DebugUI(const DebugUI&) = delete;
    DebugUI& operator=(const DebugUI&) = delete;

    void update(const Camera& camera, const World& world);

private:
    void showDebugWindow(const Camera& camera, const World& world);

    bool wireframe = false;
};

#endif // DEBUG_UI_HPP
