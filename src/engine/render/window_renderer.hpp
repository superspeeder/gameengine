//
// Created by andy on 5/1/2025.
//

#pragma once

#include "engine/render_device.hpp"
#include "engine/swapchain.hpp"
#include <vulkan/vulkan_raii.hpp>


namespace engine {

    constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    class WindowRenderer {
      public:
        WindowRenderer(const std::shared_ptr<RenderDevice> &render_device, const std::shared_ptr<Swapchain> &swapchain);
        ~WindowRenderer();

        void renderFrame(const std::function<void(const vk::raii::CommandBuffer& cmd, const SwapchainFrameInfo& frameInfo, uint32_t currentFrame)> &func);

      private:
        void recreateImageViews(const std::vector<vk::Image> &images, vk::SurfaceFormatKHR surfaceFormat, vk::Extent2D extent);

        std::shared_ptr<RenderDevice> m_RenderDevice;
        std::shared_ptr<Swapchain>    m_Swapchain;

        uint32_t m_CurrentFrame = 0;

        std::vector<vk::raii::ImageView> m_ImageViews;
        std::vector<vk::raii::Fence>     m_InFlightFences;
        std::vector<vk::raii::Semaphore> m_ImageAvailableSemaphores;
        std::vector<vk::raii::Semaphore> m_RenderFinishedSemaphores;
        vk::raii::CommandBuffers         m_CommandBuffers;

        bool m_LastFrameSkipped = false;

    };

} // namespace engine
