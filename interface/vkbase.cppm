//
// Created by gomkyung2 on 1/20/24.
//

module;

// See https://github.com/KhronosGroup/Vulkan-Hpp/issues/1798.
#if _MSC_VER && !__INTEL_COMPILER
#include <compare>
#endif

export module vkbase;

export import :primitives;
export import :builders;