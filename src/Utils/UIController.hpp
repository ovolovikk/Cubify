#ifndef UI_CONTROLLER_HPP
#define UI_CONTROLLER_HPP

//TODO

class UIController
{
public:
    UIController();

    UIController(const UIController&) = delete;
    UIController& operator=(const UIController&) = delete;

    void update();

private:
    void showFPS();
    void showCoorditanes();
    void showRayCastBlock();
};

#endif // UI_CONTROLLER_HPP
