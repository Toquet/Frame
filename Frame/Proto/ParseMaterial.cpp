#include "ParseMaterial.h"
#include "Frame/Frame.h"
#include "Frame/OpenGL/Material.h"

namespace frame::proto {

	std::optional<std::unique_ptr<frame::MaterialInterface>> 
	ParseMaterialOpenGL(
		const frame::proto::Material& proto_material, 
		LevelInterface* level)
	{
		const std::size_t texture_size = proto_material.texture_names_size();
		const std::size_t inner_size = proto_material.inner_names_size();
		if (texture_size != inner_size)
		{
			throw std::runtime_error(
				fmt::format(
					"Not the same size for texture and inner names: {} != {}.",
					texture_size,
					inner_size));
		}
		auto material = std::make_unique<frame::opengl::Material>();
		if (proto_material.program_name().empty())
		{
			throw std::runtime_error(
				fmt::format("No program name in {}.", proto_material.name()));
		}
		auto maybe_program_id = 
			level->GetIdFromName(proto_material.program_name());
		if (!maybe_program_id) return std::nullopt;
		EntityId program_id = maybe_program_id.value();
		material->SetProgramId(program_id);
		for (int i = 0; i < inner_size; ++i)
		{
			auto maybe_texture_id = 
				level->GetIdFromName(proto_material.texture_names(i));
			if (!maybe_texture_id) return std::nullopt;
			EntityId texture_id = 
				maybe_texture_id.value();
			material->AddTextureId(
				texture_id, 
				proto_material.inner_names(i));
		}
		return material;
	}

} // End namespace frame::proto.
