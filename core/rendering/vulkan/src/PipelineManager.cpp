//
// Created by eharquin on 12/20/25.
//

#include <core/rendering/vulkan/PipelineManager.hpp>
#include <core/rendering/vulkan/Vertex.hpp>

namespace Core::Rendering::Vulkan {
	PipelineManager::PipelineManager(Context &ctx, Swapchain &sc)
		: _context(ctx), _swapchain(sc) {
		createDescriptorSetLayout();
		createPipelineLayout();
	}

	void PipelineManager::createPipeline(const std::string &name, const std::vector<char> &code,
	                                     const PipelineConfig &config) {

		vk::raii::Device& device = _context.device();
		vk::Format colorFormat = _swapchain.colorFormat();
		vk::Format depthFormat = _swapchain.depthFormat();

		const vk::raii::ShaderModule shaderModule = createShaderModule(device, code);

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
			.stage = vk::ShaderStageFlagBits::eVertex,
			.module = shaderModule,
			.pName = "vertMain"
		};

		vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
			.stage = vk::ShaderStageFlagBits::eFragment,
			.module = shaderModule,
			.pName = "fragMain"
		};

		vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		auto bindingDescription = Vulkan::Vertex::getBindingDescription();
		auto attributeDescriptions = Vulkan::Vertex::getAttributeDescriptions();
		vk::PipelineVertexInputStateCreateInfo vertexInputInfo {
			.vertexBindingDescriptionCount =1,
			.pVertexBindingDescriptions = &bindingDescription,
			.vertexAttributeDescriptionCount = attributeDescriptions.size(),
			.pVertexAttributeDescriptions = attributeDescriptions.data()
		};

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly{.topology = vk::PrimitiveTopology::eTriangleList};
		vk::PipelineViewportStateCreateInfo      viewportState{.viewportCount = 1, .scissorCount = 1};

		vk::PipelineRasterizationStateCreateInfo rasterizer{
			.depthClampEnable = vk::False,
			.rasterizerDiscardEnable = vk::False,
			.polygonMode = config.polygonMode,
			.cullMode = config.cullMode,
			.frontFace = config.frontFace,
			.depthBiasEnable = vk::False,
			.depthBiasSlopeFactor = 1.0f,
			.lineWidth = 1.0f};

		vk::PipelineMultisampleStateCreateInfo multisampling{
			.rasterizationSamples = config.rasterizationSamples,
			.sampleShadingEnable = vk::False};

		vk::PipelineColorBlendAttachmentState colorBlendAttachment{
			.blendEnable    = config.blendEnable,
			.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

		vk::PipelineColorBlendStateCreateInfo colorBlending{
			.logicOpEnable = vk::False,
			.logicOp = vk::LogicOp::eCopy,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment};

		std::vector dynamicStates = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor};

		vk::PipelineDynamicStateCreateInfo dynamicState{
			.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()};

		vk::PipelineDepthStencilStateCreateInfo depthStencil{
			.depthTestEnable       = config.depthTestEnable,
			.depthWriteEnable      = config.depthWriteEnable,
			.depthCompareOp        = config.depthCompareOp,
			.depthBoundsTestEnable = vk::False,
			.stencilTestEnable     = vk::False};

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
			{.stageCount          = 2,
			 .pStages             = shaderStages,
			 .pVertexInputState   = &vertexInputInfo,
			 .pInputAssemblyState = &inputAssembly,
			 .pViewportState      = &viewportState,
			 .pRasterizationState = &rasterizer,
			 .pMultisampleState   = &multisampling,
			 .pDepthStencilState  = &depthStencil,
			 .pColorBlendState    = &colorBlending,
			 .pDynamicState       = &dynamicState,
			 .layout              = _pipelineLayout,
			 .renderPass          = nullptr},
			{.colorAttachmentCount = 1, .pColorAttachmentFormats = &colorFormat, .depthAttachmentFormat = depthFormat}};


		_pipelines.emplace(name,vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>()));
	}

	void PipelineManager::createDescriptorSetLayout() {
		std::array bindings = {
			vk::DescriptorSetLayoutBinding( 0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr),
			vk::DescriptorSetLayoutBinding( 1, vk::DescriptorType::eCombinedImageSampler, MAX_TEXTURES, vk::ShaderStageFlagBits::eFragment, nullptr)
		};

		vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = bindings.size(), .pBindings = bindings.data()};
		_descriptorSetLayout = vk::raii::DescriptorSetLayout(_context.device(), layoutInfo);
	}

	void PipelineManager::createPipelineLayout() {
		vk::PushConstantRange pushRange{
			.stageFlags = vk::ShaderStageFlagBits::eFragment,
			.offset = 0,
			.size = sizeof(uint32_t)
		};

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo{
			.setLayoutCount = 1,
			.pSetLayouts = &*_descriptorSetLayout,
			.pushConstantRangeCount = 1,
			.pPushConstantRanges = &pushRange
		};

		_pipelineLayout = vk::raii::PipelineLayout(_context.device(), pipelineLayoutInfo);
	}

	vk::raii::ShaderModule PipelineManager::createShaderModule(vk::raii::Device &device, const std::vector<char> &code) {
		const vk::ShaderModuleCreateInfo createInfo{
			.codeSize = code.size() * sizeof(char),
			.pCode = reinterpret_cast<const uint32_t*>(code.data())
		};
		return {device, createInfo };
	}
}
