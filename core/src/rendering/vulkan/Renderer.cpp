//
// Created by eharquin on 12/19/25.
//

#include <chrono>
#include <iostream>
#include <core/rendering/vulkan/Renderer.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

namespace Core::Rendering::Vulkan {
	Renderer::Renderer(Context &context, Window& window)
		: _context(context), _window(window)
	{}

	void Renderer::init() {

		_swapchain = std::make_unique<Swapchain>(_context, vk::Extent2D{
			static_cast<uint32_t>(_window.width()),
			static_cast<uint32_t>(_window.height())
		});

		_pipelineManager = std::make_unique<PipelineManager>(_context, *_swapchain);
		_pipelineManager->createPipeline("basic", "../../shaders/slang.spv");
		_meshManager = std::make_unique<MeshManager>(_context);
		_textureManager = std::make_unique<TextureManager>(_context);

		createSyncObjects();
		createCommandBuffers();
		createUniformBuffers();
		createDescriptorPool();
		createDescriptorSets();
	}

	void Renderer::drawFrame() {
		auto& device = _context.device();
		auto& swapchain = _swapchain->swapchain();
		const vk::Extent2D windowExtent =  {static_cast<uint32_t>(_window.width()), static_cast<uint32_t>(_window.height())};

		// Wait for the current frame to finish
		while (vk::Result::eTimeout == device.waitForFences(*_inFlightFences[_frameIndex], vk::True, UINT64_MAX)) {}
		device.resetFences(*_inFlightFences[_frameIndex]);

		auto [result, imageIndex] = swapchain.acquireNextImage(
			UINT64_MAX,
			*_presentCompleteSemaphores[_frameIndex],
			nullptr
		);

		if (result == vk::Result::eErrorOutOfDateKHR) {
			_swapchain->recreate(windowExtent);
			return;
		}
		if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
			throw std::runtime_error("Failed to acquire swap chain image!");

		// Update uniforms
		updateUniformBuffer(_frameIndex);

		// Reset and record command buffer for this frame
		_commandBuffers[_frameIndex].reset();
		recordCommandBuffer(imageIndex);

		// Submit command buffer
		vk::PipelineStageFlags waitStages = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		vk::SubmitInfo submitInfo{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*_presentCompleteSemaphores[_frameIndex],
			.pWaitDstStageMask = &waitStages,
			.commandBufferCount = 1,
			.pCommandBuffers = &*_commandBuffers[_frameIndex],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &*_renderFinishedSemaphores[imageIndex]
		};

		_context.graphicsQueue().submit(submitInfo, *_inFlightFences[_frameIndex]);

		// Present swapchain image
		try {
			vk::PresentInfoKHR presentInfo{
				.waitSemaphoreCount = 1,
				.pWaitSemaphores = &*_renderFinishedSemaphores[imageIndex],
				.swapchainCount = 1,
				.pSwapchains = &*swapchain,
				.pImageIndices = &imageIndex
			};

			result = _context.graphicsQueue().presentKHR(presentInfo);

			if (result == vk::Result::eSuboptimalKHR || _shouldRecreateSwapChain) {
				_shouldRecreateSwapChain = false;
				_swapchain->recreate(windowExtent);
			} else if (result != vk::Result::eSuccess) {
				throw std::runtime_error("Failed to present swap chain image!");
			}
		} catch (const vk::SystemError& e) {
			if (e.code().value() == static_cast<int>(vk::Result::eErrorOutOfDateKHR)) {
				_swapchain->recreate(windowExtent);
			} else {
				throw;
			}
		}

		// Advance to next frame
		_frameIndex = (_frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void Renderer::shutdown() {
		_context.device().waitIdle();
	}

	void Renderer::createSyncObjects() {
		assert(_presentCompleteSemaphores.empty() && _renderFinishedSemaphores.empty() && _inFlightFences.empty());

		const auto& device = _context.device();
		const auto& swapChainImages = _swapchain->images();

		for (size_t i = 0; i < swapChainImages.size(); i++)
			_renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			_presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
			_inFlightFences.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
		}
	}

	void Renderer::createCommandBuffers() {
		vk::CommandBufferAllocateInfo allocInfo{
			.commandPool = _context.commandPool(),
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = MAX_FRAMES_IN_FLIGHT };

		_commandBuffers = vk::raii::CommandBuffers(_context.device(), allocInfo);
	}

	void Renderer::createUniformBuffers() {
		_uniformBuffers.clear();
		_uniformBuffersMemory.clear();
		_uniformBuffersMapped.clear();

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
			vk::raii::Buffer buffer({});
			vk::raii::DeviceMemory bufferMem({});
			_context.createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer, bufferMem);
			_uniformBuffers.emplace_back(std::move(buffer));
			_uniformBuffersMemory.emplace_back(std::move(bufferMem));
			_uniformBuffersMapped.emplace_back( _uniformBuffersMemory[i].mapMemory(0, bufferSize));
		}
	}

	void Renderer::createDescriptorPool() {
		std::array poolSize {
			vk::DescriptorPoolSize( vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT),
			vk::DescriptorPoolSize(  vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT * MAX_TEXTURES)
		};
		vk::DescriptorPoolCreateInfo poolInfo{.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, .maxSets = MAX_FRAMES_IN_FLIGHT, .poolSizeCount = poolSize.size(), .pPoolSizes = poolSize.data()};
		_descriptorPool = vk::raii::DescriptorPool(_context.device(), poolInfo);
	}

	void Renderer::createDescriptorSets() {
		std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, _pipelineManager->descriptorSetLayout());
		vk::DescriptorSetAllocateInfo allocInfo{
			.descriptorPool = _descriptorPool,
			.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
			.pSetLayouts = layouts.data()
		};

		_descriptorSets = _context.device().allocateDescriptorSets(allocInfo);

		std::vector<vk::DescriptorImageInfo> imageInfos(MAX_TEXTURES);

	    // Fill all slots with dummy texture
	    for (uint32_t i = 0; i < MAX_TEXTURES; ++i) {
	        imageInfos[i] = vk::DescriptorImageInfo{
	            _textureManager->get(0).sampler,
	            _textureManager->get(0).view,
	            vk::ImageLayout::eShaderReadOnlyOptimal
	        };
	    }

	    for (size_t frame = 0; frame < MAX_FRAMES_IN_FLIGHT; ++frame) {
		    vk::DescriptorBufferInfo bufferInfo{
		    	.buffer = _uniformBuffers[frame],
				.offset = 0,
				.range = sizeof(UniformBufferObject)
			};

	    	std::array<vk::WriteDescriptorSet, 2> descriptorWrites{
	    		vk::WriteDescriptorSet{
	    			.dstSet = _descriptorSets[frame],
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eUniformBuffer,
					.pBufferInfo = &bufferInfo
				},
				vk::WriteDescriptorSet{
					.dstSet = _descriptorSets[frame],
					.dstBinding = 1,
					.dstArrayElement = 0,
					.descriptorCount = MAX_TEXTURES,
					.descriptorType = vk::DescriptorType::eCombinedImageSampler,
					.pImageInfo = imageInfos.data()
				}
	    	};

			_context.device().updateDescriptorSets(descriptorWrites, {});
	    }
	}

	void Renderer::updateTextureDescriptor(uint32_t textureIndex) {
		for (uint32_t frame = 0; frame < MAX_FRAMES_IN_FLIGHT; ++frame) {
			vk::DescriptorImageInfo imageInfo{
				_textureManager->get(textureIndex).sampler,
				_textureManager->get(textureIndex).view,
				vk::ImageLayout::eShaderReadOnlyOptimal
			};

			vk::WriteDescriptorSet write{
				.dstSet = _descriptorSets[frame],
				.dstBinding = 1,
				.dstArrayElement = textureIndex,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eCombinedImageSampler,
				.pImageInfo = &imageInfo
			};

			_context.device().updateDescriptorSets(write, {});
		}
	}

	void Renderer::updateUniformBuffer(uint32_t frameIndex) {
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		vk::Extent2D extent = _swapchain->extent();

		ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(extent.width) / static_cast<float>(extent.height), 0.1f, 10.0f);

		ubo.proj[1][1] *= -1;

		memcpy(_uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
	}

	void Renderer::recordCommandBuffer(uint32_t imageIndex) {
		auto& commandBuffer = _commandBuffers[_frameIndex];

		vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
		commandBuffer.begin(beginInfo);

		// Before starting rendering, transition the swapchain image to COLOR_ATTACHMENT_OPTIMAL
		recordTransitionImageLayout(
			_swapchain->images()[imageIndex],
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eColorAttachmentOptimal,
			{},                                                         // srcAccessMask (no need to wait for previous operations)
			vk::AccessFlagBits2::eColorAttachmentWrite,                 // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,         // srcStage
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,         // dstStage
			vk::ImageAspectFlagBits::eColor           		           // image_aspect_flags
		);
		// Transition for the depth image
		recordTransitionImageLayout(
			_swapchain->depthImage(),
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::eDepthAttachmentOptimal,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
			vk::ImageAspectFlagBits::eDepth);

		vk::RenderingAttachmentInfo colorAttachment{
			.imageView = _swapchain->imageViews()[imageIndex],
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = vk::ClearColorValue(std::array<float,4>{0.f,0.f,0.f,1.f})
		};

		vk::RenderingAttachmentInfo depthAttachment{
			.imageView = _swapchain->depthImageView(),
			.imageLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eDontCare,
			.clearValue = vk::ClearDepthStencilValue{1.0f, 0}
		};

		vk::Extent2D extent = _swapchain->extent();

		vk::RenderingInfo renderingInfo{
			.renderArea = vk::Rect2D{{0,0},extent},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &colorAttachment,
			.pDepthAttachment = &depthAttachment
		};

		commandBuffer.beginRendering(renderingInfo);

		// Bind pipeline
		auto& pipeline = *_pipelineManager->get("basic");
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);

		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), extent));

		for (auto& instance : _instances) {
			const auto& mesh = _meshManager->get(instance.mesh);

			vk::Buffer vertexBuffers[] = {*mesh.vertexBuffer};
			vk::DeviceSize offsets[] = {0};
			commandBuffer.bindVertexBuffers(0, vertexBuffers, offsets);
			commandBuffer.bindIndexBuffer(*mesh.indexBuffer, 0, vk::IndexType::eUint32);

			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineManager->pipelineLayout(), 0, *_descriptorSets[_frameIndex], nullptr);

			const uint32_t textureValue = instance.texture;
			vk::ArrayProxy<const uint32_t> pushDataProxy(textureValue);
			commandBuffer.pushConstants(*_pipelineManager->pipelineLayout(),vk::ShaderStageFlagBits::eFragment,0,pushDataProxy);

			commandBuffer.drawIndexed(mesh.indexCount, 1, 0, 0, 0);
		}
		commandBuffer.endRendering();

		// After rendering, transition the swapchain image to PRESENT_SRC
		recordTransitionImageLayout(
			_swapchain->images()[imageIndex],
			vk::ImageLayout::eColorAttachmentOptimal,
			vk::ImageLayout::ePresentSrcKHR,
			vk::AccessFlagBits2::eColorAttachmentWrite,             // srcAccessMask
			{},                                                     // dstAccessMask
			vk::PipelineStageFlagBits2::eColorAttachmentOutput,     // srcStage
			vk::PipelineStageFlagBits2::eBottomOfPipe,         // dstStage
			vk::ImageAspectFlagBits::eColor           		           // image_aspect_flags
		);
		commandBuffer.end();
	}

	void Renderer::recordTransitionImageLayout(
		vk::Image image,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessMask,
		vk::AccessFlags2 dstAccessMask,
		vk::PipelineStageFlags2 srcStageMask,
		vk::PipelineStageFlags2 dstStageMask,
		vk::ImageAspectFlags    image_aspect_flags
	) const {
		vk::ImageMemoryBarrier2 barrier = {
			.srcStageMask = srcStageMask,
			.srcAccessMask = srcAccessMask,
			.dstStageMask = dstStageMask,
			.dstAccessMask = dstAccessMask,
			.oldLayout = oldLayout,
			.newLayout = newLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = {
				.aspectMask = image_aspect_flags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		vk::DependencyInfo dependencyInfo = {
			.dependencyFlags = {},
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier
		};
		_commandBuffers[_frameIndex].pipelineBarrier2(dependencyInfo);
	}
}
