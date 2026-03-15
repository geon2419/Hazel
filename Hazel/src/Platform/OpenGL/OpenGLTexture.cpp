#include "hzpch.h"
#include "OpenGLTexture.h"

#include "Hazel/Core/FileSystem.h"

#include "stb_image.h"

#include <glad/glad.h>

namespace Hazel
{
static bool SupportsDirectStateAccessTextures()
{
    return GLAD_GL_VERSION_4_5 && glCreateTextures && glTextureStorage2D && glTextureParameteri &&
           glTextureSubImage2D && glBindTextureUnit;
}

static void CreateTexture(GLuint& rendererID)
{
    if (SupportsDirectStateAccessTextures())
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &rendererID);
        return;
    }

    glGenTextures(1, &rendererID);
}

static void UploadTexture2D(GLuint rendererID, GLenum internalFormat, GLenum dataFormat, uint32_t width,
                            uint32_t height, const void* data)
{
    if (SupportsDirectStateAccessTextures())
    {
        glTextureStorage2D(rendererID, 1, internalFormat, width, height);
        glTextureParameteri(rendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(rendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTextureSubImage2D(rendererID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
        return;
    }

    glBindTexture(GL_TEXTURE_2D, rendererID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0,
                 dataFormat, GL_UNSIGNED_BYTE, data);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
}

OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : m_Path(path), m_RendererID(0)
{
    int width = 0;
    int height = 0;
    int channels = 0;
    const auto resolvedPath = FileSystem::ResolvePath(path);

    stbi_set_flip_vertically_on_load(1);
    stbi_uc* data = stbi_load(resolvedPath.string().c_str(), &width, &height, &channels, 0);
    if (!data)
    {
        const char* failureReason = stbi_failure_reason();
        const std::string errorMessage = "Failed to load image '" + path + "' (w=" + std::to_string(width) +
                                         ", h=" + std::to_string(height) + ", channels=" + std::to_string(channels) +
                                         ", reason=" + (failureReason ? failureReason : "unknown") + ")";
        HZ_CORE_ASSERT(data, errorMessage);
        return;
    }

    m_Width = width;
    m_Height = height;

    GLenum internalFormat = 0;
    GLenum dataFormat = 0;

    if (channels == 4)
    {
        internalFormat = GL_RGBA8;
        dataFormat = GL_RGBA;
    }
    else if (channels == 3)
    {
        internalFormat = GL_RGB8;
        dataFormat = GL_RGB;
    }
    else if (channels == 1)
    {
        internalFormat = GL_R8;
        dataFormat = GL_RED;
    }
    else
    {
        const std::string errorMessage = "Unsupported texture format for image '" + path +
                                         "' (w=" + std::to_string(width) + ", h=" + std::to_string(height) +
                                         ", channels=" + std::to_string(channels) + ")";
        HZ_CORE_ASSERT(false, errorMessage);
        stbi_image_free(data);
        return;
    }

    CreateTexture(m_RendererID);
    UploadTexture2D(m_RendererID, internalFormat, dataFormat, m_Width, m_Height, data);

    stbi_image_free(data);
}

OpenGLTexture2D::~OpenGLTexture2D()
{
    glDeleteTextures(1, &m_RendererID);
}

void OpenGLTexture2D::Bind(uint32_t slot) const
{
    if (SupportsDirectStateAccessTextures())
    {
        glBindTextureUnit(slot, m_RendererID);
        return;
    }

    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
}

} // namespace Hazel
