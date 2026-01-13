//
// Created by eharquin on 12/20/25.
//

#pragma once

#include <core/rendering/vulkan/Context.hpp>

namespace Core::Rendering::Vulkan {
	class Swapchain {
	public:
		explicit Swapchain(Context& context, const vk::Extent2D& extent);

		~Swapchain() = default;

		Swapchain(const Swapchain&) = delete;
		Swapchain& operator=(const Swapchain&) = delete;
		Swapchain(Swapchain&&) = delete;
		Swapchain& operator=(Swapchain&&) = delete;

		void recreate(const vk::Extent2D& extent);

		vk::raii::SwapchainKHR& swapchain() {return _swapchain;}
		const std::vector<vk::Image>& images() const { return _images; }
		const std::vector<vk::raii::ImageView>& imageViews() const { return _imageViews; }
		[[nodiscard]] vk::Format colorFormat() const { return _surfaceFormat.format; }
		[[nodiscard]] vk::Extent2D extent() const { return _extent; }


		[[nodiscard]] const vk::raii::ImageView& depthImageView() const { return _depthView; }
		[[nodiscard]] const vk::raii::Image& depthImage() const { return _depthImage; }
		[[nodiscard]] vk::Format depthFormat() const { return _depthFormat; }

	private:
		void createSwapchain(const vk::Extent2D& extent);
		void cleanupSwapchain();
		void createImageViews();

		vk::Format findSupportedFormat(const std::vector<vk::Format> &candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;
		vk::Format findDepthFormat() const;
		static bool hasStencilComponent(vk::Format format);
		void createDepthResources();


		Context& _context;

		vk::SurfaceFormatKHR _surfaceFormat;
		vk::PresentModeKHR   _presentMode;
		vk::Extent2D         _extent;
		uint32_t			 _minImageCount = ~0;
		vk::raii::SwapchainKHR _swapchain = nullptr;
		std::vector<vk::Image> _images;
		std::vector<vk::raii::ImageView> _imageViews;

		vk::raii::Image _depthImage = nullptr;
		vk::raii::DeviceMemory _depthMemory = nullptr;
		vk::raii::ImageView _depthView = nullptr;
		vk::Format _depthFormat;
	};
}