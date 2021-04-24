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

        VkDescriptorImageInfo imageInfo {
                .sampler = VK_NULL_HANDLE,
                .imageView = VK_NULL_HANDLE,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };

        std::vector<VkDescriptorImageInfo> imageInfos(imageDescriptorCount, imageInfo);

        // Fill textures with nothing
        VkWriteDescriptorSet nullTexDescSet {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .dstSet = descriptors.resourceDescriptorSet,
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = imageDescriptorCount,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = imageInfos.data(),
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr
        };

        //vkUpdateDescriptorSets(device, 1, &nullTexDescSet, 0, nullptr);


        return descriptors;
    }
}
