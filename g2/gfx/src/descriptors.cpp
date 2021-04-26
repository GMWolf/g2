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
            VkDescriptorSetLayoutBinding{
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = nullptr,
            },
            // Textures
            VkDescriptorSetLayoutBinding  {
                .binding = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = imageDescriptorCount,
                .stageFlags = VK_SHADER_STAGE_ALL,
                .pImmutableSamplers = nullptr,
            }
    };

    static VkDescriptorBindingFlags resourceDescriptorBindingFlags[] {
        0,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT,
    };

    static VkDescriptorPoolSize resourcePoolSizes[] {
            VkDescriptorPoolSize {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
            },
            VkDescriptorPoolSize {
                    .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                    .descriptorCount = imageDescriptorCount,
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
        }
    };


    VkPushConstantRange pushConstantRange {
        .stageFlags = VK_SHADER_STAGE_ALL,
        .offset = 0,
        .size = sizeof(uint32_t),
    };



    static void createDescriptorSetLayout(VkDevice device, std::span<VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayout* descriptorSetLayout) {
        VkDescriptorSetLayoutBindingFlagsCreateInfo flags {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(bindings.size()),
            .pBindingFlags = resourceDescriptorBindingFlags,
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
        createDescriptorSetLayout(device, resourceDescriptorSetLayouts, &descriptors.resourceDescriptorSetLayout);
        createDescriptorSetLayout(device, sceneDescriptorSetLayouts, &descriptors.sceneDescriptorSetLayout);

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
                            .descriptorCount = static_cast<uint32_t>(frameCount),
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
