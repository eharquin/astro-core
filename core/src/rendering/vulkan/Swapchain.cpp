//
// Created by eharquin on 12/20/25.
//

#include <core/rendering/vulkan/Swapchain.hpp>

#include <iostream>
#include <ostream>

namespace Core::Rendering::Vulkan {

	Swapchain::Swapchain(Context& context, const vk::Extent2D& extent)
		: _context(context) {
		createSwapchain(extent);
		createImageViews();
		createDepthResources();
	}

	// region Swapchain
	void Swapchain::recreate(const vk::Extent2D& extent) {
		cleanupSwapchain();

		createSwapchain(extent);
		createImageViews();
		createDepthResources();
	}

	void Swapchain::cleanupSwapchain() {
		_imageViews.clear();
		_swapchain = nullptr;

		_depthView = nullptr;
		_depthImage = nullptr;
		_depthMemory = nullptr;
	}

	void Swapchain::createSwapchain(const vk::Extent2D& extent) {
		auto chooseSwapSurfaceFormat = [](const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
			for (const auto& f : availableFormats) {
				if (f.format == vk::Format::eB8G8R8A8Srgb &&
					f.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) return f;
			}
			return availableFormats[0];
		};
		auto chooseSwapPresentMode = [](const std::vector<vk::PresentModeKHR>& availablePresentModes) {
			for (const auto& mode : availablePresentModes) {
				if (mode == vk::PresentModeKHR::eMailbox) return mode;
			}
			return vk::PresentModeKHR::eFifo;
		};
		auto chooseSwapExtent = [extent](const vk::SurfaceCapabilitiesKHR& capabilities) {
			if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

			return vk::Extent2D{
				std::clamp<uint32_t>(extent.width,  capabilities.minImageExtent.width,  capabilities.maxImageExtent.width),
				std::clamp<uint32_t>(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
			};
		};
		auto chooseSwapMinImageCount = [](const vk::SurfaceCapabilitiesKHR& surfaceCapabilities) {
			auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);
			if ((surfaceCapabilities.maxImageCount > 0) && (surfaceCapabilities.maxImageCount < minImageCount)) {
				minImageCount = surfaceCapabilities.maxImageCount;
			}
			return minImageCount;
		};

		const auto& physicalDevice = _context.physicalDevice();
		const auto& device = _context.device();
		const auto& surface = _context.surface();

		const auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR( surface );
		_surfaceFormat = chooseSwapSurfaceFormat( physicalDevice.getSurfaceFormatsKHR( surface ) );
		_presentMode = chooseSwapPresentMode( physicalDevice.getSurfacePresentModesKHR( surface ) );
		_extent = chooseSwapExtent( surfaceCapabilities );
		_minImageCount = chooseSwapMinImageCount( surfaceCapabilities );

		std::cout << "[ASTRO CORE] [VULKAN] [SWAPCHAIN] surface format: " <<
				vk::to_string(_surfaceFormat.format) << " " <<
				vk::to_string(_surfaceFormat.colorSpace) << std::endl;
		std::cout << "[ASTRO CORE] [VULKAN] [SWAPCHAIN] present mode  : " <<
				vk::to_string(_presentMode) << std::endl;
		std::cout << "[ASTRO CORE] [VULKAN] [SWAPCHAIN] extent        : " <<
			_extent.width << "x" << _extent.height << std::endl;
		std::cout << "[ASTRO CORE] [VULKAN] [SWAPCHAIN] min image count: " <<
			_minImageCount << std::endl;

		vk::SwapchainCreateInfoKHR swapChainCreateInfo{
			.surface          = *surface,
		   .minImageCount    = _minImageCount,
		   .imageFormat      = _surfaceFormat.format,
		   .imageColorSpace  = _surfaceFormat.colorSpace,
		   .imageExtent      = _extent,
		   .imageArrayLayers = 1,
		   .imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
		   .imageSharingMode = vk::SharingMode::eExclusive,
		   .preTransform     = surfaceCapabilities.currentTransform,
		   .compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		   .presentMode      = _presentMode,
		   .clipped          = true};

		_swapchain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
		_images = _swapchain.getImages();
	}
	// endregion

	// region Image Views
	void Swapchain::createImageViews() {
		assert(_imageViews.empty());
		for (auto& image : _images) {
			_imageViews.emplace_back(_context.createImageView(image, _surfaceFormat.format, vk::ImageAspectFlagBits::eColor));
		}
	}
	// endregion

	// region Depth Resources

	vk::Format Swapchain::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const {
		for (const auto format : candidates) {
			vk::FormatProperties props = _context.physicalDevice().getFormatProperties(format);

			if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
				return format;
			}
			if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
				return format;
			}
		}
		throw std::runtime_error("failed to find supported format!");
	}

	vk::Format Swapchain::findDepthFormat() const {
		return findSupportedFormat(
			 {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
				 vk::ImageTiling::eOptimal,
				 vk::FormatFeatureFlagBits::eDepthStencilAttachment
			 );
	}

	bool Swapchain::hasStencilComponent(vk::Format format) {
		return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
	}

	void Swapchain::createDepthResources() {
		_depthFormat = findDepthFormat();

		_context.createImage(_extent.width, _extent.height, _depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, _depthImage, _depthMemory);
		_depthView = _context.createImageView(_depthImage, _depthFormat, vk::ImageAspectFlagBits::eDepth);
	}
	// endregion


}


