//
// Created by andy on 5/1/2025.
//

#include "window_renderer.hpp"

namespace engine {
    WindowRenderer::WindowRenderer(const std::shared_ptr<RenderDevice> &render_device, const std::shared_ptr<Swapchain> &swapchain)
        : m_RenderDevice(render_device), m_Swapchain(swapchain), m_CommandBuffers(m_RenderDevice->allocateCommandBuffers<QueueType::GRAPHICS>(MAX_FRAMES_IN_FLIGHT)) {
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            m_InFlightFences.emplace_back(m_RenderDevice->createFence(true));
            m_ImageAvailableSemaphores.emplace_back(m_RenderDevice->createSemaphore());
            m_RenderFinishedSemaphores.emplace_back(m_RenderDevice->createSemaphore());
        }

        recreateImageViews(m_Swapchain->getImages(), m_Swapchain->getSurfaceFormat(), m_Swapchain->getExtent());
        m_Swapchain->onSwapchainReconfigure.connect<&WindowRenderer::recreateImageViews>(this);
    }

    WindowRenderer::~WindowRenderer() {}

    void WindowRenderer::renderFrame(const std::function<void(const vk::raii::CommandBuffer& cmd, const SwapchainFrameInfo& frameInfo, uint32_t currentFrame)> &func) {
        const auto &fence           = m_InFlightFences[m_CurrentFrame];
        const auto &image_available = m_ImageAvailableSemaphores[m_CurrentFrame];
        const auto &render_finished = m_RenderFinishedSemaphores[m_CurrentFrame];

        m_RenderDevice->waitFence(fence);

        const auto frame_info = m_Swapchain->acquireNextFrame(image_available);
        if (!frame_info.has_value()) {
            return;
        }

        m_RenderDevice->resetFence(fence);

        const auto &cmd = m_CommandBuffers[m_CurrentFrame];

        cmd.reset();
        cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

        imageTransition(cmd, frame_info->image, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1),
                        {vk::ImageLayout::eUndefined, vk::PipelineStageFlagBits2::eTopOfPipe, vk::AccessFlagBits2::eNone, 0},
                        {vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite, 0});


        vk::RenderingAttachmentInfo color_attachment = {*m_ImageViews[frame_info->imageIndex], vk::ImageLayout::eColorAttachmentOptimal,
                                                        vk::ResolveModeFlagBits::eNone,        nullptr,
                                                        vk::ImageLayout::eUndefined,           vk::AttachmentLoadOp::eClear,
                                                        vk::AttachmentStoreOp::eStore,         vk::ClearColorValue{1.0f, 0.0f, 0.0f, 1.0f}};

        vk::RenderingInfo rendering_info{};
        rendering_info.setRenderArea({{0, 0}, frame_info->extent});
        rendering_info.setLayerCount(1);
        rendering_info.setColorAttachments(color_attachment);

        cmd.beginRendering(rendering_info);
        func(cmd, frame_info.value(), m_CurrentFrame);
        cmd.endRendering();

        imageTransition(cmd, frame_info->image, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1),
                        {vk::ImageLayout::eColorAttachmentOptimal, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite, 0},
                        {vk::ImageLayout::ePresentSrcKHR, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::AccessFlagBits2::eNone, 0});

        cmd.end();

        vk::SemaphoreSubmitInfo     wait_info{*image_available, 0, vk::PipelineStageFlagBits2::eAllCommands};
        vk::SemaphoreSubmitInfo     signal_info{*render_finished, 0, vk::PipelineStageFlagBits2::eAllCommands};
        vk::CommandBufferSubmitInfo cmd_submit_info{*cmd};
        const vk::SubmitInfo2       si{{}, wait_info, cmd_submit_info, signal_info};
        m_RenderDevice->graphicsQueue().submit2(si, fence);

        m_Swapchain->present(render_finished);

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void WindowRenderer::recreateImageViews(const std::vector<vk::Image> &images, vk::SurfaceFormatKHR surfaceFormat, vk::Extent2D extent) {
        m_ImageViews.clear();
        for (const auto image : images) {
            m_ImageViews.emplace_back(
                m_RenderDevice->device(),
                vk::ImageViewCreateInfo({}, image, vk::ImageViewType::e2D, surfaceFormat.format,
                                        vk::ComponentMapping{vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA},
                                        vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)));
        }
    }
} // namespace engine
