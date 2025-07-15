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
		, layout{ VK_IMAGE_LAYOUT_UNDEFINED }
		, final_layout{ VK_IMAGE_LAYOUT_UNDEFINED }
	{}

	RenderPassBuilder::RenderPassBuilder()
		: color_attachment{}
		, depth_attachment{}
	{}

	RenderPass RenderPassBuilder::build() {
        RenderPass render_pass;

        VkAttachmentDescription color_description{};
        color_description.format = color_attachment.format;
        color_description.samples = VK_SAMPLE_COUNT_1_BIT;
        color_description.loadOp = color_attachment.load_op;
        color_description.storeOp = color_attachment.store_op;
        color_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        color_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        color_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_description.finalLayout = color_attachment.final_layout;

        VkAttachmentDescription depth_description{};
        depth_description.format = depth_attachment.format;
        depth_description.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_description.loadOp = depth_attachment.load_op;
        depth_description.storeOp = depth_attachment.store_op;
        depth_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depth_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_description.finalLayout = depth_attachment.final_layout;

        VkAttachmentReference color_ref{};
        color_ref.attachment = 0;
        color_ref.layout = color_attachment.layout;

        VkAttachmentReference depth_ref{};
        depth_ref.attachment = 1;
        depth_ref.layout = depth_attachment.layout;

        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.colorAttachmentCount = 1;
        subpass_description.pColorAttachments = &color_ref;
        subpass_description.pDepthStencilAttachment = &depth_ref;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        std::array<VkAttachmentDescription, 2> attachments = { color_description, depth_description };

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
	}
}