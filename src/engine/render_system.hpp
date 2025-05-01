//
// Created by andy on 4/30/2025.
//

#pragma once

#include <vulkan/vulkan_raii.hpp>
namespace engine {
    enum class QueueType { GRAPHICS, TRANSFER };

    class RenderSystem {
      public:
        RenderSystem();
        ~RenderSystem() = default;

        [[nodiscard]] const vk::raii::Context        &context() const { return m_Context; }
        [[nodiscard]] const vk::raii::Instance       &instance() const { return m_Instance; }
        [[nodiscard]] const vk::raii::PhysicalDevice &physicalDevice() const { return m_PhysicalDevice; }
        [[nodiscard]] const vk::raii::Device         &device() const { return m_Device; }
        [[nodiscard]] uint32_t                        graphicsQueueFamily() const { return m_GraphicsQueueFamily; }
        [[nodiscard]] uint32_t                        presentQueueFamily() const { return m_PresentQueueFamily; }
        [[nodiscard]] uint32_t                        transferQueueFamily() const { return m_TransferQueueFamily; }
        [[nodiscard]] const vk::raii::Queue          &graphicsQueue() const { return m_GraphicsQueue; }
        [[nodiscard]] const vk::raii::Queue          &presentQueue() const { return m_PresentQueue; }
        [[nodiscard]] const vk::raii::Queue          &transferQueue() const { return m_TransferQueue; }
        [[nodiscard]] const vk::raii::CommandPool    &graphicsCommandPool() const { return m_GraphicsCommandPool; }
        [[nodiscard]] const vk::raii::CommandPool    &transferCommandPool() const { return m_TransferCommandPool; }

        [[nodiscard]] inline vk::raii::Fence createFence(bool signaled = false) const {
            return vk::raii::Fence(m_Device, {signaled ? vk::FenceCreateFlagBits::eSignaled : vk::FenceCreateFlags{}});
        }

        [[nodiscard]] vk::raii::Semaphore createSemaphore() const;

        template <QueueType QT>
        [[nodiscard]] vk::raii::CommandBuffers allocateCommandBuffers(uint32_t count) const = delete;

        void waitDeviceIdle() const;

      private:
        vk::raii::Context        m_Context;
        vk::raii::Instance       m_Instance{nullptr};
        vk::raii::PhysicalDevice m_PhysicalDevice{nullptr};
        vk::raii::Device         m_Device{nullptr};

        uint32_t m_GraphicsQueueFamily{UINT32_MAX};
        uint32_t m_PresentQueueFamily{UINT32_MAX};
        uint32_t m_TransferQueueFamily{UINT32_MAX};

        vk::raii::Queue m_GraphicsQueue{nullptr};
        vk::raii::Queue m_PresentQueue{nullptr};
        vk::raii::Queue m_TransferQueue{nullptr};

        vk::raii::CommandPool m_GraphicsCommandPool{nullptr};
        vk::raii::CommandPool m_TransferCommandPool{nullptr};
    };

    template <>
    [[nodiscard]] vk::raii::CommandBuffers RenderSystem::allocateCommandBuffers<QueueType::GRAPHICS>(uint32_t count) const;

    template <>
    [[nodiscard]] vk::raii::CommandBuffers RenderSystem::allocateCommandBuffers<QueueType::TRANSFER>(uint32_t count) const;

} // namespace engine
