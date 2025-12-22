//
// Created by eharquin on 12/19/25.
//

#pragma once

#include <memory>
#include <ranges>
#include <vector>

#include <core/rendering/vulkan/Swapchain.hpp>
#include <core/rendering/IRenderer.hpp>

#include <core/rendering/vulkan/PipelineManager.hpp>
#include <core/rendering/vulkan/MeshManager.hpp>
#include <core/rendering/vulkan/TextureManager.hpp>

namespace Core::Rendering::Vulkan {

	constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

	class Renderer : public IRenderer {

		struct UniformBufferObject {
			glm::mat4 model;
			glm::mat4 view;
			glm::mat4 proj;
		};

	public:

		Renderer(Context& context, Window& window);
		~Renderer() override = default;

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;

		void init() override;
		void drawFrame() override;
		void shutdown() override;

		void createPipeline(const std::string& name, const ShaderData& shaderData) override {
			_pipelineManager->createPipeline(name, shaderData.code);
		}

		MeshID createMesh(const MeshData& meshData) override {
			return _meshManager->createMesh(meshData);
		}

		TextureID createTexture(const TextureData& textureData) override {
			auto textureID = _textureManager->loadTexture(textureData);
			updateTextureDescriptor(textureID);
			return textureID;
		}

		void addInstance(MeshID mesh, TextureID texture) override {
			_instances.push_back({mesh, texture});
		}

	private:
		void createSyncObjects();
		void createCommandBuffers();
		void createUniformBuffers();
		void createDescriptorPool();
		void createDescriptorSets();

		void updateTextureDescriptor(uint32_t textureIndex);
		void updateUniformBuffer(uint32_t frameIndex);

		void recordCommandBuffer(uint32_t imageIndex);

		void recordTransitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
		                                 vk::AccessFlags2 srcAccessMask, vk::AccessFlags2 dstAccessMask,
		                                 vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask,
		                                 vk::ImageAspectFlags image_aspect_flags) const;

		std::vector<ModelData> _instances;

		Context& _context;
		Window& _window;

		std::unique_ptr<Swapchain> _swapchain;
		std::unique_ptr<PipelineManager> _pipelineManager;
		std::unique_ptr<MeshManager> _meshManager;
		std::unique_ptr<TextureManager> _textureManager;

		uint32_t _frameIndex = 0;
		bool _shouldRecreateSwapChain = false;

		// Synchronization objects
		std::vector<vk::raii::Semaphore> _presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore> _renderFinishedSemaphores;
		std::vector<vk::raii::Fence> _inFlightFences;

		// Command Buffers
		std::vector<vk::raii::CommandBuffer> _commandBuffers;

		// Descriptor Pool and Sets
		vk::raii::DescriptorPool _descriptorPool = nullptr;
		std::vector<vk::raii::DescriptorSet> _descriptorSets;

		// Uniform Buffers
		std::vector<vk::raii::Buffer> _uniformBuffers;
		std::vector<vk::raii::DeviceMemory> _uniformBuffersMemory;
		std::vector<void*> _uniformBuffersMapped;
	};
}

