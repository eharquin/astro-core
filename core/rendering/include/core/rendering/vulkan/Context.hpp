//
// Created by eharquin on 12/20/25.
//

#pragma once

#include <core/rendering/vulkan/header.hpp>
#include <core/window/Window.hpp>

#include <core/rendering/IContext.hpp>

#include <optional>

namespace Core::Rendering::Vulkan {
	const std::vector<const char *> validationLayers = {
		"VK_LAYER_KHRONOS_validation",
	};

	const std::vector<const char *> deviceExtensions = {
		vk::KHRSwapchainExtensionName,
		vk::KHRSpirv14ExtensionName,
		vk::KHRSynchronization2ExtensionName,
		vk::KHRCreateRenderpass2ExtensionName
	};

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
	};

	class Context : public IContext{
		friend class Swapchain;
		friend class MeshManager;
		friend class TextureManager;
		friend class PipelineManager;
		friend class Renderer;
	public:
		explicit Context() = default;
		~Context() override = default;

		Context(const Context &) = delete;
		Context &operator=(const Context &) = delete;
		Context(Context &&) = delete;
		Context &operator=(Context &&) = delete;

		void init(const Window & window) override;
		void shutdown() override;
		std::unique_ptr<IRenderer> createRenderer(Window &window) override;

	protected:
		vk::raii::Instance &instance() {return _instance;}
		vk::raii::PhysicalDevice& physicalDevice() {return _physicalDevice;}
		vk::raii::Device& device() {return _device;}
		vk::raii::Queue& graphicsQueue() {return _graphicsQueue;}
		vk::raii::Queue& presentQueue() {return _presentQueue;}
		vk::raii::SurfaceKHR& surface() {return _surface;}
		vk::raii::CommandPool& commandPool() {return _commandPool;}
		uint32_t graphicsQueueFamily() const {return _queueFamilyIndices.graphicsFamily.value();}
		uint32_t presentQueueFamily() const {return _queueFamilyIndices.presentFamily.value();}
		void waitIdle();

	private:
		void create(const Window & window);

		void createInstance();

		static std::vector<const char *> getRequiredExtensions();

		[[nodiscard]] bool checkExtensionSupport(const std::vector<const char *> &requiredExtensions) const;

		[[nodiscard]] bool checkLayerSupport(const std::vector<const char *> &requiredLayers) const;

		static vk::Bool32 debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		                                vk::DebugUtilsMessageTypeFlagsEXT type,
		                                const vk::DebugUtilsMessengerCallbackDataEXT *
		                                pCallbackData, void *);

		void setupDebugMessenger();

		void pickPhysicalDevice();

		static QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice &physicalDevice,
		                                            const vk::raii::SurfaceKHR &surface);
		void createLogicalDevice();

		void createSurface(const Window & window);

		void createCommandPool();

		// helpers
		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;
		vk::raii::ImageView createImageView(vk::raii::Image &image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;
		vk::raii::ImageView createImageView(vk::Image &image, vk::Format format, vk::ImageAspectFlags aspectFlags) const;
		void createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
						 vk::ImageUsageFlags usage,
						 vk::MemoryPropertyFlags properties, vk::raii::Image &image,
						 vk::raii::DeviceMemory &imageMemory);

		void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
		                  vk::raii::Buffer &buffer, vk::raii::DeviceMemory &bufferMemory) const;

		void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size) const;

		void createStagingBuffer(const void *data, vk::DeviceSize size,
		                         vk::raii::Buffer &stagingBuffer, vk::raii::DeviceMemory &stagingMemory) const;

		template<typename T>
		void createDeviceLocalBuffer(const std::vector<T>& data,
							 vk::BufferUsageFlags usage,
							 vk::raii::Buffer& buffer,
							 vk::raii::DeviceMemory& memory) {
			vk::raii::Buffer stagingBuffer({});
			vk::raii::DeviceMemory stagingMemory({});

			createStagingBuffer(data.data(), sizeof(T) * data.size(), stagingBuffer, stagingMemory);
			createBuffer(sizeof(T) * data.size(), usage | vk::BufferUsageFlagBits::eTransferDst,
								  vk::MemoryPropertyFlagBits::eDeviceLocal, buffer, memory);
			copyBuffer(stagingBuffer, buffer, sizeof(T) * data.size());
		}

		vk::raii::CommandBuffer beginSingleTimeCommands() const;

		void endSingleTimeCommands(vk::raii::CommandBuffer &commandBuffer) const;

		void transitionImageLayout(const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

		void copyBufferToImage(const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height);

		void createTextureImageFromData(const void *pixels, uint32_t width, uint32_t height, vk::raii::Image &outImage,
		                                vk::raii::DeviceMemory &outMemory);

		vk::raii::Sampler createTextureSampler();

		// Instance
		vk::raii::Context _context;
		vk::raii::Instance _instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT _debugMessenger = nullptr;

		// Device
		vk::raii::PhysicalDevice _physicalDevice = nullptr;
		vk::raii::Device _device = nullptr;

		// Queues
		vk::raii::Queue _graphicsQueue = nullptr;
		vk::raii::Queue _presentQueue = nullptr;
		QueueFamilyIndices _queueFamilyIndices;

		// Surface
		vk::raii::SurfaceKHR _surface = nullptr;

		// Command Pool
		vk::raii::CommandPool _commandPool = nullptr;
	};



}
