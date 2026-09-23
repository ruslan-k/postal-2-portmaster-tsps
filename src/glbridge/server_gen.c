/* generated — do not edit */
#include "ops.h"

static void tspgl_load_simple(void *(*get)(const char *))
{
    G.glActiveTexture = get("glActiveTexture");
    G.glAttachShader = get("glAttachShader");
    G.glBindFramebuffer = get("glBindFramebuffer");
    G.glBindRenderbuffer = get("glBindRenderbuffer");
    G.glBindTexture = get("glBindTexture");
    G.glBlendColor = get("glBlendColor");
    G.glBlendEquation = get("glBlendEquation");
    G.glBlendEquationSeparate = get("glBlendEquationSeparate");
    G.glBlendFunc = get("glBlendFunc");
    G.glBlendFuncSeparate = get("glBlendFuncSeparate");
    G.glCheckFramebufferStatus = get("glCheckFramebufferStatus");
    G.glClear = get("glClear");
    G.glClearColor = get("glClearColor");
    G.glClearDepthf = get("glClearDepthf");
    G.glClearStencil = get("glClearStencil");
    G.glColorMask = get("glColorMask");
    G.glCompileShader = get("glCompileShader");
    G.glCopyTexImage2D = get("glCopyTexImage2D");
    G.glCopyTexSubImage2D = get("glCopyTexSubImage2D");
    G.glCreateProgram = get("glCreateProgram");
    G.glCreateShader = get("glCreateShader");
    G.glCullFace = get("glCullFace");
    G.glDeleteProgram = get("glDeleteProgram");
    G.glDeleteShader = get("glDeleteShader");
    G.glDepthFunc = get("glDepthFunc");
    G.glDepthMask = get("glDepthMask");
    G.glDepthRangef = get("glDepthRangef");
    G.glDetachShader = get("glDetachShader");
    G.glDisable = get("glDisable");
    G.glEnable = get("glEnable");
    G.glFinish = get("glFinish");
    G.glFlush = get("glFlush");
    G.glFramebufferRenderbuffer = get("glFramebufferRenderbuffer");
    G.glFramebufferTexture2D = get("glFramebufferTexture2D");
    G.glFrontFace = get("glFrontFace");
    G.glGenerateMipmap = get("glGenerateMipmap");
    G.glGetError = get("glGetError");
    G.glHint = get("glHint");
    G.glIsBuffer = get("glIsBuffer");
    G.glIsEnabled = get("glIsEnabled");
    G.glIsFramebuffer = get("glIsFramebuffer");
    G.glIsProgram = get("glIsProgram");
    G.glIsRenderbuffer = get("glIsRenderbuffer");
    G.glIsShader = get("glIsShader");
    G.glIsTexture = get("glIsTexture");
    G.glLineWidth = get("glLineWidth");
    G.glLinkProgram = get("glLinkProgram");
    G.glPolygonOffset = get("glPolygonOffset");
    G.glReleaseShaderCompiler = get("glReleaseShaderCompiler");
    G.glRenderbufferStorage = get("glRenderbufferStorage");
    G.glSampleCoverage = get("glSampleCoverage");
    G.glScissor = get("glScissor");
    G.glStencilFunc = get("glStencilFunc");
    G.glStencilFuncSeparate = get("glStencilFuncSeparate");
    G.glStencilMask = get("glStencilMask");
    G.glStencilMaskSeparate = get("glStencilMaskSeparate");
    G.glStencilOp = get("glStencilOp");
    G.glStencilOpSeparate = get("glStencilOpSeparate");
    G.glTexParameterf = get("glTexParameterf");
    G.glTexParameteri = get("glTexParameteri");
    G.glUniform1f = get("glUniform1f");
    G.glUniform2f = get("glUniform2f");
    G.glUniform3f = get("glUniform3f");
    G.glUniform4f = get("glUniform4f");
    G.glUniform1i = get("glUniform1i");
    G.glUniform2i = get("glUniform2i");
    G.glUniform3i = get("glUniform3i");
    G.glUniform4i = get("glUniform4i");
    G.glUseProgram = get("glUseProgram");
    G.glValidateProgram = get("glValidateProgram");
    G.glVertexAttrib1f = get("glVertexAttrib1f");
    G.glVertexAttrib2f = get("glVertexAttrib2f");
    G.glVertexAttrib3f = get("glVertexAttrib3f");
    G.glVertexAttrib4f = get("glVertexAttrib4f");
    G.glViewport = get("glViewport");
    G.glBlitFramebuffer = get("glBlitFramebuffer");
    G.glGenBuffers = get("glGenBuffers");
    G.glGenFramebuffers = get("glGenFramebuffers");
    G.glGenRenderbuffers = get("glGenRenderbuffers");
    G.glGenTextures = get("glGenTextures");
    G.glGenVertexArrays = get("glGenVertexArrays");
    G.glDeleteBuffers = get("glDeleteBuffers");
    G.glDeleteFramebuffers = get("glDeleteFramebuffers");
    G.glDeleteRenderbuffers = get("glDeleteRenderbuffers");
    G.glDeleteTextures = get("glDeleteTextures");
    G.glDeleteVertexArrays = get("glDeleteVertexArrays");
}

static void tspgl_load_specials(void *(*get)(const char *))
{
    G.glGetString = get("glGetString");
    G.glGetIntegerv = get("glGetIntegerv");
    G.glGetFloatv = get("glGetFloatv");
    G.glGetBooleanv = get("glGetBooleanv");
    G.glGetShaderiv = get("glGetShaderiv");
    G.glGetProgramiv = get("glGetProgramiv");
    G.glGetShaderInfoLog = get("glGetShaderInfoLog");
    G.glGetProgramInfoLog = get("glGetProgramInfoLog");
    G.glShaderSource = get("glShaderSource");
    G.glBindAttribLocation = get("glBindAttribLocation");
    G.glGetAttribLocation = get("glGetAttribLocation");
    G.glGetUniformLocation = get("glGetUniformLocation");
    G.glBufferData = get("glBufferData");
    G.glBufferSubData = get("glBufferSubData");
    G.glTexImage2D = get("glTexImage2D");
    G.glTexSubImage2D = get("glTexSubImage2D");
    G.glCompressedTexImage2D = get("glCompressedTexImage2D");
    G.glCompressedTexSubImage2D = get("glCompressedTexSubImage2D");
    G.glReadPixels = get("glReadPixels");
    G.glVertexAttribPointer = get("glVertexAttribPointer");
    G.glDrawArrays = get("glDrawArrays");
    G.glDrawElements = get("glDrawElements");
    G.glUniform1fv = get("glUniform1fv");
    G.glUniform2fv = get("glUniform2fv");
    G.glUniform3fv = get("glUniform3fv");
    G.glUniform4fv = get("glUniform4fv");
    G.glUniform1iv = get("glUniform1iv");
    G.glUniform2iv = get("glUniform2iv");
    G.glUniform3iv = get("glUniform3iv");
    G.glUniform4iv = get("glUniform4iv");
    G.glUniformMatrix2fv = get("glUniformMatrix2fv");
    G.glUniformMatrix3fv = get("glUniformMatrix3fv");
    G.glUniformMatrix4fv = get("glUniformMatrix4fv");
    G.glGetBufferParameteriv = get("glGetBufferParameteriv");
    G.glMapBufferOES = get("glMapBufferOES");
    G.glUnmapBufferOES = get("glUnmapBufferOES");
    G.glGetBufferPointervOES = get("glGetBufferPointervOES");
    G.glMapBufferRange = get("glMapBufferRange");
    G.glBindBuffer = get("glBindBuffer");
    G.glEnableVertexAttribArray = get("glEnableVertexAttribArray");
    G.glDisableVertexAttribArray = get("glDisableVertexAttribArray");
    G.glPixelStorei = get("glPixelStorei");
    G.glUploadAttrib = get("glUploadAttrib");
    G.glTexParameterfv = get("glTexParameterfv");
    G.glTexParameteriv = get("glTexParameteriv");
    G.glVertexAttrib1fv = get("glVertexAttrib1fv");
    G.glVertexAttrib2fv = get("glVertexAttrib2fv");
    G.glVertexAttrib3fv = get("glVertexAttrib3fv");
    G.glVertexAttrib4fv = get("glVertexAttrib4fv");
    G.glGetActiveAttrib = get("glGetActiveAttrib");
    G.glGetActiveUniform = get("glGetActiveUniform");
    G.glGetAttachedShaders = get("glGetAttachedShaders");
    G.glGetFramebufferAttachmentParameteriv = get("glGetFramebufferAttachmentParameteriv");
    G.glGetRenderbufferParameteriv = get("glGetRenderbufferParameteriv");
    G.glGetShaderPrecisionFormat = get("glGetShaderPrecisionFormat");
    G.glGetShaderSource = get("glGetShaderSource");
    G.glGetTexParameterfv = get("glGetTexParameterfv");
    G.glGetTexParameteriv = get("glGetTexParameteriv");
    G.glGetUniformfv = get("glGetUniformfv");
    G.glGetUniformiv = get("glGetUniformiv");
    G.glGetVertexAttribfv = get("glGetVertexAttribfv");
    G.glGetVertexAttribiv = get("glGetVertexAttribiv");
    G.glBindVertexArray = get("glBindVertexArray");
    G.glIsVertexArray = get("glIsVertexArray");
}

static int tspgl_dispatch_simple(uint32_t op, const uint32_t *u, uint32_t nbytes,
                                 uint32_t *out, uint32_t *out_n)
{
    (void)nbytes;
    switch (op) {
    case OP_glActiveTexture:
        if (G.glActiveTexture) G.glActiveTexture(u[0]);
        *out_n = 0;
        return 0;
    case OP_glAttachShader:
        if (G.glAttachShader) G.glAttachShader(u[0], u[1]);
        *out_n = 0;
        return 0;
    case OP_glBindFramebuffer:
        if (G.glBindFramebuffer) G.glBindFramebuffer(u[0], u[1]);
        *out_n = 0;
        return 0;
    case OP_glBindRenderbuffer:
        if (G.glBindRenderbuffer) G.glBindRenderbuffer(u[0], u[1]);
        *out_n = 0;
        return 0;
    case OP_glBindTexture:
        if (G.glBindTexture) G.glBindTexture(u[0], u[1]);
        *out_n = 0;
        return 0;
    case OP_glBlendColor:
        if (G.glBlendColor) G.glBlendColor(tspgl_unpack_f32(u[0]), tspgl_unpack_f32(u[1]), tspgl_unpack_f32(u[2]), tspgl_unpack_f32(u[3]));
        *out_n = 0;
        return 0;
    case OP_glBlendEquation:
        if (G.glBlendEquation) G.glBlendEquation(u[0]);
        *out_n = 0;
        return 0;
    case OP_glBlendEquationSeparate:
        if (G.glBlendEquationSeparate) G.glBlendEquationSeparate(u[0], u[1]);
        *out_n = 0;
        return 0;
    case OP_glBlendFunc:
        if (G.glBlendFunc) G.glBlendFunc(u[0], u[1]);
        *out_n = 0;
        return 0;
    case OP_glBlendFuncSeparate:
        if (G.glBlendFuncSeparate) G.glBlendFuncSeparate(u[0], u[1], u[2], u[3]);
        *out_n = 0;
        return 0;
    case OP_glCheckFramebufferStatus:
        out[0] = G.glCheckFramebufferStatus ? (uint32_t)G.glCheckFramebufferStatus(u[0]) : 0;
        *out_n = 4;
        return 0;
    case OP_glClear:
        if (G.glClear) G.glClear(u[0]);
        *out_n = 0;
        return 0;
    case OP_glClearColor:
        if (G.glClearColor) G.glClearColor(tspgl_unpack_f32(u[0]), tspgl_unpack_f32(u[1]), tspgl_unpack_f32(u[2]), tspgl_unpack_f32(u[3]));
        *out_n = 0;
        return 0;
    case OP_glClearDepthf:
        if (G.glClearDepthf) G.glClearDepthf(tspgl_unpack_f32(u[0]));
        *out_n = 0;
        return 0;
    case OP_glClearStencil:
        if (G.glClearStencil) G.glClearStencil((int32_t)u[0]);
        *out_n = 0;
        return 0;
    case OP_glColorMask:
        if (G.glColorMask) G.glColorMask(u[0], u[1], u[2], u[3]);
        *out_n = 0;
        return 0;
    case OP_glCompileShader:
        if (G.glCompileShader) G.glCompileShader(u[0]);
        *out_n = 0;
        return 0;
    case OP_glCopyTexImage2D:
        if (G.glCopyTexImage2D) G.glCopyTexImage2D(u[0], (int32_t)u[1], u[2], (int32_t)u[3], (int32_t)u[4], (int32_t)u[5], (int32_t)u[6], (int32_t)u[7]);
        *out_n = 0;
        return 0;
    case OP_glCopyTexSubImage2D:
        if (G.glCopyTexSubImage2D) G.glCopyTexSubImage2D(u[0], (int32_t)u[1], (int32_t)u[2], (int32_t)u[3], (int32_t)u[4], (int32_t)u[5], (int32_t)u[6], (int32_t)u[7]);
        *out_n = 0;
        return 0;
    case OP_glCreateProgram:
        out[0] = G.glCreateProgram ? (uint32_t)G.glCreateProgram() : 0;
        *out_n = 4;
        return 0;
    case OP_glCreateShader:
        out[0] = G.glCreateShader ? (uint32_t)G.glCreateShader(u[0]) : 0;
        *out_n = 4;
        return 0;
    case OP_glCullFace:
        if (G.glCullFace) G.glCullFace(u[0]);
        *out_n = 0;
        return 0;
    case OP_glDeleteProgram:
        if (G.glDeleteProgram) G.glDeleteProgram(u[0]);
        *out_n = 0;
        return 0;
    case OP_glDeleteShader:
        if (G.glDeleteShader) G.glDeleteShader(u[0]);
        *out_n = 0;
        return 0;
    case OP_glDepthFunc:
        if (G.glDepthFunc) G.glDepthFunc(u[0]);
        *out_n = 0;
        return 0;
    case OP_glDepthMask:
        if (G.glDepthMask) G.glDepthMask(u[0]);
        *out_n = 0;
        return 0;
    case OP_glDepthRangef:
        if (G.glDepthRangef) G.glDepthRangef(tspgl_unpack_f32(u[0]), tspgl_unpack_f32(u[1]));
        *out_n = 0;
        return 0;
    case OP_glDetachShader:
        if (G.glDetachShader) G.glDetachShader(u[0], u[1]);
        *out_n = 0;
        return 0;
    case OP_glDisable:
        if (G.glDisable) G.glDisable(u[0]);
        *out_n = 0;
        return 0;
    case OP_glEnable:
        if (G.glEnable) G.glEnable(u[0]);
        *out_n = 0;
        return 0;
    case OP_glFinish:
        if (G.glFinish) G.glFinish();
        *out_n = 0;
        return 0;
    case OP_glFlush:
        if (G.glFlush) G.glFlush();
        *out_n = 0;
        return 0;
    case OP_glFramebufferRenderbuffer: {
        uint32_t renderbuffer = u[3];
        if (unify_depth_stencil && u[1] == GL_DEPTH_ATTACHMENT &&
            depth_stencil_rb != 0) {
            renderbuffer = depth_stencil_rb;
            fprintf(stderr,
                    "P2-FBO unify depth attachment rb=%u\\n",
                    renderbuffer);
        }
        if (G.glFramebufferRenderbuffer)
            G.glFramebufferRenderbuffer(u[0], u[1], u[2], renderbuffer);
        *out_n = 0;
        return 0;
    }
    case OP_glFramebufferTexture2D: {
        if (fbo_texture_fallback && u[1] == GL_COLOR_ATTACHMENT0 &&
            u[2] == GL_TEXTURE_2D && u[3] != 0 && G.glTexImage2D &&
            G.glBindTexture && G.glGetIntegerv) {
            int32_t old_texture = 0;
            void (*get_int)(uint32_t, int32_t *) =
                (void *)G.glGetIntegerv;
            void (*bind_tex)(uint32_t, uint32_t) =
                (void *)G.glBindTexture;
            void (*tex_image)(uint32_t, int32_t, int32_t, int32_t, int32_t,
                              int32_t, uint32_t, uint32_t, const void *) =
                (void *)G.glTexImage2D;
            get_int(GL_TEXTURE_BINDING_2D, &old_texture);
            bind_tex(GL_TEXTURE_2D, u[3]);
            tex_image(GL_TEXTURE_2D, 0, GL_RGBA, game_w, game_h, 0,
                      GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            bind_tex(GL_TEXTURE_2D, (uint32_t)old_texture);
            fprintf(stderr,
                    "P2-FBO texture fallback tex=%u -> %dx%d\\n",
                    u[3], game_w, game_h);
        }
        if (G.glFramebufferTexture2D)
            G.glFramebufferTexture2D(u[0], u[1], u[2], u[3], (int32_t)u[4]);
        *out_n = 0;
        return 0;
    }
    case OP_glFrontFace:
        if (G.glFrontFace) G.glFrontFace(u[0]);
        *out_n = 0;
        return 0;
    case OP_glGenerateMipmap:
        if (G.glGenerateMipmap) G.glGenerateMipmap(u[0]);
        *out_n = 0;
        return 0;
    case OP_glGetError:
        out[0] = G.glGetError ? (uint32_t)G.glGetError() : 0;
        *out_n = 4;
        return 0;
    case OP_glHint:
        if (G.glHint) G.glHint(u[0], u[1]);
        *out_n = 0;
        return 0;
    case OP_glIsBuffer:
        out[0] = G.glIsBuffer ? (uint32_t)G.glIsBuffer(u[0]) : 0;
        *out_n = 4;
        return 0;
    case OP_glIsEnabled:
        out[0] = G.glIsEnabled ? (uint32_t)G.glIsEnabled(u[0]) : 0;
        *out_n = 4;
        return 0;
    case OP_glIsFramebuffer:
        out[0] = G.glIsFramebuffer ? (uint32_t)G.glIsFramebuffer(u[0]) : 0;
        *out_n = 4;
        return 0;
    case OP_glIsProgram:
        out[0] = G.glIsProgram ? (uint32_t)G.glIsProgram(u[0]) : 0;
        *out_n = 4;
        return 0;
    case OP_glIsRenderbuffer:
        out[0] = G.glIsRenderbuffer ? (uint32_t)G.glIsRenderbuffer(u[0]) : 0;
        *out_n = 4;
        return 0;
    case OP_glIsShader:
        out[0] = G.glIsShader ? (uint32_t)G.glIsShader(u[0]) : 0;
        *out_n = 4;
        return 0;
    case OP_glIsTexture:
        out[0] = G.glIsTexture ? (uint32_t)G.glIsTexture(u[0]) : 0;
        *out_n = 4;
        return 0;
    case OP_glLineWidth:
        if (G.glLineWidth) G.glLineWidth(tspgl_unpack_f32(u[0]));
        *out_n = 0;
        return 0;
    case OP_glLinkProgram:
        if (G.glLinkProgram) G.glLinkProgram(u[0]);
        *out_n = 0;
        return 0;
    case OP_glPolygonOffset:
        if (G.glPolygonOffset) G.glPolygonOffset(tspgl_unpack_f32(u[0]), tspgl_unpack_f32(u[1]));
        *out_n = 0;
        return 0;
    case OP_glReleaseShaderCompiler:
        if (G.glReleaseShaderCompiler) G.glReleaseShaderCompiler();
        *out_n = 0;
        return 0;
    case OP_glRenderbufferStorage: {
        uint32_t format = u[1];
        int32_t width = (int32_t)u[2];
        int32_t height = (int32_t)u[3];
        if (rb_format_fix && format == 0x8D48u) {
            format = GL_DEPTH24_STENCIL8;
            fprintf(stderr,
                    "P2-FBO format override 0x8d48 -> 0x%x\\n",
                    format);
        }
        if (rb_zero_size && width == 0 && height == 0) {
            width = game_w;
            height = game_h;
            fprintf(stderr,
                    "P2-FBO zero-rb override fmt=0x%x -> %dx%d\\n",
                    format, width, height);
        }
        if (unify_depth_stencil && format == GL_DEPTH24_STENCIL8 &&
            G.glGetIntegerv) {
            void (*get_int)(uint32_t, int32_t *) =
                (void *)G.glGetIntegerv;
            int32_t current_rb = 0;
            get_int(GL_RENDERBUFFER_BINDING, &current_rb);
            depth_stencil_rb = (uint32_t)current_rb;
            fprintf(stderr, "P2-FBO depth-stencil rb=%u\\n",
                    depth_stencil_rb);
        }
        if (G.glRenderbufferStorage)
            G.glRenderbufferStorage(u[0], format, width, height);
        *out_n = 0;
        return 0;
    }
    case OP_glSampleCoverage:
        if (G.glSampleCoverage) G.glSampleCoverage(tspgl_unpack_f32(u[0]), u[1]);
        *out_n = 0;
        return 0;
    case OP_glScissor:
        if (G.glScissor) G.glScissor((int32_t)u[0], (int32_t)u[1], (int32_t)u[2], (int32_t)u[3]);
        *out_n = 0;
        return 0;
    case OP_glStencilFunc:
        if (G.glStencilFunc) G.glStencilFunc(u[0], (int32_t)u[1], u[2]);
        *out_n = 0;
        return 0;
    case OP_glStencilFuncSeparate:
        if (G.glStencilFuncSeparate) G.glStencilFuncSeparate(u[0], u[1], (int32_t)u[2], u[3]);
        *out_n = 0;
        return 0;
    case OP_glStencilMask:
        if (G.glStencilMask) G.glStencilMask(u[0]);
        *out_n = 0;
        return 0;
    case OP_glStencilMaskSeparate:
        if (G.glStencilMaskSeparate) G.glStencilMaskSeparate(u[0], u[1]);
        *out_n = 0;
        return 0;
    case OP_glStencilOp:
        if (G.glStencilOp) G.glStencilOp(u[0], u[1], u[2]);
        *out_n = 0;
        return 0;
    case OP_glStencilOpSeparate:
        if (G.glStencilOpSeparate) G.glStencilOpSeparate(u[0], u[1], u[2], u[3]);
        *out_n = 0;
        return 0;
    case OP_glTexParameterf:
        if (G.glTexParameterf) G.glTexParameterf(u[0], u[1], tspgl_unpack_f32(u[2]));
        *out_n = 0;
        return 0;
    case OP_glTexParameteri:
        if (G.glTexParameteri) G.glTexParameteri(u[0], u[1], (int32_t)u[2]);
        *out_n = 0;
        return 0;
    case OP_glUniform1f:
        if (G.glUniform1f) G.glUniform1f((int32_t)u[0], tspgl_unpack_f32(u[1]));
        *out_n = 0;
        return 0;
    case OP_glUniform2f:
        if (G.glUniform2f) G.glUniform2f((int32_t)u[0], tspgl_unpack_f32(u[1]), tspgl_unpack_f32(u[2]));
        *out_n = 0;
        return 0;
    case OP_glUniform3f:
        if (G.glUniform3f) G.glUniform3f((int32_t)u[0], tspgl_unpack_f32(u[1]), tspgl_unpack_f32(u[2]), tspgl_unpack_f32(u[3]));
        *out_n = 0;
        return 0;
    case OP_glUniform4f:
        if (G.glUniform4f) G.glUniform4f((int32_t)u[0], tspgl_unpack_f32(u[1]), tspgl_unpack_f32(u[2]), tspgl_unpack_f32(u[3]), tspgl_unpack_f32(u[4]));
        *out_n = 0;
        return 0;
    case OP_glUniform1i:
        if (G.glUniform1i) G.glUniform1i((int32_t)u[0], (int32_t)u[1]);
        *out_n = 0;
        return 0;
    case OP_glUniform2i:
        if (G.glUniform2i) G.glUniform2i((int32_t)u[0], (int32_t)u[1], (int32_t)u[2]);
        *out_n = 0;
        return 0;
    case OP_glUniform3i:
        if (G.glUniform3i) G.glUniform3i((int32_t)u[0], (int32_t)u[1], (int32_t)u[2], (int32_t)u[3]);
        *out_n = 0;
        return 0;
    case OP_glUniform4i:
        if (G.glUniform4i) G.glUniform4i((int32_t)u[0], (int32_t)u[1], (int32_t)u[2], (int32_t)u[3], (int32_t)u[4]);
        *out_n = 0;
        return 0;
    case OP_glUseProgram:
        if (G.glUseProgram) G.glUseProgram(u[0]);
        *out_n = 0;
        return 0;
    case OP_glValidateProgram:
        if (G.glValidateProgram) G.glValidateProgram(u[0]);
        *out_n = 0;
        return 0;
    case OP_glVertexAttrib1f:
        if (G.glVertexAttrib1f) G.glVertexAttrib1f(u[0], tspgl_unpack_f32(u[1]));
        *out_n = 0;
        return 0;
    case OP_glVertexAttrib2f:
        if (G.glVertexAttrib2f) G.glVertexAttrib2f(u[0], tspgl_unpack_f32(u[1]), tspgl_unpack_f32(u[2]));
        *out_n = 0;
        return 0;
    case OP_glVertexAttrib3f:
        if (G.glVertexAttrib3f) G.glVertexAttrib3f(u[0], tspgl_unpack_f32(u[1]), tspgl_unpack_f32(u[2]), tspgl_unpack_f32(u[3]));
        *out_n = 0;
        return 0;
    case OP_glVertexAttrib4f:
        if (G.glVertexAttrib4f) G.glVertexAttrib4f(u[0], tspgl_unpack_f32(u[1]), tspgl_unpack_f32(u[2]), tspgl_unpack_f32(u[3]), tspgl_unpack_f32(u[4]));
        *out_n = 0;
        return 0;
    case OP_glViewport:
        if (G.glViewport) G.glViewport((int32_t)u[0], (int32_t)u[1], (int32_t)u[2], (int32_t)u[3]);
        *out_n = 0;
        return 0;
    case OP_glBlitFramebuffer:
        if (G.glBlitFramebuffer) G.glBlitFramebuffer((int32_t)u[0], (int32_t)u[1], (int32_t)u[2], (int32_t)u[3], (int32_t)u[4], (int32_t)u[5], (int32_t)u[6], (int32_t)u[7], u[8], u[9]);
        *out_n = 0;
        return 0;
    case OP_glGenBuffers:
        {
            uint32_t n = u[0];
            if (n > 256) n = 256;
            if (G.glGenBuffers) G.glGenBuffers((int)n, out);
            *out_n = n * 4u;
        }
        return 0;
    case OP_glGenFramebuffers:
        {
            uint32_t n = u[0];
            if (n > 256) n = 256;
            if (G.glGenFramebuffers) G.glGenFramebuffers((int)n, out);
            *out_n = n * 4u;
        }
        return 0;
    case OP_glGenRenderbuffers:
        {
            uint32_t n = u[0];
            if (n > 256) n = 256;
            if (G.glGenRenderbuffers) G.glGenRenderbuffers((int)n, out);
            *out_n = n * 4u;
        }
        return 0;
    case OP_glGenTextures:
        {
            uint32_t n = u[0];
            if (n > 256) n = 256;
            if (G.glGenTextures) G.glGenTextures((int)n, out);
            *out_n = n * 4u;
        }
        return 0;
    case OP_glGenVertexArrays:
        {
            uint32_t n = u[0];
            if (n > 256) n = 256;
            if (G.glGenVertexArrays) G.glGenVertexArrays((int)n, out);
            *out_n = n * 4u;
        }
        return 0;
    case OP_glDeleteBuffers:
        {
            uint32_t n = u[0];
            if (n > 256) n = 256;
            if (G.glDeleteBuffers) G.glDeleteBuffers((int)n, (const uint32_t *)(u + 1));
            *out_n = 0;
        }
        return 0;
    case OP_glDeleteFramebuffers:
        {
            uint32_t n = u[0];
            if (n > 256) n = 256;
            if (G.glDeleteFramebuffers) G.glDeleteFramebuffers((int)n, (const uint32_t *)(u + 1));
            *out_n = 0;
        }
        return 0;
    case OP_glDeleteRenderbuffers:
        {
            uint32_t n = u[0];
            if (n > 256) n = 256;
            if (G.glDeleteRenderbuffers) G.glDeleteRenderbuffers((int)n, (const uint32_t *)(u + 1));
            *out_n = 0;
        }
        return 0;
    case OP_glDeleteTextures:
        {
            uint32_t n = u[0];
            if (n > 256) n = 256;
            if (G.glDeleteTextures) G.glDeleteTextures((int)n, (const uint32_t *)(u + 1));
            *out_n = 0;
        }
        return 0;
    case OP_glDeleteVertexArrays:
        {
            uint32_t n = u[0];
            if (n > 256) n = 256;
            if (G.glDeleteVertexArrays) G.glDeleteVertexArrays((int)n, (const uint32_t *)(u + 1));
            *out_n = 0;
        }
        return 0;
    default:
        return -1;
    }
}

