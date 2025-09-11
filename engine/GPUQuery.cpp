#include "GPUQuery.hpp"
#include "glad/gl.h"

void GPUQuery::Begin() {
    if (_query_object == 0) {
        glGenQueries(1, &_query_object);
    }

    glBeginQuery(GL_TIME_ELAPSED, _query_object);
}

uint64_t GPUQuery::End() {
    glEndQuery(GL_TIME_ELAPSED);

    GLuint64 data = 0;
    glGetQueryObjectui64v(_query_object, GL_QUERY_RESULT, &data); // blocking

    return data;
}
