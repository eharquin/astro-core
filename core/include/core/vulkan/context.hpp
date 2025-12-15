//
// Created by eharquin on 12/7/25.
//

#pragma once

#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif

#include <core/window/window.hpp>

#include <vector>
#include <optional>

namespace Core::Vulkan {
#ifdef NDEBUG
	constexpr bool enableValidationLayers = false;
#else
	constexpr bool enableValidationLayers = true;
#endif

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

	class Context {
	public:
		Context() = default;

		~Context() = default;

		Context(const Context &) = delete;

		Context &operator=(const Context &) = delete;

		Context(Context &&) = delete;

		Context &operator=(Context &&) = delete;

		void create(const Window & window);

		void drawFrame(Window & window);

		void stop();

		struct QueueFamilyIndices {
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
		};

	private:
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

		void createSwapChain(const Window & window);

		void cleanupSwapChain();

		void recreateSwapChain(const Window & window);

		void createImageViews();

		void createGraphicsPipeline();
		[[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;

		void createCommandPool();

		void createCommandBuffers();

		void recordCommandBuffer(uint32_t imageIndex);

		void transitionImageLayout(
			uint32_t imageIndex,
			vk::ImageLayout oldLayout,
			vk::ImageLayout newLayout,
			vk::AccessFlags2 srcAccessMask,
			vk::AccessFlags2 dstAccessMask,
			vk::PipelineStageFlags2 srcStageMask,
			vk::PipelineStageFlags2 dstStageMask
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

		// Graphics Pipeline
		vk::raii::PipelineLayout _pipelineLayout = nullptr;
		vk::raii::Pipeline _graphicsPipeline = nullptr;

		// Command Pool
		vk::raii::CommandPool _commandPool = nullptr;
		std::vector<vk::raii::CommandBuffer> _commandBuffers;

		// Sync Objects
		std::vector<vk::raii::Semaphore> _presentCompleteSemaphores;
		std::vector<vk::raii::Semaphore> _renderFinishedSemaphores;
		std::vector<vk::raii::Fence> _inFlightFences;

		uint32_t frameIndex = 0;
		bool _framebufferResized = false;

	};
}
