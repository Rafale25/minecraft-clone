#include "Frustum.hpp"

#include <glm/glm.hpp>
#include "Camera.hpp"

// https://iquilezles.org/articles/frustumcorrect/
bool AABB::isOnFrustum(const Frustum& f) const
{
    for (int i = 0; i < 6; i++)
    {
        int out = 0;
        out += ((glm::dot( f.planes[i], glm::vec4(min.x, min.y, min.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(max.x, min.y, min.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(min.x, max.y, min.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(max.x, max.y, min.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(min.x, min.y, max.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(max.x, min.y, max.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(min.x, max.y, max.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(max.x, max.y, max.z, 1.0f) ) < 0.0 )?1:0);
        if (out == 8) return false;
    }

    // Better check for very large AABB
    // check frustum outside/inside box
    // int out;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].x > box.mMaxX)?1:0); if( out==8 ) return false;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].x < box.mMinX)?1:0); if( out==8 ) return false;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].y > box.mMaxY)?1:0); if( out==8 ) return false;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].y < box.mMinY)?1:0); if( out==8 ) return false;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].z > box.mMaxZ)?1:0); if( out==8 ) return false;
    // out=0; for( int i=0; i<8; i++ ) out += ((fru.mPoints[i].z < box.mMinZ)?1:0); if( out==8 ) return false;

    return true;
}

void extractPlanesFromProjectionViewMatrix(const glm::mat4& m, glm::vec4 planes[6])
{
    for (int i = 4; i--; ) { planes[0][i] = m[i][3] + m[i][0]; } // lfft
    for (int i = 4; i--; ) { planes[1][i] = m[i][3] - m[i][0]; } // right
    for (int i = 4; i--; ) { planes[2][i] = m[i][3] + m[i][1]; } // bottom
    for (int i = 4; i--; ) { planes[3][i] = m[i][3] - m[i][1]; } // top
    for (int i = 4; i--; ) { planes[4][i] = m[i][3] + m[i][2]; } // near
    for (int i = 4; i--; ) { planes[5][i] = m[i][3] - m[i][2]; } // far
}

Frustum createFrustumFromViewProjection(const glm::mat4& view_projection)
{
    Frustum frustum;
    extractPlanesFromProjectionViewMatrix(view_projection, frustum.planes);
    return frustum;
}

Frustum createFrustumFromCamera(const Camera& camera)
{
    Frustum frustum;
    extractPlanesFromProjectionViewMatrix(camera.getProjection() * camera.getView(), frustum.planes);
    return frustum;
}
