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
#include <glm/gtc/matrix_transform.hpp>
#include "upload.h"
#include "material.h"
#include <thread>
#include "effect.h"

namespace g2::gfx {

    static const int MAX_FRAMES_IN_FLIGHT = 2;

    struct DrawData {
        uint32_t baseVertex;
        uint32_t materialId;
    };

    struct UScene{
        glm::mat4 mat;
        glm::vec3 viewPos; float pad;
        glm::mat4 shadowMat;
    };

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

        RenderGraph* renderGraph;

        MeshAssetManager meshManager;
        ImageAssetManager imageManager;
        PipelineAssetManager pipelineAssetManager;
        MaterialAssetManager materialManager;
        EffectAssetManager effectAssetManager;

        IAssetManager* assetManagers[5];

        VkSampler sampler;

        VmaAllocator allocator;

        GlobalDescriptors descriptors;

        MeshBuffer meshBuffer;
        UploadQueue uploadQueue;
        std::thread uploadWorker;

        Buffer sceneBuffer[MAX_FRAMES_IN_FLIGHT];
        UScene* sceneBufferMap[MAX_FRAMES_IN_FLIGHT];
        const size_t maxDrawCount = 1024 * 1024;
        Buffer drawDataBuffer[MAX_FRAMES_IN_FLIGHT];
        DrawData* drawDataMap[MAX_FRAMES_IN_FLIGHT];
        Buffer transformBuffer[MAX_FRAMES_IN_FLIGHT];
        Transform* transformBufferMap[MAX_FRAMES_IN_FLIGHT];
        Buffer materialBuffer;
        Material* materialBufferMap;


        VkCommandPool commandPool;
        std::vector<VkCommandBuffer> commandBuffers;

        VkSemaphore imageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];
        VkSemaphore renderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];

        VkFence inFlightFences[MAX_FRAMES_IN_FLIGHT];
        std::vector<VkFence> imagesInFlight;

        size_t currentFrame = 0;
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
                        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                .queueFamilyIndex = familyIndices.graphics.value(),
        };

        VkCommandPool pool;
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
            return VK_NULL_HANDLE;
        }

        return pool;
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

    static RenderGraph* createRenderGraph(VkDevice device, VmaAllocator allocator, std::span<VkImageView> displayViews, uint32_t displayWidth, uint32_t displayHeight, VkFormat displayFormat) {

        uint32_t shadowMapWidth = 1024;
        uint32_t shadowMapHeight = 1024;

        ImageInfo images[] = {
                {
                    .size = {displayWidth, displayHeight},
                    .format = VK_FORMAT_D32_SFLOAT,
                },
                {
                    .size = { shadowMapWidth, shadowMapHeight },
                    .format = VK_FORMAT_D32_SFLOAT,
                }
        };

        AttachmentInfo colorAttachments[] = {
                {   // Display attachment
                    .image = UINT32_MAX,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .clearValue = {
                            .color = {0.0f, 0.0f, 0.0f, 1.0f},
                    },
                },
        };

        AttachmentInfo depthAttachments = {
                .image = 0,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .clearValue = {
                        .depthStencil = {1.0f, 0},
                }
        };

        AttachmentInfo shadowAttachment = {
                .image = 1,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = {
                        .depthStencil = { 1.0f, 0},
                }
        };

        ImageInputInfo imageInputs[] = {
                {
                    .image = 1,
                    .layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                }
        };

        RenderPassInfo renderPasses[] = {
                {
                    .name = "shadow",
                    .colorAttachments = {},
                    .depthAttachment = shadowAttachment,
                    .imageInputs = {},
                },
                {
                    .name = "opaque",
                    .colorAttachments = colorAttachments,
                    .depthAttachment = depthAttachments,
                    .imageInputs = imageInputs,
                }
        };


        RenderGraphInfo renderGraphInfo {
            .images = images,
            .renderPasses = renderPasses,
            .displayImages = displayViews,
            .displayWidth = displayWidth,
            .displayHeight = displayHeight,
            .displayFormat = displayFormat,
        };

        return createRenderGraph(device, allocator, &renderGraphInfo);
    }

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

        pImpl->renderGraph = createRenderGraph(pImpl->vkDevice, pImpl->allocator, swapChain.imageViews, swapChain.extent.width, swapChain.extent.height, swapChain.format);

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
        pImpl->descriptors = createGlobalDescriptors(pImpl->vkDevice, MAX_FRAMES_IN_FLIGHT);

        createUploadQueue(pImpl->vkDevice, pImpl->allocator, queueFamilyIndices.graphics.value(), &pImpl->uploadQueue);

        initMeshBuffer(pImpl->allocator, &pImpl->meshBuffer);

        {
            VkBufferCreateInfo bufferCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = sizeof(UScene),
                    .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };

            VmaAllocationCreateInfo bufferAllocInfo {
                    .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
            };


            for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                createBuffer(pImpl->allocator, &bufferCreateInfo, &bufferAllocInfo, &pImpl->sceneBuffer[i]);

                vmaMapMemory(pImpl->allocator, pImpl->sceneBuffer[i].allocation, (void **) &pImpl->sceneBufferMap[i]);
            }

        }

        {
            VkBufferCreateInfo bufferCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = sizeof(DrawData) * pImpl->maxDrawCount,
                    .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };

            VmaAllocationCreateInfo bufferAllocInfo {
                    .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
            };

            for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                createBuffer(pImpl->allocator, &bufferCreateInfo, &bufferAllocInfo, &pImpl->drawDataBuffer[i]);
                vmaMapMemory(pImpl->allocator, pImpl->drawDataBuffer[i].allocation, (void**)&pImpl->drawDataMap[i]);
            }

        }

        {
            VkBufferCreateInfo bufferCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = sizeof(Transform) * pImpl->maxDrawCount,
                    .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };

            VmaAllocationCreateInfo bufferAllocInfo {
                    .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
            };

            for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
                createBuffer(pImpl->allocator, &bufferCreateInfo, &bufferAllocInfo, &pImpl->transformBuffer[i]);
                vmaMapMemory(pImpl->allocator, pImpl->transformBuffer[i].allocation, (void**)&pImpl->transformBufferMap[i]);
            }

        }

        {
            VkBufferCreateInfo bufferCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .size = sizeof(Material) * 2048, // TODO material count
                    .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            };

            VmaAllocationCreateInfo bufferAllocInfo {
                    .usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
            };

            createBuffer(pImpl->allocator, &bufferCreateInfo, &bufferAllocInfo, &pImpl->materialBuffer);
            vmaMapMemory(pImpl->allocator, pImpl->materialBuffer.allocation, (void**)&pImpl->materialBufferMap);
        }


        VkSamplerCreateInfo samplerInfo{
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = VK_FILTER_LINEAR,
            .minFilter = VK_FILTER_LINEAR,
            .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias = 0.0f,
            .anisotropyEnable = VK_FALSE, // TODO enable anisotropy
            .maxAnisotropy = 8, // TODO query max anisotropy
            .minLod = 0,
            .maxLod = VK_LOD_CLAMP_NONE,
        };

        vkCreateSampler(pImpl->vkDevice, &samplerInfo, nullptr, &pImpl->sampler);

        {
            pImpl->imageManager.device = pImpl->vkDevice;
            pImpl->imageManager.uploadQueue = &pImpl->uploadQueue;
            pImpl->imageManager.allocator = pImpl->allocator;
            pImpl->imageManager.resourceDescriptorSet = pImpl->descriptors.resourceDescriptorSet;
            pImpl->imageManager.sampler = pImpl->sampler;
        }
        {
            pImpl->meshManager.uploadQueue = &pImpl->uploadQueue;
            pImpl->meshManager.meshBuffer = &pImpl->meshBuffer;
        }
        {
            pImpl->pipelineAssetManager.displayFormat = pImpl->swapChain.format;
            pImpl->pipelineAssetManager.layout = pImpl->descriptors.pipelineLayout;
            pImpl->pipelineAssetManager.device = pImpl->vkDevice;
        }
        {
            pImpl->materialManager.materials = pImpl->materialBufferMap;
        }

        pImpl->assetManagers[0] = &pImpl->imageManager;
        pImpl->assetManagers[1] = &pImpl->pipelineAssetManager;
        pImpl->assetManagers[2] = &pImpl->meshManager;
        pImpl->assetManagers[3] = &pImpl->materialManager;
        pImpl->assetManagers[4] = &pImpl->effectAssetManager;

        {
            //update set
            VkDescriptorBufferInfo meshBufferInfo{
                    .buffer = pImpl->meshBuffer.vertexBuffer.buffer,
                    .offset = 0,
                    .range = pImpl->meshBuffer.vertexBuffer.size,
            };

            VkDescriptorBufferInfo materialBufferInfo{
                    .buffer = pImpl->materialBuffer.buffer,
                    .offset = 0,
                    .range = pImpl->materialBuffer.size,
            };

            VkWriteDescriptorSet descriptorWrites[]{
                {
                    .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                    .dstSet = pImpl->descriptors.resourceDescriptorSet,
                    .dstBinding = 0,
                    .dstArrayElement = 0,
                    .descriptorCount = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .pImageInfo = nullptr,
                    .pBufferInfo = &meshBufferInfo,
                    .pTexelBufferView = nullptr
                },
                {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = pImpl->descriptors.resourceDescriptorSet,
                        .dstBinding = 2,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = &materialBufferInfo,
                        .pTexelBufferView = nullptr
                },
            };

            vkUpdateDescriptorSets(pImpl->vkDevice, 2, descriptorWrites, 0, nullptr);


            for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {

                std::vector<VkWriteDescriptorSet> sceneDescriptorWrites( 3);

                VkDescriptorBufferInfo sceneBufferInfo {
                        .buffer = pImpl->sceneBuffer[i].buffer,
                        .offset = 0,
                        .range = pImpl->sceneBuffer[i].size,
                };

                sceneDescriptorWrites[0] = {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = pImpl->descriptors.sceneDescriptorSets[i],
                        .dstBinding = 0,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = &sceneBufferInfo,
                        .pTexelBufferView = nullptr
                };

                VkDescriptorBufferInfo drawDataBufferInfo {
                        .buffer = pImpl->drawDataBuffer[i].buffer,
                        .offset = 0,
                        .range = pImpl->drawDataBuffer[i].size,
                };

                sceneDescriptorWrites[1] = {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = pImpl->descriptors.sceneDescriptorSets[i],
                        .dstBinding = 1,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = &drawDataBufferInfo,
                        .pTexelBufferView = nullptr
                };

                VkDescriptorBufferInfo transformBufferInfo {
                        .buffer = pImpl->transformBuffer[i].buffer,
                        .offset = 0,
                        .range = pImpl->transformBuffer[i].size,
                };

                sceneDescriptorWrites[2] = {
                        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                        .dstSet = pImpl->descriptors.sceneDescriptorSets[i],
                        .dstBinding = 2,
                        .dstArrayElement = 0,
                        .descriptorCount = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                        .pImageInfo = nullptr,
                        .pBufferInfo = &transformBufferInfo,
                        .pTexelBufferView = nullptr
                };


                vkUpdateDescriptorSets(pImpl->vkDevice, sceneDescriptorWrites.size(), sceneDescriptorWrites.data(), 0, nullptr);
            }

        }

        pImpl->uploadWorker = std::thread([&](){
            pImpl->uploadQueue.processJobs();
        });
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

    //Adapted from Grasslands
    static glm::mat4 computeShadowMat(const glm::mat4& camera, float farPlane, const glm::vec3& lightDir) {
        glm::mat4 invViewProj = glm::inverse(camera);
        float zNear = -1;
        float zFar = 1;

        glm::vec3 centroid(0,0,0);
        glm::vec4 corners[8];
        glm::vec2 a(1, -1);
        for(int i = 0; i < 8; i++) {
            corners[i].x = (i & 1) ? 1 : -1;
            corners[i].y = (i & 2) ? 1 : -1;
            corners[i].z = (i & 4) ? zFar : zNear;
            corners[i].w = 1.0;
            corners[i] = (invViewProj * corners[i]);
            corners[i] /= corners[i].w;
            centroid += glm::vec3(corners[i]);
        }

        centroid /= 8;

        //compute view
        glm::vec3 lightPos = centroid - (lightDir * farPlane);

        glm::mat4 view = glm::lookAt(lightPos, centroid, glm::normalize(glm::cross(glm::normalize(glm::cross(lightDir, glm::vec3(0,1,0))), lightDir)));
        //get min max
        glm::vec3 min(std::numeric_limits<float>::max());
        glm::vec3 max(-std::numeric_limits<float>::max());

        for(glm::vec4& c : corners) {
            glm::vec3 p = glm::vec3(view * c);
            min = glm::min(min, p);
            max = glm::max(max, p);
        }

        glm::mat4 projection = glm::ortho(min.x, max.x, min.y, max.y, 0.01f, -min.z);

        return projection * view;
    }

    void Instance::draw(std::span<DrawItem> drawItems,  std::span<Transform> transforms, Transform camera) {

        assert(drawItems.size() == transforms.size());

        pImpl->uploadQueue.update(pImpl->vkDevice, pImpl->graphicsQueue);

        vkWaitForFences(pImpl->vkDevice, 1,
                        &pImpl->inFlightFences[pImpl->currentFrame], true,
                        UINT64_MAX);

        auto view = camera.inverse().matrix();
        float zfar = 75;
        auto proj = glm::perspective(glm::radians(60.0f), pImpl->swapChain.extent.width / (float)pImpl->swapChain.extent.height, 0.1f, zfar);

        glm::mat4 shadowMat = computeShadowMat(proj * view, zfar, glm::vec3(0, 1, 0));

        *pImpl->sceneBufferMap[pImpl->currentFrame] = {
                proj * view,
                camera.pos, 0,
                shadowMat
        };

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

            pImpl->renderGraph = createRenderGraph(pImpl->vkDevice, pImpl->allocator, pImpl->swapChain.imageViews,
                                                   pImpl->swapChain.extent.width, pImpl->swapChain.extent.height,
                                                   pImpl->swapChain.format);

            return;
        } else if (acquire != VK_SUCCESS) {
            std::cerr << "Error acquiring image\n";
            return;
        }


        VkCommandBuffer cmd = pImpl->commandBuffers[pImpl->currentFrame];
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
                .pInheritanceInfo = nullptr,
        };

        if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
            std::cerr << "Failed to begin command buffer" << std::endl;
        }


        uint32_t passIndex = 0;
        for(auto renderPassInfo : getRenderPassInfos(pImpl->renderGraph, imageIndex)) {

            if(strcmp(renderPassInfo.name, "shadow") == 0) continue;

            VkViewport viewport {
                    .x = 0,
                    .y = 0,
                    .width = static_cast<float>(renderPassInfo.passBeginInfo.renderArea.extent.width),
                    .height = static_cast<float>(renderPassInfo.passBeginInfo.renderArea.extent.height),
                    .minDepth = 0.0f,
                    .maxDepth = 1.0f,
            };
            VkRect2D scissor {
                    .offset = {0, 0},
                    .extent = {renderPassInfo.passBeginInfo.renderArea.extent.width, renderPassInfo.passBeginInfo.renderArea.extent.height},
            };


            auto effect = pImpl->effectAssetManager.effects[0];

            auto findPass = [&effect](const char* name) -> Effect::Pass& {
                return *std::find_if(effect.passes.begin(), effect.passes.end(), [&name](Effect::Pass pass) {
                    return strcmp(pass.passId, name) == 0;
                });
            };

            auto pass = findPass(renderPassInfo.name);
            auto pipeline =  pImpl->pipelineAssetManager.pipelines[pass.pipelineIndex];
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            vkCmdSetViewport(cmd, 0, 1, &viewport);
            vkCmdSetScissor(cmd, 0, 1, &scissor);

            vkCmdBeginRenderPass(cmd, &renderPassInfo.passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkDescriptorSet descriptorSets[] = {
                    pImpl->descriptors.resourceDescriptorSet,
                    pImpl->descriptors.sceneDescriptorSets[pImpl->currentFrame],
            };

            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pImpl->descriptors.pipelineLayout,
                                    0, 2, descriptorSets, 0, nullptr);

            vkCmdBindIndexBuffer(cmd, pImpl->meshBuffer.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

            //Do a draw now
            if (!drawItems.empty()) {

                memcpy(pImpl->transformBufferMap[pImpl->currentFrame], transforms.data(), transforms.size_bytes());

                DrawData *drawData = pImpl->drawDataMap[pImpl->currentFrame];

                uint32_t drawIndex = 0;
                uint32_t itemIndex = 0;
                for (DrawItem &item : drawItems) {
                    Mesh mesh = pImpl->meshManager.meshes[item.mesh];
                    for (Primitive &prim : mesh.primitives) {

                        pImpl->transformBufferMap[pImpl->currentFrame][drawIndex] = transforms[itemIndex];

                        drawData[drawIndex] = {
                                .baseVertex = static_cast<uint32_t>(prim.baseVertex),
                                .materialId = prim.material,
                        };

                        vkCmdPushConstants(cmd, pImpl->descriptors.pipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(uint32_t),&drawIndex);
                        vkCmdDrawIndexed(cmd, prim.indexCount, 1, prim.baseIndex, 0, 0);

                        drawIndex++;
                    }
                    itemIndex++;
                }
            }

            vkCmdEndRenderPass(cmd);

            passIndex++;
        }


        if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
            std::cerr << "Failed to record command buffer\n";
        }

        if (pImpl->imagesInFlight[imageIndex]) {
            vkWaitForFences(pImpl->vkDevice, 1, &pImpl->imagesInFlight[imageIndex],
                            true, UINT64_MAX);
        }

        pImpl->imagesInFlight[imageIndex] =
                pImpl->inFlightFences[pImpl->currentFrame];

        VkSemaphore imageAvailableSemaphores[] = {
                pImpl->imageAvailableSemaphores[pImpl->currentFrame]};
        VkSemaphore submitCompleteSemaphores[] = {
                pImpl->renderFinishedSemaphores[pImpl->currentFrame]};
        VkPipelineStageFlags waitStages[] = {
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

        VkSubmitInfo submitInfo {
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = 1,
                .pWaitSemaphores = imageAvailableSemaphores,
                .pWaitDstStageMask = waitStages,
                .commandBufferCount = 1,
                .pCommandBuffers = &cmd,
                .signalSemaphoreCount = 1,
                .pSignalSemaphores = submitCompleteSemaphores,
        };

        vkResetFences(pImpl->vkDevice, 1,
                      &pImpl->inFlightFences[pImpl->currentFrame]);

        if (vkQueueSubmit(pImpl->graphicsQueue, 1, &submitInfo,
                          pImpl->inFlightFences[pImpl->currentFrame]) != VK_SUCCESS) {
            std::cerr << "failed to submit command buffers\n";
        }

        pImpl->swapChain.present(pImpl->presentQueue, submitCompleteSemaphores, imageIndex);

        pImpl->currentFrame = (pImpl->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    std::span<IAssetManager *> Instance::getAssetManagers() {
        return pImpl->assetManagers;
    }


}  // namespace g2::gfx