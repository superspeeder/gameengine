//
// Created by andy on 4/30/2025.
//

#pragma once

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

        void run();

    private:
        glfw_lib _glfw{};

        std::shared_ptr<engine::Window> m_Window;
    };

} // namespace app
