//
// Created by andy on 4/30/2025.
//

#include "render_system.hpp"

#include "GLFW/glfw3.h"

#include <iostream>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace engine {
    RenderSystem::RenderSystem() {
        {
            VULKAN_HPP_DEFAULT_DISPATCHER.init();

            vk::ApplicationInfo app_info{};
            app_info.apiVersion = vk::ApiVersion14;

            vk::InstanceCreateInfo create_info{};
            create_info.setPApplicationInfo(&app_info);

            std::vector<const char *> instance_extensions{};

            uint32_t     extension_count     = 0;
            const char **required_extensions = glfwGetRequiredInstanceExtensions(&extension_count);
            instance_extensions.assign(required_extensions, required_extensions + extension_count);

            create_info.setPEnabledExtensionNames(instance_extensions);

            m_Instance = vk::raii::Instance(m_Context, create_info);
            VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Instance);
        }

        {
            m_PhysicalDevice = m_Instance.enumeratePhysicalDevices()[0];
        }

        {
            std::vector<const char *> device_extensions{VK_EXT_SHADER_OBJECT_EXTENSION_NAME, VK_KHR_SWAPCHAIN_EXTENSION_NAME};

            auto qfps = m_PhysicalDevice.getQueueFamilyProperties();
            for (uint32_t i = 0; i < qfps.size(); ++i) {
                const auto &qfp = qfps[i];
                if (m_GraphicsQueueFamily == UINT32_MAX && qfp.queueFlags & vk::QueueFlagBits::eGraphics) {
                    m_GraphicsQueueFamily = i;
                    if (glfwGetPhysicalDevicePresentationSupport(*m_Instance, *m_PhysicalDevice, i)) {
                        m_PresentQueueFamily = i;
                    }
                }

                if (m_PresentQueueFamily == UINT32_MAX && glfwGetPhysicalDevicePresentationSupport(*m_Instance, *m_PhysicalDevice, i)) {
                    m_PresentQueueFamily = i;
                }

                if (m_TransferQueueFamily == UINT32_MAX && !(qfp.queueFlags & vk::QueueFlagBits::eGraphics) && !(qfp.queueFlags & vk::QueueFlagBits::eCompute) &&
                    (qfp.queueFlags & vk::QueueFlagBits::eTransfer)) {
                    m_TransferQueueFamily = i;
                }

                if (m_GraphicsQueueFamily != UINT32_MAX && m_TransferQueueFamily != UINT32_MAX && m_PresentQueueFamily != UINT32_MAX) {
                    break;
                }
            }

            vk::PhysicalDeviceFeatures2 f2{};
            f2.features.wideLines          = true;
            f2.features.largePoints        = true;
            f2.features.geometryShader     = true;
            f2.features.tessellationShader = true;
            f2.features.sparseBinding      = true;
            f2.features.multiDrawIndirect  = true;
            f2.features.fillModeNonSolid   = true;

            vk::PhysicalDeviceVulkan11Features v11f{};
            v11f.shaderDrawParameters = true;

            vk::PhysicalDeviceVulkan12Features v12f{};
            v12f.timelineSemaphore      = true;
            v12f.runtimeDescriptorArray = true;
            v12f.drawIndirectCount      = true;

            vk::PhysicalDeviceVulkan13Features v13f{};
            v13f.synchronization2   = true;
            v13f.dynamicRendering   = true;
            v13f.inlineUniformBlock = true;

            vk::PhysicalDeviceVulkan14Features v14f{};
            v14f.pushDescriptor = true;

            vk::PhysicalDeviceShaderObjectFeaturesEXT sof{};
            sof.shaderObject = true;

            f2.pNext   = &v11f;
            v11f.pNext = &v12f;
            v12f.pNext = &v13f;
            v13f.pNext = &v14f;
            v14f.pNext = &sof;

            std::array<float, 1>                   queuePriorities = {1.0f};
            std::vector<vk::DeviceQueueCreateInfo> qcis{};
            qcis.emplace_back(vk::DeviceQueueCreateInfo({}, m_GraphicsQueueFamily, queuePriorities));

            if (m_TransferQueueFamily == UINT32_MAX) {
                m_TransferQueueFamily = m_GraphicsQueueFamily;
            } else {
                qcis.emplace_back(vk::DeviceQueueCreateInfo({}, m_TransferQueueFamily, queuePriorities));
            }

            if (m_PresentQueueFamily != m_GraphicsQueueFamily && m_PresentQueueFamily != m_TransferQueueFamily) {
                qcis.emplace_back(vk::DeviceQueueCreateInfo({}, m_PresentQueueFamily, queuePriorities));
            }

            vk::DeviceCreateInfo create_info{};
            create_info.pNext = &f2;
            create_info.setQueueCreateInfos(qcis);
            create_info.setPEnabledExtensionNames(device_extensions);
            m_Device = vk::raii::Device(m_PhysicalDevice, create_info);
            VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_Device);

            m_GraphicsQueue = m_Device.getQueue(m_GraphicsQueueFamily, 0);
            m_PresentQueue  = m_Device.getQueue(m_PresentQueueFamily, 0);
            m_TransferQueue = m_Device.getQueue(m_TransferQueueFamily, 0);
        }

        {
            m_GraphicsCommandPool = vk::raii::CommandPool(m_Device, {vk::CommandPoolCreateFlagBits::eResetCommandBuffer, m_GraphicsQueueFamily});
            m_TransferCommandPool = vk::raii::CommandPool(m_Device, {vk::CommandPoolCreateFlagBits::eResetCommandBuffer, m_TransferQueueFamily});
        }
    }

    vk::raii::Semaphore RenderSystem::createSemaphore() const {
        return vk::raii::Semaphore(m_Device, {});
    }

    void RenderSystem::waitDeviceIdle() const {
        m_Device.waitIdle();
    }

    template <>
    vk::raii::CommandBuffers RenderSystem::allocateCommandBuffers<QueueType::GRAPHICS>(uint32_t count) const {
        return vk::raii::CommandBuffers(m_Device, {*m_GraphicsCommandPool, vk::CommandBufferLevel::ePrimary, count});
    }

    template <>
    vk::raii::CommandBuffers RenderSystem::allocateCommandBuffers<QueueType::TRANSFER>(uint32_t count) const {
        return vk::raii::CommandBuffers(m_Device, {*m_TransferCommandPool, vk::CommandBufferLevel::ePrimary, count});
    }
} // namespace engine
