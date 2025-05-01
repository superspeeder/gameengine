#pragma once
#include "window.hpp"

namespace engine {

    struct SwapchainFrameInfo {
        vk::Image image;
        uint32_t imageIndex;
        vk::SurfaceFormatKHR surfaceFormat;
        vk::Extent2D extent;
    };

    class Swapchain {
    public:
        Swapchain(const std::shared_ptr<RenderSystem>& render_system, const std::shared_ptr<Window>& window);

        const std::vector<vk::Image>& getImages() const;
        vk::SurfaceFormatKHR getSurfaceFormat() const;
        vk::PresentModeKHR getPresentMode() const;
        vk::Extent2D getExtent() const;

        void reconfigure();

        [[nodiscard]] inline SwapchainFrameInfo getCurrentFrameInfo() const { return m_CurrentFrameInfo; }

        SwapchainFrameInfo acquireNextFrame(const vk::raii::Semaphore &availableSignal);

    private:
        std::shared_ptr<Window> m_Window;
        std::shared_ptr<RenderSystem> m_RenderSystem;
        vk::raii::SwapchainKHR m_Swapchain;

        std::vector<vk::Image> m_Images;
        vk::SurfaceFormatKHR m_SurfaceFormat;
        vk::PresentModeKHR m_PresentMode;
        vk::Extent2D m_Extent;

        SwapchainFrameInfo m_CurrentFrameInfo;

    };

} // namespace engine
