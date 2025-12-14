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

    void showFPS();
    void showCoorditanes();
    void showRayCastBlock();

private:


};

#endif // UI_CONTROLLER_HPP
