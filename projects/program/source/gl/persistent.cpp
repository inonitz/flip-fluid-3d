#include "glbinding/gl/functions.h"
#include "persistentbuf.hpp"
#include <glbinding/gl/gl.h>



void PersistentBuffer::create(
    u32 vertexSizeBytes, 
    u32 vertexCount,
    gl::BufferStorageMask read_or_write
) {
    void* temptr;
    u64 sizeBytes = vertexSizeBytes * vertexCount;
    gl::glCreateBuffers(1, &m_id);
    gl::glNamedBufferStorage(m_id, 
        sizeBytes, 
        nullptr, 
        gl::GL_NONE_BIT
        | read_or_write
        | gl::GL_MAP_PERSISTENT_BIT
        | gl::GL_MAP_COHERENT_BIT
    );
    temptr = gl::glMapNamedBufferRange(m_id, 
        0, sizeBytes, 
        gl::GL_NONE_BIT
        | gl::GL_MAP_READ_BIT
        | gl::GL_MAP_PERSISTENT_BIT
        | gl::GL_MAP_COHERENT_BIT
    );

    m_vertexSize = vertexSizeBytes;
    m_cpuSide.create(temptr, sizeBytes);
    return;
}


void PersistentBuffer::destroy()
{
    m_cpuSide.destroy();
    gl::glUnmapNamedBuffer(m_id);
    gl::glDeleteBuffers(1, &m_id);
    return;
}


void PersistentBuffer::waitForResultVisibillity()
{
    if(m_fence) {
        gl::glDeleteSync(__rcast(gl::__GLsync*, m_fence));
    }
    m_fence = gl::glFenceSync(gl::GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    return;
}
