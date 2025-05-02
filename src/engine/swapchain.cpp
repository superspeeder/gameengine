//
// Created by andy on 4/30/25.
//

#include "swapchain.hpp"

#include <iostream>

namespace engine {
    void SwapchainFrameInfo::setViewportAndScissor(const vk::raii::CommandBuffer &cmd) const {
        const vk::Viewport viewport = {0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f};
        const vk::Rect2D   scissor  = {{0, 0}, extent};

        cmd.setViewportWithCount(viewport);
        cmd.setScissorWithCount(scissor);
    }

    Swapchain::Swapchain(const std::shared_ptr<RenderDevice> &render_system, const std::shared_ptr<Window> &window)
        : m_Window(window), m_RenderDevice(render_system), m_Swapchain(nullptr) {
        reconfigure();
    }

    const std::vector<vk::Image> &Swapchain::getImages() const {
        return m_Images;
    }

    vk::SurfaceFormatKHR Swapchain::getSurfaceFormat() const {
        return m_SurfaceFormat;
    }

    vk::PresentModeKHR Swapchain::getPresentMode() const {
        return m_PresentMode;
    }

    vk::Extent2D Swapchain::getExtent() const {
        return m_Extent;
    }

    void Swapchain::reconfigure() {
        m_RenderDevice->waitDeviceIdle();

        vk::SwapchainCreateInfoKHR createInfo{};
        if (m_Swapchain != nullptr) {
            createInfo.oldSwapchain = m_Swapchain;
        }

        const auto formats    = m_Window->getSurfaceFormats();
        m_SurfaceFormat = formats.front();
        for (const auto &format : formats) {
            if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                m_SurfaceFormat = format;
                break;
            }
        }

        const auto presentModes = m_Window->getPresentModes();
        m_PresentMode     = vk::PresentModeKHR::eFifo;
        for (const auto &mode : presentModes) {
            if (mode == vk::PresentModeKHR::eMailbox) {
                m_PresentMode = mode;
                break;
            }
        }

        const auto caps = m_Window->getSurfaceCapabilities();

        m_Extent = m_Window->getSurfaceCompatibleExtent();

        createInfo.minImageCount    = caps.maxImageCount > 0 ? std::min(caps.minImageCount + 1, caps.maxImageCount) : caps.minImageCount + 1;
        createInfo.clipped          = true;
        createInfo.presentMode      = m_PresentMode;
        createInfo.imageFormat      = m_SurfaceFormat.format;
        createInfo.imageColorSpace  = m_SurfaceFormat.colorSpace;
        createInfo.imageExtent      = m_Extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;
        createInfo.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        createInfo.surface          = *m_Window->surface();
        createInfo.preTransform     = caps.currentTransform;
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;


        vk::raii::SwapchainKHR swc{m_RenderDevice->device(), createInfo};
        std::swap(m_Swapchain, swc);

        m_Images = m_Swapchain.getImages();
        std::cout << "Swapchain created (" << m_Images.size() << " images)" << std::endl;

        m_OnSwapchainReconfigure.publish(m_Images, m_SurfaceFormat, m_Extent);
    }

    void Swapchain::update() {
        if (m_RequiresReconfigure) {
            reconfigure();
            m_RequiresReconfigure = false;
        }
    }

    std::optional<SwapchainFrameInfo> Swapchain::acquireNextFrame(const vk::raii::Semaphore &semaphore) {
        try {
            auto [result, index] = m_Swapchain.acquireNextImage(UINT64_MAX, semaphore);
            switch (result) {
            case vk::Result::eSuccess:
                break;
            case vk::Result::eSuboptimalKHR:
                m_RequiresReconfigure = true;
                break;
            case vk::Result::eErrorOutOfDateKHR:
                m_RequiresReconfigure = true;
                return std::nullopt;
            default:
                vk::detail::throwResultException(result, "vkAcquireNextImageKHR");
            }

            m_CurrentFrameInfo = {.image = m_Images[index], .imageIndex = index, .surfaceFormat = m_SurfaceFormat, .extent = m_Extent};

            return m_CurrentFrameInfo;
        } catch (vk::OutOfDateKHRError& ignored) {
            m_RequiresReconfigure = true;
            return std::nullopt;
        }
    }

    void Swapchain::present(const vk::raii::Semaphore &renderedSignal) {
        vk::PresentInfoKHR pi{};
        pi.setSwapchains(*m_Swapchain);
        pi.setImageIndices(m_CurrentFrameInfo.imageIndex);
        pi.setWaitSemaphores(*renderedSignal);
        try {
            if (const auto res = m_RenderDevice->presentQueue().presentKHR(pi); res == vk::Result::eErrorOutOfDateKHR || res == vk::Result::eSuboptimalKHR) {
                m_RequiresReconfigure = true;
            }
        } catch (vk::OutOfDateKHRError& ignored) {
            m_RequiresReconfigure = true;
        }
    }
} // namespace engine
