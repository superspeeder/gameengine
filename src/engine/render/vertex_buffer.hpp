//
// Created by andy on 5/2/2025.
//

#pragma once

#include "engine/render_device.hpp"

#include <memory>
#include <ranges>
#include <vulkan/vulkan_raii.hpp>

namespace engine {
    enum class VertexBufferStorage {
        Static, // Static vertex buffer storage is gpu-only and copied from cpu. Creating this will make a temporary dynamic buffer and use it to copy.
        Dynamic // Dynamic vertex buffer storage is host-visible + host-coherent
    };

    struct VertexBufferLayout {
        vk::VertexInputBindingDescription2EXT                binding;
        std::vector<vk::VertexInputAttributeDescription2EXT> attributes;
    };

    class VertexBuffer {
      public:
        template <std::ranges::contiguous_range R>
        static inline std::shared_ptr<VertexBuffer> create(const std::shared_ptr<RenderDevice> &device, const VertexBufferStorage storage, R range, const VertexBufferLayout &layout) {
            return std::shared_ptr<VertexBuffer>(new VertexBuffer(device, storage, std::forward<R>(range), layout));
        }

        template <std::ranges::contiguous_range R>
        VertexBuffer(const std::shared_ptr<RenderDevice> &device, const VertexBufferStorage storage, R range, const VertexBufferLayout &layout) : m_Layout(layout) {
            if (storage == VertexBufferStorage::Dynamic) {
                m_Buffer = createHostBuffer(device, std::forward<R &&>(range), false);
            } else {
                auto tempBuffer = createHostBuffer(device, std::forward<R &&>(range), true);

                using range_value_t                     = std::ranges::range_value_t<R>;
                constexpr static std::size_t value_size = sizeof(range_value_t);
                const std::size_t            range_size = std::ranges::size(range) * value_size;

                auto [buffer, _] = device->createBuffer(
                    range_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
                    MemoryUsage::AutoPreferDevice, 0, {}
                );
                m_Buffer = std::move(buffer);

                device->copyBufferToBuffer(tempBuffer, m_Buffer, 0, 0, range_size);
            }
        }

        inline const VertexBufferLayout &layout() const { return m_Layout; };

        void bindAndSetState(const vk::raii::CommandBuffer &cmd, vk::DeviceSize offset = 0);

        void bind(const vk::raii::CommandBuffer &cmd, uint32_t binding, vk::DeviceSize offset = 0);


      private:
        RawBuffer          m_Buffer{nullptr};
        VertexBufferLayout m_Layout;

        template <std::ranges::contiguous_range R>
        static RawBuffer createHostBuffer(const std::shared_ptr<RenderDevice> &device, R range, bool forCopy) {
            using range_value_t = std::ranges::range_value_t<R>;
            static_assert(std::is_standard_layout_v<range_value_t> && "Invalid buffer value type (must be a standard layout type to ensure that it is safe to copy)");
            constexpr static std::size_t value_size = sizeof(range_value_t);
            const std::size_t            range_size = std::ranges::size(range) * value_size;

            auto [buffer, _] = device->createBuffer(
                range_size,
                forCopy ? vk::BufferUsageFlagBits::eTransferSrc
                        : vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst,
                MemoryUsage::AutoPreferHost, forCopy ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT, {}
            );
            buffer.write(range_size, std::ranges::cdata(range));
            return std::move(buffer);
        }
    };

} // namespace engine
