#include "ShaderProgram.hpp"
#include "Logger.hpp"
#include "stb_include.h"
#include <glm/detail/type_mat4x4.hpp>
#include <glad/gl.h>

ShaderProgram::ShaderProgram(const char* vertexPath, const char* fragmentPath, const char* geometryPath):
_vertexPath(vertexPath), _fragmentPath(fragmentPath), _geometryPath(geometryPath)
{
    load(vertexPath, fragmentPath, geometryPath);
}

void ShaderProgram::load(const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
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

        if (size > 1) { // support uniform arrays
            for (int i = 0 ; i < size ; ++i) {
                std::string str_name(name, length);
                str_name.resize(length - 2);
                str_name += std::to_string(i) + ']';

                _uniformsLocations[str_name] = glGetUniformLocation(id, str_name.c_str());
                // logD("str uniform: {}", str_name);
            }
        } else {
            _uniformsLocations[std::string(name, length)] = glGetUniformLocation(id, name);
        }

        // logD("Program {} - UNIFORM: {} - size {}", id, name, size);
    }

    // for(const auto& [key, value] : _uniformsLocations) {
    //     logD("Program {} - {} - Location {}", id, key, value);
    // }

    logI("[Shader] Compiled shader program: {}, {} {} {}", ID, _vertexPath, _fragmentPath, geometryPath ? geometryPath : "");
}

void ShaderProgram::reload() {
    load(_vertexPath, _fragmentPath, _geometryPath);
}

void ShaderProgram::use() const {
    glUseProgram(ID);
}

GLint ShaderProgram::getUniformLocation(const std::string &name) const {
    const auto it = _uniformsLocations.find(name);
    if (it == _uniformsLocations.end()) {
        logE("Invalid uniform {} for Program {};\n{};\n{}", name, ID, _vertexPath, _fragmentPath);
    }
    return it->second;
}

void ShaderProgram::setBool(const std::string &name, bool value) const {
    glUniform1i(getUniformLocation(name), (int)value);
}

void ShaderProgram::setInt(const std::string &name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

void ShaderProgram::setFloat(const std::string &name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void ShaderProgram::setVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(getUniformLocation(name), 1, &value[0]);
}

void ShaderProgram::setVec2(const std::string &name, float x, float y) const {
    glUniform2f(getUniformLocation(name), x, y);
}

void ShaderProgram::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(getUniformLocation(name), 1, &value[0]);
}

void ShaderProgram::setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(getUniformLocation(name), x, y, z);
}

void ShaderProgram::setVec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(getUniformLocation(name), 1, &value[0]);
}

void ShaderProgram::setVec4(const std::string &name, float x, float y, float z, float w) {
    glUniform4f(getUniformLocation(name), x, y, z, w);
}

void ShaderProgram::setMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::setMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

void ShaderProgram::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, &mat[0][0]);
}

std::string ShaderProgram::_loadWithInclude(const char* path)
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

int ShaderProgram::checkCompileErrors(GLuint shader, const std::string& type, const char* path)
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

std::vector<std::string> ShaderProgram::getUniformsNames() const {
    std::vector<std::string> names;
    for (const auto& [key, _] : _uniformsLocations) {
        names.emplace_back(key);
    }
    return names;
}
