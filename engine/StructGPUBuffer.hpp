#pragma once

// #include "Logger.hpp"
#include "VAO.hpp"
#include <cassert>
#include <cstddef>

#define BUFFER_SET(buffer, member, value) (buffer).template set<&decltype(buffer)::Struct::member>(value)

template <typename T>
class StructGPUBuffer {
public:
    using Struct = T;

    StructGPUBuffer() {
        m_buffer = createBufferStorage(nullptr, sizeof(T));
    }

    void bind(int id) const {
        glBindBufferBase(GL_UNIFORM_BUFFER, id, m_buffer);
    }

    template <typename StructT, typename MemberT>
    std::size_t offset_of(MemberT StructT::*member) {
        return reinterpret_cast<std::size_t>(
            &(reinterpret_cast<StructT const volatile*>(0)->*member)
        );
    }

    template <typename MemberT>
    void set(MemberT T::*member, const MemberT& value) {
        int offset = offset_of(member);

        memcpy(
            reinterpret_cast<char*>(&m_data) + offset,
            &value,
            sizeof(MemberT)
        );

        glNamedBufferSubData(m_buffer, offset, sizeof(MemberT), &value);
    }

    void upload() const {
        glNamedBufferSubData(m_buffer, 0, sizeof(T), &m_data);
    }

// private:

public:
    T m_data = {};
    GLuint m_buffer = 0;
};
