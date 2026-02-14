#pragma once

#include "Hazel/Renderer/RendererAPI.h"
#include "Hazel/Renderer/VertexArray.h"

#include <glm/fwd.hpp>
#include <memory>

namespace Hazel
{

class OpenGLRendererAPI : public RendererAPI
{
  public:
	virtual void SetClearColor(const glm::vec4& color) override;
	virtual void Clear() override;

	virtual void DrawIndexed(const std::shared_ptr<VertexArray>& vertexArray) override;
};

} // namespace Hazel
