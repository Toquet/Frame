#pragma once

#include <gtest/gtest.h>
#include "Frame/Window.h"
#include "Frame/OpenGL/StaticMesh.h"

namespace test {

	class StaticMeshTest : public testing::Test
	{
	public:
		StaticMeshTest() { window_ = frame::CreateSDLOpenGL({ 320, 200 }); }

	protected:
		std::shared_ptr<frame::WindowInterface> window_ = nullptr;
		std::shared_ptr<frame::opengl::StaticMesh> static_mesh_ = nullptr;
	};

} // End namespace test.
