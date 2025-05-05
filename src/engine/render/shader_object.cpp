//
// Created by andy on 5/1/2025.
//

#include "shader_object.hpp"

#include <fstream>

namespace engine {
    LinkedShader::LinkedShader(std::vector<std::unique_ptr<Shader>> shaders) : shaders(std::move(shaders)) {
        stages.resize(this->shaders.size());
        shader_objects.resize(this->shaders.size());
        uint32_t i = 0;
        for (const auto &shader : this->shaders) {
            stages[i]           = shader->stage();
            shader_objects[i++] = *shader->handle();
        }
    }

    void LinkedShader::bindTo(const vk::raii::CommandBuffer &cmd) const {
        cmd.bindShadersEXT(stages, shader_objects);
    }

    Shader::Shader(const std::shared_ptr<RenderDevice> &render_device, const ShaderInfo &info)
        : m_RenderDevice(render_device), m_Shader(
                                             m_RenderDevice->device(),
                                             vk::ShaderCreateInfoEXT(
                                                 {}, info.stage, info.nextStage, vk::ShaderCodeTypeEXT::eSpirv, vk::ArrayProxyNoTemporaries<const uint32_t>(info.code),
                                                 info.name.c_str(), info.sil.descriptor_set_layouts, info.sil.push_constant_ranges
                                             )
                                         ),
          m_Stage(info.stage) {}

    std::shared_ptr<LinkedShader> Shader::create_linked(const std::shared_ptr<RenderDevice> &render_device, const std::vector<ShaderInfo> &shader_infos) {
        std::vector<vk::ShaderCreateInfoEXT> shader_create_infos;
        for (const auto &[stage, nextStage, name, code, sil] : shader_infos) {
            shader_create_infos.push_back(
                vk::ShaderCreateInfoEXT(
                    vk::ShaderCreateFlagBitsEXT::eLinkStage, stage, nextStage, vk::ShaderCodeTypeEXT::eSpirv, vk::ArrayProxyNoTemporaries<const uint32_t>(code), name.c_str(),
                    sil.descriptor_set_layouts, sil.push_constant_ranges
                )
            );
        }

        auto                                 shaders = vk::raii::ShaderEXTs(render_device->device(), shader_create_infos);
        uint32_t                             i       = 0;
        std::vector<std::unique_ptr<Shader>> shaders_;
        for (auto &shader : shaders) {
            shaders_.emplace_back(new Shader(render_device, std::move(shader), shader_infos[i++].stage));
        }

        return std::make_shared<LinkedShader>(std::move(shaders_));
    }

    void Shader::bindNull(const vk::raii::CommandBuffer &cmd) {
        cmd.bindShadersEXT(
            {vk::ShaderStageFlagBits::eVertex, vk::ShaderStageFlagBits::eFragment, vk::ShaderStageFlagBits::eTessellationControl, vk::ShaderStageFlagBits::eTessellationEvaluation,
             vk::ShaderStageFlagBits::eGeometry},
            {VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE}
        );
    }

    void Shader::setGenericState(const vk::raii::CommandBuffer &cmd) {
        cmd.setRasterizerDiscardEnable(false);

        cmd.setVertexInputEXT({}, {});
        cmd.setPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
        cmd.setPrimitiveRestartEnable(false);

        cmd.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);
        cmd.setSampleMaskEXT(vk::SampleCountFlagBits::e1, {~0U});
        cmd.setAlphaToCoverageEnableEXT(false);
        cmd.setAlphaToOneEnableEXT(false);
        cmd.setPolygonModeEXT(vk::PolygonMode::eFill);
        cmd.setLineWidth(1.0f);
        cmd.setCullMode(vk::CullModeFlagBits::eBack);
        cmd.setFrontFace(vk::FrontFace::eClockwise);
        cmd.setDepthWriteEnable(false);
        cmd.setDepthTestEnable(false);
        cmd.setDepthBoundsTestEnable(false);
        cmd.setDepthBiasEnable(false);
        cmd.setDepthClampEnableEXT(false);
        cmd.setStencilTestEnable(false);

        cmd.setLogicOpEnableEXT(false);
        cmd.setColorBlendEnableEXT(0, true);
        cmd.setColorWriteMaskEXT(0, vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
        cmd.setColorBlendEquationEXT(0, vk::ColorBlendEquationEXT(vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd));

        constexpr static float blend_constants[4] = {0.0f,0.0f,0.0f,0.0f};
        cmd.setBlendConstants(blend_constants);
    }

    std::vector<uint32_t> Shader::load_code(const std::filesystem::path &path) {
        if (std::ifstream file(path, std::ios::binary | std::ios::ate); file.is_open()) {
            const auto size = file.tellg();
            file.seekg(0, std::ios::beg);
            std::vector<uint32_t> code(size / sizeof(uint32_t));
            file.read(reinterpret_cast<char *>(code.data()), static_cast<std::streamsize>(size) - (size % sizeof(uint32_t)));
            file.close();
            return code;
        }
        throw std::invalid_argument("Failed to open file " + path.string());
    }

    void Shader::bindTo(const vk::raii::CommandBuffer &cmd) const {
        cmd.bindShadersEXT(m_Stage, *m_Shader);
    }

    Shader::Shader(const std::shared_ptr<RenderDevice> &render_device, vk::raii::ShaderEXT shader, vk::ShaderStageFlagBits stage)
        : m_RenderDevice(render_device), m_Shader(std::move(shader)), m_Stage(stage) {}
} // namespace engine
