//
// Created by andy on 5/2/2025.
//

#include "vertex_buffer.hpp"

namespace engine {
    void VertexBuffer::bindAndSetState(const vk::raii::CommandBuffer &cmd, vk::DeviceSize offset) {
        cmd.setVertexInputEXT(m_Layout.binding, m_Layout.attributes);
        bind(cmd, m_Layout.binding.binding, offset);
    }

    void VertexBuffer::bind(const vk::raii::CommandBuffer &cmd, uint32_t binding, vk::DeviceSize offset) {
        cmd.bindVertexBuffers(binding, {*m_Buffer.buffer}, {offset});
    }
} // engine
