//
// Created by felix on 15/04/2021.
//

#include "renderpass.h"

namespace g2::gfx {

vk::RenderPass createRenderPass(vk::Device device, vk::Format imageFormat) {
  vk::AttachmentDescription colorAttachment{
      .format = imageFormat,
      .samples = vk::SampleCountFlagBits::e1,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
      .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
      .initialLayout = vk::ImageLayout::eUndefined,
      .finalLayout = vk::ImageLayout::ePresentSrcKHR,
  };

  vk::AttachmentReference colorAttachmentRef{
      .attachment = 0,
      .layout = vk::ImageLayout::eColorAttachmentOptimal,
  };

  vk::SubpassDescription subpass{
      .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentRef,
  };

  vk::SubpassDependency dependency{
      .srcSubpass = VK_SUBPASS_EXTERNAL,
      .dstSubpass = 0,
      .srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
      .dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput,
      .srcAccessMask = {},
      .dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite,
  };

  vk::RenderPassCreateInfo renderPassCreateInfo{
      .attachmentCount = 1,
      .pAttachments = &colorAttachment,
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 1,
      .pDependencies = &dependency,
  };

  auto renderPassResult = device.createRenderPass(renderPassCreateInfo);

  if (renderPassResult.result != vk::Result::eSuccess) {
    return {};
  }

  return renderPassResult.value;
}
vk::RenderPass createCompatibilityRenderPass(vk::Device device, std::span<vk::Format> imageFormats) {

  std::vector<vk::AttachmentDescription> attachments;
  attachments.reserve(imageFormats.size());

  for(vk::Format format : imageFormats) {
    vk::AttachmentDescription attachment{
        .format = format,
        .samples = vk::SampleCountFlagBits::e1,
        .loadOp = vk::AttachmentLoadOp::eDontCare,
        .storeOp = vk::AttachmentStoreOp::eDontCare,
        .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout = vk::ImageLayout::eUndefined,
        .finalLayout = vk::ImageLayout::ePresentSrcKHR,
    };

    attachments.push_back(attachment);
  }

  vk::AttachmentReference colorAttachmentRef{
      .attachment = 0,
      .layout = vk::ImageLayout::eColorAttachmentOptimal,
  };

  vk::SubpassDescription subpass {
      .pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
      .colorAttachmentCount = 1,
      .pColorAttachments = &colorAttachmentRef,
  };

  vk::RenderPassCreateInfo renderPassCreateInfo{
      .attachmentCount = static_cast<uint32_t>(attachments.size()),
      .pAttachments = attachments.data(),
      .subpassCount = 1,
      .pSubpasses = &subpass,
      .dependencyCount = 0,
      .pDependencies = nullptr,
  };

  auto renderPassResult = device.createRenderPass(renderPassCreateInfo);

  if (renderPassResult.result != vk::Result::eSuccess) {
    return {};
  }

  return renderPassResult.value;

}
}  // namespace g2::gfx