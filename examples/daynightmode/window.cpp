#include "window.hpp"

void Window::onCreate() {
  auto const *vertexShader{R"gl(#version 300 es

    layout(location = 0) in vec2 inPosition;
    layout(location = 1) in vec4 inColor;

    uniform float scale;

    out vec4 fragColor;

    void main() {
      vec2 newPosition = inPosition * scale;
      gl_Position = vec4(newPosition, 0, 1);
      fragColor = inColor;
    }
  )gl"};

  auto const *fragmentShader{R"gl(#version 300 es

    precision mediump float;  

    in vec4 fragColor;

    out vec4 outColor;

    void main() { outColor = fragColor; }
  )gl"};

  m_program = abcg::createOpenGLProgram(
      {{.source = vertexShader, .stage = abcg::ShaderStage::Vertex},
       {.source = fragmentShader, .stage = abcg::ShaderStage::Fragment}});

  // abcg::glClearColor(0, 0, 0, 1);
  // abcg::glClear(GL_COLOR_BUFFER_BIT);

  m_randomEngine.seed(
      std::chrono::steady_clock::now().time_since_epoch().count());

  restartUI();
}

void Window::restartUI() {
  abcg::glClear(GL_COLOR_BUFFER_BIT);
  m_dayColor = {0.15f, 0.463f, 1.0f, .0f};
  m_nightColor = {0.0f, 0.0f, 0.262f, .0f};
  // TODO: Reset Colors and Sliders
}

void Window::onResize(glm::ivec2 const &size) {
  m_viewportSize = size;
  abcg::glClear(GL_COLOR_BUFFER_BIT);
}

void Window::onDestroy() {
  abcg::glDeleteProgram(m_program);
  abcg::glDeleteBuffers(1, &m_VBOPositions);
  abcg::glDeleteBuffers(1, &m_VBOColors);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}

void Window::onPaint() {
  // Set the clear color
  if (m_isDay) {
    abcg::glClearColor(m_dayColor.at(0), m_dayColor.at(1), m_dayColor.at(2),
                     m_dayColor.at(3));
  }
  else {
    abcg::glClearColor(m_nightColor.at(0), m_nightColor.at(1), m_nightColor.at(2),
                     m_nightColor.at(3));
  }
  // Clear the color buffer
  abcg::glClear(GL_COLOR_BUFFER_BIT);

  
  if (animation_frame < 200) {
    
    if (m_timer.elapsed() < m_delay / 1000.0)
      animation_frame = animation_frame + 2;
    m_timer.restart();

    setupModel(sides);

    abcg::glViewport(0, 0, m_viewportSize.x, m_viewportSize.y);

    abcg::glUseProgram(m_program);

    auto const scale{animation_frame/100.0f};
    auto const scaleLocation{abcg::glGetUniformLocation(m_program, "scale")};
    abcg::glUniform1f(scaleLocation, scale);

    // Render
    abcg::glBindVertexArray(m_VAO);
    abcg::glDrawArrays(GL_TRIANGLE_FAN, 0, sides + 2);
    abcg::glBindVertexArray(0);

    abcg::glUseProgram(0);

    if (animation_frame >= 200) {
        m_isDay = !m_isDay;
      }
  }
}

void Window::onPaintUI() {
  abcg::OpenGLWindow::onPaintUI();
  {
    // Window begin
    ImGui::Begin(" ");
    ImGui::Text("Animation frame %d\nMode: %s", animation_frame, m_isDay ? "Day" : "Night");
    ImGui::Text("Day Mode Color:");
    ImGui::SliderFloat("Day_R", &m_dayColor.at(0), 0.0f, 1.0f);
    ImGui::SliderFloat("Day_G", &m_dayColor.at(1), 0.0f, 1.0f);
    ImGui::SliderFloat("Day_B", &m_dayColor.at(2), 0.0f, 1.0f);

    ImGui::Text("Night Mode Color:");
    ImGui::SliderFloat("Night_R", &m_nightColor.at(0), 0.0f, 1.0f);
    ImGui::SliderFloat("Night_G", &m_nightColor.at(1), 0.0f, 1.0f);
    ImGui::SliderFloat("Night_B", &m_nightColor.at(2), 0.0f, 1.0f);

    ImGui::SliderInt("Sides", &sides, 3, 50);
    ImGui::SliderFloat("Speed", &speed, 1.00f, 10.00f);


    if (ImGui::Button("Change Mode", ImVec2(-1, 30))) {
      animation_frame = 0;
    }

    if (ImGui::Button("Restart All", ImVec2(-1, 30))) {
      restartUI();
    }

    // Window end
    ImGui::End();
  }
}


void Window::setupModel(int sides) {
  // Release previous resources, if any
  abcg::glDeleteBuffers(1, &m_VBOPositions);
  abcg::glDeleteBuffers(1, &m_VBOColors);
  abcg::glDeleteVertexArrays(1, &m_VAO);

  // Select color
  glm::vec3 color{};
  if (m_isDay) {
    color[0] = m_nightColor.at(0);
    color[1] = m_nightColor.at(1);
    color[2] = m_nightColor.at(2);
  }
  else {
    color[0] = m_dayColor.at(0);
    color[1] = m_dayColor.at(1);
    color[2] = m_dayColor.at(2);
  }

  std::vector<glm::vec2> positions;
  std::vector<glm::vec3> colors;

  // Polygon center
  positions.emplace_back(0, 0);
  colors.push_back(color);

  // Border vertices
  auto const step{M_PI * 2 / sides};
  for (auto const angle : iter::range(0.0, M_PI * 2, step)) {
    positions.emplace_back(std::cos(angle), std::sin(angle));
    colors.push_back(color);
  }

  // Duplicate second vertex
  positions.push_back(positions.at(1));
  colors.push_back(color);

  // Generate VBO of positions
  abcg::glGenBuffers(1, &m_VBOPositions);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOPositions);
  abcg::glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec2),
                     positions.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate VBO of colors
  abcg::glGenBuffers(1, &m_VBOColors);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOColors);
  abcg::glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3),
                     colors.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Get location of attributes in the program
  auto const positionAttribute{
      abcg::glGetAttribLocation(m_program, "inPosition")};
  auto const colorAttribute{abcg::glGetAttribLocation(m_program, "inColor")};

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_VAO);

  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOPositions);
  abcg::glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glEnableVertexAttribArray(colorAttribute);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBOColors);
  abcg::glVertexAttribPointer(colorAttribute, 3, GL_FLOAT, GL_FALSE, 0,
                              nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}