/* generated — do not edit */
#include "ops.h"
#include "xport.h"
#include <stdint.h>
#include <string.h>

void glActiveTexture(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glActiveTexture, u, sizeof(u), NULL, 0);
}

void glAttachShader(uint32_t a0, uint32_t a1)
{
    uint32_t u[2];
    u[0] = a0;
    u[1] = a1;
    struct tspgl_hdr rh;
    tspgl_call(OP_glAttachShader, u, sizeof(u), NULL, 0);
}

void glBindFramebuffer(uint32_t a0, uint32_t a1)
{
    uint32_t u[2];
    u[0] = a0;
    u[1] = a1;
    struct tspgl_hdr rh;
    tspgl_call(OP_glBindFramebuffer, u, sizeof(u), NULL, 0);
}

void glBindRenderbuffer(uint32_t a0, uint32_t a1)
{
    uint32_t u[2];
    u[0] = a0;
    u[1] = a1;
    struct tspgl_hdr rh;
    tspgl_call(OP_glBindRenderbuffer, u, sizeof(u), NULL, 0);
}

void glBindTexture(uint32_t a0, uint32_t a1)
{
    uint32_t u[2];
    u[0] = a0;
    u[1] = a1;
    struct tspgl_hdr rh;
    tspgl_call(OP_glBindTexture, u, sizeof(u), NULL, 0);
}

void glBlendColor(float a0, float a1, float a2, float a3)
{
    uint32_t u[4];
    u[0] = tspgl_pack_f32(a0);
    u[1] = tspgl_pack_f32(a1);
    u[2] = tspgl_pack_f32(a2);
    u[3] = tspgl_pack_f32(a3);
    struct tspgl_hdr rh;
    tspgl_call(OP_glBlendColor, u, sizeof(u), NULL, 0);
}

void glBlendEquation(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glBlendEquation, u, sizeof(u), NULL, 0);
}

void glBlendEquationSeparate(uint32_t a0, uint32_t a1)
{
    uint32_t u[2];
    u[0] = a0;
    u[1] = a1;
    struct tspgl_hdr rh;
    tspgl_call(OP_glBlendEquationSeparate, u, sizeof(u), NULL, 0);
}

void glBlendFunc(uint32_t a0, uint32_t a1)
{
    uint32_t u[2];
    u[0] = a0;
    u[1] = a1;
    struct tspgl_hdr rh;
    tspgl_call(OP_glBlendFunc, u, sizeof(u), NULL, 0);
}

void glBlendFuncSeparate(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    uint32_t u[4];
    u[0] = a0;
    u[1] = a1;
    u[2] = a2;
    u[3] = a3;
    struct tspgl_hdr rh;
    tspgl_call(OP_glBlendFuncSeparate, u, sizeof(u), NULL, 0);
}

uint32_t glCheckFramebufferStatus(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    uint32_t out = 0;
    tspgl_call(OP_glCheckFramebufferStatus, u, sizeof(u), &out, sizeof(out));
    return (uint32_t)out;
}

void glClear(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glClear, u, sizeof(u), NULL, 0);
}

void glClearColor(float a0, float a1, float a2, float a3)
{
    uint32_t u[4];
    u[0] = tspgl_pack_f32(a0);
    u[1] = tspgl_pack_f32(a1);
    u[2] = tspgl_pack_f32(a2);
    u[3] = tspgl_pack_f32(a3);
    struct tspgl_hdr rh;
    tspgl_call(OP_glClearColor, u, sizeof(u), NULL, 0);
}

void glClearDepthf(float a0)
{
    uint32_t u[1];
    u[0] = tspgl_pack_f32(a0);
    struct tspgl_hdr rh;
    tspgl_call(OP_glClearDepthf, u, sizeof(u), NULL, 0);
}

void glClearStencil(int32_t a0)
{
    uint32_t u[1];
    u[0] = (uint32_t)a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glClearStencil, u, sizeof(u), NULL, 0);
}

void glColorMask(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    uint32_t u[4];
    u[0] = a0;
    u[1] = a1;
    u[2] = a2;
    u[3] = a3;
    struct tspgl_hdr rh;
    tspgl_call(OP_glColorMask, u, sizeof(u), NULL, 0);
}

void glCompileShader(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glCompileShader, u, sizeof(u), NULL, 0);
}

void glCopyTexImage2D(uint32_t a0, int32_t a1, uint32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7)
{
    uint32_t u[8];
    u[0] = a0;
    u[1] = (uint32_t)a1;
    u[2] = a2;
    u[3] = (uint32_t)a3;
    u[4] = (uint32_t)a4;
    u[5] = (uint32_t)a5;
    u[6] = (uint32_t)a6;
    u[7] = (uint32_t)a7;
    struct tspgl_hdr rh;
    tspgl_call(OP_glCopyTexImage2D, u, sizeof(u), NULL, 0);
}

void glCopyTexSubImage2D(uint32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7)
{
    uint32_t u[8];
    u[0] = a0;
    u[1] = (uint32_t)a1;
    u[2] = (uint32_t)a2;
    u[3] = (uint32_t)a3;
    u[4] = (uint32_t)a4;
    u[5] = (uint32_t)a5;
    u[6] = (uint32_t)a6;
    u[7] = (uint32_t)a7;
    struct tspgl_hdr rh;
    tspgl_call(OP_glCopyTexSubImage2D, u, sizeof(u), NULL, 0);
}

uint32_t glCreateProgram(void)
{
    uint32_t out = 0;
    tspgl_call(OP_glCreateProgram, NULL, 0, &out, sizeof(out));
    return (uint32_t)out;
}

uint32_t glCreateShader(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    uint32_t out = 0;
    tspgl_call(OP_glCreateShader, u, sizeof(u), &out, sizeof(out));
    return (uint32_t)out;
}

void glCullFace(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glCullFace, u, sizeof(u), NULL, 0);
}

void glDeleteProgram(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glDeleteProgram, u, sizeof(u), NULL, 0);
}

void glDeleteShader(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glDeleteShader, u, sizeof(u), NULL, 0);
}

void glDepthFunc(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glDepthFunc, u, sizeof(u), NULL, 0);
}

void glDepthMask(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glDepthMask, u, sizeof(u), NULL, 0);
}

void glDepthRangef(float a0, float a1)
{
    uint32_t u[2];
    u[0] = tspgl_pack_f32(a0);
    u[1] = tspgl_pack_f32(a1);
    struct tspgl_hdr rh;
    tspgl_call(OP_glDepthRangef, u, sizeof(u), NULL, 0);
}

void glDetachShader(uint32_t a0, uint32_t a1)
{
    uint32_t u[2];
    u[0] = a0;
    u[1] = a1;
    struct tspgl_hdr rh;
    tspgl_call(OP_glDetachShader, u, sizeof(u), NULL, 0);
}

void glDisable(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glDisable, u, sizeof(u), NULL, 0);
}

void glEnable(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glEnable, u, sizeof(u), NULL, 0);
}

void glFinish(void)
{
    tspgl_call(OP_glFinish, NULL, 0, NULL, 0);
}

void glFlush(void)
{
    tspgl_call(OP_glFlush, NULL, 0, NULL, 0);
}

void glFramebufferRenderbuffer(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    uint32_t u[4];
    u[0] = a0;
    u[1] = a1;
    u[2] = a2;
    u[3] = a3;
    struct tspgl_hdr rh;
    tspgl_call(OP_glFramebufferRenderbuffer, u, sizeof(u), NULL, 0);
}

void glFramebufferTexture2D(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, int32_t a4)
{
    uint32_t u[5];
    u[0] = a0;
    u[1] = a1;
    u[2] = a2;
    u[3] = a3;
    u[4] = (uint32_t)a4;
    struct tspgl_hdr rh;
    tspgl_call(OP_glFramebufferTexture2D, u, sizeof(u), NULL, 0);
}

void glFrontFace(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glFrontFace, u, sizeof(u), NULL, 0);
}

void glGenerateMipmap(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glGenerateMipmap, u, sizeof(u), NULL, 0);
}

uint32_t glGetError(void)
{
    uint32_t out = 0;
    tspgl_call(OP_glGetError, NULL, 0, &out, sizeof(out));
    return (uint32_t)out;
}

void glHint(uint32_t a0, uint32_t a1)
{
    uint32_t u[2];
    u[0] = a0;
    u[1] = a1;
    struct tspgl_hdr rh;
    tspgl_call(OP_glHint, u, sizeof(u), NULL, 0);
}

uint32_t glIsBuffer(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    uint32_t out = 0;
    tspgl_call(OP_glIsBuffer, u, sizeof(u), &out, sizeof(out));
    return (uint32_t)out;
}

uint32_t glIsEnabled(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    uint32_t out = 0;
    tspgl_call(OP_glIsEnabled, u, sizeof(u), &out, sizeof(out));
    return (uint32_t)out;
}

uint32_t glIsFramebuffer(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    uint32_t out = 0;
    tspgl_call(OP_glIsFramebuffer, u, sizeof(u), &out, sizeof(out));
    return (uint32_t)out;
}

uint32_t glIsProgram(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    uint32_t out = 0;
    tspgl_call(OP_glIsProgram, u, sizeof(u), &out, sizeof(out));
    return (uint32_t)out;
}

uint32_t glIsRenderbuffer(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    uint32_t out = 0;
    tspgl_call(OP_glIsRenderbuffer, u, sizeof(u), &out, sizeof(out));
    return (uint32_t)out;
}

uint32_t glIsShader(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    uint32_t out = 0;
    tspgl_call(OP_glIsShader, u, sizeof(u), &out, sizeof(out));
    return (uint32_t)out;
}

uint32_t glIsTexture(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    uint32_t out = 0;
    tspgl_call(OP_glIsTexture, u, sizeof(u), &out, sizeof(out));
    return (uint32_t)out;
}

void glLineWidth(float a0)
{
    uint32_t u[1];
    u[0] = tspgl_pack_f32(a0);
    struct tspgl_hdr rh;
    tspgl_call(OP_glLineWidth, u, sizeof(u), NULL, 0);
}

void glLinkProgram(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glLinkProgram, u, sizeof(u), NULL, 0);
}

void glPolygonOffset(float a0, float a1)
{
    uint32_t u[2];
    u[0] = tspgl_pack_f32(a0);
    u[1] = tspgl_pack_f32(a1);
    struct tspgl_hdr rh;
    tspgl_call(OP_glPolygonOffset, u, sizeof(u), NULL, 0);
}

void glReleaseShaderCompiler(void)
{
    tspgl_call(OP_glReleaseShaderCompiler, NULL, 0, NULL, 0);
}

void glRenderbufferStorage(uint32_t a0, uint32_t a1, int32_t a2, int32_t a3)
{
    uint32_t u[4];
    u[0] = a0;
    u[1] = a1;
    u[2] = (uint32_t)a2;
    u[3] = (uint32_t)a3;
    struct tspgl_hdr rh;
    tspgl_call(OP_glRenderbufferStorage, u, sizeof(u), NULL, 0);
}

void glSampleCoverage(float a0, uint32_t a1)
{
    uint32_t u[2];
    u[0] = tspgl_pack_f32(a0);
    u[1] = a1;
    struct tspgl_hdr rh;
    tspgl_call(OP_glSampleCoverage, u, sizeof(u), NULL, 0);
}

void glScissor(int32_t a0, int32_t a1, int32_t a2, int32_t a3)
{
    uint32_t u[4];
    u[0] = (uint32_t)a0;
    u[1] = (uint32_t)a1;
    u[2] = (uint32_t)a2;
    u[3] = (uint32_t)a3;
    struct tspgl_hdr rh;
    tspgl_call(OP_glScissor, u, sizeof(u), NULL, 0);
}

void glStencilFunc(uint32_t a0, int32_t a1, uint32_t a2)
{
    uint32_t u[3];
    u[0] = a0;
    u[1] = (uint32_t)a1;
    u[2] = a2;
    struct tspgl_hdr rh;
    tspgl_call(OP_glStencilFunc, u, sizeof(u), NULL, 0);
}

void glStencilFuncSeparate(uint32_t a0, uint32_t a1, int32_t a2, uint32_t a3)
{
    uint32_t u[4];
    u[0] = a0;
    u[1] = a1;
    u[2] = (uint32_t)a2;
    u[3] = a3;
    struct tspgl_hdr rh;
    tspgl_call(OP_glStencilFuncSeparate, u, sizeof(u), NULL, 0);
}

void glStencilMask(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glStencilMask, u, sizeof(u), NULL, 0);
}

void glStencilMaskSeparate(uint32_t a0, uint32_t a1)
{
    uint32_t u[2];
    u[0] = a0;
    u[1] = a1;
    struct tspgl_hdr rh;
    tspgl_call(OP_glStencilMaskSeparate, u, sizeof(u), NULL, 0);
}

void glStencilOp(uint32_t a0, uint32_t a1, uint32_t a2)
{
    uint32_t u[3];
    u[0] = a0;
    u[1] = a1;
    u[2] = a2;
    struct tspgl_hdr rh;
    tspgl_call(OP_glStencilOp, u, sizeof(u), NULL, 0);
}

void glStencilOpSeparate(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3)
{
    uint32_t u[4];
    u[0] = a0;
    u[1] = a1;
    u[2] = a2;
    u[3] = a3;
    struct tspgl_hdr rh;
    tspgl_call(OP_glStencilOpSeparate, u, sizeof(u), NULL, 0);
}

void glTexParameterf(uint32_t a0, uint32_t a1, float a2)
{
    uint32_t u[3];
    u[0] = a0;
    u[1] = a1;
    u[2] = tspgl_pack_f32(a2);
    struct tspgl_hdr rh;
    tspgl_call(OP_glTexParameterf, u, sizeof(u), NULL, 0);
}

void glTexParameteri(uint32_t a0, uint32_t a1, int32_t a2)
{
    uint32_t u[3];
    u[0] = a0;
    u[1] = a1;
    u[2] = (uint32_t)a2;
    struct tspgl_hdr rh;
    tspgl_call(OP_glTexParameteri, u, sizeof(u), NULL, 0);
}

void glUniform1f(int32_t a0, float a1)
{
    uint32_t u[2];
    u[0] = (uint32_t)a0;
    u[1] = tspgl_pack_f32(a1);
    struct tspgl_hdr rh;
    tspgl_call(OP_glUniform1f, u, sizeof(u), NULL, 0);
}

void glUniform2f(int32_t a0, float a1, float a2)
{
    uint32_t u[3];
    u[0] = (uint32_t)a0;
    u[1] = tspgl_pack_f32(a1);
    u[2] = tspgl_pack_f32(a2);
    struct tspgl_hdr rh;
    tspgl_call(OP_glUniform2f, u, sizeof(u), NULL, 0);
}

void glUniform3f(int32_t a0, float a1, float a2, float a3)
{
    uint32_t u[4];
    u[0] = (uint32_t)a0;
    u[1] = tspgl_pack_f32(a1);
    u[2] = tspgl_pack_f32(a2);
    u[3] = tspgl_pack_f32(a3);
    struct tspgl_hdr rh;
    tspgl_call(OP_glUniform3f, u, sizeof(u), NULL, 0);
}

void glUniform4f(int32_t a0, float a1, float a2, float a3, float a4)
{
    uint32_t u[5];
    u[0] = (uint32_t)a0;
    u[1] = tspgl_pack_f32(a1);
    u[2] = tspgl_pack_f32(a2);
    u[3] = tspgl_pack_f32(a3);
    u[4] = tspgl_pack_f32(a4);
    struct tspgl_hdr rh;
    tspgl_call(OP_glUniform4f, u, sizeof(u), NULL, 0);
}

void glUniform1i(int32_t a0, int32_t a1)
{
    uint32_t u[2];
    u[0] = (uint32_t)a0;
    u[1] = (uint32_t)a1;
    struct tspgl_hdr rh;
    tspgl_call(OP_glUniform1i, u, sizeof(u), NULL, 0);
}

void glUniform2i(int32_t a0, int32_t a1, int32_t a2)
{
    uint32_t u[3];
    u[0] = (uint32_t)a0;
    u[1] = (uint32_t)a1;
    u[2] = (uint32_t)a2;
    struct tspgl_hdr rh;
    tspgl_call(OP_glUniform2i, u, sizeof(u), NULL, 0);
}

void glUniform3i(int32_t a0, int32_t a1, int32_t a2, int32_t a3)
{
    uint32_t u[4];
    u[0] = (uint32_t)a0;
    u[1] = (uint32_t)a1;
    u[2] = (uint32_t)a2;
    u[3] = (uint32_t)a3;
    struct tspgl_hdr rh;
    tspgl_call(OP_glUniform3i, u, sizeof(u), NULL, 0);
}

void glUniform4i(int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4)
{
    uint32_t u[5];
    u[0] = (uint32_t)a0;
    u[1] = (uint32_t)a1;
    u[2] = (uint32_t)a2;
    u[3] = (uint32_t)a3;
    u[4] = (uint32_t)a4;
    struct tspgl_hdr rh;
    tspgl_call(OP_glUniform4i, u, sizeof(u), NULL, 0);
}

void glUseProgram(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glUseProgram, u, sizeof(u), NULL, 0);
}

void glValidateProgram(uint32_t a0)
{
    uint32_t u[1];
    u[0] = a0;
    struct tspgl_hdr rh;
    tspgl_call(OP_glValidateProgram, u, sizeof(u), NULL, 0);
}

void glVertexAttrib1f(uint32_t a0, float a1)
{
    uint32_t u[2];
    u[0] = a0;
    u[1] = tspgl_pack_f32(a1);
    struct tspgl_hdr rh;
    tspgl_call(OP_glVertexAttrib1f, u, sizeof(u), NULL, 0);
}

void glVertexAttrib2f(uint32_t a0, float a1, float a2)
{
    uint32_t u[3];
    u[0] = a0;
    u[1] = tspgl_pack_f32(a1);
    u[2] = tspgl_pack_f32(a2);
    struct tspgl_hdr rh;
    tspgl_call(OP_glVertexAttrib2f, u, sizeof(u), NULL, 0);
}

void glVertexAttrib3f(uint32_t a0, float a1, float a2, float a3)
{
    uint32_t u[4];
    u[0] = a0;
    u[1] = tspgl_pack_f32(a1);
    u[2] = tspgl_pack_f32(a2);
    u[3] = tspgl_pack_f32(a3);
    struct tspgl_hdr rh;
    tspgl_call(OP_glVertexAttrib3f, u, sizeof(u), NULL, 0);
}

void glVertexAttrib4f(uint32_t a0, float a1, float a2, float a3, float a4)
{
    uint32_t u[5];
    u[0] = a0;
    u[1] = tspgl_pack_f32(a1);
    u[2] = tspgl_pack_f32(a2);
    u[3] = tspgl_pack_f32(a3);
    u[4] = tspgl_pack_f32(a4);
    struct tspgl_hdr rh;
    tspgl_call(OP_glVertexAttrib4f, u, sizeof(u), NULL, 0);
}

void glViewport(int32_t a0, int32_t a1, int32_t a2, int32_t a3)
{
    uint32_t u[4];
    u[0] = (uint32_t)a0;
    u[1] = (uint32_t)a1;
    u[2] = (uint32_t)a2;
    u[3] = (uint32_t)a3;
    struct tspgl_hdr rh;
    tspgl_call(OP_glViewport, u, sizeof(u), NULL, 0);
}

void glBlitFramebuffer(int32_t a0, int32_t a1, int32_t a2, int32_t a3, int32_t a4, int32_t a5, int32_t a6, int32_t a7, uint32_t a8, uint32_t a9)
{
    uint32_t u[10];
    u[0] = (uint32_t)a0;
    u[1] = (uint32_t)a1;
    u[2] = (uint32_t)a2;
    u[3] = (uint32_t)a3;
    u[4] = (uint32_t)a4;
    u[5] = (uint32_t)a5;
    u[6] = (uint32_t)a6;
    u[7] = (uint32_t)a7;
    u[8] = a8;
    u[9] = a9;
    struct tspgl_hdr rh;
    tspgl_call(OP_glBlitFramebuffer, u, sizeof(u), NULL, 0);
}

void tspgl_raw_glGenBuffers(int32_t n, uint32_t *ids)
{
    uint32_t buf[1 + 256];
    uint32_t count;
    if (n <= 0) return;
    count = (uint32_t)n;
    if (count > 256) count = 256;
    buf[0] = count;
    tspgl_call(OP_glGenBuffers, buf, 4u, buf + 1, count * 4u);
    if (ids) memcpy(ids, buf + 1, count * 4u);
}

void tspgl_raw_glGenFramebuffers(int32_t n, uint32_t *ids)
{
    uint32_t buf[1 + 256];
    uint32_t count;
    if (n <= 0) return;
    count = (uint32_t)n;
    if (count > 256) count = 256;
    buf[0] = count;
    tspgl_call(OP_glGenFramebuffers, buf, 4u, buf + 1, count * 4u);
    if (ids) memcpy(ids, buf + 1, count * 4u);
}

void tspgl_raw_glGenRenderbuffers(int32_t n, uint32_t *ids)
{
    uint32_t buf[1 + 256];
    uint32_t count;
    if (n <= 0) return;
    count = (uint32_t)n;
    if (count > 256) count = 256;
    buf[0] = count;
    tspgl_call(OP_glGenRenderbuffers, buf, 4u, buf + 1, count * 4u);
    if (ids) memcpy(ids, buf + 1, count * 4u);
}

void tspgl_raw_glGenTextures(int32_t n, uint32_t *ids)
{
    uint32_t buf[1 + 256];
    uint32_t count;
    if (n <= 0) return;
    count = (uint32_t)n;
    if (count > 256) count = 256;
    buf[0] = count;
    tspgl_call(OP_glGenTextures, buf, 4u, buf + 1, count * 4u);
    if (ids) memcpy(ids, buf + 1, count * 4u);
}

void tspgl_raw_glGenVertexArrays(int32_t n, uint32_t *ids)
{
    uint32_t buf[1 + 256];
    uint32_t count;
    if (n <= 0) return;
    count = (uint32_t)n;
    if (count > 256) count = 256;
    buf[0] = count;
    tspgl_call(OP_glGenVertexArrays, buf, 4u, buf + 1, count * 4u);
    if (ids) memcpy(ids, buf + 1, count * 4u);
}

void tspgl_raw_glDeleteBuffers(int32_t n, const uint32_t *ids)
{
    uint32_t buf[1 + 256];
    uint32_t count;
    if (n <= 0) return;
    count = (uint32_t)n;
    if (count > 256) count = 256;
    buf[0] = count;
    if (ids) memcpy(buf + 1, ids, count * 4u);
    else memset(buf + 1, 0, count * 4u);
    tspgl_call(OP_glDeleteBuffers, buf, (1 + count) * 4u, NULL, 0);
}

void tspgl_raw_glDeleteFramebuffers(int32_t n, const uint32_t *ids)
{
    uint32_t buf[1 + 256];
    uint32_t count;
    if (n <= 0) return;
    count = (uint32_t)n;
    if (count > 256) count = 256;
    buf[0] = count;
    if (ids) memcpy(buf + 1, ids, count * 4u);
    else memset(buf + 1, 0, count * 4u);
    tspgl_call(OP_glDeleteFramebuffers, buf, (1 + count) * 4u, NULL, 0);
}

void tspgl_raw_glDeleteRenderbuffers(int32_t n, const uint32_t *ids)
{
    uint32_t buf[1 + 256];
    uint32_t count;
    if (n <= 0) return;
    count = (uint32_t)n;
    if (count > 256) count = 256;
    buf[0] = count;
    if (ids) memcpy(buf + 1, ids, count * 4u);
    else memset(buf + 1, 0, count * 4u);
    tspgl_call(OP_glDeleteRenderbuffers, buf, (1 + count) * 4u, NULL, 0);
}

void tspgl_raw_glDeleteTextures(int32_t n, const uint32_t *ids)
{
    uint32_t buf[1 + 256];
    uint32_t count;
    if (n <= 0) return;
    count = (uint32_t)n;
    if (count > 256) count = 256;
    buf[0] = count;
    if (ids) memcpy(buf + 1, ids, count * 4u);
    else memset(buf + 1, 0, count * 4u);
    tspgl_call(OP_glDeleteTextures, buf, (1 + count) * 4u, NULL, 0);
}

void tspgl_raw_glDeleteVertexArrays(int32_t n, const uint32_t *ids)
{
    uint32_t buf[1 + 256];
    uint32_t count;
    if (n <= 0) return;
    count = (uint32_t)n;
    if (count > 256) count = 256;
    buf[0] = count;
    if (ids) memcpy(buf + 1, ids, count * 4u);
    else memset(buf + 1, 0, count * 4u);
    tspgl_call(OP_glDeleteVertexArrays, buf, (1 + count) * 4u, NULL, 0);
}
