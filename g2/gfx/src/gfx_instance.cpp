//
// Created by felix on 13/04/2021.
//

#include <g2/gfx_instance.h>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#include <glm/glm.hpp>
#include <iostream>
#include <iterator>
#include <vector>

#include "device.h"
#include "fstream"
#include "mesh.h"
#include "pipeline.h"
#include "renderpass.h"
#include "shader.h"
#include "swapchain.h"
#include "validation.h"
#include "descriptors.h"
#include "image.h"

namespace g2::gfx {

static const int MAX_FRAMES_IN_FLIGHT = 2;

struct Instance::Impl {
  VkInstance vkInstance;
  VkPhysicalDevice physicalDevice;
  VkDevice vkDevice;
  QueueFamilyIndices queue_family_indices;
  VkQueue graphicsQueue;
  VkQueue presentQueue;
  VkSurfaceKHR surface;
  SwapChain swapChain;
  VkExtent2D framebufferExtent;

  VkRenderPass renderPass;

  std::vector<VkPipeline> pipelines;

  std::vector<VkFramebuffer> frameBuffers;

  VmaAllocator allocator;

  GlobalDescriptors descriptors;

  MeshBuffer meshBuffer;
  Mesh mesh;

  VkCommandPool commandPool;
  std::vector<VkCommandBuffer> commandBuffers;

  VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
  VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];

  VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
  std::vector<VkFence> imagesInFlight;

  size_t currentFrame = 0;
  size_t currentFrameBufferIndex;
};

static VkInstance createVkInstance(const InstanceConfig &config) {
  if (!checkValidationSupport()) {
    std::cerr << "Validation support check failed\n";
    return {};
  }

  VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "unnamed g2 app",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "g2",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_2,
  };

  auto validationLayers = getValidationLayerNames();

  VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
      .enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
      .ppEnabledLayerNames = validationLayers.data(),
      .enabledExtensionCount =
          static_cast<uint32_t>(config.vkExtensions.size()),
      .ppEnabledExtensionNames = config.vkExtensions.data(),
  };

  VkInstance instance;
  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    std::cerr << "Failed to create vulkan instance\n";
    return VK_NULL_HANDLE;
  }

  return instance;
}

static VkCommandPool createCommandPool(VkDevice device,
                                       QueueFamilyIndices familyIndices) {
  VkCommandPoolCreateInfo poolInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT |
               VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
      .queueFamilyIndex = familyIndices.graphics.value(),
  };

  VkCommandPool pool;
  if (vkCreateCommandPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
    return VK_NULL_HANDLE;
  }

  return pool;
}

static void createFrameBuffers(VkDevice device,
                               std::span<VkFramebuffer> framebuffers,
                               std::span<VkImageView> imageViews,
                               VkExtent2D extent, VkRenderPass renderPass) {
  for (size_t i = 0; i < imageViews.size(); i++) {
    VkImageView attachments[] = {imageViews[i]};

    VkFramebufferCreateInfo frameBufferInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderPass,
        .attachmentCount = 1,
        .pAttachments = attachments,
        .width = extent.width,
        .height = extent.height,
        .layers = 1,
    };

    if (vkCreateFramebuffer(device, &frameBufferInfo, nullptr,
                            &framebuffers[i]) != VK_SUCCESS) {
      std::cerr << "Error creating framebuffer\n";
    }
  }
}

static VmaAllocator createAllocator(VkDevice device, VkInstance instance,
                                    VkPhysicalDevice physical_device) {
  VmaAllocatorCreateInfo allocatorInfo{
      .physicalDevice = physical_device,
      .device = device,
      .instance = instance,
      .vulkanApiVersion = VK_API_VERSION_1_2,
  };

  VmaAllocator allocator;
  vmaCreateAllocator(&allocatorInfo, &allocator);
  return allocator;
}

void init() {}

Instance::Instance(const InstanceConfig &config) {
  pImpl = std::make_unique<Impl>();

  glm::ivec2 appSize = config.application->getWindowSize();
  VkExtent2D appExtent{static_cast<uint32_t>(appSize.x),
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

  vkGetDeviceQueue(pImpl->vkDevice, queueFamilyIndices.graphics.value(), 0,
                   &pImpl->graphicsQueue);
  vkGetDeviceQueue(pImpl->vkDevice, queueFamilyIndices.present.value(), 0,
                   &pImpl->presentQueue);

  pImpl->allocator = createAllocator(pImpl->vkDevice, pImpl->vkInstance,
                                     pImpl->physicalDevice);

  auto swapChain =
      createSwapChain(pImpl->vkDevice, pImpl->physicalDevice, pImpl->surface,
                      appExtent, queueFamilyIndices);
  if (!swapChain) {
    std::cerr << "failed to create swapchain\n";
    return;
  }

  pImpl->swapChain = swapChain;

  pImpl->renderPass = createRenderPass(pImpl->vkDevice, swapChain.format);

  // Create framebuffers
  pImpl->frameBuffers.resize(swapChain.imageViews.size());
  createFrameBuffers(pImpl->vkDevice, pImpl->frameBuffers, swapChain.imageViews,
                     swapChain.extent, pImpl->renderPass);

  pImpl->commandPool = createCommandPool(pImpl->vkDevice, queueFamilyIndices);

  VkCommandBufferAllocateInfo allocInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = pImpl->commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
  };

  pImpl->commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
  if (vkAllocateCommandBuffers(pImpl->vkDevice, &allocInfo,
                               pImpl->commandBuffers.data()) != VK_SUCCESS) {
    std::cerr << "failed to alloc command buffers\n";
    return;
  }

  // Create semaphores & fences
  VkSemaphoreCreateInfo semaphoreInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
  };
  VkFenceCreateInfo fenceInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                              .flags = VK_FENCE_CREATE_SIGNALED_BIT};

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkCreateSemaphore(pImpl->vkDevice, &semaphoreInfo, nullptr,
                      &pImpl->imageAvailableSemaphores[i]);
    vkCreateSemaphore(pImpl->vkDevice, &semaphoreInfo, nullptr,
                      &pImpl->renderFinishedSemaphores[i]);
    vkCreateFence(pImpl->vkDevice, &fenceInfo, nullptr,
                  &pImpl->inFlightFences[i]);
  }

  pImpl->imagesInFlight.resize(pImpl->swapChain.images.size());

  // create gfx pipeline layout
  pImpl->descriptors = createGlobalDescriptors(pImpl->vkDevice);

  initMeshBuffer(pImpl->allocator, &pImpl->meshBuffer);

  {
    //update set

    VkDescriptorBufferInfo bufferInfo {
        .buffer = pImpl->meshBuffer.vertexBuffer.buffer,
        .offset = 0,
        .range = pImpl->meshBuffer.vertexBuffer.size,
    };

    VkWriteDescriptorSet descriptorWrite {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = pImpl->descriptors.resourceDescriptorSet,
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pImageInfo = nullptr,
        .pBufferInfo = &bufferInfo,
        .pTexelBufferView = nullptr,
    };

    vkUpdateDescriptorSets(pImpl->vkDevice, 1, &descriptorWrite, 0, nullptr);
  }

  // Add some mesh data
  {
    struct Vertex {
      float x, y, z, w;
      float r, g, b, a;
    } vertices[]{
        {0.0, -0.5, 0, 1, 1, 0, 1, 1},
        {0.5, 0.5, 0, 1, 1, 1, 0, 1},
        {-0.5, 0.5, 0, 1, 0, 1, 1},
    };

    uint32_t indices[]{0, 1, 2};

    MeshFormat meshFormat{
        .vertexByteSize = sizeof(Vertex),
        .indexType = VK_INDEX_TYPE_UINT32,
    };

    MeshBuffer *meshBuffer = &pImpl->meshBuffer;
    VkCommandBuffer cmd = pImpl->commandBuffers[0];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = {},
        .pInheritanceInfo = nullptr,
    };

    vkBeginCommandBuffer(cmd, &beginInfo);

    //add mesh

    pImpl->mesh =
        addMesh(cmd, meshBuffer, &meshFormat, vertices, 3, indices, 3);

    //add image
    std::ifstream imageStream("OldWoodPlanks.ktx2", std::ios::binary);
    std::vector<char> imageBytes((std::istreambuf_iterator<char>(imageStream)),
                                      (std::istreambuf_iterator<char>()));
    Image image = loadImage(cmd, pImpl->allocator, imageBytes);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd,
    };

    vkQueueSubmit(pImpl->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(pImpl->graphicsQueue);
  }
}

Instance::~Instance() {
  vkDeviceWaitIdle(pImpl->vkDevice);

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(pImpl->vkDevice, pImpl->imageAvailableSemaphores[i],
                       nullptr);
    vkDestroySemaphore(pImpl->vkDevice, pImpl->renderFinishedSemaphores[i],
                       nullptr);
    vkDestroyFence(pImpl->vkDevice, pImpl->inFlightFences[i], nullptr);
  }

  vkDestroyCommandPool(pImpl->vkDevice, pImpl->commandPool, nullptr);

  for (auto framebuffer : pImpl->frameBuffers) {
    vkDestroyFramebuffer(pImpl->vkDevice, framebuffer, nullptr);
  }

  for (auto &pipeline : pImpl->pipelines) {
    vkDestroyPipeline(pImpl->vkDevice, pipeline, nullptr);
  }

  vkDestroyRenderPass(pImpl->vkDevice, pImpl->renderPass, nullptr);

  pImpl->swapChain.shutdown(pImpl->vkDevice);

  vmaDestroyAllocator(pImpl->allocator);

  vkDestroyDevice(pImpl->vkDevice, nullptr);
  vkDestroySurfaceKHR(pImpl->vkInstance, pImpl->surface, nullptr);
  vkDestroyInstance(pImpl->vkInstance, nullptr);
}

void Instance::setFramebufferExtent(glm::ivec2 size) {
  pImpl->framebufferExtent.width = size.x;
  pImpl->framebufferExtent.height = size.y;
}

VkPipeline Instance::createPipeline(const PipelineDef *pipeline_def) {
  VkPipeline pipeline = ::g2::gfx::createPipeline(pImpl->vkDevice, pipeline_def,
                                                  pImpl->descriptors.pipelineLayout,
                                                  pImpl->swapChain.format);
  pImpl->pipelines.push_back(pipeline);
  return pipeline;
}

CommandEncoder Instance::beginRenderpass() {
  VkClearValue clearValue = {0.0f, 0.0f, 0.0f, 1.0f};

  VkRenderPassBeginInfo renderPassInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = pImpl->renderPass,
      .framebuffer = pImpl->frameBuffers[pImpl->currentFrameBufferIndex],
      .renderArea =
          VkRect2D{
              .offset = {0, 0},
              .extent = pImpl->swapChain.extent,
          },
      .clearValueCount = 1,
      .pClearValues = &clearValue,
  };

  VkViewport viewport{
      .x = 0,
      .y = 0,
      .width = static_cast<float>(pImpl->swapChain.extent.width),
      .height = static_cast<float>(pImpl->swapChain.extent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f,
  };
  VkRect2D scissor{
      .offset = {0, 0},
      .extent = pImpl->swapChain.extent,
  };

  VkCommandBuffer cmd = pImpl->commandBuffers[pImpl->currentFrame];

  vkCmdSetViewport(cmd, 0, 1, &viewport);
  vkCmdSetScissor(cmd, 0, 1, &scissor);
  vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pImpl->descriptors.pipelineLayout,
                          0, 1, &pImpl->descriptors.resourceDescriptorSet, 0, nullptr);

    vkCmdBindIndexBuffer(cmd, pImpl->meshBuffer.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

  return CommandEncoder{
      .cmdbuf = reinterpret_cast<uintptr_t>(cmd),
  };
}
void Instance::endRenderpass(CommandEncoder &command_encoder) {
  VkCommandBuffer cmd =
      reinterpret_cast<VkCommandBuffer>(command_encoder.cmdbuf);
  vkCmdEndRenderPass(cmd);
  command_encoder.cmdbuf = 0;
}
bool Instance::beginFrame() {
  vkWaitForFences(pImpl->vkDevice, 1,
                  &pImpl->inFlightFences[pImpl->currentFrame], true,
                  UINT64_MAX);

  uint32_t imageIndex;
  auto acquire = vkAcquireNextImageKHR(
      pImpl->vkDevice, pImpl->swapChain.swapchain, UINT64_MAX,
      pImpl->imageAvailableSemaphores[pImpl->currentFrame], nullptr,
      &imageIndex);

  if (acquire == VK_ERROR_OUT_OF_DATE_KHR || acquire == VK_SUBOPTIMAL_KHR) {
    vkDeviceWaitIdle(pImpl->vkDevice);

    // We need to reset semaphore. simple way is to recreate it
    vkDestroySemaphore(pImpl->vkDevice,
                       pImpl->imageAvailableSemaphores[pImpl->currentFrame],
                       nullptr);
    VkSemaphoreCreateInfo semaphoreInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    };
    vkCreateSemaphore(pImpl->vkDevice, &semaphoreInfo, nullptr,
                      &pImpl->imageAvailableSemaphores[pImpl->currentFrame]);

    auto prevFormat = pImpl->swapChain.format;
    pImpl->swapChain.shutdown(pImpl->vkDevice);
    pImpl->swapChain =
        createSwapChain(pImpl->vkDevice, pImpl->physicalDevice, pImpl->surface,
                        pImpl->framebufferExtent, pImpl->queue_family_indices);
    assert(prevFormat == pImpl->swapChain.format);

    for (auto framebuffer : pImpl->frameBuffers) {
      vkDestroyFramebuffer(pImpl->vkDevice, framebuffer, nullptr);
    }
    createFrameBuffers(pImpl->vkDevice, pImpl->frameBuffers,
                       pImpl->swapChain.imageViews, pImpl->swapChain.extent,
                       pImpl->renderPass);

    return false;
  } else if (acquire != VK_SUCCESS) {
    std::cerr << "Error acquiring image\n";
    return false;
  }

  pImpl->currentFrameBufferIndex = imageIndex;

  VkCommandBuffer cmd = pImpl->commandBuffers[pImpl->currentFrame];
  vkResetCommandBuffer(cmd, 0);

  VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = {},
      .pInheritanceInfo = nullptr,
  };

  if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
    std::cerr << "Failed to begin command buffer" << std::endl;
  }

  return true;
}
void Instance::endFrame() {
  uint32_t imageIndex = pImpl->currentFrameBufferIndex;
  VkCommandBuffer cmd = pImpl->commandBuffers[pImpl->currentFrame];

  if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
    std::cerr << "Failed to record command buffer\n";
  }

  if (pImpl->imagesInFlight[imageIndex]) {
    vkWaitForFences(pImpl->vkDevice, 1, &pImpl->imagesInFlight[imageIndex],
                    true, UINT64_MAX);
  }

  pImpl->imagesInFlight[imageIndex] =
      pImpl->inFlightFences[pImpl->currentFrame];

  VkSemaphore waitSemaphores[] = {
      pImpl->imageAvailableSemaphores[pImpl->currentFrame]};
  VkSemaphore signalSemaphores[] = {
      pImpl->renderFinishedSemaphores[pImpl->currentFrame]};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

  VkSubmitInfo submitInfo{
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = waitSemaphores,
      .pWaitDstStageMask = waitStages,
      .commandBufferCount = 1,
      .pCommandBuffers = &cmd,
      .signalSemaphoreCount = 1,
      .pSignalSemaphores = signalSemaphores,
  };

  vkResetFences(pImpl->vkDevice, 1,
                &pImpl->inFlightFences[pImpl->currentFrame]);

  if (vkQueueSubmit(pImpl->graphicsQueue, 1, &submitInfo,
                    pImpl->inFlightFences[pImpl->currentFrame]) != VK_SUCCESS) {
    std::cerr << "failed to submit command buffers\n";
  }

  VkSwapchainKHR swapChains[] = {pImpl->swapChain.swapchain};

  VkPresentInfoKHR presentInfo{
      .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores = signalSemaphores,
      .swapchainCount = 1,
      .pSwapchains = swapChains,
      .pImageIndices = &imageIndex,
      .pResults = nullptr,
  };

  if (vkQueuePresentKHR(pImpl->presentQueue, &presentInfo) != VK_SUCCESS) {
    std::cerr << "Failed to present image\n";
  }

  pImpl->currentFrame = (pImpl->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}
}  // namespace g2::gfx