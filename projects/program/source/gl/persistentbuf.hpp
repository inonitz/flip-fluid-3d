#pragma once
#include <util/bufferptr.hpp>
#include <glbinding/gl/bitfield.h>




struct PersistentBuffer
{
public:
    void create(
        u32 vertexSizeBytes, 
        u32 vertexCount,
        gl::BufferStorageMask cpu_side_read_or_write
    );
    void destroy();

    u8& operator[](u32 index) {
        return m_cpuSide.data()[index * m_vertexSize];
    }

    const u8* begin() const { return m_cpuSide.begin(); }
    const u8* end()   const { return m_cpuSide.end();   }
    void waitForResultVisibillity();


    u32 id() const { return m_id; }


private:
    u32   m_id;
    u32   m_vertexSize;
    void* m_fence;
    util::BufferPointer<u8> m_cpuSide;
};