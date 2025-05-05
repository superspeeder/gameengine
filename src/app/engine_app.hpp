//
// Created by andy on 4/30/2025.
//

#pragma once

#include "engine/render/material.hpp"
#include "engine/render/shader_object.hpp"
#include "engine/render/vertex_buffer.hpp"
#include "engine/render/window_renderer.hpp"
#include "engine/swapchain.hpp"
#include "engine/window.hpp"

#include <memory>

namespace app {

    struct glfw_lib {
        glfw_lib();
        ~glfw_lib();
    };

    class EngineApp {
      public:
        EngineApp();
        ~EngineApp();

        void run();

      private:
        glfw_lib _glfw{};

        std::shared_ptr<engine::Window>         m_Window;
        std::shared_ptr<engine::RenderDevice>   m_RenderDevice;
        std::shared_ptr<engine::Swapchain>      m_Swapchain;
        std::shared_ptr<engine::WindowRenderer> m_WindowRenderer;
        std::shared_ptr<engine::MaterialShader> m_Shader;
        std::shared_ptr<engine::VertexBuffer>   m_VertexBuffer;
    };

} // namespace app
