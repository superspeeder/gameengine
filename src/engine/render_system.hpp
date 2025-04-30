//
// Created by andy on 4/30/2025.
//

#pragma once

#include <vulkan/vulkan_raii.hpp>
namespace engine {
    class RenderSystem {
      public:
        RenderSystem();
        ~RenderSystem();

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

      private:
        vk::raii::Context        m_Context;
        vk::raii::Instance       m_Instance;
        vk::raii::PhysicalDevice m_PhysicalDevice;
        vk::raii::Device         m_Device;

        uint32_t m_GraphicsQueueFamily;
        uint32_t m_PresentQueueFamily;
        uint32_t m_TransferQueueFamily;

        vk::raii::Queue m_GraphicsQueue;
        vk::raii::Queue m_PresentQueue;
        vk::raii::Queue m_TransferQueue;
    };
} // namespace engine
