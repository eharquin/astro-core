//
// Created by eharquin on 12/20/25.
//
#pragma once

#include <core/rendering/vulkan/Swapchain.hpp>

#include <core/utils/FileUtils.hpp>

#include <unordered_map>

namespace Core::Rendering::Vulkan {

	constexpr uint32_t MAX_TEXTURES = 256;

	struct PipelineConfig {
		uint32_t depthTestEnable = vk::True;
		uint32_t depthWriteEnable = vk::True;
		vk::CompareOp depthCompareOp = vk::CompareOp::eLess;

		uint32_t blendEnable = vk::False;
		vk::PolygonMode polygonMode = vk::PolygonMode::eFill;
		vk::CullModeFlags cullMode = vk::CullModeFlagBits::eBack;
		vk::FrontFace frontFace = vk::FrontFace::eCounterClockwise;

		vk::SampleCountFlagBits rasterizationSamples = vk::SampleCountFlagBits::e1;
	};

	class PipelineManager {
	public:
		PipelineManager(Context& ctx, Swapchain& sc);

		void createPipeline(const std::string &name, const std::string &filePath,
		                    const PipelineConfig &config = PipelineConfig());
		vk::raii::PipelineLayout& pipelineLayout() { return _pipelineLayout; }
		vk::raii::DescriptorSetLayout& descriptorSetLayout() { return _descriptorSetLayout; }

		vk::raii::Pipeline& get(const std::string& name) {
			auto it = _pipelines.find(name);
			if (it == _pipelines.end()) {
				throw std::runtime_error("Pipeline not found: " + name);
			}
			return it->second;
		}


	private:
		void createDescriptorSetLayout();
		void createPipelineLayout();

		Context& _context;
		Swapchain& _swapchain;

		vk::raii::DescriptorSetLayout _descriptorSetLayout = nullptr;
		vk::raii::PipelineLayout _pipelineLayout = nullptr;

		std::unordered_map<std::string, vk::raii::Pipeline> _pipelines;

		vk::raii::ShaderModule createShaderModule(vk::raii::Device& device, const std::vector<char>& code);
	};
}
