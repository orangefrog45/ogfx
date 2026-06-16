#include "ogfx/pch.h"

#include <backends/imgui_impl_vulkan.h>
#include <backends/imgui_impl_glfw.h>

#include "ogfx/Gui.h"
#include "ogfx/Window.h"

using namespace ogfx;

void ogfx::RecordRenderImGui(ogfx::Image2D& img, vk::CommandBuffer& cmd) {
    vk::RenderingAttachmentInfo colour_attachment{};
    colour_attachment.imageView = img.GetImageView();
    colour_attachment.imageLayout = vk::ImageLayout::eGeneral;
    colour_attachment.loadOp = vk::AttachmentLoadOp::eLoad;
    colour_attachment.storeOp = vk::AttachmentStoreOp::eStore;

    vk::RenderingInfo rendering_info{};
    rendering_info.colorAttachmentCount = 1;
    rendering_info.pColorAttachments = &colour_attachment;
    rendering_info.renderArea.extent.width = img.GetSpec().size.x;
    rendering_info.renderArea.extent.height = img.GetSpec().size.y;
    rendering_info.layerCount = 1;

    cmd.beginRendering(rendering_info);
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
    cmd.endRendering();
}

vk::DescriptorPool ogfx::InitImGui(Window& window) {
    VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkDescriptorPool imgui_pool;
    OGFX_VK_CHECK(vkCreateDescriptorPool(VkContext::GetLogicalDevice().device, &pool_info, nullptr, &imgui_pool));

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(window.GetGlfwWindow(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = VkContext::GetInstance();
    init_info.PhysicalDevice = VkContext::GetPhysicalDevice().device;
    init_info.Device = VkContext::GetLogicalDevice().device;
    init_info.Queue = VkContext::GetLogicalDevice().GetGraphicsQueue();
    init_info.DescriptorPool = imgui_pool;
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.UseDynamicRendering = true;

    VkFormat swapchain_format = (VkFormat)window.GetSwapchainImage(0).GetSpec().format;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchain_format;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info);

    return imgui_pool;
}