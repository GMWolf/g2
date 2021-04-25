//
// Created by felix on 23/04/2021.
//

#ifndef G2_G2_GFX_SRC_DESCRIPTORS_H_
#define G2_G2_GFX_SRC_DESCRIPTORS_H_

#include <vulkan/vulkan.h>
#include <vector>

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

}

#endif  // G2_G2_GFX_SRC_DESCRIPTORS_H_
