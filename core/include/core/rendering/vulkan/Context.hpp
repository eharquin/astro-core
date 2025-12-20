//
// Created by eharquin on 12/7/25.
//

#pragma once

#include <core/rendering/vulkan/Header.hpp>
#include <core/rendering/vulkan/Vertex.hpp>

#include <core/window/Window.hpp>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <optional>
#include <chrono>

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

	constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	const std::vector<Vertex> vertices = {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};

	const std::vector<uint16_t> vertexIndices = {
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4
	};

	struct UniformBufferObject {
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};

	class Context {
	public:
		Context() = default;

		~Context() = default;

		Context(const Context &) = delete;

		Context &operator=(const Context &) = delete;

		Context(Context &&) = delete;

		Context &operator=(Context &&) = delete;

		void create(const Window & window);

		void drawFrame(int width, int height);

		void stop();

		void shouldRecreateSwapChain();

	private:

		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
		};

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

		void createSwapChain(int width, int height);

		void cleanupSwapChain();

		void recreateSwapChain(int width, int height);


		void createImageViews();

		void createDescriptorSetLayout();

		void createGraphicsPipeline();
		[[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;

		void createCommandPool();

		vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling,
		                               vk::FormatFeatureFlags features) const;

		vk::Format findDepthFormat() const;

		static bool hasStencilComponent(vk::Format format);

		void createDepthResources();

		void createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
		                  vk::raii::Buffer &buffer, vk::raii::DeviceMemory &bufferMemory);

		void copyBuffer(vk::raii::Buffer & srcBuffer, vk::raii::Buffer & dstBuffer, vk::DeviceSize size);

		void createTextureImage();

		void createTextureImageView();

		void createTextureSampler();

		vk::raii::ImageView createImageView(vk::raii::Image &image, vk::Format format, vk::ImageAspectFlags aspectFlags);

		void createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
		                 vk::ImageUsageFlags usage,
		                 vk::MemoryPropertyFlags properties, vk::raii::Image &image,
		                 vk::raii::DeviceMemory &imageMemory);

		void transitionImageLayout(const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

		void copyBufferToImage(const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height);

		void loadModel();

		void createVertexBuffer();

		void createIndexBuffer();

		void createUniformBuffers();

		void createDescriptorPool();

		void createDescriptorSets();

		void updateUniformBuffer(uint32_t imageIndex);

		uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties);

		void createCommandBuffers();

		void recordCommandBuffer(uint32_t imageIndex);

		vk::raii::CommandBuffer beginSingleTimeCommands();

		void endSingleTimeCommands(vk::raii::CommandBuffer &commandBuffer);

		void recordTransitionImageLayout(
			vk::Image image,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags2 srcAccessMask,
			vk::AccessFlags2 dstAccessMask,
			vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask, vk::ImageAspectFlags image_aspect_flags
		) const;

		void createSyncObjects();


		// Instance
		vk::raii::Context _context;
		vk::raii::Instance _instance = VK_NULL_HANDLE;
		vk::raii::DebugUtilsMessengerEXT _debugMessenger = nullptr;

		// Device
		vk::raii::PhysicalDevice _physicalDevice = nullptr;
		vk::raii::Device _device = nullptr;

		vk::raii::Queue _graphicsQueue = nullptr;
		vk::raii::Queue _presentQueue = nullptr;
		QueueFamilyIndices _queueFamilyIndices;

		// Surface
		vk::raii::SurfaceKHR _surface = nullptr;

		// SwapChain
		vk::SurfaceFormatKHR _swapChainSurfaceFormat;
		vk::PresentModeKHR   _swapChainPresentMode;
		vk::Extent2D         _swapChainExtent;
		uint32_t			 _swapChainMinImageCount = ~0;
		vk::raii::SwapchainKHR _swapChain = nullptr;
		std::vector<vk::Image>  _swapChainImages;
		std::vector<vk::raii::ImageView> _swapChainImageViews;
		bool _shouldRecreateSwapChain = false;

		// Graphics Pipeline
		vk::raii::PipelineLayout _pipelineLayout = nullptr;
		vk::raii::Pipeline _graphicsPipeline = nullptr;

		// Descriptor Set Layout
		vk::raii::DescriptorSetLayout _descriptorSetLayout = nullptr;

		// Command Pool
		vk::raii::CommandPool _commandPool = nullptr;

		// Command Buffers
		std::vector<vk::raii::CommandBuffer> _commandBuffers;

		// Depth Resources
		vk::raii::Image _depthImage = nullptr;
		vk::raii::DeviceMemory _depthImageMemory = nullptr;
		vk::raii::ImageView _depthImageView = nullptr;

		// Texture Image
		vk::raii::Image        _textureImage       = nullptr;
		vk::raii::DeviceMemory _textureImageMemory = nullptr;

		// Texture Image View
		vk::raii::ImageView _textureImageView = nullptr;

		// Texture Image Sampler
		vk::raii::Sampler _textureSampler = nullptr;

		// Vertex Buffer
		vk::raii::Buffer _vertexBuffer = nullptr;
		vk::raii::DeviceMemory _vertexBufferMemory = nullptr;

		// Index Buffer
		vk::raii::Buffer _indexBuffer = nullptr;
		vk::raii::DeviceMemory _indexBufferMemory = nullptr;

		// Descriptor Pool and Sets
		vk::raii::DescriptorPool _descriptorPool = nullptr;
		std::vector<vk::raii::DescriptorSet> _descriptorSets;

		// Uniform Buffers
		std::vector<vk::raii::Buffer> _uniformBuffers;
		std::vector<vk::raii::DeviceMemory> _uniformBuffersMemory;
		std::vector<void*> _uniformBuffersMapped;

		// Sync Objects
		std::vector<vk::raii::Semaphore> _presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore> _renderFinishedSemaphores;
		std::vector<vk::raii::Fence> _inFlightFences;

		uint32_t _frameIndex = 0;
		bool _framebufferResized = false;

	};
}
