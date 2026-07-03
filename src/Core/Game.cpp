#include "Game.hpp"

#include "PrecompilerHeader.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Core/Application.hpp"
#include "Core/BlockType.hpp"
#include "Core/Camera.hpp"
#include "Core/Input/IInputController.hpp"
#include "Core/Logging/Log.hpp"
#include "Core/Sound/AudioEngine.hpp"
#include "Core/Window.hpp"
#include "Graphics/IRendererBackend.hpp"
#include "Math/Frustum.hpp"
#include "Player/Player.hpp"
#include "UI/DebugUI.hpp"
#include "Utils/Config.hpp"
#include "World/World.hpp"
#include "World/WorldSettings.hpp"

Game::Game(Window &window, IRendererBackend &renderer,
           IInputController &inputController, WorldType worldType, bool testMode)
    : m_window(window), m_renderer(renderer),
      m_inputController(inputController), m_worldType(worldType), m_testMode(testMode) {
  LOGI("[Game] Constructing...");
  init();
  LOGI("[Game] Ready");
}

Game::~Game() { LOGI("[Game] Destroying..."); }

void Game::init() {
  GLFWwindow *nativeWindow = m_window.GetGLFWWindow();
  auto &cfg = Config::Get();
  WorldSettings settings = WorldSettings::getForWorldType(m_worldType);
  m_renderer.setWorldSettings(settings);
  LOGI("[Game] Applied world settings for type: %d",
       static_cast<int>(m_worldType));

  float aspect = (float)m_window.getWidth() / (float)m_window.getHeight();
  camera = std::make_unique<Camera>(CAMERA_START_POS, cfg.cConfig.fov, aspect);
  world = std::make_unique<World>(m_worldType);
  player = std::make_unique<Player>(*camera, m_inputController, *world,
                                    glm::vec3(0, 200, 0));
}

void Game::onUpdate(float deltaTime) {
  if (deltaTime > 0.1f)
    deltaTime = 0.1f;

  m_inputController.update();
  if (!m_testMode) {
    handleInput(deltaTime);
  }

  onAtmosphere();

  if (!free_cam_mode && world_rendered) {
    if (!player_spawned) {
      world->prepareAllChunks(m_renderer);
      player->setPosition(world->getSpawnPoint());
      player_spawned = true;

      if (!music_started) {
        AudioEngine::Instance().PlayMusic("assets/sounds/main_game_theme.ogg");
        music_started = true;
      }
    }

    if (m_testMode) {
      player->setPosition(world->getSpawnPoint());
    } else {
      player->update(deltaTime);
    }
  }

  world->update(camera->GetPosition());
}

bool Game::isReadyForTest() const {
  return world_rendered && player_spawned;
}

void Game::onRender() {
  m_renderer.beginFrame();

  glm::mat4 view = camera->GetViewMatrix();
  glm::mat4 projection = camera->GetProjectionMatrix();
  glm::mat4 viewProj = projection * view;

  m_renderer.setViewProjection(view, projection);

  // frustum culling
  float maxDistance = (float)(world->GetRenderDistance() * CHUNK_SIZE);
  Frustum frustum(viewProj);

  world->draw(m_renderer, frustum);
  world_rendered = true;
}

void Game::onRenderDebug(DebugUI *debugUI) {
  if (debugUI != nullptr) {
    debugUI->renderBlockSelector(selectedBlock);

    if (debugUI->isVisible()) {
      debugUI->renderGameInfo(*camera.get(), *world.get());
    }
  }
}

void Game::handleInput(float deltaTime) {
  GLFWwindow *nativeWindow = m_window.GetGLFWWindow();

  if (m_inputController.wasKeyJustPressed(GLFW_KEY_ESCAPE)) {
    APP.returnToMenu();
  }
  // Free cam toogle
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_F1)) {
    free_cam_mode = !free_cam_mode;
  }

  // Cursor toogle
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_F3)) {
    cursor_visible = !cursor_visible;
    m_inputController.setCursorEnabled(cursor_visible);
  }

  // Fullscreen toggle
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_F11)) {
    m_window.toggleFullscreen();
  }

  // Choosing a block
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_1)) {
    selectedBlock = BlockType::GRASS;
  }
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_2))
    selectedBlock = BlockType::DIRT;
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_3))
    selectedBlock = BlockType::STONE;
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_4))
    selectedBlock = BlockType::SAND;
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_5))
    selectedBlock = BlockType::WOODEN_PLANK;
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_6))
    selectedBlock = BlockType::BEDROCK;
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_7))
    selectedBlock = BlockType::ICE;
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_8))
    selectedBlock = BlockType::SECTORR_SAND;
  if (m_inputController.wasKeyJustPressed(GLFW_KEY_9))
    selectedBlock = BlockType::SECTORR_STONE;

  float scrollDelta = m_inputController.getScrollDelta();
  if (scrollDelta != 0.0f) {
    if (scrollDelta > 0.0f) {
      selectedBlockIndex = (selectedBlockIndex + 1) % NUM_PLACEABLE_BLOCKS;
    } else {
      selectedBlockIndex = (selectedBlockIndex - 1 + NUM_PLACEABLE_BLOCKS) %
                           NUM_PLACEABLE_BLOCKS;
    }
    selectedBlock = PLACEABLE_BLOCKS[selectedBlockIndex];
  }

  // Destroy blocks
  if (m_inputController.wasMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT)) {
    AudioEngine::Instance().PlayShortSound("assets/sounds/breaking_block.mp3");
    world->rayCastBreakBlock(camera->GetPosition(), camera->GetFront(), 4.0f);
  }
  // Place blocks
  if (m_inputController.wasMouseButtonJustPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
    world->rayCastPlaceBlock(camera->GetPosition(), camera->GetFront(), 4.0f,
                             selectedBlock);
  }

  // Move input
  if (free_cam_mode) {
    camera->processInput(m_inputController, deltaTime);
  }
}

void Game::onResize(int width, int height) {
  m_window.setSize(width, height);
  m_renderer.resize(width, height);
  if (camera && height > 0)
    camera->SetAspect((float)width / (float)height);
}

void Game::onAtmosphere() {
  glm::vec3 pos = camera->GetPosition();
  int x = (int)std::floor(pos.x);
  int y = (int)std::floor(pos.y);
  int z = (int)std::floor(pos.z);

  BlockType currentBlock = world->getBlock(x, y, z);

  WorldSettings settings = WorldSettings::getForWorldType(m_worldType);

  float renderDistance = (float)Config::Get().gConfig.renderDistance;
  if (renderDistance > 0) {
    settings.fogDensity *= (32.0f / renderDistance);
  }

  if (currentBlock == BlockType::WATER ||
      currentBlock == BlockType::SECTORR_WATER) {
    settings.skyColor = glm::vec3(0.0f, 0.1f, 0.4f);
    settings.fogDensity = 0.15f;
    settings.fogPower = 2.0f;
  } else if (currentBlock == BlockType::UTOPIA_WATER) {
    settings.skyColor = glm::vec3(0.2f, 0.25f, 0.3f);
    settings.fogDensity = 0.08f;
    settings.fogPower = 1.5f;
  }
  m_renderer.setWorldSettings(settings);
}
