//
// Created by andy on 4/30/2025.
//

#pragma once

#include "glm/vec2.hpp"
#include "render_device.hpp"


#include <GLFW/glfw3.h>

namespace engine {

    class Window {
      public:
        explicit Window(const std::shared_ptr<RenderDevice> &RenderDevice);
        ~Window();

        [[nodiscard]] bool       shouldClose() const;
        [[nodiscard]] glm::uvec2 getSize() const;
        [[nodiscard]] glm::dvec2 getMousePosition() const;
        [[nodiscard]] bool       getKey(int key) const;
        [[nodiscard]] bool       getButton(int button) const;

        void close() const;

        [[nodiscard]] vk::SurfaceCapabilitiesKHR         getSurfaceCapabilities() const;
        [[nodiscard]] std::vector<vk::SurfaceFormatKHR>  getSurfaceFormats() const;
        [[nodiscard]] std::vector<vk::PresentModeKHR>    getPresentModes() const;
        [[nodiscard]] vk::Extent2D                       getSurfaceCompatibleExtent() const;
        [[nodiscard]] inline const vk::raii::SurfaceKHR &surface() const { return m_Surface; }

      private:
        GLFWwindow                   *m_Window;
        std::shared_ptr<RenderDevice> m_RenderDevice;
        vk::raii::SurfaceKHR          m_Surface{nullptr};
    };

} // namespace engine
