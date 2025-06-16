#include "texturearray.hpp"
#include <util/marker2.hpp>
#include <glbinding/gl/functions.h>




void TextureDescriptorArray::create(u32 size) 
{
    m_id.resize(size);
    m_fmt.resize(size);
    m_size.resize(size);
    gl::glCreateTextures(gl::GL_TEXTURE_3D, size, m_id.data());
    return;
}


void TextureDescriptorArray::destroy()
{
    gl::glDeleteTextures(m_size.size(), m_id.data());
    m_id.resize(0);
    m_fmt.resize(0);
    m_size.resize(0);
    return;
}




TextureDescriptorArray::TextureAuxillary::TextureAuxillary(TextureDescriptorArray& arr_to_interface, u8 id) 
    : m_array{arr_to_interface}, m_inarray_id{id} {}


void TextureDescriptorArray::TextureAuxillary::initialize(
    TextureFormat     const& fmt, 
    util::math::vec3u const& size
) {
    auto texid = m_array.m_id[m_inarray_id];
    m_array.m_fmt[m_inarray_id]  = fmt;
    m_array.m_size[m_inarray_id] = size;
    gl::glTextureParameteri(texid, gl::GL_TEXTURE_WRAP_S, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(texid, gl::GL_TEXTURE_WRAP_T, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(texid, gl::GL_TEXTURE_WRAP_R, gl::GL_CLAMP_TO_EDGE);
    gl::glTextureParameteri(texid, gl::GL_TEXTURE_MIN_FILTER, gl::GL_NEAREST);
    gl::glTextureParameteri(texid, gl::GL_TEXTURE_MAG_FILTER, gl::GL_NEAREST);
    markfmt("Texture %u ( 0x%x, 0x%x, 0x%x ) %s", texid, 
        fmt.m_imageFormat,
        fmt.m_format,
        fmt.m_type,
        size.to_string()
    );
    gl::glTextureStorage3D(texid, 1, fmt.m_imageFormat, size.x, size.y, size.z);
    gl::glClearTexImage(texid, 0, fmt.m_format, fmt.m_type, nullptr);
    return;
}


void TextureDescriptorArray::TextureAuxillary::set(void* buffer)
{
    auto& texid   = m_array.m_id[m_inarray_id];
    auto& texfmt  = m_array.m_fmt[m_inarray_id];
    auto& texsize = m_array.m_size[m_inarray_id];


    gl::glTextureSubImage3D(texid, 0, 
        0, 0, 0, 
        texsize.x, texsize.y, texsize.z,
        texfmt.m_format,
        texfmt.m_type,
        buffer
    );
    return;
}


void TextureDescriptorArray::TextureAuxillary::clear()
{
    gl::glClearTexImage(
        m_array.m_id[m_inarray_id], 
        0, 
        m_array.m_fmt[m_inarray_id].m_format, 
        m_array.m_fmt[m_inarray_id].m_type, 
        nullptr
    );
    return;
}


void TextureDescriptorArray::TextureAuxillary::bindUnit(u32 unit) const
{
    gl::glBindTextureUnit(unit, m_array.m_id[m_inarray_id]);
    return;
}


void TextureDescriptorArray::TextureAuxillary::bindImage(u32 unit, gl::GLenum access_type) const
{
    gl::glBindImageTexture(
        unit, m_array.m_id[m_inarray_id], 
        0, 
        true, 
        0, 
        access_type, 
        m_array.m_fmt[m_inarray_id].m_imageFormat
    );
    return;
}

