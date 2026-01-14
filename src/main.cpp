#include "Core/Application.hpp"
#include "Core/Logging/LogScope.hpp"
#include "Core/Logging/Log.hpp"

int main()
{   
    // for RAII
    LogScope log;
    LOGI("=== Cubify Start ===");

    ApplicationConfig config;
    config.title = "Cubify";
    config.width = 1920;
    config.height = 1080;

    Application::Create(config);

    APP.run();

    Application::Destroy();
    LOGI("=== Cubify End ===");

    return 0;
}
