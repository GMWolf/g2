//
// Created by felix on 14/04/2021.
//

#include "swapchain.h"

#include <vector>

#include "device.h"
#include <span>
#include <iostream>

namespace g2::gfx {
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,
                                                  VkSurfaceKHR surface) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        {
            uint32_t surfaceFormatCounts;
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCounts, nullptr);
            details.formats.resize(surfaceFormatCounts);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &surfaceFormatCounts, details.formats.data());
        }
        {
            uint32_t surfacePresentModeCounts;
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &surfacePresentModeCounts, nullptr);
            details.presentModes.resize(surfacePresentModeCounts);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &surfacePresentModeCounts,
                                                      details.presentModes.data());
        }

        return details;
    }

    bool checkSwapChainAdequate(const SwapChainSupportDetails &swapChainSupport) {
        return !swapChainSupport.formats.empty() &&
               !swapChainSupport.presentModes.empty();
    }

    static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats) {
        for (const auto &format : availableFormats) {
            if (format.format == VK_FORMAT_R8G8B8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
                format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }
        return availableFormats[0];
    }

    static VkPresentModeKHR choosePresentMode(const std::span<VkPresentModeKHR> availablePresentModes) {
        for (const auto &presentMode : availablePresentModes) {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return presentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, VkExtent2D windowExtent) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        } else {
            VkExtent2D extent{};
            extent.width =
                    std::clamp(windowExtent.width, capabilities.minImageExtent.width,
                               capabilities.maxImageExtent.width);
            extent.height =
                    std::clamp(windowExtent.height, capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height);
            return extent;
        }
    }

    struct vkSwapChain {
        VkSwapchainKHR swapchainKhr;
        VkSurfaceFormatKHR surfaceFormat;
        VkExtent2D extent2D;
    };

    static std::optional<vkSwapChain> createVKSwapChain(
            VkDevice device, VkPhysicalDevice physicalDevice,
            VkSurfaceKHR surface, VkExtent2D windowExtent,
            QueueFamilyIndices familyIndices) {
        SwapChainSupportDetails swapChainSupport =
                querySwapChainSupport(physicalDevice, surface);

        VkSurfaceFormatKHR surfaceFormat =
                chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode =
                choosePresentMode(swapChainSupport.presentModes);
        VkExtent2D extent =
                chooseSwapExtent(swapChainSupport.capabilities, windowExtent);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 &&
            imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .surface = surface,
                .minImageCount = imageCount,
                .imageFormat = surfaceFormat.format,
                .imageColorSpace = surfaceFormat.colorSpace,
                .imageExtent = extent,
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .preTransform = swapChainSupport.capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = presentMode,
                .clipped = true,
                .oldSwapchain = {},
        };

        if (familyIndices.graphics != familyIndices.present) {
            uint32_t indices[] = {familyIndices.graphics.value(),
                                  familyIndices.present.value()};
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = indices;
        } else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        VkSwapchainKHR swapchain;
        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
            return {};
        }

        return vkSwapChain{
                .swapchainKhr = swapchain,
                .surfaceFormat = surfaceFormat,
                .extent2D = extent,
        };
    }

    static std::vector<VkImageView> createImageViews(
            VkDevice device, const std::vector<VkImage> &images,
            VkFormat format) {
        std::vector<VkImageView> views(images.size());

        for (size_t i = 0; i < images.size(); i++) {
            VkComponentMapping components{
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            };

            VkImageSubresourceRange subresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
            };

            VkImageViewCreateInfo createInfo{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = images[i],
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = format,
                    .components = components,
                    .subresourceRange = subresourceRange,
            };

            vkCreateImageView(device, &createInfo, nullptr, &views[i]);
        }

        return std::move(views);
    }

    SwapChain createSwapChain(VkDevice device, VkPhysicalDevice physicalDevice,
                              VkSurfaceKHR surface, VkExtent2D windowExtent,
                              QueueFamilyIndices familyIndices) {
        auto vkSwapChain = createVKSwapChain(device, physicalDevice, surface,
                                             windowExtent, familyIndices);
        if (!vkSwapChain) {
            return {};
        }

        uint32_t imageCount;
        vkGetSwapchainImagesKHR(device, vkSwapChain->swapchainKhr, &imageCount, nullptr);
        std::vector<VkImage> images(imageCount);
        vkGetSwapchainImagesKHR(device, vkSwapChain->swapchainKhr, &imageCount, images.data());

        std::vector<VkImageView> imageViews = createImageViews(device, images, vkSwapChain->surfaceFormat.format);


        return SwapChain{
                .swapchain = vkSwapChain.value().swapchainKhr,
                .images = std::move(images),
                .imageViews = std::move(imageViews),
                .format = vkSwapChain.value().surfaceFormat.format,
                .extent = vkSwapChain.value().extent2D,
        };
    }

    void SwapChain::shutdown(VkDevice device) {
        for (auto &view : imageViews) {
            vkDestroyImageView(device, view, nullptr);
            view = VK_NULL_HANDLE;
        }
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
    }

    uint32_t SwapChain::aquireImage(VkDevice device, VkSemaphore imageAvailableSemaphore) {
        uint32_t imageIndex;
        auto acquire = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, imageAvailableSemaphore, nullptr, &imageIndex);
        if (acquire == VK_ERROR_OUT_OF_DATE_KHR || acquire == VK_SUBOPTIMAL_KHR) {
            vkDeviceWaitIdle(device);


        }

        return imageIndex;
    }

    void SwapChain::present(VkQueue presentQueue, std::span<VkSemaphore> waitSemaphores, uint32_t imageIndex) {
        VkPresentInfoKHR presentInfo {
                .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
                .waitSemaphoreCount = static_cast<uint32_t>(waitSemaphores.size()),
                .pWaitSemaphores = waitSemaphores.data(),
                .swapchainCount = 1,
                .pSwapchains = &swapchain,
                .pImageIndices = &imageIndex,
                .pResults = nullptr,
        };

        if (vkQueuePresentKHR(presentQueue, &presentInfo) != VK_SUCCESS) {
            std::cerr << "Failed to present image\n";
        }
    }
}  // namespace g2::gfx