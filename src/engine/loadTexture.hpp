#pragma once

#include <glad/gl.h>
#include "stb_image.h"

GLuint createTextureFromPath(const char *path, int32_t format=GL_RGB, int32_t min_filter=GL_LINEAR, int32_t max_filter=GL_LINEAR, int32_t wrap=GL_REPEAT);
