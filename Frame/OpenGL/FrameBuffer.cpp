#include "FrameBuffer.h"
#include <stdexcept>
#include <GL/glew.h>
#include <cassert>
#include <sstream>
#include "Texture.h"

namespace frame::opengl {

	FrameBuffer::FrameBuffer()
	{
		glGenFramebuffers(1, &frame_id_);
		error_.Display(__FILE__, __LINE__ - 1);
	}

	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &frame_id_);
	}

	void FrameBuffer::Bind(const unsigned int slot /*= 0*/) const
	{
		assert(slot == 0);
		if (locked_bind_) return;
		glBindFramebuffer(GL_FRAMEBUFFER, frame_id_);
		error_.Display(__FILE__, __LINE__ - 1);
	}

	void FrameBuffer::UnBind() const
	{
		if (locked_bind_) return;
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		error_.Display(__FILE__, __LINE__ - 1);
	}

	void FrameBuffer::AttachRender(const RenderBuffer& render) const
	{
		Bind();
		render.Bind();
		glFramebufferRenderbuffer(
			GL_FRAMEBUFFER, 
			GL_DEPTH_STENCIL_ATTACHMENT,
			GL_RENDERBUFFER, 
			render.GetId());
		error_.Display(__FILE__, __LINE__ - 5);
		auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			auto error_pair = GetError();
			error_.CreateError(
				"frame buffer is not completed: " + error_pair.second, 
				__FILE__, 
				__LINE__ - 4);
		}
		render.UnBind();
		UnBind();
	}

	void FrameBuffer::AttachTexture(
		const std::shared_ptr<TextureInterface> texture,
		const FrameColorAttachment frame_color_attachment /*=
			FrameColorAttachment::COLOR_ATTACHMENT0*/,
		const int mipmap /*= 0*/,
		const FrameTextureType frame_texture_type /*= 
			FrameTextureType::TEXTURE_2D*/) const
	{
		Bind();
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			static_cast<GLenum>(frame_color_attachment),
			GetFrameTextureType(frame_texture_type),
			texture->GetId(),
			mipmap);
		error_.Display(__FILE__, __LINE__ - 6);
		UnBind();
	}

	FrameColorAttachment FrameBuffer::GetFrameColorAttachment(const int i)
	{
		switch (i)
		{
			case 0:
				return FrameColorAttachment::COLOR_ATTACHMENT0;
			case 1:
				return FrameColorAttachment::COLOR_ATTACHMENT1;
			case 2:
				return FrameColorAttachment::COLOR_ATTACHMENT2;
			case 3:
				return FrameColorAttachment::COLOR_ATTACHMENT3;
			case 4:
				return FrameColorAttachment::COLOR_ATTACHMENT4;
			case 5:
				return FrameColorAttachment::COLOR_ATTACHMENT5;
			case 6:
				return FrameColorAttachment::COLOR_ATTACHMENT6;
			case 7:
				return FrameColorAttachment::COLOR_ATTACHMENT7;
			default :
				throw std::runtime_error("Only [0-7] level allowed.");
		}
	}

	const int FrameBuffer::GetFrameTextureType(
		const FrameTextureType frame_texture_type) const
	{
		int value = static_cast<int>(frame_texture_type);
		if (value >= 0)
		{
			return GL_TEXTURE_CUBE_MAP_POSITIVE_X + value;
		}
		return GL_TEXTURE_2D;
	}

	FrameTextureType FrameBuffer::GetFrameTextureType(const int i)
	{
		return static_cast<FrameTextureType>(i);
	}

	void FrameBuffer::DrawBuffers(const std::uint32_t size /*= 1*/)
	{
		Bind();
		assert(size < 9);
		std::vector<unsigned int> draw_buffer = {};
		for (std::uint32_t i = 0; i < size; ++i)
		{
			draw_buffer.emplace_back(
				static_cast<unsigned int>(
					FrameBuffer::GetFrameColorAttachment(i)));
		}
		glDrawBuffers(
			static_cast<GLsizei>(draw_buffer.size()), 
			draw_buffer.data());
		error_.Display(__FILE__, __LINE__ - 3);
		UnBind();
	}

	const std::pair<bool, std::string> FrameBuffer::GetError() const
	{
		Bind();
		std::pair<bool, std::string> status_error;
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		UnBind();
		switch (status)
		{
		case GL_FRAMEBUFFER_COMPLETE:
			status_error.second = "no error";
			status_error.first = true;
			return status_error;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			status_error.second = "[ERROR] : incomplete attachment.";
			status_error.first = false;
			return status_error;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			status_error.second = "[ERROR] : missing attachment.";
			status_error.first = false;
			return status_error;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			status_error.second = "[ERROR] : incomplete draw buffer.";
			status_error.first = false;
			return status_error;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			status_error.second = "[ERROR] : incomplete read buffer.";
			status_error.first = false;
			return status_error;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			status_error.second = "[ERROR] : unsupported.";
			status_error.first = false;
			return status_error;
		default:
			status_error.second = 
				"[ERROR] : unknown(" + std::to_string(status) + ").";
			status_error.first = false;
			return status_error;
		}
	}

	const std::string FrameBuffer::GetStatus() const
	{
		Bind();
		std::stringstream ss;
		ss << "===== FBO STATUS =====\n";
		int color_buffer_count = 0;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &color_buffer_count);
		ss << "color buffer count     : " << color_buffer_count << std::endl;
		int multi_sample_count = 0;
		glGetIntegerv(GL_MAX_SAMPLES, &multi_sample_count);
		ss << "multi sample count     : " << multi_sample_count << std::endl;

		int obj_type = 0;
		int obj_id = 0;

		// Take care of the attachment color.
		for (int i = 0; i < color_buffer_count; ++i)
		{
			glGetFramebufferAttachmentParameteriv(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + i,
				GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
				&obj_type);
			if (obj_type != GL_NONE)
			{
				glGetFramebufferAttachmentParameteriv(
					GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + i,
					GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
					&obj_id);
				ss << "color attachment       : (" << i << ")";
				switch (obj_type)
				{
				case GL_TEXTURE:
					ss << " GL_TEXTURE";
					break;
				case GL_RENDERBUFFER:
					ss << " GL_RENDERBUFFER";
					break;
				}
				ss << std::endl;
			}
		}

		// Take care of the depth buffer.
		glGetFramebufferAttachmentParameteriv(
			GL_FRAMEBUFFER,
			GL_DEPTH_ATTACHMENT,
			GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
			&obj_type);
		if (obj_type != GL_NONE)
		{
			glGetFramebufferAttachmentParameteriv(
				GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
				&obj_id);
			ss << "depth attachment       : ";
			switch (obj_type)
			{
			case GL_TEXTURE:
				ss << "GL_TEXTURE";
				break;
			case GL_RENDERBUFFER:
				ss << "GL_RENDERBUFFER";
				break;
			}
			ss << std::endl;
		}

		// Take care of the stencil buffer.
		glGetFramebufferAttachmentParameteriv(
			GL_FRAMEBUFFER,
			GL_STENCIL_ATTACHMENT,
			GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
			&obj_type);
		if (obj_type != GL_NONE)
		{
			glGetFramebufferAttachmentParameteriv(
				GL_FRAMEBUFFER,
				GL_STENCIL_ATTACHMENT,
				GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
				&obj_id);
			ss << "stencil attachment     : ";
			switch (obj_type)
			{
			case GL_TEXTURE:
				ss << "GL_TEXTURE";
				break;
			case GL_RENDERBUFFER:
				ss << "GL_RENDERBUFFER";
				break;
			}
			ss << std::endl;
		}
		UnBind();
		return ss.str();
	}

} // End namespace frame::opengl.
