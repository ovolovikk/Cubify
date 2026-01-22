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

    void begin();
    void end();

    void renderAppInfo();
    void renderGameInfo(const Camera& camera, const World& world);

    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }
    void toggleVisible() { m_visible = !m_visible; }

private:
    float m_volume = 50.0f;
    bool m_visible = false;
    bool m_wireframe = false;
};

#endif // DEBUG_UI_HPP
