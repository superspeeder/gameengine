//
// Created by andy on 5/5/25.
//

#include "material.hpp"

namespace engine {
    void ShaderInternal_Unlinked::bindTo(const vk::raii::CommandBuffer &cmd) const {
        for (const auto &shader : stages) {
            shader->bindTo(cmd);
        }
    }

    void ShaderInternal_Linked::bindTo(const vk::raii::CommandBuffer &cmd) const {
        linkedShader->bindTo(cmd);
    }

    struct Mat_StageInfo {
        MaterialShaderStage     stage;
        vk::ShaderStageFlagBits next_stage;
    };

    constexpr std::array raster_stages = {
        vk::ShaderStageFlagBits::eVertex,   vk::ShaderStageFlagBits::eTessellationControl, vk::ShaderStageFlagBits::eTessellationEvaluation, vk::ShaderStageFlagBits::eGeometry,
        vk::ShaderStageFlagBits::eFragment,
    };

    MaterialShader::MaterialShader(const std::shared_ptr<RenderDevice> &renderDevice, const std::vector<MaterialShaderStage> &stages) : m_RenderDevice(renderDevice) {
        std::unordered_map<vk::ShaderStageFlagBits, Mat_StageInfo> stageInfos;
        for (const auto &stage : stages) {
            if (stageInfos.contains(stage.stage)) {
                throw std::invalid_argument("MaterialShader::MaterialShader(): Only one shader is allowed per stage");
            }

            stageInfos[stage.stage] = {stage, vk::ShaderStageFlagBits::eAll};
        }

        vk::ShaderStageFlagBits lastIncludedStage = raster_stages.back();
        if (!stageInfos.contains(lastIncludedStage)) {
            throw std::invalid_argument("MaterialShader::MaterialShader(): Missing required stage: " + vk::to_string(lastIncludedStage));
        }

        for (int i = raster_stages.size() - 2; i >= 0; i--) {
            if (auto stage = raster_stages[i]; stageInfos.contains(stage)) {
                stageInfos[stage].next_stage = lastIncludedStage;
                lastIncludedStage            = stage;
            }
        }

        if (lastIncludedStage != raster_stages.front()) {
            throw std::invalid_argument("MaterialShader::MaterialShader(): Missing required stage: " + vk::to_string(raster_stages.front()));
        }

        std::vector<ShaderInfo> shaderInfos;
        for (const auto &[_, stage] : stageInfos) {
            shaderInfos.push_back(
                ShaderInfo{
                    .stage     = stage.stage.stage,
                    .nextStage = stage.next_stage,
                    .name      = stage.stage.entryPoint.c_str(),
                    .code      = Shader::load_code(stage.stage.path),
                    .sil       = stage.stage.sil,
                }
            );
        }

        m_Shader = std::make_unique<ShaderInternal_Linked>(Shader::create_linked(m_RenderDevice, shaderInfos));
    }

    MaterialShader::MaterialShader(const std::shared_ptr<RenderDevice> &renderDevice, const std::vector<std::shared_ptr<Shader>> &stages)
        : m_RenderDevice(renderDevice), m_Shader(std::make_unique<ShaderInternal_Unlinked>(stages)) {}

    void MaterialShader::bindTo(const vk::raii::CommandBuffer &cmd) const {
        m_Shader->bindTo(cmd);
    }
} // namespace engine
