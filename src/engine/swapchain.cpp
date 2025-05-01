//
// Created by andy on 4/30/25.
//

#include "swapchain.hpp"

#include <iostream>

namespace engine {
    Swapchain::Swapchain(const std::shared_ptr<RenderSystem> &render_system, const std::shared_ptr<Window> &window)
        : m_Window(window), m_RenderSystem(render_system), m_Swapchain(nullptr) {
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
        m_RenderSystem->waitDeviceIdle();

        vk::SwapchainCreateInfoKHR createInfo{};
        if (m_Swapchain != nullptr) {
            createInfo.oldSwapchain = m_Swapchain;
        }

        auto formats    = m_Window->getSurfaceFormats();
        m_SurfaceFormat = formats.front();
        for (const auto &format : formats) {
            if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
                m_SurfaceFormat = format;
                break;
            }
        }

        auto presentModes = m_Window->getPresentModes();
        m_PresentMode     = vk::PresentModeKHR::eFifo;
        for (const auto &mode : presentModes) {
            if (mode == vk::PresentModeKHR::eMailbox) {
                m_PresentMode = mode;
                break;
            }
        }

        auto caps = m_Window->getSurfaceCapabilities();

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


        vk::raii::SwapchainKHR swc{m_RenderSystem->device(), createInfo};
        std::swap(m_Swapchain, swc);

        m_Images = m_Swapchain.getImages();
        std::cout << "Swapchain created (" << m_Images.size() << " images)" << std::endl;
    }

    SwapchainFrameInfo Swapchain::acquireNextFrame(const vk::raii::Semaphore &semaphore) {
        auto r = m_Swapchain.acquireNextImage(UINT64_MAX, semaphore);
        // TODO: process result, find some way to return the error

        return m_CurrentFrameInfo;
    }
} // namespace engine
