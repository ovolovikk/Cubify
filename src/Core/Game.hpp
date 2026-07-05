#pragma once

#include <glm/vec3.hpp>

#include "Core/BlockType.hpp"
#include "World/WorldType.hpp"

struct GLFWwindow;
class IInputController;
class IRendererBackend;
class Camera;
class World;
class Player;
class Window;
class DebugUI;

// Engine level
// Here goes all game-systems logic
class Game {
public:
  Game(Window &window, IRendererBackend &renderer, IInputController &inputController,
       WorldType worldType, bool testMode = false);
  ~Game();

  Game(const Game &) = delete;
  Game &operator=(const Game &) = delete;

  // Called each frame by APP
  void onUpdate(float DeltaTime);
  void onRender();
  void onRenderDebug(DebugUI *debugUI);

  void onResize(int width, int height);
  bool isReadyForTest() const;

  BlockType getSelectedBlock() const { return selectedBlock; }

private:
  void init();

  void handleInput(float deltaTime);
  void onAtmosphere();

  // Borrowed from APP
  Window &m_window;
  IRendererBackend &m_renderer;
  IInputController &m_inputController;
  WorldType m_worldType;

  // Owned by a Game
  std::unique_ptr<Camera> camera;
  std::unique_ptr<World> world;
  std::unique_ptr<Player> player;

private:
  // Input state
  bool free_cam_mode = false;
  bool cursor_visible = false;
  bool world_rendered = false;
  bool player_spawned = false;
  bool music_started = false;
  bool m_testMode = false;
  BlockType selectedBlock = BlockType::GRASS;

  static constexpr auto CAMERA_START_POS = glm::vec3(0.0f, 200.0f, 0.0f);

  static constexpr BlockType PLACEABLE_BLOCKS[] = {
      BlockType::DIRT,          BlockType::STONE,
      BlockType::GRASS,         BlockType::SAND,
      BlockType::WOODEN_PLANK,  BlockType::WATER,
      BlockType::BEDROCK,       BlockType::ICE,
      BlockType::SECTORR_GRASS, BlockType::SECTORR_DIRT,
      BlockType::SECTORR_STONE, BlockType::SECTORR_SAND,
      BlockType::SECTORR_WATER, BlockType::UTOPIA_SAND,
      BlockType::UTOPIA_SILT,   BlockType::UTOPIA_WATER};
  static constexpr int NUM_PLACEABLE_BLOCKS =
      sizeof(PLACEABLE_BLOCKS) / sizeof(PLACEABLE_BLOCKS[0]);
  int selectedBlockIndex = 2;
};
