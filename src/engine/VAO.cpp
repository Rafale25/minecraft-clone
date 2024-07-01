#include <stdio.h>
#include "VAO.hpp"

FormatInfo FormatIterator::info() {
    FormatInfo info{
        .size = 0,
        .nodes = 0,
        .divisor = 0,
        .valid = true
    };

    FormatIterator it(ptr);

    while (true) {
        FormatNode node = it.next();
        if (node.valid == false) break;

        info.size += node.size;
        if (node.type) ++info.nodes;
    }

    return info;
}

FormatNode FormatIterator::next() {
    node = {
        .size = 0,
        .count = 0,
        .type = 0,
        .normalize = false,
        .valid = true,
    };

    while (true) {
        char chr = *ptr++;

        switch (chr)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                node.count = node.count * 10 + chr - '0';
                break;
            case 'f':
                if (node.count == 0) { node.count = 1; }
                node.size = 4 * node.count;
                node.type = GL_FLOAT;
                node.normalize = false;
                return node;

            case 'i':
                if (node.count == 0) { node.count = 1; }
                node.normalize = false;
                node.size = 4 * node.count;
                node.type = GL_INT;
                return node;

            case 'x':
                if (node.count == 0) { node.count = 1; }
                node.type = 0;
                node.normalize = false;

                switch (*ptr++) {
                    case '1':
                        if (*ptr && *ptr != ' ') return FormatNode::invalid();
                        node.size = 1 * node.count;
                        break;
                    case '2':
                        if (*ptr && *ptr != ' ') return FormatNode::invalid();
                        node.size = 2 * node.count;
                        break;
                    case '4':
                        if (*ptr && *ptr != ' ') return FormatNode::invalid();
                        node.size = 4 * node.count;
                        break;
                    case '8':
                        if (*ptr && *ptr != ' ') return FormatNode::invalid();
                        node.size = 8 * node.count;
                        break;
                    case 0:
                    case ' ':
                        node.size = 1 * node.count;
                        break;
                    default:
                        return FormatNode::invalid();
                }
                return node;

            case ' ':
                break;

            // case 0:
            // case '/':
            //     break;

            default:
                return FormatNode::invalid();
        }
    }
}

GLuint createBufferData(const void* data, GLsizeiptr size, GLenum usage) {
    GLuint buffer;
    glCreateBuffers(1, &buffer);
    glNamedBufferData(buffer, size, data, usage);

    return buffer;
}

GLuint createBufferStorage(const void* data, GLsizeiptr size, GLbitfield usage) {
    GLuint buffer;
    glCreateBuffers(1, &buffer);
    glNamedBufferStorage(buffer, size, data, usage);

    return buffer;
}

GLuint createVAO(GLuint buffer, const char * const format, GLuint EBO) {
    GLuint VAO;
    glCreateVertexArrays(1, &VAO);

    FormatIterator it{format};
    FormatInfo info = it.info();

    printf("info: size:%d nodes:%d valid:%d\n", info.size, info.nodes, info.valid);

    int attribute = 0;
    GLuint relative_offset = 0;

    while (1) {
        FormatNode node = it.next();
        if (node.valid == false) break;
        printf("node: %d %d %d\n", node.size, node.count, node.type);

        if (node.type) {
            glEnableVertexArrayAttrib(VAO, attribute);
            glVertexArrayAttribBinding(VAO, attribute, 0);
            switch (node.type)
            {
                case GL_FLOAT: glVertexArrayAttribFormat(VAO, attribute, node.count, node.type, node.normalize, relative_offset); break;
                case GL_DOUBLE: glVertexArrayAttribLFormat(VAO, attribute, node.count, node.type, relative_offset); break;
                case GL_INT: glVertexArrayAttribIFormat(VAO, attribute, node.count, node.type, relative_offset); break;
                case GL_UNSIGNED_INT: glVertexArrayAttribIFormat(VAO, attribute, node.count, node.type, relative_offset); break;
            }

            attribute += 1;
        }

        relative_offset += node.size;
    }

    glVertexArrayVertexBuffer(VAO, 0, buffer, 0, info.size);

    if (EBO != GL_INVALID_INDEX) {
        glVertexArrayElementBuffer(VAO, EBO);
    }

    return VAO;
}
