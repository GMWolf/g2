//
// Created by felix on 23/04/2021.
//

#include "descriptors.h"
#include <span>

namespace g2::gfx {

    static VkDescriptorSetLayoutBinding resourceDescriptorSetLayouts[]{
            // Vertex Data
            VkDescriptorSetLayoutBinding{
                    .binding = 0,
                    .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                    .descriptorCount = 1,
                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                    .pImmutableSamplers = nullptr,
            },
    };

    static VkDescriptorPoolSize resourcePoolSizes[] {
            VkDescriptorPoolSize {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
            }
    };



    static void createDescriptorSetLayout(VkDevice device, std::span<VkDescriptorSetLayoutBinding> bindings, VkDescriptorSetLayout* descriptorSetLayout) {
        VkDescriptorSetLayoutCreateInfo descriptorSetInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(bindings.size()),
                .pBindings = bindings.data(),
        };

        vkCreateDescriptorSetLayout(device, &descriptorSetInfo, nullptr, descriptorSetLayout);
    }


    GlobalDescriptors createGlobalDescriptors(VkDevice device) {

        GlobalDescriptors descriptors{};
        createDescriptorSetLayout(device, resourceDescriptorSetLayouts, &descriptors.resourceDescriptorSetLayout);

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = 1,
                .pSetLayouts = &descriptors.resourceDescriptorSetLayout,
                .pushConstantRangeCount = 0,
                .pPushConstantRanges = nullptr,
        };

        vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &descriptors.pipelineLayout);


        VkDescriptorPoolSize poolSize{
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
        };

        VkDescriptorPoolCreateInfo poolInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .maxSets = 1,
                .poolSizeCount = 1,
                .pPoolSizes = &poolSize,
        };

        vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptors.resourceDescriptorPool);

        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = descriptors.resourceDescriptorPool,
                .descriptorSetCount = 1,
                .pSetLayouts = &descriptors.resourceDescriptorSetLayout,
        };

        vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &descriptors.resourceDescriptorSet);


        return descriptors;
    }
}
