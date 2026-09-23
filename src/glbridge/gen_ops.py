#!/usr/bin/env python3
"""Generate GLES2 marshal stubs for the 32->64 TSP GL bridge."""

from pathlib import Path

OUT = Path(__file__).resolve().parent

# name, arg types (each is u32 or f32), ret: void|u32|i32
SIMPLE = [
    ("glActiveTexture", ["u32"], "void"),
    ("glAttachShader", ["u32", "u32"], "void"),
    ("glBindFramebuffer", ["u32", "u32"], "void"),
    ("glBindRenderbuffer", ["u32", "u32"], "void"),
    ("glBindTexture", ["u32", "u32"], "void"),
    ("glBlendColor", ["f32", "f32", "f32", "f32"], "void"),
    ("glBlendEquation", ["u32"], "void"),
    ("glBlendEquationSeparate", ["u32", "u32"], "void"),
    ("glBlendFunc", ["u32", "u32"], "void"),
    ("glBlendFuncSeparate", ["u32", "u32", "u32", "u32"], "void"),
    ("glCheckFramebufferStatus", ["u32"], "u32"),
    ("glClear", ["u32"], "void"),
    ("glClearColor", ["f32", "f32", "f32", "f32"], "void"),
    ("glClearDepthf", ["f32"], "void"),
    ("glClearStencil", ["i32"], "void"),
    ("glColorMask", ["u32", "u32", "u32", "u32"], "void"),
    ("glCompileShader", ["u32"], "void"),
    ("glCopyTexImage2D", ["u32", "i32", "u32", "i32", "i32", "i32", "i32", "i32"], "void"),
    ("glCopyTexSubImage2D", ["u32", "i32", "i32", "i32", "i32", "i32", "i32", "i32"], "void"),
    ("glCreateProgram", [], "u32"),
    ("glCreateShader", ["u32"], "u32"),
    ("glCullFace", ["u32"], "void"),
    ("glDeleteProgram", ["u32"], "void"),
    ("glDeleteShader", ["u32"], "void"),
    ("glDepthFunc", ["u32"], "void"),
    ("glDepthMask", ["u32"], "void"),
    ("glDepthRangef", ["f32", "f32"], "void"),
    ("glDetachShader", ["u32", "u32"], "void"),
    ("glDisable", ["u32"], "void"),
    ("glEnable", ["u32"], "void"),
    ("glFinish", [], "void"),
    ("glFlush", [], "void"),
    ("glFramebufferRenderbuffer", ["u32", "u32", "u32", "u32"], "void"),
    ("glFramebufferTexture2D", ["u32", "u32", "u32", "u32", "i32"], "void"),
    ("glFrontFace", ["u32"], "void"),
    ("glGenerateMipmap", ["u32"], "void"),
    ("glGetError", [], "u32"),
    ("glHint", ["u32", "u32"], "void"),
    ("glIsBuffer", ["u32"], "u32"),
    ("glIsEnabled", ["u32"], "u32"),
    ("glIsFramebuffer", ["u32"], "u32"),
    ("glIsProgram", ["u32"], "u32"),
    ("glIsRenderbuffer", ["u32"], "u32"),
    ("glIsShader", ["u32"], "u32"),
    ("glIsTexture", ["u32"], "u32"),
    ("glLineWidth", ["f32"], "void"),
    ("glLinkProgram", ["u32"], "void"),
    ("glPolygonOffset", ["f32", "f32"], "void"),
    ("glReleaseShaderCompiler", [], "void"),
    ("glRenderbufferStorage", ["u32", "u32", "i32", "i32"], "void"),
    ("glSampleCoverage", ["f32", "u32"], "void"),
    ("glScissor", ["i32", "i32", "i32", "i32"], "void"),
    ("glStencilFunc", ["u32", "i32", "u32"], "void"),
    ("glStencilFuncSeparate", ["u32", "u32", "i32", "u32"], "void"),
    ("glStencilMask", ["u32"], "void"),
    ("glStencilMaskSeparate", ["u32", "u32"], "void"),
    ("glStencilOp", ["u32", "u32", "u32"], "void"),
    ("glStencilOpSeparate", ["u32", "u32", "u32", "u32"], "void"),
    ("glTexParameterf", ["u32", "u32", "f32"], "void"),
    ("glTexParameteri", ["u32", "u32", "i32"], "void"),
    ("glUniform1f", ["i32", "f32"], "void"),
    ("glUniform2f", ["i32", "f32", "f32"], "void"),
    ("glUniform3f", ["i32", "f32", "f32", "f32"], "void"),
    ("glUniform4f", ["i32", "f32", "f32", "f32", "f32"], "void"),
    ("glUniform1i", ["i32", "i32"], "void"),
    ("glUniform2i", ["i32", "i32", "i32"], "void"),
    ("glUniform3i", ["i32", "i32", "i32", "i32"], "void"),
    ("glUniform4i", ["i32", "i32", "i32", "i32", "i32"], "void"),
    ("glUseProgram", ["u32"], "void"),
    ("glValidateProgram", ["u32"], "void"),
    ("glVertexAttrib1f", ["u32", "f32"], "void"),
    ("glVertexAttrib2f", ["u32", "f32", "f32"], "void"),
    ("glVertexAttrib3f", ["u32", "f32", "f32", "f32"], "void"),
    ("glVertexAttrib4f", ["u32", "f32", "f32", "f32", "f32"], "void"),
    ("glViewport", ["i32", "i32", "i32", "i32"], "void"),
    ("glBlitFramebuffer", ["i32", "i32", "i32", "i32", "i32", "i32", "i32", "i32", "u32", "u32"], "void"),
]

# name, count_arg_index (0-based among the u32 args before pointer)
GENDEL = [
    ("glGenBuffers", "glGenBuffers"),
    ("glGenFramebuffers", "glGenFramebuffers"),
    ("glGenRenderbuffers", "glGenRenderbuffers"),
    ("glGenTextures", "glGenTextures"),
    ("glGenVertexArrays", "glGenVertexArrays"),
    ("glDeleteBuffers", "glDeleteBuffers"),
    ("glDeleteFramebuffers", "glDeleteFramebuffers"),
    ("glDeleteRenderbuffers", "glDeleteRenderbuffers"),
    ("glDeleteTextures", "glDeleteTextures"),
    ("glDeleteVertexArrays", "glDeleteVertexArrays"),
]


def ctype(t):
    return {"u32": "uint32_t", "i32": "int32_t", "f32": "float"}[t]


def pack_arg(t, name):
    if t == "f32":
        return f"tspgl_pack_f32({name})"
    if t == "i32":
        return f"(uint32_t){name}"
    return name


def main():
    lines_h = ["#ifndef TSPGL_OPS_H", "#define TSPGL_OPS_H", "", "enum {"]
    op = 16
    for name, args, ret in SIMPLE:
        lines_h.append(f"    OP_{name} = {op},")
        op += 1
    for name, _ in GENDEL:
        lines_h.append(f"    OP_{name} = {op},")
        op += 1
    specials = [
        "glGetString", "glGetIntegerv", "glGetFloatv", "glGetBooleanv",
        "glGetShaderiv", "glGetProgramiv", "glGetShaderInfoLog", "glGetProgramInfoLog",
        "glShaderSource", "glBindAttribLocation", "glGetAttribLocation", "glGetUniformLocation",
        "glBufferData", "glBufferSubData", "glTexImage2D", "glTexSubImage2D",
        "glCompressedTexImage2D", "glCompressedTexSubImage2D",
        "glReadPixels", "glVertexAttribPointer", "glDrawArrays", "glDrawElements",
        "glUniform1fv", "glUniform2fv", "glUniform3fv", "glUniform4fv",
        "glUniform1iv", "glUniform2iv", "glUniform3iv", "glUniform4iv",
        "glUniformMatrix2fv", "glUniformMatrix3fv", "glUniformMatrix4fv",
        "glGetBufferParameteriv", "glMapBufferOES", "glUnmapBufferOES",
        "glGetBufferPointervOES", "glMapBufferRange",
        "glBindBuffer", "glEnableVertexAttribArray", "glDisableVertexAttribArray",
        "glPixelStorei", "glUploadAttrib", "glTexParameterfv", "glTexParameteriv",
        "glVertexAttrib1fv", "glVertexAttrib2fv", "glVertexAttrib3fv", "glVertexAttrib4fv",
        "glGetActiveAttrib", "glGetActiveUniform", "glGetAttachedShaders",
        "glGetFramebufferAttachmentParameteriv", "glGetRenderbufferParameteriv",
        "glGetShaderPrecisionFormat", "glGetShaderSource",
        "glGetTexParameterfv", "glGetTexParameteriv",
        "glGetUniformfv", "glGetUniformiv",
        "glGetVertexAttribfv", "glGetVertexAttribiv",
        "glBindVertexArray", "glIsVertexArray",
    ]
    for name in specials:
        lines_h.append(f"    OP_{name} = {op},")
        op += 1
    lines_h += ["    OP_GLES_LAST", "};", "", "#endif"]
    (OUT / "ops.h").write_text("\n".join(lines_h) + "\n", encoding="utf-8")

    cl = [
        "/* generated — do not edit */",
        "#include \"ops.h\"",
        "#include \"xport.h\"",
        "#include <stdint.h>",
        "#include <string.h>",
        "",
    ]
    for name, args, ret in SIMPLE:
        params = []
        pack = []
        for i, t in enumerate(args):
            params.append(f"{ctype(t)} a{i}")
            pack.append(f"    u[{i}] = {pack_arg(t, f'a{i}')};")
        sig = ", ".join(params) if params else "void"
        rctype = {"void": "void", "u32": "uint32_t", "i32": "int32_t"}[ret]
        cl.append(f"{rctype} {name}({sig})")
        cl.append("{")
        if args:
            cl.append(f"    uint32_t u[{len(args)}];")
            cl.extend(pack)
            cl.append(f"    struct tspgl_hdr rh;")
            if ret == "void":
                cl.append(f"    tspgl_call(OP_{name}, u, sizeof(u), NULL, 0);")
            else:
                cl.append(f"    uint32_t out = 0;")
                cl.append(f"    tspgl_call(OP_{name}, u, sizeof(u), &out, sizeof(out));")
                cl.append(f"    return ({rctype})out;")
        else:
            if ret == "void":
                cl.append(f"    tspgl_call(OP_{name}, NULL, 0, NULL, 0);")
            else:
                cl.append(f"    uint32_t out = 0;")
                cl.append(f"    tspgl_call(OP_{name}, NULL, 0, &out, sizeof(out));")
                cl.append(f"    return ({rctype})out;")
        cl.append("}")
        cl.append("")

    for name, _ in GENDEL:
        is_del = name.startswith("glDelete")
        cl.append(f"void tspgl_raw_{name}(int32_t n, {'const ' if is_del else ''}uint32_t *ids)")
        cl.append("{")
        cl.append("    uint32_t buf[1 + 256];")
        cl.append("    uint32_t count;")
        cl.append("    if (n <= 0) return;")
        cl.append("    count = (uint32_t)n;")
        cl.append("    if (count > 256) count = 256;")
        cl.append("    buf[0] = count;")
        if is_del:
            cl.append("    if (ids) memcpy(buf + 1, ids, count * 4u);")
            cl.append("    else memset(buf + 1, 0, count * 4u);")
            cl.append(f"    tspgl_call(OP_{name}, buf, (1 + count) * 4u, NULL, 0);")
        else:
            cl.append(f"    tspgl_call(OP_{name}, buf, 4u, buf + 1, count * 4u);")
            cl.append("    if (ids) memcpy(ids, buf + 1, count * 4u);")
        cl.append("}")
        cl.append("")

    (OUT / "client_gen.c").write_text("\n".join(cl), encoding="utf-8")

    sv = [
        "/* generated — do not edit */",
        "#include \"ops.h\"",
        "",
        "static void tspgl_load_simple(void *(*get)(const char *))",
        "{",
    ]
    for name, args, ret in SIMPLE:
        sv.append(f"    G.{name} = get(\"{name}\");")
    for name, _ in GENDEL:
        sv.append(f"    G.{name} = get(\"{name}\");")
    sv.append("}")
    sv.append("")
    sv.append("static void tspgl_load_specials(void *(*get)(const char *))")
    sv.append("{")
    for name in specials:
        sv.append(f"    G.{name} = get(\"{name}\");")
    sv.append("}")
    sv.append("")
    sv.append("static int tspgl_dispatch_simple(uint32_t op, const uint32_t *u, uint32_t nbytes,")
    sv.append("                                 uint32_t *out, uint32_t *out_n)")
    sv.append("{")
    sv.append("    (void)nbytes;")
    sv.append("    switch (op) {")

    for name, args, ret in SIMPLE:
        sv.append(f"    case OP_{name}:")
        call_args = []
        for i, t in enumerate(args):
            if t == "f32":
                call_args.append(f"tspgl_unpack_f32(u[{i}])")
            elif t == "i32":
                call_args.append(f"(int32_t)u[{i}]")
            else:
                call_args.append(f"u[{i}]")
        joined = ", ".join(call_args)
        if ret == "void":
            sv.append(f"        if (G.{name}) G.{name}({joined});")
            sv.append("        *out_n = 0;")
        else:
            sv.append(f"        out[0] = G.{name} ? (uint32_t)G.{name}({joined}) : 0;")
            sv.append("        *out_n = 4;")
        sv.append("        return 0;")

    for name, _ in GENDEL:
        is_del = name.startswith("glDelete")
        sv.append(f"    case OP_{name}:")
        sv.append("        {")
        sv.append("            uint32_t n = u[0];")
        sv.append("            if (n > 256) n = 256;")
        if is_del:
            sv.append(f"            if (G.{name}) G.{name}((int)n, (const uint32_t *)(u + 1));")
            sv.append("            *out_n = 0;")
        else:
            sv.append(f"            if (G.{name}) G.{name}((int)n, out);")
            sv.append("            *out_n = n * 4u;")
        sv.append("        }")
        sv.append("        return 0;")

    sv.append("    default:")
    sv.append("        return -1;")
    sv.append("    }")
    sv.append("}")
    sv.append("")

    # struct G fields — write gl_api.h
    gh = ["#ifndef TSPGL_API_H", "#define TSPGL_API_H", "", "#include <stdint.h>", "", "struct tspgl_api {"]
    for name, args, ret in SIMPLE:
        at = []
        for t in args:
            at.append(ctype(t))
        r = {"void": "void", "u32": "uint32_t", "i32": "int32_t"}[ret]
        gh.append(f"    {r} (*{name})({', '.join(at) if at else 'void'});")
    for name, _ in GENDEL:
        if name.startswith("glDelete"):
            gh.append(f"    void (*{name})(int32_t, const uint32_t *);")
        else:
            gh.append(f"    void (*{name})(int32_t, uint32_t *);")
    # specials as void* then cast in server_extra
    for name in specials:
        gh.append(f"    void *{name};")
    gh += ["};", "", "extern struct tspgl_api G;", "", "#endif"]
    (OUT / "gl_api.h").write_text("\n".join(gh) + "\n", encoding="utf-8")
    (OUT / "server_gen.c").write_text("\n".join(sv) + "\n", encoding="utf-8")
    print("wrote", op, "ops")


if __name__ == "__main__":
    main()
