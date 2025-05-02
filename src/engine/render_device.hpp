//
// Created by andy on 4/30/2025.
//

#pragma once

#include "spdlog/fmt/bundled/format.h"


#include <vulkan/vulkan_raii.hpp>

#include <vk_mem_alloc.h>

namespace engine {
    enum class QueueType { GRAPHICS, TRANSFER };

    class Allocation {
      public:
        inline Allocation(const VmaAllocation allocation, const VmaAllocator allocator) : allocation(allocation), allocator(allocator) {}
        inline ~Allocation() {
            if (allocation)
                vmaFreeMemory(allocator, allocation);
        }

        inline Allocation(std::nullptr_t) : allocation(nullptr), allocator(nullptr) {}

        Allocation(const Allocation &other)                = delete;
        Allocation(Allocation &&other) noexcept            = default;
        Allocation &operator=(const Allocation &other)     = delete;
        Allocation &operator=(Allocation &&other) noexcept = default;

        void* map() const;
        void unmap() const;

      private:
        VmaAllocation allocation;
        VmaAllocator  allocator;
    };

    struct RawImage {
        vk::raii::Image image;
        Allocation      allocation;
    };

    struct RawBuffer {
        vk::raii::Buffer buffer;
        Allocation       allocation;

        inline RawBuffer(vk::raii::Buffer buffer, Allocation allocation) : buffer(std::move(buffer)), allocation(std::move(allocation)) {}
        inline RawBuffer(std::nullptr_t) : buffer(nullptr), allocation(nullptr) {}

        void write(std::size_t size, const void* data) const;
    };

    struct Allocator {
        VmaAllocator allocator;

        explicit inline Allocator(const VmaAllocator allocator) : allocator(allocator) {}
        inline ~Allocator() { vmaDestroyAllocator(allocator); }

        Allocator(const Allocator &other)                = delete;
        Allocator(Allocator &&other) noexcept            = default;
        Allocator &operator=(const Allocator &other)     = delete;
        Allocator &operator=(Allocator &&other) noexcept = default;

        // ReSharper disable once CppNonExplicitConversionOperator
        inline operator VmaAllocator() const { return allocator; }
    };

    enum class MemoryUsage {
        Auto             = VMA_MEMORY_USAGE_AUTO,
        AutoPreferDevice = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        AutoPreferHost   = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
    };

    class RenderDevice {
      public:
        explicit RenderDevice();
        ~RenderDevice() = default;

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

        std::pair<RawImage, VmaAllocationInfo>  createImage(const vk::ImageCreateInfo &image_create_info, const VmaAllocationCreateInfo &allocation_create_info) const;
        std::pair<RawBuffer, VmaAllocationInfo> createBuffer(const vk::BufferCreateInfo &buffer_create_info, const VmaAllocationCreateInfo &allocation_create_info) const;

        std::pair<RawBuffer, VmaAllocationInfo> createBuffer(
            vk::DeviceSize size, vk::BufferUsageFlags usage, const MemoryUsage &memory_usage, VmaAllocationCreateFlags allocationFlags,
            const std::vector<uint32_t> &queue_families = {}
        ) const;
        std::pair<RawImage, VmaAllocationInfo> createImage(
            vk::Extent3D size, vk::Format format, vk::ImageUsageFlags usage, bool preinitialized, bool linear, const MemoryUsage &memory_usage,
            VmaAllocationCreateFlags allocationFlags, const std::vector<uint32_t> &queue_families = {}
        ) const;

        inline void waitFence(const vk::raii::Fence &fence, const uint64_t timeout = UINT64_MAX) const { [[maybe_unused]] auto _ = m_Device.waitForFences(*fence, true, timeout); };
        inline void resetFence(const vk::raii::Fence &fence) const { m_Device.resetFences(*fence); };

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

        Allocator m_Allocator{nullptr};
    };

    template <>
    [[nodiscard]] vk::raii::CommandBuffers RenderDevice::allocateCommandBuffers<QueueType::GRAPHICS>(uint32_t count) const;

    template <>
    [[nodiscard]] vk::raii::CommandBuffers RenderDevice::allocateCommandBuffers<QueueType::TRANSFER>(uint32_t count) const;

    void imageTransition(
        const vk::raii::CommandBuffer &cmd, vk::Image image, const vk::ImageSubresourceRange &isr,
        std::tuple<vk::ImageLayout, vk::PipelineStageFlagBits2, vk::AccessFlags2, uint32_t> sourceState,
        std::tuple<vk::ImageLayout, vk::PipelineStageFlagBits2, vk::AccessFlags2, uint32_t> destinationState
    );
} // namespace engine
