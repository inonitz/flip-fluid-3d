#pragma once
#include <vector>
#include <util/vec2.hpp>
#include <glbinding/gl/enum.h>




struct TextureDescriptorArray
{
public:
    struct TextureFormat {
        gl::GLenum m_imageFormat = __scast(gl::GLenum, DEFAULT32);
        gl::GLenum m_format      = __scast(gl::GLenum, DEFAULT32);
        gl::GLenum m_type        = __scast(gl::GLenum, DEFAULT32);
    };


    struct TextureAuxillary
    {
    public:
        TextureAuxillary(TextureDescriptorArray& arr_to_interface, u8 id);
    
        void initialize(TextureFormat const& fmt, util::math::vec3u const& size);
        void set(void* buffer); /* buffer must be of equal size to the texture */
        void clear();
        
        void bindUnit(u32 unit) const;
        void bindImage(u32 unit, gl::GLenum access_type) const;


        TextureFormat     const& format() const { return m_array.m_fmt[m_inarray_id];  }
        util::math::vec3u const& size()   const { return m_array.m_size[m_inarray_id]; }
        u32                      id()     const { return m_array.m_id[m_inarray_id];   }
    private:
        TextureDescriptorArray& m_array;
        u8 m_inarray_id;
    };


public:
    void create(u32 size);
    void destroy();
    u8   size() const { return m_id.size(); };




    TextureAuxillary operator[](u8 id) {
        return TextureAuxillary{ *this, id };
    }


protected:
    friend struct TextureAuxillary;
    // will sort textures in buckets of each type
    // enum class TextureType : u8 {
    //     D1  = 0,
    //     D2  = 1,
    //     D3  = 2,
    //     MAX = 3
    // };
    std::vector<u32>               m_id;
    std::vector<TextureFormat>     m_fmt;
    std::vector<util::math::vec3u> m_size;
};