#include "RenderPass.h"
#include "vk_core/Context.h"

namespace vk {
	RenderPass::RenderPass()
		: render_pass{ VK_NULL_HANDLE }
	{}

	void RenderPass::destroy() {
		vkDestroyRenderPass(Context::get()->get_device(), render_pass, nullptr);
	}



	AttachmentInfo::AttachmentInfo()
		: format{ VK_FORMAT_UNDEFINED }
		, load_op{ VK_ATTACHMENT_LOAD_OP_CLEAR }
		, store_op{ VK_ATTACHMENT_STORE_OP_STORE }
		, final_layout{ VK_IMAGE_LAYOUT_UNDEFINED }
	{}

    AttachmentInfo::Ref AttachmentInfo::use_swapchain() {
        const auto& swapchain = Context::get()->get_swapchain();

        format = swapchain.get_format();
        load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
        store_op = VK_ATTACHMENT_STORE_OP_STORE;
        final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        return *this;
    }

	RenderPassBuilder::RenderPassBuilder()
		: color_attachment{}
		, depth_attachment{}
        , use_depth_attachment{false}
	{}

	RenderPass RenderPassBuilder::build() {
        RenderPass render_pass;

        VkAttachmentDescription base_description{};
        base_description.samples = VK_SAMPLE_COUNT_1_BIT;
        base_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        base_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        base_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkAttachmentDescription color_description = base_description;
        color_description.format = color_attachment.format;
        color_description.loadOp = color_attachment.load_op;
        color_description.storeOp = color_attachment.store_op;
        color_description.finalLayout = color_attachment.final_layout;

        VkAttachmentReference color_ref{};
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depth_description{};
        VkAttachmentReference depth_ref{};

        if (use_depth_attachment) {
            depth_description = base_description;

            depth_description.format = depth_attachment.format;
            depth_description.loadOp = depth_attachment.load_op;
            depth_description.storeOp = depth_attachment.store_op;
            depth_description.finalLayout = depth_attachment.final_layout;

            depth_ref.attachment = 1;
            depth_ref.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.colorAttachmentCount = 1;
        subpass_description.pColorAttachments = &color_ref;
        subpass_description.pDepthStencilAttachment = use_depth_attachment ? &depth_ref : nullptr;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::vector<VkAttachmentDescription> attachments = { color_description };
        if (use_depth_attachment) {
            attachments.push_back(depth_description);
        }

        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = static_cast<uint32_t>(attachments.size());
        info.pAttachments = attachments.data();
        info.subpassCount = 1;
        info.pSubpasses = &subpass_description;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;

        if (vkCreateRenderPass(Context::get()->get_device(), &info, nullptr, &render_pass.render_pass) != VK_SUCCESS) {
            throw std::runtime_error("Render pass creation failed!");
        }

        dbg_log("render_pass: %p", render_pass.render_pass);

        return render_pass;
	}
}