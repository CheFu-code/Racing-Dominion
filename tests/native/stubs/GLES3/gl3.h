#pragma once
// Host-only types. These stubs must never be on the Android build's include path.
using GLuint = unsigned int;
using GLenum = unsigned int;
using GLsizei = int;
constexpr GLenum GL_BLEND = 1, GL_SRC_ALPHA = 2, GL_ONE_MINUS_SRC_ALPHA = 3, GL_CULL_FACE = 4;
constexpr unsigned char GL_FALSE = 0, GL_TRUE = 1;
inline void glEnable(GLenum) {}
inline void glDisable(GLenum) {}
inline void glBlendFunc(GLenum, GLenum) {}
inline void glDepthMask(unsigned char) {}
