cmake_minimum_required(VERSION 3.31)
project(gameengine)

set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.4)
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG 1.0.1)
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog.git GIT_TAG v1.15.2)
FetchContent_Declare(stb GIT_REPOSITORY https://github.com/nothings/stb.git GIT_TAG master CONFIGURE_COMMAND "" BUILD_COMMAND "")
FetchContent_Declare(VulkanHeaders GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git GIT_TAG v1.4.313)
FetchContent_Declare(vkmemalloc GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git GIT_TAG v3.2.1)
FetchContent_Declare(entt GIT_REPOSITORY https://github.com/skypjack/entt.git GIT_TAG v3.15.0)

set(SPDLOG_USE_STD_FORMAT ON)

FetchContent_MakeAvailable(glfw glm spdlog stb VulkanHeaders vkmemalloc entt)

add_executable(gameengine src/main.cpp
        src/engine/window.cpp
        src/engine/window.hpp
        src/app/engine_app.cpp
        src/app/engine_app.hpp
        src/engine/render_device.cpp
        src/engine/render_device.hpp
        src/engine/swapchain.cpp
        src/engine/swapchain.hpp
        src/engine/render/window_renderer.cpp
        src/engine/render/window_renderer.hpp
        src/engine/utils.hpp
        src/engine/render/shader_object.cpp
        src/engine/render/shader_object.hpp
        src/engine/render/vertex_buffer.cpp
        src/engine/render/vertex_buffer.hpp
        src/engine/render/material.cpp
        src/engine/render/material.hpp
)
target_include_directories(gameengine PRIVATE src/ ${stb_SOURCE_DIR})
target_link_libraries(gameengine PRIVATE glfw glm::glm spdlog::spdlog Vulkan::Headers GPUOpen::VulkanMemoryAllocator EnTT::EnTT)
target_compile_definitions(gameengine PRIVATE GLFW_INCLUDE_NONE GLFW_INCLUDE_VULKAN GLM_ENABLE_EXPERIMENTAL VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VMA_STATIC_VULKAN_FUNCTIONS=0 VMA_DYNAMIC_VULKAN_FUNCTIONS=1)

