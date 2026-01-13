//
// Created by eharquin on 1/1/26.
//

#pragma once

#include <core/rendering/vulkan/header.hpp>

namespace Core::Rendering::Vulkan {

	class GPUManager;
	class CommandContext;

	class MemoryManager {
	public:
		struct Allocation {
			vk::raii::DeviceMemory memory = nullptr;
			vk::DeviceSize size = 0;
			vk::DeviceSize offset = 0;
		};

		struct Buffer {
			vk::raii::Buffer buffer = nullptr;
			Allocation allocation;
			vk::DeviceSize size = 0;
		};

		struct Image {
			vk::raii::Image image = nullptr;
			Allocation allocation;
			vk::raii::ImageView view = nullptr;
			vk::Format format;
			uint32_t width;
			uint32_t height;
		};

		struct ImageDesc {
			uint32_t width;
			uint32_t height;
			vk::Format format;
			vk::ImageUsageFlags usage;
			vk::ImageAspectFlags aspect;
		};

	public:
		MemoryManager(GPUManager& gpu, CommandContext& commands);
		~MemoryManager() = default;

		// Buffers
		Buffer createBuffer(vk::DeviceSize size,
					vk::BufferUsageFlags usage,
					vk::MemoryPropertyFlags properties);

		Buffer createVertexBuffer(const void* data, size_t size);
		Buffer createIndexBuffer(const void* data, size_t size);
		Buffer createUniformBuffer(size_t size);
		Buffer createStagingBuffer(size_t size);

		// Images
		Image createImage(const ImageDesc& desc);
		Image createTexture2D(const TextureData& texture);

		void destroy(Buffer& buffer);
		void destroy(Image& image);

	private:
		Allocation allocate(vk::MemoryRequirements req,
							vk::MemoryPropertyFlags properties);

		vk::raii::Buffer createBuffer(vk::DeviceSize size,
									  vk::BufferUsageFlags usage);

		vk::raii::Image createVkImage(const ImageDesc& desc);

		uint32_t findMemoryType(uint32_t typeBits,
								vk::MemoryPropertyFlags properties) const;

	private:
		GPUManager& _gpu;
		CommandContext& _commands;
	};

} // namespace Core::Rendering::Vulkan
