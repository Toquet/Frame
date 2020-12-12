#include "Device.h"
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <random>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include "FrameBuffer.h"
#include "RenderBuffer.h"
#include "Fill.h"

namespace sgl {

	Device::Device(
		void* gl_context, 
		const std::pair<std::uint32_t, std::uint32_t> size) : 
		gl_context_(gl_context),
		size_(size)
	{
		// Initialize GLEW.
		if (GLEW_OK != glewInit())
		{
			throw std::runtime_error("couldn't initialize GLEW");
		}

		// This should maintain the culling to none.
		// FIXME(anirul): Change this as to be working!
		glDisable(GL_CULL_FACE);
		error_.Display(__FILE__, __LINE__ - 1);
		// glCullFace(GL_BACK);
		// error_.Display(__FILE__, __LINE__ - 1);
		// glFrontFace(GL_CW);
		// error_.Display(__FILE__, __LINE__ - 1);
		// Enable blending to 1 - source alpha.
		glEnable(GL_BLEND);
		error_.Display(__FILE__, __LINE__ - 1);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		error_.Display(__FILE__, __LINE__ - 1);
		// Enable Z buffer.
		glEnable(GL_DEPTH_TEST);
		error_.Display(__FILE__, __LINE__ - 1);
		glDepthFunc(GL_LEQUAL);
		error_.Display(__FILE__, __LINE__ - 1);
		// Enable seamless cube map.
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		error_.Display(__FILE__, __LINE__ - 1);

		// Create a frame buffer and a render buffer.
		frame_ = std::make_shared<FrameBuffer>();
		render_ = std::make_shared<RenderBuffer>();
	}

	void Device::Startup(
		const frame::proto::Level& proto_level,
		const frame::proto::EffectFile& proto_effect_file,
		const frame::proto::SceneFile& proto_scene_file,
		const frame::proto::TextureFile& proto_texture_file)
	{
		// Load other files.
		if (proto_level.default_scene_name().empty())
			throw std::runtime_error("should have a default scene name.");
		if (proto_level.default_texture_name().empty())
			throw std::runtime_error("should have a default texture name.");
		if (proto_level.default_camera_name().empty())
			throw std::runtime_error("should have a default camera name.");

		// Load scenes from proto.
		frame::proto::SceneTree proto_scene_tree;
		for (const auto& proto : proto_scene_file.scene_trees())
		{
			if (proto.name() == proto_level.default_scene_name())
				proto_scene_tree = proto;
		}
		scene_tree_ = std::make_shared<SceneTree>(proto_scene_tree);
		scene_tree_->SetDefaultCamera(proto_level.default_camera_name());

		// Load textures from proto.
		for (const auto& proto_texture : proto_texture_file.textures())
		{
			std::shared_ptr<Texture> texture = nullptr;
			if (proto_texture.cubemap())
			{
				texture = std::make_shared<TextureCubeMap>(
					proto_texture, 
					size_);
			}
			else
			{
				texture = std::make_shared<Texture>(proto_texture, size_);
			}
			texture_map_.insert({ proto_texture.name(), texture });
		}
		out_texture_name_ = proto_level.default_texture_name();
		if (texture_map_.find(out_texture_name_) ==	texture_map_.end())
		{
			throw std::runtime_error(
				"no default texture is loaded: " +
				proto_level.default_texture_name());
		}

		// Load effects from proto.
		for (const auto& proto_effect : proto_effect_file.effects())
		{
			auto effect = 
				std::make_shared<sgl::Effect>(proto_effect, texture_map_);
			effect_map_.insert({ proto_effect.name(), effect });
		}

		SetupCamera();

		// Startup effects.
		for (auto& effect_pair : effect_map_)
		{
			effect_pair.second->Startup(size_, shared_from_this());
		}

		cube_ = CreateCubeStaticMesh();
		quad_ = CreateQuadStaticMesh();
	}

	void Device::DrawMultiTextures(
		const std::shared_ptr<ProgramInterface> program,
		const std::vector<std::shared_ptr<Texture>>& out_textures,
		const double dt)
	{
		dt_ = dt;
		assert(!out_textures.empty());

		// Setup the camera.
		SetupCamera();

		ScopedBind scoped_bind_frame(*frame_);
		ScopedBind scoped_bind_render(*render_);
		render_->CreateStorage(size_);
		frame_->AttachRender(*render_);

		// Set the view port for rendering.
		glViewport(0, 0, size_.first, size_.second);
		error_.Display(__FILE__, __LINE__ - 1);

		// Clear the screen.
		glClearColor(.2f, 0.f, .2f, 1.0f);
		error_.Display(__FILE__, __LINE__ - 1);


		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		error_.Display(__FILE__, __LINE__ - 1);

		int i = 0;
		for (const auto& texture : out_textures)
		{
			frame_->AttachTexture(*texture, FrameBuffer::GetFrameColorAttachment(i));
			++i;
		}
		frame_->DrawBuffers(static_cast<std::uint32_t>(out_textures.size()));

		for (const auto& pair : scene_tree_->GetSceneMap())
		{
			const auto& scene = pair.second;
			const std::shared_ptr<StaticMesh>& mesh = scene->GetLocalMesh();
			if (!mesh) continue;
			if (mesh->IsClearDepthBuffer()) continue;

			auto material_name = mesh->GetMaterialName();
			if (material_map_.find(material_name) != material_map_.end())
			{
				std::shared_ptr<Material> mat = 
					std::make_shared<Material>(*material_map_[material_name]);
				if (environment_material_) *mat += *environment_material_;
				mesh->SetMaterial(mat);
			}
			else if (environment_material_)
			{
				// Special case this is suppose to be the environment map.
				auto material = std::make_shared<Material>();
				material->AddTexture(
					"Environment", 
					environment_material_->GetTexture("Environment"));
				mesh->SetMaterial(material);
			}

			program->Use();
			program->Uniform("projection", perspective_);
			program->Uniform("view", view_);
			program->Uniform(
				"model", 
				scene->GetLocalModel(
				[this](const std::string& name)
				{
					return scene_tree_->GetSceneByName(name);
				},
				dt));

			// Draw the mesh.
			mesh->Draw(program);
		}
	}

	void Device::Display(const double dt)
	{
		dt_ = dt;
		for (const auto& name_effect_pair : effect_map_)
		{
			const auto effect = name_effect_pair.second;
			switch (effect->GetRenderOutputType())
			{
				case frame::proto::Effect::TEXTURE_2D:
				{
					effect->Draw(quad_, dt_);
					break;
				}
				case frame::proto::Effect::TEXTURE_3D:
				{
					effect->Draw(cube_, dt_);
					break;
				}
				case frame::proto::Effect::SCENE:
				{
					throw std::runtime_error(
						"Scene output not implemented yet!");
				}
				case frame::proto::Effect::INVALID:
					[[fallthrough]];
				default:
					throw std::runtime_error("INVALID effect?");
			}
		}
		static auto program = CreateProgram("Display");
		static auto quad = CreateQuadStaticMesh();
		auto material = std::make_shared<Material>();
		material->AddTexture("Display", texture_map_.at(out_texture_name_));
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		quad->SetMaterial(material);
		quad->Draw(program);
	}

	void Device::AddEnvironment(const std::string& environment_map)
	{
		// Create the skybox.
		auto texture = std::make_shared<TextureCubeMap>(
			environment_map,
			std::make_pair<std::uint32_t, std::uint32_t>(512, 512),
			sgl::PixelElementSize_HALF(),
			sgl::PixelStructure_RGB_ALPHA());
		auto cubemap_program = CreateProgram("CubeMapDeferred");
		cubemap_program->Uniform("projection", GetProjection());
		auto cube_mesh = CreateCubeStaticMesh();
		environment_material_ = std::make_shared<Material>();
		environment_material_->AddTexture("Skybox", texture);
		cube_mesh->ClearDepthBuffer(true);
		scene_tree_->AddNode(std::make_shared<SceneStaticMesh>(cube_mesh));
		
		// Add the default texture to the texture manager.
		environment_material_->AddTexture("Environment", texture);

		// Create the Monte-Carlo filter.
		auto monte_carlo_prefilter = std::make_shared<sgl::TextureCubeMap>(
			std::make_pair<std::uint32_t, std::uint32_t>(128, 128),
			sgl::PixelElementSize_FLOAT());
		FillProgramMultiTextureCubeMapMipmap(
			std::vector<std::shared_ptr<sgl::Texture>>{ monte_carlo_prefilter },
			std::map<std::string, std::shared_ptr<Texture>>
			{{
				"Environment", 
				environment_material_->GetTexture("Environment") 
			}},
			CreateProgram("MonteCarloPrefilter"),
			5,
			[](
				const int mipmap, 
				const std::shared_ptr<sgl::ProgramInterface> program)
			{
				float roughness = static_cast<float>(mipmap) / 4.0f;
				program->Uniform("roughness", roughness);
			});
		environment_material_->AddTexture(
			"MonteCarloPrefilter", 
			monte_carlo_prefilter);

		// Create the Irradiance cube map texture.
		auto irradiance = std::make_shared<sgl::TextureCubeMap>(
			std::make_pair<std::uint32_t, std::uint32_t>(32, 32),
			pixel_element_size_);
		FillProgramMultiTextureCubeMap(
			std::vector<std::shared_ptr<sgl::Texture>>{ irradiance },
			std::map<std::string, std::shared_ptr<Texture>>
				{{ 
					"Environment", 
					environment_material_->GetTexture("Environment") 
				}},
			CreateProgram("IrradianceCubeMap"));
		environment_material_->AddTexture("Irradiance", irradiance);

		// Create the LUT BRDF.
		auto integrate_brdf = std::make_shared<sgl::Texture>(
			std::make_pair<std::uint32_t, std::uint32_t>(512, 512),
			pixel_element_size_);
		FillProgramMultiTexture(
			std::vector<std::shared_ptr<Texture>>{ integrate_brdf },
			std::map<std::string, std::shared_ptr<Texture>>
				{{ 
					"Environment", 
					environment_material_->GetTexture("Environment") 
				}},
			CreateProgram("IntegrateBRDF"));
		environment_material_->AddTexture("IntegrateBRDF", integrate_brdf);
	}

	void Device::SetupCamera()
	{
		const auto& camera = scene_tree_->GetDefaultCamera();
		perspective_ = camera.ComputeProjection(size_);
		view_ = camera.ComputeView();
	}

	void Device::LoadSceneFromObjFile(const std::string& obj_file)
	{
		if (obj_file.empty())
			throw std::runtime_error("Error invalid file name: " + obj_file);
		std::string mtl_file = "";
		std::string mtl_path = obj_file;
		while (mtl_path.back() != '/' && mtl_path.back() != '\\')
		{
			mtl_path.pop_back();
		}
		std::ifstream obj_ifs(obj_file);
		if (!obj_ifs.is_open())
			throw std::runtime_error("Could not open file: " + obj_file);
		std::string obj_content = "";
		while (!obj_ifs.eof())
		{
			std::string line = "";
			if (!std::getline(obj_ifs, line)) break;
			if (line.empty()) continue;
			std::istringstream iss(line);
			std::string dump;
			if (!(iss >> dump))
				throw std::runtime_error("Error parsing file: " + obj_file);
			if (dump[0] == '#') continue;
			if (dump == "mtllib")
			{
				if (!(iss >> mtl_file))
					throw std::runtime_error("Error parsing file: " + obj_file);
				mtl_file = mtl_path + mtl_file;
				continue;
			}
			obj_content += line + "\n";
		}
		std::ifstream mtl_ifs(mtl_file);
		if (!mtl_ifs.is_open())
			throw std::runtime_error("Error cannot open file: " + mtl_file);
		scene_tree_ = LoadSceneFromObjStream(
			std::istringstream(obj_content),
			obj_file);
		material_map_ = LoadMaterialFromMtlStream(mtl_ifs, mtl_file);
	}

	const Camera& Device::GetCamera() const
	{
		return scene_tree_->GetDefaultCamera();
	}

} // End namespace sgl.
