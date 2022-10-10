#ifndef WINDOW_HPP_
#define WINDOW_HPP_

#include <random>
#include "abcgOpenGL.hpp"

class Window : public abcg::OpenGLWindow {
protected:
  void onCreate() override;
  void onPaint() override;
  void onPaintUI() override;

  void onResize(glm::ivec2 const &size) override;
  void onDestroy() override;

private:
  enum class GameState { Day, Night };
  GameState m_State;

  bool m_isDay{true};

  glm::ivec2 m_viewportSize{};

  GLuint m_VAO{};
  GLuint m_VBOPositions{};
  GLuint m_VBOColors{};
  GLuint m_program{};

  std::default_random_engine m_randomEngine;

  abcg::Timer m_timer;
  int m_delay{160};

  void setupModel(int sides);

  std::array<float, 4> m_dayColor{0.15f, 0.463f, 1.0f, .0f};
  std::array<float, 4> m_nightColor{0.0f, 0.0f, 0.262f, .0f};
  int sides{3};
  int animation_frame{0};
  float speed{1.0};
  void restartUI();
};

#endif