#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <json.hpp>

#include <string>

struct WindowConfig {
    int width;
    int height;
    bool vsync;
};

struct GraphicsConfig {
    uint8_t renderDistance;
};

struct PlayerConfig {
    float moveSpeed;
    float sprintMultiplier;
    float jumpForce;
};

struct CameraConfig {
    float fov;
    float sensitivity;
};

struct GameConfig {
    WindowConfig wConfig;
    GraphicsConfig gConfig;
    PlayerConfig pConfig;
    CameraConfig cConfig;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(WindowConfig, width, height, vsync)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GraphicsConfig, renderDistance)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PlayerConfig, moveSpeed, sprintMultiplier, jumpForce)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CameraConfig, fov, sensitivity)
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(GameConfig, wConfig, gConfig, pConfig, cConfig)

class Config
{
public:
    static const GameConfig& Get();
    
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    static bool Load(const std::string &path = "config.json");

    static void Save(const std::string& path = "config.json");
private:
    GameConfig m_config;
    void Validate();

    Config() = default;

    static Config& Instance();
};

#endif // CONFIG_HPP