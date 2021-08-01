//
// Created by felix on 23/04/2021.
//

#ifndef G2_G2_GFX_SRC_DESCRIPTORS_H_
#define G2_G2_GFX_SRC_DESCRIPTORS_H_

#include <vulkan/vulkan.h>
#include <vector>
#include <span>

namespace g2::gfx {

enum DescriptorSetIndex {
  // Holds data that is largely static such as vertex data, textures, etc
  DESCRIPTOR_SET_RESOURCES = 0,
  // Holds per frame data
  DESCRIPTOR_SET_SCENE = 1,
  // Holds per draw data in dynamic uniform buffers
  DESCRIPTOR_SET_DRAW = 2,
};


struct GlobalDescriptors {

    VkPipelineLayout pipelineLayout;

    VkDescriptorSetLayout resourceDescriptorSetLayout;
    VkDescriptorPool resourceDescriptorPool;
    VkDescriptorSet resourceDescriptorSet;

    VkDescriptorSetLayout sceneDescriptorSetLayout;
    VkDescriptorPool sceneDescriptorPool;
    std::vector<VkDescriptorSet> sceneDescriptorSets;
};

GlobalDescriptors createGlobalDescriptors(VkDevice device, size_t frameCount);

struct DescriptorSetInfo {
    std::span<VkDescriptorSetLayoutBinding> bindings;
    std::span<VkDescriptorBindingFlags> bindingFlags;
    uint32_t count;
};

struct DescriptorsInfo {
    std::span<DescriptorSetInfo> setInfos;
};

struct Descriptors {
    VkDescriptorPool pool;
    std::vector<VkDescriptorSetLayout> layouts;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<uint32_t> descriptorSetOffsets;
};

Descriptors createDescriptors(VkDevice device, const DescriptorsInfo* descriptorsInfo);


}

#endif  // G2_G2_GFX_SRC_DESCRIPTORS_H_
