#include "chunk_mesh.hpp"

#include <stdio.h>

ChunkMesh::~ChunkMesh()
{
    if (VAO != 0)
        deleteAll();
}

void ChunkMesh::deleteAll()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &ssbo_texture_handles);
}
