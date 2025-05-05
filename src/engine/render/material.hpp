#pragma once

#include "engine/render/shader_object.hpp"
#include "engine/render_device.hpp"

#include <filesystem>
#include <vulkan/vulkan_raii.hpp>

namespace engine {
    struct MaterialShaderStage {
        std::filesystem::path   path;
        vk::ShaderStageFlagBits stage;
        std::string             entryPoint;

        ShaderInputLayout sil;
    };

    struct UnlinkedMaterialShaderStages {
        std::vector<std::shared_ptr<Shader>> stages;
    };

    class ShaderInternal {
      public:
        virtual ~ShaderInternal()                                     = default;
        virtual void bindTo(const vk::raii::CommandBuffer &cmd) const = 0;
    };

    class ShaderInternal_Unlinked : public ShaderInternal {
      public:
        explicit ShaderInternal_Unlinked(const std::vector<std::shared_ptr<Shader>> &stages) : stages(stages) {}

        void bindTo(const vk::raii::CommandBuffer &cmd) const override;

      private:
        std::vector<std::shared_ptr<Shader>> stages;
    };

    class ShaderInternal_Linked : public ShaderInternal {
      public:
        explicit ShaderInternal_Linked(const std::shared_ptr<LinkedShader> &linked_shader) : linkedShader(linked_shader) {}

        void bindTo(const vk::raii::CommandBuffer &cmd) const override;

      private:
        std::shared_ptr<LinkedShader> linkedShader;
    };

    class MaterialShader {
      public:
        MaterialShader(const std::shared_ptr<RenderDevice> &renderDevice, const std::vector<MaterialShaderStage> &stages);
        MaterialShader(const std::shared_ptr<RenderDevice> &renderDevice, const std::vector<std::shared_ptr<Shader>> &stages);
        ~MaterialShader() = default;

        void bindTo(const vk::raii::CommandBuffer &cmd) const;

        inline static std::shared_ptr<MaterialShader> create_shared(const std::shared_ptr<RenderDevice> &renderDevice, const std::vector<MaterialShaderStage> &stages) {
            return std::make_shared<MaterialShader>(renderDevice, stages);
        }

        inline static std::shared_ptr<MaterialShader> create_shared(const std::shared_ptr<RenderDevice> &renderDevice, const std::vector<std::shared_ptr<Shader>> &stages) {
            return std::make_shared<MaterialShader>(renderDevice, stages);
        }

      private:
        std::shared_ptr<RenderDevice>   m_RenderDevice;
        std::unique_ptr<ShaderInternal> m_Shader;
    };

} // namespace engine
