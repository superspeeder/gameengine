//
// Created by andy on 5/2/2025.
//

#include "vertex_buffer.hpp"

namespace engine {
    void VertexBuffer::bindAndSetState(const vk::raii::CommandBuffer &cmd, vk::DeviceSize offset) {
        cmd.setVertexInputEXT(m_Layout.binding, m_Layout.attributes);
        
    }


} // engine