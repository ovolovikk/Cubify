#include "Core/Application.hpp"
#include "Core/Logging/LogScope.hpp"
#include "Core/Logging/Log.hpp"
#include "Utils/Config.hpp"

#ifdef _WIN32
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main()
#endif
{   
    // for RAII
    LogScope log;
    LOGI("=== Cubify Start ===");

    Config::Load("config.json");
    auto& cfg = Config::Get();

    ApplicationConfig config;
    config.title = "Cubify";
    config.width = cfg.wConfig.width;
    config.height = cfg.wConfig.height;
    config.vsync = cfg.wConfig.vsync;
    config.testMode = cfg.gConfig.testMode;
    if(config.testMode)
    {
        config.width = 800;
        config.height = 600;
    }

    Application::Create(config);

    APP.run();

    Application::Destroy();
    LOGI("=== Cubify End ===");

    return 0;
}
