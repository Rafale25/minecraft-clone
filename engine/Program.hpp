#pragma once

#include <string>
#include <unordered_map>
#include <glm/detail/type_mat4x4.hpp>
#include <glad/gl.h>
#include "Logger.hpp"

#include "stb_include.h"

class Program
{
private:
    GLuint ID = 0;
    const char* _vertexPath;
    const char* _fragmentPath;
    const char* _geometryPath;

    std::unordered_map<std::string, int> _uniformsLocations;

public:
    Program(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr):
    _vertexPath(vertexPath), _fragmentPath(fragmentPath), _geometryPath(geometryPath)
    {
        load(vertexPath, fragmentPath, geometryPath);
    }

    void load(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr) {
        std::string vertexCode = _loadWithInclude(vertexPath);
        std::string fragmentCode = _loadWithInclude(fragmentPath);

        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();
        GLuint vertex, fragment;

        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        if (!checkCompileErrors(vertex, "VERTEX", vertexPath)) {
            exit(0);
            return;
        };

        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        if (!checkCompileErrors(fragment, "FRAGMENT", fragmentPath)) {
            exit(0);
            return;
        };

        // if geometry shader is given, compile geometry shader
        GLuint geometry = -1;
        if (geometryPath != nullptr)
        {
            std::string geometryCode = _loadWithInclude(geometryPath);
            const char* gShaderCode = geometryCode.c_str();
            geometry = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometry, 1, &gShaderCode, NULL);
            glCompileShader(geometry);
            if (!checkCompileErrors(geometry, "GEOMETRY", geometryPath)) {
                exit(0);
                return;
            };
        }

        // shader Program
        GLuint id = glCreateProgram();
        glAttachShader(id, vertex);
        glAttachShader(id, fragment);
        if (geometryPath != nullptr)
            glAttachShader(id, geometry);

        // Call this line to prevent this OpenGL warning : Program/shader state performance warning: Vertex shader in program <n> is being recompiled based on GL state
        // https://stackoverflow.com/questions/57454921/what-causes-glsl-recompilation-of-vertex-shader-based-on-state
        glEnableVertexAttribArray(0);

        glLinkProgram(id);
        if (!checkCompileErrors(id, "PROGRAM")) return;
        // delete the shaders as they're linked into our program now and no longer necessery
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        if (geometryPath != nullptr)
            glDeleteShader(geometry);

        if (ID != 0) glDeleteProgram(ID);
        ID = id;

        GLint activeUniforms = 0;
        glGetProgramiv(id, GL_ACTIVE_UNIFORMS, &activeUniforms);

        GLsizei length = 0;
        GLint size = 0;
        GLenum type = 0;
        GLchar name[256] = {};
        for (int i = 0 ; i < activeUniforms ; ++i) {
            glGetActiveUniform(id, i, 256, &length, &size, &type, name);

            if (length == 0) {
                logE("Failed to glGetActiveUniform index {}", i);
                exit(-1);
            }

            _uniformsLocations[std::string(name, length)] = glGetUniformLocation(id, name);
            // logD("Program {} - UNIFORM: {}", id, name);
        }

        // for(const auto& [key, value] : _uniformsLocations) {
        //     logD("Program {} - {} - Location {}", id, key, value);
        // }

        logI("[Shader] Compiled shader program: {}, {} {} {}", ID, _vertexPath, _fragmentPath, geometryPath ? geometryPath : "");
    }

    void reload() {
        load(_vertexPath, _fragmentPath, _geometryPath);
    }

    void use() const {
        glUseProgram(ID);
    }

    void setBool(const std::string &name, bool value) const {
        glUniform1i(_uniformsLocations.at(name), (int)value);
    }

    void setInt(const std::string &name, int value) const {
        glUniform1i(_uniformsLocations.at(name), value);
    }

    void setFloat(const std::string &name, float value) const {
        glUniform1f(_uniformsLocations.at(name), value);
    }

    void setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(_uniformsLocations.at(name), 1, &value[0]);
    }

    void setVec2(const std::string &name, float x, float y) const {
        glUniform2f(_uniformsLocations.at(name), x, y);
    }

    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(_uniformsLocations.at(name), 1, &value[0]);
    }

    void setVec3(const std::string &name, float x, float y, float z) const {
        glUniform3f(_uniformsLocations.at(name), x, y, z);
    }

    void setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(_uniformsLocations.at(name), 1, &value[0]);
    }

    void setVec4(const std::string &name, float x, float y, float z, float w) {
        glUniform4f(_uniformsLocations.at(name), x, y, z, w);
    }

    void setMat2(const std::string &name, const glm::mat2 &mat) const {
        glUniformMatrix2fv(_uniformsLocations.at(name), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat3(const std::string &name, const glm::mat3 &mat) const {
        glUniformMatrix3fv(_uniformsLocations.at(name), 1, GL_FALSE, &mat[0][0]);
    }

    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(_uniformsLocations.at(name), 1, GL_FALSE, &mat[0][0]);
    }

private:
    std::string _loadWithInclude(const char* path)
    {
        char error[256] = {0};
        char *res = stb_include_file((char*)path, nullptr, (char*)(RESSOURCE_PATH "shaders"), error);

        if (res == nullptr) {
            logE("Failed to load shader {}:\nError: {}", path, error);
            return {};
        }

        std::string output{res};
        free(res);

        return output;
    }

    int checkCompileErrors(GLuint shader, const std::string& type, const char* path = "")
    {
        GLint success;
        GLchar infoLog[4096] = {0};

        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 4096, NULL, infoLog);
                logE("Error in file: {}", path);
                logE("SHADER_COMPILATION_ERROR of type: {}\n{}--------------", type, infoLog);
                return 0;
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 4096, NULL, infoLog);

                logE("PROGRAM_LINKING_ERROR of type: {}", type);
                logE("Files concerned: {} {} {}", _vertexPath, _fragmentPath, _geometryPath ? _geometryPath : "");
                logE("\n{}--------------", infoLog);
                return 0;
            }
        }
        return 1;
    }
};
