#include "Utils/Config.hpp"

#include "Core/Logging/Log.hpp"
#include <fstream>

Config& Config::Instance()
{
    static Config s_instance;
    return s_instance;
}

const GameConfig& Config::Get()
{
    return Instance().m_config;
}

bool Config::Load(const std::string& path)
{
    std::ifstream f(path);
    if(!f.is_open())
    {
        LOGE("[Config] Failed to open %s, using defaults", path.c_str());
        return false;
    }

    try {
        nlohmann::json j = nlohmann::json::parse(f);
        Instance().m_config = j.get<GameConfig>();
        Instance().Validate();
        LOGI("[Config] Loaded from %s", path.c_str());
        return true;
    } catch (const nlohmann::json::exception& e) {
        LOGE("[Config] Parse error: %s", e.what());
        return false;
    }
}

void Config::Save(const std::string& path)
{
    std::ofstream f(path);
    if (!f.is_open()) {
        LOGE("[Config] Failed to save to %s", path.c_str());
        return;
    }

    nlohmann::json j = Instance().m_config;
    f << j.dump(4);
    LOGI("[Config] Saved to %s", path.c_str());
}

void Config::Validate()
{
    auto clamp = [](auto& v, auto lo, auto hi, auto def) { 
        v = (v < lo || v > hi) ? def : v; 
    };

    auto& w = m_config.wConfig;
    clamp(w.width, 640, 7680, 1920);
    clamp(w.height, 480, 4320, 1080);

    auto& g = m_config.gConfig;
    clamp(g.renderDistance, (uint8_t)2, (uint8_t)128, (uint8_t)64);

    auto& p = m_config.pConfig;
    clamp(p.moveSpeed, 1.0f, 200.0f, 45.0f);
    clamp(p.sprintMultiplier, 1.0f, 5.0f, 2.03f);
    clamp(p.jumpForce, 0.0f, 50.0f, 9.0f);

    auto& c = m_config.cConfig;
    clamp(c.fov, 30.0f, 120.0f, 60.0f);
    clamp(c.sensitivity, 0.01f, 2.0f, 0.1f);
}
