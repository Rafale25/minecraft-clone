#include "Frustum.hpp"
#include "AABB.hpp"

// https://iquilezles.org/articles/frustumcorrect/
bool isAABBOnFrustum(const AABB& aabb, const Frustum& f)
{
    for (int32_t i = 0; i < 6; i++)
    {
        int32_t out = 0;
        out += ((glm::dot( f.planes[i], glm::vec4(aabb.min.x, aabb.min.y, aabb.min.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(aabb.max.x, aabb.min.y, aabb.min.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(aabb.min.x, aabb.max.y, aabb.min.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(aabb.max.x, aabb.max.y, aabb.min.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(aabb.min.x, aabb.min.y, aabb.max.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(aabb.max.x, aabb.min.y, aabb.max.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(aabb.min.x, aabb.max.y, aabb.max.z, 1.0f) ) < 0.0 )?1:0);
        out += ((glm::dot( f.planes[i], glm::vec4(aabb.max.x, aabb.max.y, aabb.max.z, 1.0f) ) < 0.0 )?1:0);
        if (out == 8) return false;
    }

    // Better check for very large AABB
    // check frustum outside/inside box
    // int32_t out;
    // out=0; for( int32_t i=0; i<8; i++ ) out += ((fru.mPoints[i].x > box.mMaxX)?1:0); if( out==8 ) return false;
    // out=0; for( int32_t i=0; i<8; i++ ) out += ((fru.mPoints[i].x < box.mMinX)?1:0); if( out==8 ) return false;
    // out=0; for( int32_t i=0; i<8; i++ ) out += ((fru.mPoints[i].y > box.mMaxY)?1:0); if( out==8 ) return false;
    // out=0; for( int32_t i=0; i<8; i++ ) out += ((fru.mPoints[i].y < box.mMinY)?1:0); if( out==8 ) return false;
    // out=0; for( int32_t i=0; i<8; i++ ) out += ((fru.mPoints[i].z > box.mMaxZ)?1:0); if( out==8 ) return false;
    // out=0; for( int32_t i=0; i<8; i++ ) out += ((fru.mPoints[i].z < box.mMinZ)?1:0); if( out==8 ) return false;

    return true;
}

void extractPlanesFromProjectionViewMatrix(const glm::mat4& m, glm::vec4 planes[6])
{
    for (int32_t i = 4; i--; ) { planes[0][i] = m[i][3] + m[i][0]; } // lfft
    for (int32_t i = 4; i--; ) { planes[1][i] = m[i][3] - m[i][0]; } // right
    for (int32_t i = 4; i--; ) { planes[2][i] = m[i][3] + m[i][1]; } // bottom
    for (int32_t i = 4; i--; ) { planes[3][i] = m[i][3] - m[i][1]; } // top
    for (int32_t i = 4; i--; ) { planes[4][i] = m[i][3] + m[i][2]; } // near
    for (int32_t i = 4; i--; ) { planes[5][i] = m[i][3] - m[i][2]; } // far
}

std::vector<glm::vec3> extractFrustumCornersWorldSpace(const glm::mat4& view_projection)
{
    const auto inv = glm::inverse(view_projection);

    std::vector<glm::vec3> frustumCorners;
    frustumCorners.reserve(8);
    for (uint32_t z = 0; z < 2; ++z) {
        for (uint32_t y = 0; y < 2; ++y) {
            for (uint32_t x = 0; x < 2; ++x) {
                const glm::vec4 pt =
                    inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);
                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

Frustum createFrustumFromViewProjection(const glm::mat4& view_projection)
{
    Frustum frustum;
    extractPlanesFromProjectionViewMatrix(view_projection, frustum.planes);
    return frustum;
}
