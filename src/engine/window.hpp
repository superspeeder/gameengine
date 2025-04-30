//
// Created by andy on 4/30/2025.
//

#pragma once

#include "glm/vec2.hpp"


#include <GLFW/glfw3.h>

namespace engine {

    class Window {
      public:
        Window();
        ~Window();

        [[nodiscard]] bool       shouldClose() const;
        [[nodiscard]] glm::uvec2 getSize() const;
        [[nodiscard]] glm::dvec2 getMousePosition() const;
        [[nodiscard]] bool getKey(int key) const;
        [[nodiscard]] bool getButton(int button) const;

        void close() const;

      private:
        GLFWwindow* m_Window;
    };

} // namespace engine
