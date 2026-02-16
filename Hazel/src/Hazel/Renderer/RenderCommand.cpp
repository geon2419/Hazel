#include "hzpch.h"
#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "RendererAPI.h"

namespace Hazel
{
RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}
