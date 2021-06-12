//
// Created by felix on 23/04/2021.
//

#include "descriptors.h"
#include <span>
#include <vector>

namespace g2::gfx {

    static const uint32_t imageDescriptorCount = 1024;


    static VkDescriptorSetLayoutBinding resourceDescriptorSetLayouts[]{
            // Vertex Data
            VkDescriptorSetLayoutBinding {
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = nullptr,
            },
            // Indices
            VkDescriptorSetLayoutBinding {
                    .binding = 1,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = nullptr,
            },
            // Textures
            VkDescriptorSetLayoutBinding {
                .binding = 2,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = imageDescriptorCount,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr,
            },
            // Material
            VkDescriptorSetLayoutBinding {
                .binding = 3,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = nullptr,
            },
            // Shadow map
            VkDescriptorSetLayoutBinding  {
                    .binding = 4,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = nullptr,
            },
            // Visibility
            VkDescriptorSetLayoutBinding  {
                    .binding = 5,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = nullptr,
            },
            // matid
            VkDescriptorSetLayoutBinding  {
                    .binding = 6,
                    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    .pImmutableSamplers = nullptr,
            },
    };

    static VkDescriptorBindingFlags resourceDescriptorBindingFlags[] {
        0,
        0,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
        0,
        0,
        0,
        0,
    };

    static VkDescriptorPoolSize resourcePoolSizes[] {
            VkDescriptorPoolSize {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 2,
            },
            VkDescriptorPoolSize {
                    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = imageDescriptorCount + 5,
            },
    };




    static VkDescriptorSetLayoutBinding sceneDescriptorSetLayouts[]{
        //Uniforms
        VkDescriptorSetLayoutBinding {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        },
        //Draw data
        VkDescriptorSetLayoutBinding {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_ALL,
            .pImmutableSamplers = nullptr,
        },
        // Transform data
        VkDescriptorSetLayoutBinding  {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT |  VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        } ,
    };

    static VkDescriptorBindingFlags sceneDescriptorBindingFlags[] {
            0,0,0
    };


    VkPushConstantRange pushConstantRange {
        .stageFlags = VK_SHADER_STAGE_ALL,
        .offset = 0,
        .size = sizeof(uint32_t),
    };



    static void createDescriptorSetLayout(VkDevice device, std::span<VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayout* descriptorSetLayout, std::span<VkDescriptorBindingFlags> bindingFlags) {
        VkDescriptorSetLayoutBindingFlagsCreateInfo flags {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(bindingFlags.size()),
            .pBindingFlags = bindingFlags.data(),
        };

        VkDescriptorSetLayoutCreateInfo descriptorSetInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .pNext = &flags,
                .bindingCount = static_cast<uint32_t>(bindings.size()),
                .pBindings = bindings.data(),
        };

        vkCreateDescriptorSetLayout(device, &descriptorSetInfo, nullptr, descriptorSetLayout);
    }


    GlobalDescriptors createGlobalDescriptors(VkDevice device, size_t frameCount) {

        GlobalDescriptors descriptors{};
        createDescriptorSetLayout(device, resourceDescriptorSetLayouts, &descriptors.resourceDescriptorSetLayout, resourceDescriptorBindingFlags);
        createDescriptorSetLayout(device, sceneDescriptorSetLayouts, &descriptors.sceneDescriptorSetLayout, sceneDescriptorBindingFlags);

        { // create pipeline
            VkDescriptorSetLayout layouts[] = {
                    descriptors.resourceDescriptorSetLayout,
                    descriptors.sceneDescriptorSetLayout
            };


            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                    .setLayoutCount = 2,
                    .pSetLayouts = layouts,
                    .pushConstantRangeCount = 1,
                    .pPushConstantRanges = &pushConstantRange,
            };

            vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &descriptors.pipelineLayout);
        }

        { // Resource set
            VkDescriptorPoolCreateInfo poolInfo{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                    .maxSets = 1,
                    .poolSizeCount = 2,
                    .pPoolSizes = resourcePoolSizes,
            };

            vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptors.resourceDescriptorPool);

            VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    .descriptorPool = descriptors.resourceDescriptorPool,
                    .descriptorSetCount = 1,
                    .pSetLayouts = &descriptors.resourceDescriptorSetLayout,
            };

            vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &descriptors.resourceDescriptorSet);
        }

        { //scene set
            VkDescriptorPoolSize scenePoolSizes[]{
                    VkDescriptorPoolSize{
                            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .descriptorCount = static_cast<uint32_t>(frameCount),
                    },
                    VkDescriptorPoolSize{
                            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                            .descriptorCount = 2 * static_cast<uint32_t>(frameCount),
                    }
            };

            VkDescriptorPoolCreateInfo poolInfo{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                    .maxSets = static_cast<uint32_t>(frameCount),
                    .poolSizeCount = 2,
                    .pPoolSizes = scenePoolSizes,
            };

            vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptors.sceneDescriptorPool);

            std::vector<VkDescriptorSetLayout> layouts(frameCount, descriptors.sceneDescriptorSetLayout);

            descriptors.sceneDescriptorSets.resize(frameCount);

            VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                    .descriptorPool = descriptors.sceneDescriptorPool,
                    .descriptorSetCount = static_cast<uint32_t>(frameCount),
                    .pSetLayouts = layouts.data(),
            };

            vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, descriptors.sceneDescriptorSets.data());


        }

        return descriptors;
    }



}
