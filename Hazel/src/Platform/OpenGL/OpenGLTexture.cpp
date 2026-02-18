#include "hzpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"

#include <glad/glad.h>

namespace Hazel
{

OpenGLTexture2D::OpenGLTexture2D(const std::string& path) : m_Path(path), m_RendererID(0)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    stbi_set_flip_vertically_on_load(1);
    stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
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

    glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
    glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

    glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // 업로드되는 각 픽셀 행(row)을 1바이트 정렬로 해석해 채널/너비 조합에 따른 행 틀어짐을 방지한다.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);
    // 전역 OpenGL 상태이므로 기본값(4바이트 정렬)으로 복원해 다른 텍스처 업로드 경로에 영향이 없게 한다.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    stbi_image_free(data);
}

OpenGLTexture2D::~OpenGLTexture2D()
{
    glDeleteTextures(1, &m_RendererID);
}

void OpenGLTexture2D::Bind(uint32_t slot) const
{
    glBindTextureUnit(slot, m_RendererID);
}

} // namespace Hazel
