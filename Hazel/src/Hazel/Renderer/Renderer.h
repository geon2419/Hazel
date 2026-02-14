#pragma once

// #include "RenderCommand.h"
#include "RendererAPI.h"
#include "VertexArray.h"
#include "OrthographicCamera.h"
#include "Shader.h"

#include <memory>
// #include <glm/ext/matrix_float4x4.hpp>
#include <glm/fwd.hpp>

namespace Hazel
{

class Renderer
{
  public:
	static void BeginScene(const OrthographicCamera& camera);
	static void EndScene();

	static void Submit(const std::shared_ptr<Shader>& shader, const std::shared_ptr<VertexArray>& vertexArray,
	                   const glm::mat4& transform = glm::mat4(1.0f));

	inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }

  private:
	struct SceneData
	{
		glm::mat4 ViewProjectionMatrix;
	};

	static SceneData* s_SceneData;
};
} // namespace Hazel