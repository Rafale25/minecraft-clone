#pragma once

#include <glad/gl.h>
#include "stb_image.h"

GLuint createTextureFromPath(const char *path, int format=GL_RGB, int min_filter=GL_LINEAR, int max_filter=GL_LINEAR, int wrap=GL_REPEAT);
