#include "renderer.h"

#include <GL/glew.h>
#include <fmt/core.h>

#include <stdexcept>

#include "frame/node_matrix.h"
#include "frame/node_static_mesh.h"
#include "frame/opengl/file/load_program.h"
#include "frame/opengl/material.h"
#include "frame/opengl/static_mesh.h"
#include "frame/opengl/texture.h"
#include "frame/opengl/texture_cube_map.h"
#include "frame/uniform_wrapper.h"

namespace frame::opengl {

namespace {
// Get the 6 view for the cube map.
const std::array<glm::mat4, 6> views_cubemap = {
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, 1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                glm::vec3(0.0f, 0.0f, -1.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                glm::vec3(0.0f, -1.0f, 0.0f)),
    glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                glm::vec3(0.0f, -1.0f, 0.0f))
};
// Projection cube map.
const glm::mat4 projection_cubemap = glm::perspective(glm::radians(90.0f), 1.0f, 0.01f, 10.0f);
}  // namespace

Renderer::Renderer(LevelInterface& level, glm::uvec4 viewport)
    : level_(level), viewport_(viewport) {
    // TODO(anirul): Check viewport!!!
    render_buffer_.CreateStorage({ viewport_.z - viewport_.x, viewport_.w - viewport_.y });
    frame_buffer_.AttachRender(render_buffer_);
    auto program = file::LoadProgramFromName("display");
    if (!program) throw std::runtime_error("No program!");
    auto material = std::make_unique<Material>();
    program->SetName("DisplayProgram");
    auto maybe_display_program_id = level_.AddProgram(std::move(program));
    if (!maybe_display_program_id) throw std::runtime_error("No display program id.");
    display_program_id_ = maybe_display_program_id;
    material->SetName("DisplayMaterial");
    auto maybe_display_material_id = level_.AddMaterial(std::move(material));
    if (!maybe_display_material_id) throw std::runtime_error("No display material id.");
    display_material_id_      = maybe_display_material_id;
    auto maybe_out_texture_id = level_.GetDefaultOutputTextureId();
    if (!maybe_out_texture_id) throw std::runtime_error("No output texture id.");
    auto out_texture_id = maybe_out_texture_id;
    auto& out_texture   = level_.GetTextureFromId(out_texture_id);
    // Get material from level as material was moved away.
    level_.GetMaterialFromId(display_material_id_).SetProgramId(display_program_id_);
    if (!level_.GetMaterialFromId(display_material_id_).AddTextureId(out_texture_id, "Display")) {
        throw std::runtime_error("Couldn't add texture to material.");
    }
}

void Renderer::RenderNode(EntityId node_id, EntityId material_id, const glm::mat4& projection,
                          const glm::mat4& view, double t /* = 0.0*/) {
    // Bail out in case of no node.
    if (node_id == NullId) return;
    // Keep in memory the time.
    latest_time_ = t;
    // Check current node.
    auto& node = level_.GetSceneNodeFromId(node_id);
    // Try to cast to a node static mesh.
    auto& node_static_mesh = dynamic_cast<NodeStaticMesh&>(node);
    auto mesh_id           = node.GetLocalMesh();
    // In case no mesh then this is a clear event.
    if (!mesh_id) {
        GLbitfield bit_field       = 0;
        std::uint32_t clean_buffer = node_static_mesh.GetCleanBuffer();
        if (clean_buffer | proto::CleanBuffer::CLEAR_COLOR) bit_field += GL_COLOR_BUFFER_BIT;
        if (clean_buffer | proto::CleanBuffer::CLEAR_DEPTH) bit_field += GL_DEPTH_BUFFER_BIT;
        if (bit_field) glClear(bit_field);
        return;
    }
    auto& static_mesh = level_.GetStaticMeshFromId(mesh_id);
    // Try to find the material for the mesh.
    if (material_id == NullId) {
        throw std::runtime_error("No material?");
    }
    MaterialInterface& material = level_.GetMaterialFromId(material_id);
    RenderMesh(static_mesh, material, projection, view, node.GetLocalModel(t), t);
}

void Renderer::RenderMesh(StaticMeshInterface& static_mesh, MaterialInterface& material,
                          const glm::mat4& projection, const glm::mat4& view,
                          const glm::mat4& model /* = glm::mat4(1.0f)*/, double t /* = 0.0*/) {
    ScopedBind scoped_frame(frame_buffer_);

    // Keep in memory the time.
    latest_time_ = t;

    if (static_mesh.IsClearBuffer()) {
        glClear(GL_DEPTH_BUFFER_BIT);
    }

    auto program_id  = material.GetProgramId();
    auto& program    = level_.GetProgramFromId(program_id);
    last_program_id_ = program_id;
    assert(program.GetOutputTextureIds().size());

    // In case the camera doesn't exist it will create a basic one.
    UniformWrapper uniform_wrapper(projection, view, model, level_.GetDefaultEnvironmentModel(), t);
    // Go through the callback.
    callback_(uniform_wrapper, static_mesh, material);
    program.Use(uniform_wrapper);

    auto texture_out_ids = program.GetOutputTextureIds();
    auto& texture_ref    = level_.GetTextureFromId(*texture_out_ids.cbegin());
    auto size            = texture_ref.GetSize();

    glViewport(viewport_.x, viewport_.y, viewport_.z, viewport_.w);

    int i = 0;
    for (const auto& texture_id : program.GetOutputTextureIds()) {
        if (level_.GetTextureFromId(texture_id).IsCubeMap()) {
            auto& opengl_texture =
                dynamic_cast<TextureCubeMap&>(level_.GetTextureFromId(texture_id));
            // TODO(anirul): Check the mipmap level (last parameter)!
            frame_buffer_.AttachTexture(opengl_texture.GetId(),
                                        FrameBuffer::GetFrameColorAttachment(i),
                                        FrameBuffer::GetFrameTextureType(texture_frame_), 0);
        } else {
            auto& opengl_texture = dynamic_cast<Texture&>(level_.GetTextureFromId(texture_id));
            // TODO(anirul): Check the mipmap level (last parameter)!
            frame_buffer_.AttachTexture(opengl_texture.GetId(),
                                        FrameBuffer::GetFrameColorAttachment(i),
                                        FrameTextureType::TEXTURE_2D, 0);
        }
        i++;
    }
    frame_buffer_.DrawBuffers(static_cast<std::uint32_t>(texture_out_ids.size()));

    std::map<std::string, std::vector<std::int32_t>> uniform_include;
    for (const auto& id : material.GetIds()) {
        EntityId texture_id = NullId;
        if (level_.GetEnumTypeFromId(id) == EntityTypeEnum::TEXTURE) {
            texture_id = id;
        } else {
            // TODO(anirul): Find a better way to find the texture associated with the stream.
            texture_id = id + 1;
        }
        // TODO(anirul): Why? id and not texture id?
        const auto p  = material.EnableTextureId(id);
        auto& texture = level_.GetTextureFromId(texture_id);
        if (texture.IsCubeMap()) {
            auto& gl_texture = dynamic_cast<TextureCubeMap&>(level_.GetTextureFromId(texture_id));
            gl_texture.Bind(p.second);
            program.Uniform(p.first, p.second);
        } else {
            auto& gl_texture = dynamic_cast<Texture&>(level_.GetTextureFromId(texture_id));
            gl_texture.Bind(p.second);
            program.Uniform(p.first, p.second);
        }
    }

    auto& gl_static_mesh = dynamic_cast<StaticMesh&>(static_mesh);
    glBindVertexArray(gl_static_mesh.GetId());

    auto& index_buffer    = level_.GetBufferFromId(static_mesh.GetIndexBufferId());
    auto& gl_index_buffer = dynamic_cast<Buffer&>(index_buffer);
    // This was crashing the driver so...
    if (static_mesh.GetIndexSize()) {
        gl_index_buffer.Bind();
        switch (static_mesh.GetRenderPrimitive()) {
            case proto::SceneStaticMesh::TRIANGLE:
                glDrawElements(
                    GL_TRIANGLES,
                    static_cast<GLsizei>(static_mesh.GetIndexSize()) / sizeof(std::uint32_t),
                    GL_UNSIGNED_INT, nullptr);
                break;
            case proto::SceneStaticMesh::POINT:
                glDrawElements(
                    GL_POINTS,
                    static_cast<GLsizei>(static_mesh.GetIndexSize()) / sizeof(std::uint32_t),
                    GL_UNSIGNED_INT, nullptr);
                break;
            case proto::SceneStaticMesh::LINE:
                glDrawElements(
                    GL_LINES,
                    static_cast<GLsizei>(static_mesh.GetIndexSize()) / sizeof(std::uint32_t),
                    GL_UNSIGNED_INT, nullptr);
                break;
            default:
                throw std::runtime_error(fmt::format(
                    "Couldn't draw primitive {}", proto::SceneStaticMesh_RenderPrimitiveEnum_Name(
                                                      static_mesh.GetRenderPrimitive())));
        }
        gl_index_buffer.UnBind();
    }
    program.UnUse();
    glBindVertexArray(0);

    for (const auto id : material.GetIds()) {
        EntityId texture_id = id;
        if (level_.GetEnumTypeFromId(id) != EntityTypeEnum::TEXTURE) {
            texture_id = id + 1;
        }
        auto& texture = level_.GetTextureFromId(texture_id);
        if (texture.IsCubeMap()) {
            auto& gl_texture = dynamic_cast<TextureCubeMap&>(level_.GetTextureFromId(texture_id));
            gl_texture.UnBind();
        } else {
            auto& gl_texture = dynamic_cast<Texture&>(level_.GetTextureFromId(texture_id));
            gl_texture.UnBind();
        }
    }
    material.DisableAll();
}

void Renderer::FakeMesh(StaticMeshInterface& static_mesh, MaterialInterface& material,
                        const glm::mat4& projection, const glm::mat4& view,
                        const glm::mat4& model /* = glm::mat4(1.0f)*/, double dt /* = 0.0*/) {
    // frame_buffer_.Bind(0);
    // glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::Display(double dt /* = 0.0*/) {
    glViewport(viewport_.x, viewport_.y, viewport_.z, viewport_.w);
    auto maybe_quad_id = level_.GetDefaultStaticMeshQuadId();
    if (maybe_quad_id == NullId) throw std::runtime_error("No quad id.");
    auto& quad    = level_.GetStaticMeshFromId(maybe_quad_id);
    auto& program = level_.GetProgramFromId(display_program_id_);
    UniformWrapper uniform_wrapper{};
    program.Use(uniform_wrapper);
    auto& material = level_.GetMaterialFromId(display_material_id_);
    for (const auto id : material.GetIds()) {
        auto& opengl_texture = dynamic_cast<Texture&>(level_.GetTextureFromId(id));
        const auto p         = material.EnableTextureId(id);
        opengl_texture.Bind(p.second);
        program.Uniform(p.first, p.second);
    }
    auto& gl_quad = dynamic_cast<StaticMesh&>(quad);
    glBindVertexArray(gl_quad.GetId());
    auto& index_buffer    = level_.GetBufferFromId(quad.GetIndexBufferId());
    auto& gl_index_buffer = dynamic_cast<Buffer&>(index_buffer);

    gl_index_buffer.Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(quad.GetIndexSize()) / sizeof(std::int32_t),
                   GL_UNSIGNED_INT, nullptr);
    gl_index_buffer.UnBind();

    program.UnUse();
    glBindVertexArray(0);

    for (const auto id : material.GetIds()) {
        auto& opengl_texture = dynamic_cast<Texture&>(level_.GetTextureFromId(id));
        opengl_texture.UnBind();
    }
    material.DisableAll();
}

void Renderer::SetDepthTest(bool enable) {
    if (enable) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void Renderer::RenderAllMeshes(const glm::mat4& projection, const glm::mat4& view,
                               double t /*= 0.0*/) {
    // Keep in memory the time.
    latest_time_ = t;
    // This will ensure that it is only true once.
    auto first_render = std::exchange(first_render_, false);
    for (const auto& p : level_.GetStaticMeshMaterialIds()) {
        auto [material_id, render_time_enum] = p.second;
        // Check this is a pre render action and this is the first render.
        if (render_time_enum == proto::SceneStaticMesh::PRE_RENDER) {
            auto temp_viewport = viewport_;
            if (first_render) {
                // Now this get the image size from the environment map.
                auto& material = level_.GetMaterialFromId(material_id);
                auto ids       = material.GetIds();
                assert(!ids.empty());
                auto& texture = level_.GetTextureFromId(ids[0]);
                auto size     = texture.GetSize();
                viewport_     = glm::ivec4(0, 0, size.x / 2, size.y / 2);
                for (std::uint32_t i = 0; i < 6; ++i) {
                    SetCubeMapTarget(GetTextureFrameFromPosition(i));
                    RenderNode(p.first, material_id, projection_cubemap, views_cubemap[i], t);
                }
            }
            viewport_ = temp_viewport;
        } else {
            // This should also call clear buffers.
            RenderNode(p.first, material_id, projection, view, t);
        }
    }
}

double Renderer::GetLatestTime() const { return latest_time_; }

}  // End namespace frame::opengl.
