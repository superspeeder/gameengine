# build an interface library so that we can share the sources to the engine without needing to link dependencies here
add_library(imgui INTERFACE imgui.cpp imgui_demo.cpp imgui_draw.cpp imgui_impl_dx12.cpp imgui_impl_glfw.cpp imgui_impl_opengl3.cpp imgui_impl_vulkan.cpp)
target_include_directories(imgui INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})


