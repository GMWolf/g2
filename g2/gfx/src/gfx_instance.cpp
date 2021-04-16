//
// Created by felix on 13/04/2021.
//

#include <g2/gfx_instance.h>

#include <glm/glm.hpp>
#include <iostream>
#include <iterator>
#include <vector>

#include "device.h"
#include "fstream"
#include "pipeline.h"
#include "renderpass.h"
#include "shader.h"
#include "swapchain.h"
#include "validation.h"
#include "vk.h"

static void recordCommands(const vk::CommandBuffer &cmd,
                           const vk::Extent2D &renderExtent,
                           vk::RenderPass renderpass, vk::Pipeline pipeline,
                           vk::Framebuffer framebuffer) {
  vk::CommandBufferBeginInfo beginInfo{
      .flags = {},
      .pInheritanceInfo = nullptr,
  };

  if (cmd.begin(beginInfo) != vk::Result::eSuccess) {
    std::cerr << "Failed to begin command buffer" << std::endl;
  }

  vk::ClearValue clearValue =
      vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f});

  vk::RenderPassBeginInfo renderPassInfo{
      .renderPass = renderpass,
      .framebuffer = framebuffer,
      .renderArea =
          vk::Rect2D{
              .offset = {0, 0},
              .extent = renderExtent,
          },
      .clearValueCount = 1,
      .pClearValues = &clearValue,
  };

  vk::Viewport viewport{
      .x = 0,
      .y = 0,
      .width = static_cast<float>(renderExtent.width),
      .height = static_cast<float>(renderExtent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  vk::Rect2D scissor{
      .offset = {0, 0},
      .extent = renderExtent,
  };
  cmd.setViewport(0, viewport);
  cmd.setScissor(0, scissor);
  cmd.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
  cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline);
  cmd.draw(3, 1, 0, 0);

  cmd.endRenderPass();

  if (cmd.end() != vk::Result::eSuccess) {
    std::cerr << "Failed to record command buffer\n";
  }
}

namespace g2::gfx {

static const int MAX_FRAMES_IN_FLIGHT = 2;

struct Instance::Impl {
  vk::Instance vkInstance;
  vk::PhysicalDevice physicalDevice;
  vk::Device vkDevice;
  QueueFamilyIndices queue_family_indices;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;
  vk::SurfaceKHR surface;
  SwapChain swapChain;
  vk::Extent2D framebufferExtent;

  vk::RenderPass renderPass;
  Pipeline pipeline;

  std::vector<vk::Framebuffer> frameBuffers;

  vk::CommandPool commandPool;
  std::vector<vk::CommandBuffer> commandBuffers;

  vk::Semaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
  vk::Semaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];

  vk::Fence inFlightFences[MAX_FRAMES_IN_FLIGHT];
  std::vector<vk::Fence> imagesInFlight;

  size_t currentFrame = 0;
};

static vk::Instance createVkInstance(const InstanceConfig &config) {

  if (!checkValidationSupport()) {
    return {};
  }

  vk::ApplicationInfo appInfo{
      .pApplicationName = "unnamed g2 app",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "g2",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_2,
  };

  auto validationLayers = getValidationLayerNames();

  vk::InstanceCreateInfo createInfo{
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
      .ppEnabledLayerNames = validationLayers.data(),
      .enabledExtensionCount =
          static_cast<uint32_t>(config.vkExtensions.size()),
      .ppEnabledExtensionNames = config.vkExtensions.data(),
  };

  auto instanceResult = vk::createInstance(createInfo);

  if (instanceResult.result != vk::Result::eSuccess) {
    return {};
  }

  return instanceResult.value;
}

static vk::CommandPool createCommandPool(vk::Device device,
                                         QueueFamilyIndices familyIndices) {
  vk::CommandPoolCreateInfo poolInfo{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
               vk::CommandPoolCreateFlagBits::eTransient,
      .queueFamilyIndex = familyIndices.graphics.value(),
  };

  auto pool = device.createCommandPool(poolInfo);
  if (pool.result != vk::Result::eSuccess) {
    return {};
  }
  return pool.value;
}

static void createFrameBuffers(vk::Device device, std::span<vk::Framebuffer> framebuffers, std::span<vk::ImageView> imageViews, vk::Extent2D extent, vk::RenderPass renderPass) {
  for (size_t i = 0; i < imageViews.size(); i++) {
    vk::ImageView attachments[] = {imageViews[i]};

    vk::FramebufferCreateInfo frameBufferInfo{
        .renderPass = renderPass,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .width = extent.width,
        .height = extent.height,
        .layers = 1,
    };

    auto frameBufferResult = device.createFramebuffer(frameBufferInfo);
    if (frameBufferResult.result != vk::Result::eSuccess) {
      std::cerr << "Error creating framebuffer\n";
    }

    framebuffers[i] = frameBufferResult.value;
  }
}


Instance::Instance(const InstanceConfig &config) {
  pImpl = std::make_unique<Impl>();

  glm::ivec2 appSize = config.application->getWindowSize();
  vk::Extent2D appExtent{static_cast<uint32_t>(appSize.x),
                         static_cast<uint32_t>(appSize.y)};

  pImpl->framebufferExtent = appExtent;

  pImpl->vkInstance = createVkInstance(config);

  pImpl->surface = config.application->createSurface(pImpl->vkInstance);

  auto physicalDeviceResult =
      pickPhysicalDevice(pImpl->vkInstance, pImpl->surface);
  if (!physicalDeviceResult) {
    std::cerr << "failed to get physical device\n";
    return;
  }

  pImpl->physicalDevice = physicalDeviceResult.value();

  auto deviceResult = createDevice(pImpl->physicalDevice, pImpl->surface);
  if (!deviceResult) {
    std::cerr << "failed to create device result\n";
    return;
  }
  pImpl->vkDevice = deviceResult.value().first;
  QueueFamilyIndices queueFamilyIndices = deviceResult.value().second;

  pImpl->graphicsQueue =
      pImpl->vkDevice.getQueue(queueFamilyIndices.graphics.value(), 0);
  pImpl->presentQueue =
      pImpl->vkDevice.getQueue(queueFamilyIndices.present.value(), 0);



  auto swapChain =
      createSwapChain(pImpl->vkDevice, pImpl->physicalDevice, pImpl->surface,
                      appExtent, queueFamilyIndices);
  if (!swapChain) {
    std::cerr << "failed to create swapchain\n";
    return;
  }

  pImpl->swapChain = swapChain;

  pImpl->renderPass = createRenderPass(pImpl->vkDevice, swapChain.format);

  std::ifstream vertexInput("vert.spv", std::ios::binary);
  std::vector<char> vertexBytes((std::istreambuf_iterator<char>(vertexInput)),
                                (std::istreambuf_iterator<char>()));

  std::ifstream fragmentInput("frag.spv", std::ios::binary);
  std::vector<char> fragmentBytes(
      (std::istreambuf_iterator<char>(fragmentInput)),
      (std::istreambuf_iterator<char>()));

  vk::ShaderModule vertexModule =
      createShaderModule(pImpl->vkDevice, vertexBytes);
  vk::ShaderModule fragmentModule =
      createShaderModule(pImpl->vkDevice, fragmentBytes);

  // TODO potentially leaking one of the shaders
  if (!vertexModule || !fragmentModule) {
    std::cerr << "failed to create shader modules\n";
    return;
  }

  pImpl->pipeline = createPipeline(pImpl->vkDevice, vertexModule,
                                   fragmentModule, pImpl->renderPass, 0);

  pImpl->vkDevice.destroyShaderModule(vertexModule);
  pImpl->vkDevice.destroyShaderModule(fragmentModule);

  if (!pImpl->pipeline) {
    std::cerr << "failed to create pipeline\n";
    return;
  }

  // Create framebuffers

  pImpl->frameBuffers.resize(swapChain.imageViews.size());
  createFrameBuffers(pImpl->vkDevice, pImpl->frameBuffers, swapChain.imageViews, swapChain.extent, pImpl->renderPass);

  pImpl->commandPool = createCommandPool(pImpl->vkDevice, queueFamilyIndices);

  vk::CommandBufferAllocateInfo allocInfo{
      .commandPool = pImpl->commandPool,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
  };

  auto allocResult = pImpl->vkDevice.allocateCommandBuffers(allocInfo);

  if (allocResult.result != vk::Result::eSuccess) {
    std::cerr << "failed to alloc command buffers\n";
    return;
  }

  pImpl->commandBuffers = std::move(allocResult.value);

  // Create semaphores & fences
  vk::SemaphoreCreateInfo semaphoreInfo{};
  vk::FenceCreateInfo fenceInfo{.flags = vk::FenceCreateFlagBits::eSignaled};

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    pImpl->imageAvailableSemaphores[i] =
        pImpl->vkDevice.createSemaphore(semaphoreInfo).value;
    pImpl->renderFinishedSemaphores[i] =
        pImpl->vkDevice.createSemaphore(semaphoreInfo).value;
    pImpl->inFlightFences[i] = pImpl->vkDevice.createFence(fenceInfo).value;
  }

  pImpl->imagesInFlight.resize(pImpl->swapChain.images.size());
}

Instance::~Instance() {
  vk::Result waitIdleResult = pImpl->vkDevice.waitIdle();

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    pImpl->vkDevice.destroySemaphore(pImpl->imageAvailableSemaphores[i]);
    pImpl->vkDevice.destroySemaphore(pImpl->renderFinishedSemaphores[i]);
    pImpl->vkDevice.destroyFence(pImpl->inFlightFences[i]);
  }

  pImpl->vkDevice.destroyCommandPool(pImpl->commandPool);

  for (auto framebuffer : pImpl->frameBuffers) {
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

void Instance::drawFrame() {
  // Wait for cmd buffer to be done being used
  vk::Result waitResult = pImpl->vkDevice.waitForFences(
      pImpl->inFlightFences[pImpl->currentFrame], true, UINT64_MAX);

  auto acquire = pImpl->vkDevice.acquireNextImageKHR(
      pImpl->swapChain.swapchain, UINT64_MAX,
      pImpl->imageAvailableSemaphores[pImpl->currentFrame]);

  if (acquire.result == vk::Result::eErrorOutOfDateKHR || acquire.result == vk::Result::eSuboptimalKHR ) {

    auto waitIdleResult = pImpl->vkDevice.waitIdle();

    //We need to reset semaphore. simple way is to recreate it
    pImpl->vkDevice.destroySemaphore(pImpl->imageAvailableSemaphores[pImpl->currentFrame]);
    vk::SemaphoreCreateInfo semaphoreInfo{};
    pImpl->imageAvailableSemaphores[pImpl->currentFrame] = pImpl->vkDevice.createSemaphore(semaphoreInfo).value;

    auto prevFormat = pImpl->swapChain.format;
    pImpl->swapChain.shutdown(pImpl->vkDevice);
    pImpl->swapChain = createSwapChain(pImpl->vkDevice, pImpl->physicalDevice, pImpl->surface, pImpl->framebufferExtent, pImpl->queue_family_indices);
    assert(prevFormat == pImpl->swapChain.format);

    for(auto framebuffer : pImpl->frameBuffers) {
      pImpl->vkDevice.destroyFramebuffer(framebuffer);
    }
    createFrameBuffers(pImpl->vkDevice, pImpl->frameBuffers, pImpl->swapChain.imageViews, pImpl->swapChain.extent, pImpl->renderPass);

    return;
  } else if (acquire.result != vk::Result::eSuccess && acquire.result != vk::Result::eSuboptimalKHR) {
    std::cerr << "Error acquiring image\n";
    return;
  }

  uint32_t imageIndex = acquire.value;

  vk::CommandBuffer cmd = pImpl->commandBuffers[pImpl->currentFrame];
  vk::Extent2D renderExtent = pImpl->swapChain.extent;

  cmd.reset({});
  recordCommands(cmd, renderExtent, pImpl->renderPass, pImpl->pipeline.pipeline,
                 pImpl->frameBuffers[imageIndex]);

  if (pImpl->imagesInFlight[imageIndex]) {
    vk::Result r = pImpl->vkDevice.waitForFences(
        pImpl->imagesInFlight[imageIndex], true, UINT64_MAX);
  }
  pImpl->imagesInFlight[imageIndex] =
      pImpl->inFlightFences[pImpl->currentFrame];

  vk::Semaphore waitSemaphores[] = {
      pImpl->imageAvailableSemaphores[pImpl->currentFrame]};
  vk::Semaphore signalSemaphores[] = {
      pImpl->renderFinishedSemaphores[pImpl->currentFrame]};
  vk::PipelineStageFlags waitStages[] = {
      vk::PipelineStageFlagBits::eColorAttachmentOutput};

  vk::SubmitInfo submitInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = waitSemaphores,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &cmd,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = signalSemaphores,
  };

  pImpl->vkDevice.resetFences(pImpl->inFlightFences[pImpl->currentFrame]);
  if (pImpl->graphicsQueue.submit(1, &submitInfo,
                                  pImpl->inFlightFences[pImpl->currentFrame]) !=
      vk::Result::eSuccess) {
    std::cerr << "failed to submit command buffers\n";
  }

  vk::SwapchainKHR swapChains[] = {pImpl->swapChain.swapchain};

  vk::PresentInfoKHR presentInfo{
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signalSemaphores,
      .swapchainCount = 1,
      .pSwapchains = swapChains,
      .pImageIndices = &imageIndex,
      .pResults = nullptr,
  };

  if (pImpl->presentQueue.presentKHR(presentInfo) != vk::Result::eSuccess) {
    std::cerr << "Failed to present image\n";
  }

  pImpl->currentFrame = (pImpl->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
void Instance::setFramebufferExtent(glm::ivec2 size) {
  pImpl->framebufferExtent.width = size.x;
  pImpl->framebufferExtent.height = size.y;
}
}  // namespace g2::gfx