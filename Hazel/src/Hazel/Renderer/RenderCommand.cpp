#include "RenderCommand.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"
#include "RendererAPI.h"
#include "hzpch.h"

namespace Hazel
{
RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;
}