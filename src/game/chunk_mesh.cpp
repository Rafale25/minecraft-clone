#include "chunk_mesh.hpp"

#include <stdio.h>

// ChunkMesh::~ChunkMesh()
// {
    // Note: Dangerous to to in destructor in case we pass ChunkMesh by value and create copies
    // if (VAO != 0)
    //     deleteAll();
// }

void ChunkMesh::deleteAll()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}
