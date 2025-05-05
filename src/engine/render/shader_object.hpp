//
// Created by andy on 5/1/2025.
//

#pragma once

#include "engine/render_device.hpp"

#include <vulkan/vulkan_raii.hpp>

#include <filesystem>
#include <vector>

namespace engine {

    struct ShaderInputLayout {
        std::vector<vk::PushConstantRange>   push_constant_ranges;
        std::vector<vk::DescriptorSetLayout> descriptor_set_layouts;
    };

    struct ShaderInfo {
        vk::ShaderStageFlagBits      stage;
        vk::ShaderStageFlags         nextStage;
        std::string                  name;
        const std::vector<uint32_t> &code;
        const ShaderInputLayout     &sil;
    };

    class Shader;

    struct LinkedShader {
        std::vector<std::unique_ptr<Shader>> shaders;

        explicit LinkedShader(std::vector<std::unique_ptr<Shader>> shaders);

        std::vector<vk::ShaderStageFlagBits> stages;
        std::vector<vk::ShaderEXT>           shader_objects;


        void bindTo(const vk::raii::CommandBuffer &cmd) const;
    };

    class Shader {
      public:
        Shader(const std::shared_ptr<RenderDevice> &render_device, const ShaderInfo &info);

        static std::shared_ptr<LinkedShader> create_linked(
            const std::shared_ptr<RenderDevice> &render_device,
            const std::vector<ShaderInfo>       &shader_infos
        ); // special way to create shaders, doesn't use the standard constructor

        inline const vk::raii::ShaderEXT &handle() const { return m_Shader; }

        static void bindNull(const vk::raii::CommandBuffer &cmd);
        static void setGenericState(const vk::raii::CommandBuffer& cmd);

        static std::vector<uint32_t> load_code(const std::filesystem::path &path);

        inline vk::ShaderStageFlagBits stage() const { return m_Stage; };

        void bindTo(const vk::raii::CommandBuffer &cmd) const;

      private:
        std::shared_ptr<RenderDevice> m_RenderDevice;
        vk::raii::ShaderEXT           m_Shader;
        vk::ShaderStageFlagBits       m_Stage;

        // private so that only `create_linked` can use this since it's used to decompose the `vk::raii::ShaderEXTs` object we create which is the linked shader objects.
        Shader(const std::shared_ptr<RenderDevice> &render_device, vk::raii::ShaderEXT shader, vk::ShaderStageFlagBits stage);
    };


} // namespace engine
