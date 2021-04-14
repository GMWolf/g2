//
// Created by felix on 14/04/2021.
//

#ifndef G2_SWAPCHAIN_H
#define G2_SWAPCHAIN_H

#include "vk.h"
#include <vector>

#include "device.h"
namespace g2::gfx
{
    struct SwapChainSupportDetails
    {
        vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR> presentModes;
    };

    struct SwapChain
    {
        vk::SwapchainKHR swapchain;
        std::vector<vk::Image> images;
        std::vector<vk::ImageView> imageViews;
        vk::Format format;
        vk::Extent2D extent;

        void shutdown(vk::Device device);
    };

    SwapChainSupportDetails querySwapChainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);

    bool checkSwapChainAdequate(const SwapChainSupportDetails &swapChainSupport);

    std::optional<SwapChain> createSwapChain(vk::Device device, vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, vk::Extent2D windowExtent, QueueFamilyIndices familyIndices);
}

#endif //G2_SWAPCHAIN_H
