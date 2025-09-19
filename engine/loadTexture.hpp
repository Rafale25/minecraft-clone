#pragma once

#include "stb_image.h"
#include <glad/gl.h>
#include <cstdint>

GLuint createTextureFromPath(const char *path, int32_t format=GL_RGB, int32_t min_filter=GL_LINEAR, int32_t max_filter=GL_LINEAR, int32_t wrap=GL_REPEAT);
