//
// Created by andy on 4/30/2025.
//

#define VMA_IMPLEMENTATION
#ifdef WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "render_device.hpp"

#include "GLFW/glfw3.h"

#include <iostream>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE;

namespace engine {
    std::vector<const char *> availableExtensions(const std::vector<const char *> &requestedExtensions, const vk::raii::PhysicalDevice &physicalDevice) {
        auto                      available = physicalDevice.enumerateDeviceExtensionProperties();
        std::vector<const char *> extensions;
        for (const char *ext : requestedExtensions) {
            for (const auto &eprop : available) {
                if (strcmp(ext, eprop.extensionName.data()) == 0) {
                    extensions.push_back(ext);
                }
            }
        }

        return extensions;
    }

    void *Allocation::map() const {
        void *m;
        vmaMapMemory(allocator, allocation, &m);
        return m;
    }

    void Allocation::unmap() const {
        vmaUnmapMemory(allocator, allocation);
    }

    void RawBuffer::write(const std::size_t size, const void *data) const {
        void *dst = allocation->map();
        std::memcpy(dst, data, size);
        allocation->unmap();
    }

    RenderDevice::RenderDevice() {
        VmaAllocatorCreateFlags allocatorFlags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT | VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT;

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
            v12f.bufferDeviceAddress    = true;

            vk::PhysicalDeviceVulkan13Features v13f{};
            v13f.synchronization2   = true;
            v13f.dynamicRendering   = true;
            v13f.inlineUniformBlock = true;
            v13f.maintenance4 = true;

            vk::PhysicalDeviceVulkan14Features v14f{};
            v14f.maintenance5 = true;

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

            std::vector<const char *> wanted_extensions = {
                VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
                VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
                VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
                VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME,
            };

#ifdef WIN32
            wanted_extensions.push_back("VK_KHR_external_memory_win32");
#endif

            for (auto available = availableExtensions(wanted_extensions, m_PhysicalDevice); const auto &ext : available) {
                device_extensions.push_back(ext);
                if (strcmp(ext, VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME) == 0) {
                    allocatorFlags |= VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT;
                }
                if (strcmp(ext, VK_KHR_BIND_MEMORY_2_EXTENSION_NAME) == 0) {
                    allocatorFlags |= VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT;
                }
                if (strcmp(ext, VK_EXT_MEMORY_BUDGET_EXTENSION_NAME) == 0) {
                    allocatorFlags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
                }
                if (strcmp(ext, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) == 0) {
                    allocatorFlags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
                }
                if (strcmp(ext, "VK_KHR_external_memory_win32") == 0) {
                    allocatorFlags |= VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT;
                }
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

        {
            VmaVulkanFunctions vkf{};
            vkf.vkGetInstanceProcAddr = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetInstanceProcAddr;
            vkf.vkGetDeviceProcAddr   = VULKAN_HPP_DEFAULT_DISPATCHER.vkGetDeviceProcAddr;

            VmaAllocatorCreateInfo aci{};
            aci.instance         = *m_Instance;
            aci.physicalDevice   = *m_PhysicalDevice;
            aci.device           = *m_Device;
            aci.flags            = allocatorFlags;
            aci.vulkanApiVersion = vk::ApiVersion14;
            aci.pVulkanFunctions = &vkf;

            vmaCreateAllocator(&aci, &m_Allocator.allocator);
        }
    }

    vk::raii::Semaphore RenderDevice::createSemaphore() const {
        return vk::raii::Semaphore(m_Device, vk::SemaphoreCreateInfo{});
    }

    void RenderDevice::waitDeviceIdle() const {
        m_Device.waitIdle();
    }

    std::pair<RawImage, VmaAllocationInfo> RenderDevice::createImage(const vk::ImageCreateInfo &image_create_info, const VmaAllocationCreateInfo &allocation_create_info) const {
        VmaAllocation           alloc;
        VmaAllocationInfo       ai;
        VkImage                 img;
        const VkImageCreateInfo ici = image_create_info;

        const auto result = static_cast<vk::Result>(vmaCreateImage(m_Allocator, &ici, &allocation_create_info, &img, &alloc, &ai));
        vk::detail::resultCheck(result, "vmaCreateImage");
        return std::pair(RawImage(vk::raii::Image(m_Device, img), std::make_unique<Allocation>(alloc, m_Allocator)), ai);
    }

    std::pair<RawBuffer, VmaAllocationInfo>
    RenderDevice::createBuffer(const vk::BufferCreateInfo &buffer_create_info, const VmaAllocationCreateInfo &allocation_create_info) const {
        VmaAllocation            alloc;
        VmaAllocationInfo        ai;
        VkBuffer                 buf;
        const VkBufferCreateInfo bci = buffer_create_info;

        const auto result = static_cast<vk::Result>(vmaCreateBuffer(m_Allocator, &bci, &allocation_create_info, &buf, &alloc, &ai));
        vk::detail::resultCheck(result, "vmaCreateBuffer");

        return std::pair(RawBuffer(vk::raii::Buffer(m_Device, buf), std::make_unique<Allocation>(alloc, m_Allocator)), ai);
    }

    std::pair<RawBuffer, VmaAllocationInfo> RenderDevice::createBuffer(
        const vk::DeviceSize size, const vk::BufferUsageFlags usage, const MemoryUsage &memory_usage, VmaAllocationCreateFlags allocationFlags,
        const std::vector<uint32_t> &queue_families
    ) const {
        vk::BufferCreateInfo bci{};
        bci.size  = size;
        bci.usage = usage;

        if (queue_families.empty()) {
            bci.sharingMode = vk::SharingMode::eExclusive;
        } else {
            bci.sharingMode = vk::SharingMode::eConcurrent;
            bci.setQueueFamilyIndices(queue_families);
        }

        VmaAllocationCreateInfo aci{};
        aci.flags = allocationFlags;
        aci.usage = static_cast<VmaMemoryUsage>(memory_usage);

        return std::move(createBuffer(bci, aci));
    }

    std::pair<RawImage, VmaAllocationInfo> RenderDevice::createImage(
        const vk::Extent3D size, const vk::Format format, const vk::ImageUsageFlags usage, const bool preinitialized, const bool linear, const MemoryUsage &memory_usage,
        const VmaAllocationCreateFlags allocationFlags, const std::vector<uint32_t> &queue_families
    ) const {
        vk::ImageCreateInfo ici{};
        ici.format        = format;
        ici.extent        = size;
        ici.arrayLayers   = 1;
        ici.mipLevels     = 1;
        ici.usage         = usage;
        ici.samples       = vk::SampleCountFlagBits::e1;
        ici.initialLayout = preinitialized ? vk::ImageLayout::ePreinitialized : vk::ImageLayout::eUndefined;
        ici.tiling        = linear ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal;

        if (queue_families.empty()) {
            ici.sharingMode = vk::SharingMode::eExclusive;
        } else {
            ici.sharingMode = vk::SharingMode::eConcurrent;
            ici.setQueueFamilyIndices(queue_families);
        }

        VmaAllocationCreateInfo aci{};
        aci.flags = allocationFlags;
        aci.usage = static_cast<VmaMemoryUsage>(memory_usage);

        return createImage(ici, aci);
    }

    void RenderDevice::copyBufferToBuffer(
        const RawBuffer &srcBuffer, const RawBuffer &dstBuffer, const vk::DeviceSize srcOffset, const vk::DeviceSize dstOffset, vk::DeviceSize size
    ) const {
        auto fence = createFence();
        singleTimeCommands<QueueType::TRANSFER>(
            [&](const vk::raii::CommandBuffer &cmd) { cmd.copyBuffer(*srcBuffer.buffer, *dstBuffer.buffer, vk::BufferCopy(srcOffset, dstOffset, size)); }, fence
        );
        waitFence(fence);
    }

    template <>
    vk::raii::CommandBuffers RenderDevice::allocateCommandBuffers<QueueType::GRAPHICS>(uint32_t count) const {
        return vk::raii::CommandBuffers(m_Device, {*m_GraphicsCommandPool, vk::CommandBufferLevel::ePrimary, count});
    }

    template <>
    vk::raii::CommandBuffers RenderDevice::allocateCommandBuffers<QueueType::TRANSFER>(uint32_t count) const {
        return vk::raii::CommandBuffers(m_Device, {*m_TransferCommandPool, vk::CommandBufferLevel::ePrimary, count});
    }

    void imageTransition(
        const vk::raii::CommandBuffer &cmd, const vk::Image image, const vk::ImageSubresourceRange &isr,
        std::tuple<vk::ImageLayout, vk::PipelineStageFlagBits2, vk::AccessFlags2, uint32_t> sourceState,
        std::tuple<vk::ImageLayout, vk::PipelineStageFlagBits2, vk::AccessFlags2, uint32_t> destinationState
    ) {
        vk::ImageMemoryBarrier2 imb{};
        imb.image                                        = image;
        imb.subresourceRange                             = isr;
        auto [srcLayout, srcStage, srcAccess, srcFamily] = sourceState;
        auto [dstLayout, dstStage, dstAccess, dstFamily] = destinationState;
        imb.srcAccessMask                                = srcAccess;
        imb.dstAccessMask                                = dstAccess;
        imb.srcQueueFamilyIndex                          = srcFamily;
        imb.dstQueueFamilyIndex                          = dstFamily;
        imb.srcStageMask                                 = srcStage;
        imb.dstStageMask                                 = dstStage;
        imb.oldLayout                                    = srcLayout;
        imb.newLayout                                    = dstLayout;
        cmd.pipelineBarrier2({{}, {}, {}, imb});
    }
} // namespace engine
