#pragma once

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

inline constexpr const char *validationLayers[] = {
		"VK_LAYER_KHRONOS_validation"
};
