#include "RenderPass.h"
#include "vk_core/Context.h"

namespace vk {
	
    ClearValue ClearValue::color(std::array<float, 4> color) {
        VkClearValue value;
        for (int i = 0; i < color.size(); i++) {
            value.color.float32[i] = color[i];
        }
        return { value };
    }

    ClearValue ClearValue::color(std::array<int32_t, 4> color) {
        VkClearValue value;
        for (int i = 0; i < color.size(); i++) {
            value.color.int32[i] = color[i];
        }
        return { value };
    }

    ClearValue ClearValue::color(std::array<uint32_t, 4> color) {
        VkClearValue value;
        for (int i = 0; i < color.size(); i++) {
            value.color.uint32[i] = color[i];
        }
        return { value };
    }



    ClearValue ClearValue::depth(float depth) {
        VkClearValue value;
        value.depthStencil.depth = depth;
        return { value };
    }

    Attachment::Attachment()
		: type{ AttachmentType::Color }
        , format{ VK_FORMAT_UNDEFINED }
		, load_op{ VK_ATTACHMENT_LOAD_OP_CLEAR }
		, store_op{ VK_ATTACHMENT_STORE_OP_STORE }
		, final_layout{ VK_IMAGE_LAYOUT_UNDEFINED }
	{}

    Attachment::Ref Attachment::from_swapchain() {
        const auto& swapchain = Context::get()->get_swapchain();

        type = AttachmentType::Color;
        format = swapchain.format();
        load_op = VK_ATTACHMENT_LOAD_OP_CLEAR;
        store_op = VK_ATTACHMENT_STORE_OP_STORE;
        final_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        return *this;
    }

	RenderPassBuilder::RenderPassBuilder()
		: _attachments{}
	{}

	RenderPass RenderPassBuilder::build() {
        RenderPass render_pass;

        VkAttachmentDescription base_description{};
        base_description.samples = VK_SAMPLE_COUNT_1_BIT;
        base_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        base_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        base_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        std::vector<VkAttachmentDescription> descriptions{ _attachments.size() };
        std::vector<VkAttachmentReference> color_refs{};
        std::vector<VkAttachmentReference> depth_refs{};

        for (int i = 0; i < _attachments.size(); i++) {
            const auto& attachment = _attachments[i];

            VkAttachmentDescription description = base_description;
            description.format = attachment.format;
            description.loadOp = attachment.load_op;
            description.storeOp = attachment.store_op;
            description.finalLayout = attachment.final_layout;

            descriptions[i] = description;

            VkAttachmentReference ref{};
            ref.attachment = i;
            ref.layout = attachment.type == AttachmentType::Color ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

            if (attachment.type == AttachmentType::Color) {
                color_refs.push_back(ref);
            }
            else {
                depth_refs.push_back(ref);
            }
        }

        VkSubpassDescription subpass_description{};
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_description.colorAttachmentCount = static_cast<uint32_t>(color_refs.size());
        subpass_description.pColorAttachments = color_refs.data();
        subpass_description.pDepthStencilAttachment = depth_refs.size() > 0 ?  &depth_refs.back() : nullptr;

        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = static_cast<uint32_t>(descriptions.size());
        info.pAttachments = descriptions.data();
        info.subpassCount = 1;
        info.pSubpasses = &subpass_description;
        info.dependencyCount = 1;
        info.pDependencies = &dependency;

        if (vkCreateRenderPass(Context::get()->get_device(), &info, nullptr, &*render_pass.render_pass) != VK_SUCCESS) {
            throw std::runtime_error("Render pass creation failed!");
        }

        render_pass._attachments = _attachments;

        dbg_log("render_pass: %p", *render_pass.render_pass);

        return render_pass;
	}
}