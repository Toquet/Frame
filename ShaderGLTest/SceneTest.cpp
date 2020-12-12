#include "SceneTest.h"

namespace test {

	TEST_F(SceneTest, CheckConstructorMatrixTest)
	{
		EXPECT_EQ(GLEW_OK, glewInit());
		EXPECT_FALSE(scene_);
		glm::mat4 test(1.0f);
		scene_ = std::make_shared<sgl::SceneMatrix>(test);
		EXPECT_TRUE(scene_);
	}

	TEST_F(SceneTest, CheckConstructorMeshTest)
	{
		EXPECT_EQ(GLEW_OK, glewInit());
		EXPECT_FALSE(scene_);
		auto program = sgl::CreateProgram("SceneSimple");
		auto mesh = sgl::CreateStaticMeshFromObjFile("../Asset/Model/Cube.obj");
		scene_ = std::make_shared<sgl::SceneStaticMesh>(mesh);
		EXPECT_TRUE(scene_);
	}

	// Simple test scene with populate tree.
	TEST_F(SceneTest, CheckTreeConstructTest)
	{
		EXPECT_EQ(GLEW_OK, glewInit());
		EXPECT_FALSE(scene_tree_);
		scene_tree_ = std::make_shared<sgl::SceneTree>();
		EXPECT_TRUE(scene_tree_);
		PopulateTree();
		unsigned int count_mesh = 0;
		unsigned int count_matrix = 0;
		for (const auto& pair : scene_tree_->GetSceneMap())
		{
			const auto& scene = pair.second;
			const auto& mesh = scene->GetLocalMesh();
			if (mesh)
			{
				count_mesh++;
			}
			else
			{
				count_matrix++;
			}
		}
		EXPECT_EQ(2, count_mesh);
		EXPECT_EQ(2, count_matrix);
		EXPECT_EQ(4, scene_tree_->GetSceneMap().size());
	}

} // End namespace test.