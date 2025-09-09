#pragma once

#include <string>
#include <unordered_map>
#include <glm/detail/type_mat4x4.hpp>

typedef unsigned int GLuint;
typedef int GLint;

class Program
{
private:
    GLuint ID = 0;
    const char* _vertexPath;
    const char* _fragmentPath;
    const char* _geometryPath;

    std::unordered_map<std::string, int> _uniformsLocations;

public:
    Program(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

    void load(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);
    void reload();
    void use() const;

    GLint getUniformLocation(const std::string &name) const;

    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec2(const std::string &name, float x, float y) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w);
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    std::string _loadWithInclude(const char* path);
    int checkCompileErrors(GLuint shader, const std::string& type, const char* path = "");
};
