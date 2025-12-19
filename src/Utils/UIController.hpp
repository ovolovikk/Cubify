#ifndef UI_CONTROLLER_HPP
#define UI_CONTROLLER_HPP

struct GLFWwindow;
class Camera;
class World;

class UIController
{
public:
    UIController(GLFWwindow* window);
    ~UIController();

    UIController(const UIController&) = delete;
    UIController& operator=(const UIController&) = delete;

    void update(const Camera& camera, const World& world);

private:
    void showDebugWindow(const Camera& camera, const World& world);

    bool wireframe = false;
};

#endif // UI_CONTROLLER_HPP
