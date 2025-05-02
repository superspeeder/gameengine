//
// Created by andy on 5/2/2025.
//

#pragma once

#include "engine/render_device.hpp"


#include <ranges>
#include <vulkan/vulkan_raii.hpp>

namespace engine {
    enum class VertexBufferStorage {
        Static, // Static vertex buffer storage is gpu-only and copied from cpu. Creating this will make a temporary dynamic buffer and use it to copy.
        Dynamic // Dynamic vertex buffer storage is host-visible + host-coherent
    };

    class VertexBuffer {
      public:
        template<std::ranges::contiguous_range R>
        VertexBuffer(const std::shared_ptr<RenderDevice>& device, const VertexBufferStorage storage, R&& range) {
            if (storage == VertexBufferStorage::Dynamic) {
                m_Buffer = createHostBuffer(device, std::forward<R&&>(range), false);
            }
        }
      private:
        RawBuffer m_Buffer{nullptr};

        template<std::ranges::contiguous_range R>
        static RawBuffer createHostBuffer(const std::shared_ptr<RenderDevice>& device, R&& range, bool forCopy) {
            using range_value_t = std::ranges::range_value_t<R>;
            static_assert(std::is_standard_layout_v<range_value_t> && "Invalid buffer value type (must be a standard layout type to ensure that it is safe to copy)");
            constexpr static std::size_t value_size = sizeof(range_value_t);
            const std::size_t range_size = std::ranges::size(range) * value_size;

            auto [buffer, _] = device->createBuffer(range_size, forCopy ? vk::BufferUsageFlagBits::eTransferSrc : vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, MemoryUsage::AutoPreferHost, forCopy ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT, {});
            buffer.write(range_size, std::ranges::cdata(range));
        }
    };

} // namespace engine
