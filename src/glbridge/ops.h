#ifndef TSPGL_OPS_H
#define TSPGL_OPS_H

enum {
    OP_glActiveTexture = 16,
    OP_glAttachShader = 17,
    OP_glBindFramebuffer = 18,
    OP_glBindRenderbuffer = 19,
    OP_glBindTexture = 20,
    OP_glBlendColor = 21,
    OP_glBlendEquation = 22,
    OP_glBlendEquationSeparate = 23,
    OP_glBlendFunc = 24,
    OP_glBlendFuncSeparate = 25,
    OP_glCheckFramebufferStatus = 26,
    OP_glClear = 27,
    OP_glClearColor = 28,
    OP_glClearDepthf = 29,
    OP_glClearStencil = 30,
    OP_glColorMask = 31,
    OP_glCompileShader = 32,
    OP_glCopyTexImage2D = 33,
    OP_glCopyTexSubImage2D = 34,
    OP_glCreateProgram = 35,
    OP_glCreateShader = 36,
    OP_glCullFace = 37,
    OP_glDeleteProgram = 38,
    OP_glDeleteShader = 39,
    OP_glDepthFunc = 40,
    OP_glDepthMask = 41,
    OP_glDepthRangef = 42,
    OP_glDetachShader = 43,
    OP_glDisable = 44,
    OP_glEnable = 45,
    OP_glFinish = 46,
    OP_glFlush = 47,
    OP_glFramebufferRenderbuffer = 48,
    OP_glFramebufferTexture2D = 49,
    OP_glFrontFace = 50,
    OP_glGenerateMipmap = 51,
    OP_glGetError = 52,
    OP_glHint = 53,
    OP_glIsBuffer = 54,
    OP_glIsEnabled = 55,
    OP_glIsFramebuffer = 56,
    OP_glIsProgram = 57,
    OP_glIsRenderbuffer = 58,
    OP_glIsShader = 59,
    OP_glIsTexture = 60,
    OP_glLineWidth = 61,
    OP_glLinkProgram = 62,
    OP_glPolygonOffset = 63,
    OP_glReleaseShaderCompiler = 64,
    OP_glRenderbufferStorage = 65,
    OP_glSampleCoverage = 66,
    OP_glScissor = 67,
    OP_glStencilFunc = 68,
    OP_glStencilFuncSeparate = 69,
    OP_glStencilMask = 70,
    OP_glStencilMaskSeparate = 71,
    OP_glStencilOp = 72,
    OP_glStencilOpSeparate = 73,
    OP_glTexParameterf = 74,
    OP_glTexParameteri = 75,
    OP_glUniform1f = 76,
    OP_glUniform2f = 77,
    OP_glUniform3f = 78,
    OP_glUniform4f = 79,
    OP_glUniform1i = 80,
    OP_glUniform2i = 81,
    OP_glUniform3i = 82,
    OP_glUniform4i = 83,
    OP_glUseProgram = 84,
    OP_glValidateProgram = 85,
    OP_glVertexAttrib1f = 86,
    OP_glVertexAttrib2f = 87,
    OP_glVertexAttrib3f = 88,
    OP_glVertexAttrib4f = 89,
    OP_glViewport = 90,
    OP_glBlitFramebuffer = 91,
    OP_glGenBuffers = 92,
    OP_glGenFramebuffers = 93,
    OP_glGenRenderbuffers = 94,
    OP_glGenTextures = 95,
    OP_glGenVertexArrays = 96,
    OP_glDeleteBuffers = 97,
    OP_glDeleteFramebuffers = 98,
    OP_glDeleteRenderbuffers = 99,
    OP_glDeleteTextures = 100,
    OP_glDeleteVertexArrays = 101,
    OP_glGetString = 102,
    OP_glGetIntegerv = 103,
    OP_glGetFloatv = 104,
    OP_glGetBooleanv = 105,
    OP_glGetShaderiv = 106,
    OP_glGetProgramiv = 107,
    OP_glGetShaderInfoLog = 108,
    OP_glGetProgramInfoLog = 109,
    OP_glShaderSource = 110,
    OP_glBindAttribLocation = 111,
    OP_glGetAttribLocation = 112,
    OP_glGetUniformLocation = 113,
    OP_glBufferData = 114,
    OP_glBufferSubData = 115,
    OP_glTexImage2D = 116,
    OP_glTexSubImage2D = 117,
    OP_glCompressedTexImage2D = 118,
    OP_glCompressedTexSubImage2D = 119,
    OP_glReadPixels = 120,
    OP_glVertexAttribPointer = 121,
    OP_glDrawArrays = 122,
    OP_glDrawElements = 123,
    OP_glUniform1fv = 124,
    OP_glUniform2fv = 125,
    OP_glUniform3fv = 126,
    OP_glUniform4fv = 127,
    OP_glUniform1iv = 128,
    OP_glUniform2iv = 129,
    OP_glUniform3iv = 130,
    OP_glUniform4iv = 131,
    OP_glUniformMatrix2fv = 132,
    OP_glUniformMatrix3fv = 133,
    OP_glUniformMatrix4fv = 134,
    OP_glGetBufferParameteriv = 135,
    OP_glMapBufferOES = 136,
    OP_glUnmapBufferOES = 137,
    OP_glGetBufferPointervOES = 138,
    OP_glMapBufferRange = 139,
    OP_glBindBuffer = 140,
    OP_glEnableVertexAttribArray = 141,
    OP_glDisableVertexAttribArray = 142,
    OP_glPixelStorei = 143,
    OP_glUploadAttrib = 144,
    OP_glTexParameterfv = 145,
    OP_glTexParameteriv = 146,
    OP_glVertexAttrib1fv = 147,
    OP_glVertexAttrib2fv = 148,
    OP_glVertexAttrib3fv = 149,
    OP_glVertexAttrib4fv = 150,
    OP_glGetActiveAttrib = 151,
    OP_glGetActiveUniform = 152,
    OP_glGetAttachedShaders = 153,
    OP_glGetFramebufferAttachmentParameteriv = 154,
    OP_glGetRenderbufferParameteriv = 155,
    OP_glGetShaderPrecisionFormat = 156,
    OP_glGetShaderSource = 157,
    OP_glGetTexParameterfv = 158,
    OP_glGetTexParameteriv = 159,
    OP_glGetUniformfv = 160,
    OP_glGetUniformiv = 161,
    OP_glGetVertexAttribfv = 162,
    OP_glGetVertexAttribiv = 163,
    OP_glBindVertexArray = 164,
    OP_glIsVertexArray = 165,
    OP_GLES_LAST
};

static inline int tspgl_needs_reply(uint32_t op)
{
    if (op < 16)
        return 1;
    switch (op) {
    case OP_glCheckFramebufferStatus:
    case OP_glCopyTexImage2D:
    case OP_glCopyTexSubImage2D:
    case OP_glGenerateMipmap:
    case OP_glBlitFramebuffer:
    case OP_glFramebufferTexture2D:
    case OP_glTexImage2D:
    case OP_glCompressedTexImage2D:
    case OP_glCreateProgram:
    case OP_glCreateShader:
    case OP_glFinish:
    case OP_glGetError:
    case OP_glIsBuffer:
    case OP_glIsEnabled:
    case OP_glIsFramebuffer:
    case OP_glIsProgram:
    case OP_glIsRenderbuffer:
    case OP_glIsShader:
    case OP_glIsTexture:
    case OP_glGenBuffers:
    case OP_glGenFramebuffers:
    case OP_glGenRenderbuffers:
    case OP_glGenTextures:
    case OP_glGenVertexArrays:
    case OP_glGetString:
    case OP_glGetIntegerv:
    case OP_glGetFloatv:
    case OP_glGetBooleanv:
    case OP_glGetShaderiv:
    case OP_glGetProgramiv:
    case OP_glGetShaderInfoLog:
    case OP_glGetProgramInfoLog:
    case OP_glGetAttribLocation:
    case OP_glGetUniformLocation:
    case OP_glReadPixels:
    case OP_glGetBufferParameteriv:
    case OP_glMapBufferOES:
    case OP_glUnmapBufferOES:
    case OP_glGetBufferPointervOES:
    case OP_glMapBufferRange:
    case OP_glGetActiveAttrib:
    case OP_glGetActiveUniform:
    case OP_glGetAttachedShaders:
    case OP_glGetFramebufferAttachmentParameteriv:
    case OP_glGetRenderbufferParameteriv:
    case OP_glGetShaderPrecisionFormat:
    case OP_glGetShaderSource:
    case OP_glGetTexParameterfv:
    case OP_glGetTexParameteriv:
    case OP_glGetUniformfv:
    case OP_glGetUniformiv:
    case OP_glGetVertexAttribfv:
    case OP_glGetVertexAttribiv:
    case OP_glIsVertexArray:
        return 1;
    default:
        return 0;
    }
}

#endif
