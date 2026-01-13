//
// Created by eharquin on 1/1/26.
//

#pragma once

#include <core/rendering/vulkan/header.hpp>

namespace Core::Rendering::Vulkan {
	class CommandContext {
	public:
		CommandContext(vk::raii::Device& device,
				   vk::raii::Queue& queue,
				   uint32_t queueFamily);

		vk::raii::CommandBuffer allocate();

		void begin(vk::raii::CommandBuffer&, vk::CommandBufferUsageFlags);
		void end(vk::raii::CommandBuffer&);

		vk::raii::CommandBuffer beginSingleTimeCommands();
		void endSingleTimeCommands(vk::raii::CommandBuffer&);

		void copyBuffer(vk::raii::Buffer& srcBuffer,
					vk::raii::Buffer& dstBuffer,
					vk::DeviceSize size) const;

		void copyBufferToImage(const vk::raii::Buffer& buffer,
							   vk::raii::Image& image,
							   uint32_t width,
							   uint32_t height);

		void transitionImageLayout(const vk::raii::Image& image,
								   vk::ImageLayout oldLayout,
								   vk::ImageLayout newLayout);

	private:
		vk::raii::Device& _device;
		vk::raii::Queue&  _queue;
		vk::raii::CommandPool _commandPool;

	};
}