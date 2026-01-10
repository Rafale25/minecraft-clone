#include "GPUQuery.hpp"
#include "glad/gl.h"

void GPUQuery::Begin() {
    if (m_queryObject == 0) {
        glGenQueries(1, &m_queryObject);
    }

    glBeginQuery(GL_TIME_ELAPSED, m_queryObject);
}

uint64_t GPUQuery::End() {
    glEndQuery(GL_TIME_ELAPSED);

    GLuint64 data = 0;
    glGetQueryObjectui64v(m_queryObject, GL_QUERY_RESULT, &data); // blocking

    return data;
}
