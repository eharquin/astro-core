//
// Created by eharquin on 1/1/26.
//

#pragma once

#include <core/rendering/vulkan/header.hpp>

namespace Core::Rendering::Vulkan {

	struct GPURequirements {
		std::vector<const char*> deviceExtensions;
		vk::PhysicalDeviceFeatures features{};
		bool requireGraphicsQueue = true;
		bool requirePresentQueue = false;
	};


	class GPUManager {
	public:
		GPUManager() = default;
		~GPUManager() = default;

		GPUManager(const GPUManager &) = delete;
		GPUManager &operator=(const GPUManager &) = delete;

		void init(const GPURequirements& requirements, bool enableValidationLayers = true);
		void shutdown();

		vk::raii::Instance& instance() { return _instance; }
		vk::raii::PhysicalDevice& physicalDevice() { return _physicalDevice; }
		vk::raii::Device& device() { return _device; }
		vk::raii::Queue& graphicsQueue() { return _graphicsQueue; }
		vk::raii::Queue& presentQueue() { return _presentQueue; }

		bool isInitialized() const;
		bool hasPresentQueue() const;

		uint32_t graphicsQueueFamily() const { return _graphicsFamily.value(); }
		uint32_t presentQueueFamily() const { return _presentFamily.value(); }

	private:
		void createInstance();
		void setupDebugMessenger();
		void pickPhysicalDevice();
		void createLogicalDevice();

		vk::raii::Instance _instance = nullptr;
		vk::raii::DebugUtilsMessengerEXT _debugMessenger = nullptr;
		vk::raii::PhysicalDevice _physicalDevice = nullptr;
		vk::raii::Device _device = nullptr;
		vk::raii::Queue _graphicsQueue = nullptr;
		vk::raii::Queue _presentQueue = nullptr;

		std::optional<uint32_t> _graphicsFamily;
		std::optional<uint32_t> _presentFamily;
	};
}