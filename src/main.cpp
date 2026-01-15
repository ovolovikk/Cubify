#include "Core/Application.hpp"
#include "Core/Logging/LogScope.hpp"
#include "Core/Logging/Log.hpp"
#include "Utils/Config.hpp"

int main()
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

    Application::Create(config);

    APP.run();

    Application::Destroy();
    LOGI("=== Cubify End ===");

    return 0;
}
