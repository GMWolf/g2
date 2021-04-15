//
// Created by felix on 13/04/2021.
//

#include <g2/gfx_instance.h>

#include "vk.h"
#include "validation.h"
#include "device.h"
#include "swapchain.h"
#include <glm/glm.hpp>
#include "shader.h"
#include "renderpass.h"
#include "pipeline.h"
#include "fstream"
#include <vector>
#include <iterator>
#include <iostream>

namespace g2::gfx
{
    struct Instance::Impl
    {
        vk::Instance vkInstance;
        vk::PhysicalDevice physicalDevice;
        vk::Device vkDevice;
        vk::Queue graphicsQueue;
        vk::Queue presentQueue;
        vk::SurfaceKHR surface;
        SwapChain swapChain;

        vk::RenderPass renderPass;
        Pipeline pipeline;

        std::vector<vk::Framebuffer> frameBuffers;

        vk::CommandPool commandPool;
        std::vector<vk::CommandBuffer> commandBuffers;

        vk::Semaphore imageAvailableSemaphore;
        vk::Semaphore renderFinishedSemaphore;

    };


    static vk::Instance createVkInstance( const InstanceConfig& config )
    {
        if (!checkValidationSupport())
        {
            return {};
        }

        vk::ApplicationInfo appInfo
        {
            .pApplicationName = "unnamed g2 app",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "g2",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_2,
        };

        auto validationLayers = getValidationLayerNames();

        vk::InstanceCreateInfo createInfo
        {
            .pApplicationInfo = &appInfo,
            .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
            .ppEnabledLayerNames = validationLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(config.vkExtensions.size()),
            .ppEnabledExtensionNames = config.vkExtensions.data(),
        };

        auto instanceResult = vk::createInstance(createInfo);

        if (instanceResult.result != vk::Result::eSuccess)
        {
            return {};
        }

        return instanceResult.value;
    }

    static vk::CommandPool createCommandPool(vk::Device device, QueueFamilyIndices familyIndices)
    {
        vk::CommandPoolCreateInfo poolInfo
        {
            .flags = {},
            .queueFamilyIndex = familyIndices.graphics.value(),
        };

        auto pool = device.createCommandPool(poolInfo);
        if (pool.result != vk::Result::eSuccess)
        {
            return {};
        }
        return pool.value;
    }


    Instance::Instance(const InstanceConfig& config)
    {
        pImpl = std::make_unique<Impl>();

        pImpl->vkInstance = createVkInstance( config );

        pImpl->surface = config.application->createSurface(pImpl->vkInstance );

        auto physicalDeviceResult = pickPhysicalDevice(pImpl->vkInstance, pImpl->surface);
        if (!physicalDeviceResult)
        {
            std::cerr << "failed to get physical device\n";
            return;
        }

        pImpl->physicalDevice = physicalDeviceResult.value();

        auto deviceResult = createDevice(pImpl->physicalDevice, pImpl->surface);
        if (!deviceResult)
        {
            std::cerr << "failed to create device result\n";
            return;
        }
        pImpl->vkDevice = deviceResult.value().first;
        QueueFamilyIndices queueFamilyIndices = deviceResult.value().second;

        pImpl->graphicsQueue = pImpl->vkDevice.getQueue(queueFamilyIndices.graphics.value(), 0);
        pImpl->presentQueue = pImpl->vkDevice.getQueue(queueFamilyIndices.present.value(), 0);

        glm::ivec2 appSize = config.application->getWindowSize();
        vk::Extent2D appExtent{static_cast<uint32_t>(appSize.x), static_cast<uint32_t>(appSize.y)};

        auto swapChain = createSwapChain(pImpl->vkDevice, pImpl->physicalDevice, pImpl->surface, appExtent, queueFamilyIndices);
        if (!swapChain)
        {
            std::cerr << "failed to create swapchain\n";
            return;
        }

        pImpl->swapChain = swapChain;

        pImpl->renderPass = createRenderPass(pImpl->vkDevice, swapChain.format);

        std::ifstream vertexInput("vert.spv", std::ios::binary);
        std::vector<char> vertexBytes((std::istreambuf_iterator<char>(vertexInput)), (std::istreambuf_iterator<char>()));

        std::ifstream fragmentInput("frag.spv", std::ios::binary);
        std::vector<char> fragmentBytes((std::istreambuf_iterator<char>(fragmentInput)), (std::istreambuf_iterator<char>()));

        vk::ShaderModule vertexModule = createShaderModule(pImpl->vkDevice, vertexBytes);
        vk::ShaderModule fragmentModule = createShaderModule(pImpl->vkDevice, fragmentBytes);

        //TODO potentially leaking one of the shaders
        if (!vertexModule || !fragmentModule)
        {
            std::cerr << "failed to create shader modules\n";
            return;
        }

        pImpl->pipeline = createPipeline(pImpl->vkDevice, vertexModule, fragmentModule, pImpl->renderPass, 0);

        pImpl->vkDevice.destroyShaderModule(vertexModule);
        pImpl->vkDevice.destroyShaderModule(fragmentModule);

        if (!pImpl->pipeline)
        {
            std::cerr << "failed to create pipeline\n";
            return;
        }

        //Create framebuffers

        pImpl->frameBuffers.resize(swapChain.imageViews.size());

        for(size_t i = 0; i < swapChain.imageViews.size(); i++)
        {
            vk::ImageView attachments[] = { swapChain.imageViews[i]};

            vk::FramebufferCreateInfo frameBufferInfo
            {
                .renderPass = pImpl->renderPass,
                .attachmentCount = 1,
                .pAttachments = attachments,
                .width = swapChain.extent.width,
                .height = swapChain.extent.height,
                .layers = 1,
            };

            auto frameBufferResult = pImpl->vkDevice.createFramebuffer(frameBufferInfo);
            if (frameBufferResult.result != vk::Result::eSuccess)
            {
                return;
            }

            pImpl->frameBuffers[i] = frameBufferResult.value;
        }

        pImpl->commandPool = createCommandPool(pImpl->vkDevice, queueFamilyIndices);

        vk::CommandBufferAllocateInfo allocInfo
        {
            .commandPool = pImpl->commandPool,
            .level = vk::CommandBufferLevel::ePrimary,
            .commandBufferCount = static_cast<uint32_t>(pImpl->frameBuffers.size()),
        };

        auto allocResult = pImpl->vkDevice.allocateCommandBuffers(allocInfo);

        if (allocResult.result != vk::Result::eSuccess)
        {
            std::cerr << "failed to alloc command buffers\n";
            return;
        }

        pImpl->commandBuffers = allocResult.value;


        for(size_t i = 0; i < pImpl->commandBuffers.size(); i++)
        {

            vk::CommandBuffer cmd = pImpl->commandBuffers[i];

            vk::CommandBufferBeginInfo beginInfo
            {
                .flags = {},
                .pInheritanceInfo = nullptr,
            };

            if (cmd.begin(beginInfo) != vk::Result::eSuccess)
            {
                std::cerr << "Failed to begin command buffer" << std::endl;
            }

            vk::ClearValue clearValue = vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});

            vk::RenderPassBeginInfo renderPassInfo
            {
                .renderPass = pImpl->renderPass,
                .framebuffer = pImpl->frameBuffers[i],
                .renderArea = vk::Rect2D
                {
                    .offset = {0,0},
                    .extent = swapChain.extent,
                },
                .clearValueCount = 1,
                .pClearValues = &clearValue,
            };

            vk::Viewport viewport
            {
                .x = 0,
                .y = 0,
                .width = static_cast<float>(pImpl->swapChain.extent.width),
                .height = static_cast<float>(pImpl->swapChain.extent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f,
            };
            vk::Rect2D scissor
            {
                .offset = {0,0},
                .extent = pImpl->swapChain.extent,
            };
            cmd.setViewport(0, viewport);
            cmd.setScissor(0, scissor);
            cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pImpl->pipeline.pipeline);
            cmd.draw(3, 1, 0, 0);

            cmd.endRenderPass();

            if (cmd.end() != vk::Result::eSuccess)
            {
                std::cerr << "Failed to record command buffer\n";
                return;
            }
        }

        vk::SemaphoreCreateInfo semaphoreInfo{};

        pImpl->imageAvailableSemaphore = pImpl->vkDevice.createSemaphore(semaphoreInfo).value;
        pImpl->renderFinishedSemaphore = pImpl->vkDevice.createSemaphore(semaphoreInfo).value;

    }

    Instance::~Instance()
    {

        pImpl->vkDevice.destroySemaphore(pImpl->imageAvailableSemaphore);
        pImpl->vkDevice.destroySemaphore(pImpl->renderFinishedSemaphore);

        pImpl->vkDevice.destroyCommandPool(pImpl->commandPool);

        for(auto framebuffer : pImpl->frameBuffers)
        {
            pImpl->vkDevice.destroyFramebuffer(framebuffer);
        }

        pImpl->vkDevice.destroyPipelineLayout(pImpl->pipeline.pipelineLayout);
        pImpl->vkDevice.destroyPipeline(pImpl->pipeline.pipeline);
        pImpl->vkDevice.destroyRenderPass(pImpl->renderPass);

        pImpl->swapChain.shutdown(pImpl->vkDevice);

        pImpl->vkDevice.destroy();
        pImpl->vkInstance.destroySurfaceKHR(pImpl->surface);
        pImpl->vkInstance.destroy();
    }

    void Instance::drawFrame()
    {

        auto acquire = pImpl->vkDevice.acquireNextImageKHR(pImpl->swapChain.swapchain, UINT64_MAX, pImpl->imageAvailableSemaphore);

        if (acquire.result != vk::Result::eSuccess)
        {
            std::cerr << "Error acquiring image\n";
        }

        uint32_t imageIndex = acquire.value;

        vk::Semaphore waitSemaphores[] = {pImpl->imageAvailableSemaphore};
        vk::Semaphore signalSemaphores[] = {pImpl->renderFinishedSemaphore};
        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

        vk::SubmitInfo submitInfo
        {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .pWaitDstStageMask = waitStages,
            .commandBufferCount = 1,
            .pCommandBuffers = &pImpl->commandBuffers[imageIndex],
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = signalSemaphores,
        };

        if (pImpl->graphicsQueue.submit(1, &submitInfo, {}) != vk::Result::eSuccess)
        {
            std::cerr << "failed to submit command buffers\n";
        }

        vk::SwapchainKHR swapChains[] = {pImpl->swapChain.swapchain};

        vk::PresentInfoKHR presentInfo
        {
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = signalSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapChains,
            .pImageIndices = &imageIndex,
            .pResults = nullptr,
        };

        if (pImpl->presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess)
        {
            std::cerr << "Failed to present image\n";
        }


        auto r = pImpl->graphicsQueue.waitIdle();
        if (r != vk::Result::eSuccess)
        {
            std::cerr << "Error wating idle\n";
        }

    }
}