#pragma once
#include "utils.hpp"
#include "window.hpp"

#include <entt/signal/sigh.hpp>

namespace engine {

    struct SwapchainFrameInfo {
        vk::Image            image;
        uint32_t             imageIndex;
        vk::SurfaceFormatKHR surfaceFormat;
        vk::Extent2D         extent;

        void setViewportAndScissor(const vk::raii::CommandBuffer& cmd) const;
    };

    class Swapchain {
      public:
        Swapchain(const std::shared_ptr<RenderDevice> &render_system, const std::shared_ptr<Window> &window);

        const std::vector<vk::Image> &getImages() const;
        vk::SurfaceFormatKHR          getSurfaceFormat() const;
        vk::PresentModeKHR            getPresentMode() const;
        vk::Extent2D                  getExtent() const;

        void reconfigure();
        void update();

        [[nodiscard]] inline SwapchainFrameInfo getCurrentFrameInfo() const { return m_CurrentFrameInfo; };

        std::optional<SwapchainFrameInfo> acquireNextFrame(const vk::raii::Semaphore &availableSignal);
        void                              present(const vk::raii::Semaphore &renderedSignal);

      private:
        std::shared_ptr<Window>       m_Window;
        std::shared_ptr<RenderDevice> m_RenderDevice;
        vk::raii::SwapchainKHR        m_Swapchain;

        std::vector<vk::Image> m_Images;
        vk::SurfaceFormatKHR   m_SurfaceFormat;
        vk::PresentModeKHR     m_PresentMode;
        vk::Extent2D           m_Extent;

        SwapchainFrameInfo m_CurrentFrameInfo;

        entt::sigh<void(const std::vector<vk::Image> &, vk::SurfaceFormatKHR, vk::Extent2D)> m_OnSwapchainReconfigure;

        bool m_RequiresReconfigure = false;

      public:
        ENTT_SINK_FOR(m_OnSwapchainReconfigure, onSwapchainReconfigure);
    };

} // namespace engine
