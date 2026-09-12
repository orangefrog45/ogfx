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
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    auto& style = ImGui::GetStyle();
    auto& colors = style.Colors;

    style.WindowRounding = 5.0f;
    style.ChildRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.PopupRounding = 5.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding = 5.0f;
    style.TabRounding = 5.0f;

    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    style.PopupBorderSize = 0.0f;
    style.FrameBorderSize = 0.0f;
    style.TabBorderSize = 0.0f;

    // Dark grey primary palette
    const ImVec4 dark_grey = ImVec4{0.12f, 0.12f, 0.12f, 1.0f};
    const ImVec4 medium_grey = ImVec4{0.18f, 0.18f, 0.18f, 1.0f};
    const ImVec4 light_grey = ImVec4{0.25f, 0.25f, 0.25f, 1.0f};
    const ImVec4 text_color = ImVec4{0.90f, 0.90f, 0.90f, 1.0f};

    // Orange secondary palette
    const ImVec4 orange = ImVec4{0.85f, 0.25f, 0.10f, 1.0f};
    const ImVec4 orange_hover = ImVec4{1.00f, 0.25f, 0.20f, 1.0f};
    const ImVec4 orange_active = ImVec4{0.70f, 0.25f, 0.05f, 1.0f};

    // Accents
    const ImVec4 blue = ImVec4{0.10f, 0.45f, 0.85f, 1.0f};
    const ImVec4 yellow = ImVec4{0.85f, 0.85f, 0.10f, 1.0f};

    colors[ImGuiCol_Text] = text_color;
    colors[ImGuiCol_TextDisabled] = ImVec4{0.50f, 0.50f, 0.50f, 1.0f};
    colors[ImGuiCol_WindowBg] = dark_grey;
    colors[ImGuiCol_ChildBg] = dark_grey;
    colors[ImGuiCol_PopupBg] = dark_grey;
    colors[ImGuiCol_Border] = light_grey;
    colors[ImGuiCol_BorderShadow] = ImVec4{0.00f, 0.00f, 0.00f, 0.00f};
    colors[ImGuiCol_FrameBg] = medium_grey;
    colors[ImGuiCol_FrameBgHovered] = light_grey;
    colors[ImGuiCol_FrameBgActive] = light_grey;
    colors[ImGuiCol_TitleBg] = medium_grey;
    colors[ImGuiCol_TitleBgActive] = medium_grey;
    colors[ImGuiCol_TitleBgCollapsed] = medium_grey;
    colors[ImGuiCol_MenuBarBg] = medium_grey;
    colors[ImGuiCol_ScrollbarBg] = dark_grey;
    colors[ImGuiCol_ScrollbarGrab] = light_grey;
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{0.40f, 0.40f, 0.40f, 1.0f};
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{0.50f, 0.50f, 0.50f, 1.0f};
    colors[ImGuiCol_CheckMark] = orange;
    colors[ImGuiCol_SliderGrab] = orange;
    colors[ImGuiCol_SliderGrabActive] = orange_active;
    colors[ImGuiCol_Button] = medium_grey;
    colors[ImGuiCol_ButtonHovered] = light_grey;
    colors[ImGuiCol_ButtonActive] = orange;
    colors[ImGuiCol_Header] = medium_grey;
    colors[ImGuiCol_HeaderHovered] = light_grey;
    colors[ImGuiCol_HeaderActive] = orange;
    colors[ImGuiCol_Separator] = light_grey;
    colors[ImGuiCol_SeparatorHovered] = orange;
    colors[ImGuiCol_SeparatorActive] = orange_active;
    colors[ImGuiCol_ResizeGrip] = medium_grey;
    colors[ImGuiCol_ResizeGripHovered] = orange;
    colors[ImGuiCol_ResizeGripActive] = orange_active;
    colors[ImGuiCol_TabHovered] = orange_hover;
    colors[ImGuiCol_Tab] = medium_grey;
    colors[ImGuiCol_TabSelected] = orange;
    colors[ImGuiCol_TabSelectedOverline] = orange;
    colors[ImGuiCol_TabDimmed] = medium_grey;
    colors[ImGuiCol_TabDimmedSelected] = light_grey;
    colors[ImGuiCol_TabDimmedSelectedOverline] = orange;
    colors[ImGuiCol_DockingPreview] = orange;
    colors[ImGuiCol_DockingEmptyBg] = dark_grey;
    colors[ImGuiCol_PlotLines] = blue;
    colors[ImGuiCol_PlotLinesHovered] = orange;
    colors[ImGuiCol_PlotHistogram] = blue;
    colors[ImGuiCol_PlotHistogramHovered] = orange;
    colors[ImGuiCol_TableHeaderBg] = medium_grey;
    colors[ImGuiCol_TableBorderStrong] = light_grey;
    colors[ImGuiCol_TableBorderLight] = light_grey;
    colors[ImGuiCol_TableRowBg] = dark_grey;
    colors[ImGuiCol_TableRowBgAlt] = medium_grey;
    colors[ImGuiCol_TextSelectedBg] = orange;
    colors[ImGuiCol_DragDropTarget] = yellow;
    colors[ImGuiCol_NavHighlight] = orange;
    colors[ImGuiCol_NavWindowingHighlight] = orange;
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4{0.00f, 0.00f, 0.00f, 0.70f};
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4{0.00f, 0.00f, 0.00f, 0.70f};

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