#define _GNU_SOURCE
#include "xport.h"
#include "ops.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef uint32_t GLenum;
typedef uint32_t GLbitfield;
typedef uint32_t GLuint;
typedef int32_t GLint;
typedef int32_t GLsizei;
typedef uint8_t GLboolean;
typedef float GLfloat;
typedef uint8_t GLubyte;
typedef int16_t GLshort;
typedef uint16_t GLushort;
typedef void GLvoid;

#define GL_UNSIGNED_BYTE 0x1401
#define GL_UNSIGNED_SHORT 0x1403
#define GL_UNSIGNED_INT 0x1405
#define GL_BYTE 0x1400
#define GL_SHORT 0x1402
#define GL_FLOAT 0x1406
#define GL_ALPHA 0x1906
#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909
#define GL_LUMINANCE_ALPHA 0x190A
#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_UNPACK_ALIGNMENT 0x0CF5

#define active_tex (tspgl_shared()->active_tex)
#define tex_bind_2d (tspgl_shared()->tex_bind_2d)
#define tex_bind_cube (tspgl_shared()->tex_bind_cube)
#define bound_fb (tspgl_shared()->bound_fb)
#define bound_read_fb (tspgl_shared()->bound_read_fb)
#define bound_rb (tspgl_shared()->bound_rb)
#define bound_array (tspgl_shared()->bound_array)
#define bound_element (tspgl_shared()->bound_element)
#define bound_vao (tspgl_shared()->bound_vao)
#define cur_prog (tspgl_shared()->cur_prog)
#define st_viewport (tspgl_shared()->st_viewport)
#define st_scissor (tspgl_shared()->st_scissor)
#define unpack_align (tspgl_shared()->unpack_align)

static int vao_diag = -1;
static unsigned vao_diag_calls;

#define SH_MAX 2048
struct shbuf {
    uint32_t id;
    uint32_t size;
    uint32_t usage;
    uint8_t *cpu;
    int mapped;
    uint32_t map_off;
    uint32_t map_len;
};
static struct shbuf shbufs[SH_MAX];

static struct shbuf *sh_find(uint32_t id, int create)
{
    unsigned i;
    unsigned empty = SH_MAX;
    if (id == 0)
        return NULL;
    for (i = 0; i < SH_MAX; ++i) {
        if (shbufs[i].id == id)
            return &shbufs[i];
        if (empty == SH_MAX && shbufs[i].id == 0)
            empty = i;
    }
    if (!create || empty == SH_MAX)
        return NULL;
    memset(&shbufs[empty], 0, sizeof(shbufs[empty]));
    shbufs[empty].id = id;
    return &shbufs[empty];
}

static void sh_free_id(uint32_t id)
{
    struct shbuf *s = sh_find(id, 0);
    if (!s)
        return;
    free(s->cpu);
    memset(s, 0, sizeof(*s));
}

static uint32_t sh_bound(GLenum target)
{
    if (target == GL_ARRAY_BUFFER)
        return bound_array;
    if (target == GL_ELEMENT_ARRAY_BUFFER)
        return bound_element;
    return 0;
}

static void sh_store(GLenum target, uint32_t size, uint32_t usage,
                     const void *data)
{
    struct shbuf *s = sh_find(sh_bound(target), 1);
    if (!s)
        return;
    free(s->cpu);
    s->cpu = NULL;
    s->size = size;
    s->usage = usage;
    s->mapped = 0;
    if (size) {
        s->cpu = malloc(size);
        if (s->cpu) {
            if (data)
                memcpy(s->cpu, data, size);
            else
                memset(s->cpu, 0, size);
        }
    }
}

static void sh_sub(GLenum target, uint32_t offset, uint32_t size,
                   const void *data)
{
    struct shbuf *s = sh_find(sh_bound(target), 0);
    if (!s || !s->cpu || !data)
        return;
    if (offset >= s->size)
        return;
    if (offset + size > s->size)
        size = s->size - offset;
    memcpy(s->cpu + offset, data, size);
}

struct attrib {
    int enabled;
    int size;
    GLenum type;
    int normalized;
    int stride;
    uint32_t ptr;
    int is_offset;
    GLuint source_buffer;
};
struct vao_shadow {
    GLuint id;
    GLuint element_buffer;
    struct attrib attrib[16];
};
static struct attrib attribs[16];
static struct vao_shadow vaos[128];
static struct vao_shadow vao0;
static struct vao_shadow *current_vao = &vao0;

static struct vao_shadow *find_vao(GLuint id, int create)
{
    unsigned i;
    if (id == 0)
        return &vao0;
    for (i = 0; i < 128; ++i)
        if (vaos[i].id == id)
            return &vaos[i];
    if (!create)
        return NULL;
    for (i = 0; i < 128; ++i) {
        if (vaos[i].id == 0) {
            vaos[i].id = id;
            vaos[i].element_buffer = 0;
            memset(vaos[i].attrib, 0, sizeof(vaos[i].attrib));
            return &vaos[i];
        }
    }
    return NULL;
}

__attribute__((constructor)) static void init_vao0(void)
{
    memset(&vao0, 0, sizeof(vao0));
    vao0.id = 0;
    current_vao = &vao0;
}

static unsigned type_bytes(GLenum type)
{
    switch (type) {
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
        return 1;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
        return 2;
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
        return 4;
    default:
        return 4;
    }
}

static unsigned image_bytes(GLsizei w, GLsizei h, GLenum format, GLenum type)
{
    unsigned cpp = 4;
    unsigned row, pad;
    unsigned comp = 4;

    if (format == GL_RGBA || format == 0x80E1) /* BGRA_EXT */
        comp = 4;
    else if (format == GL_RGB)
        comp = 3;
    else if (format == GL_LUMINANCE_ALPHA)
        comp = 2;
    else if (format == GL_ALPHA || format == GL_LUMINANCE ||
             format == 0x1902 || format == 0x1903) /* DEPTH / STENCIL */
        comp = 1;
    else if (format == 0x84F9) /* DEPTH_STENCIL */
        comp = 1;

    if (type == GL_UNSIGNED_BYTE)
        cpp = comp;
    else if (type == 0x8033 || type == 0x8034 || type == 0x8363)
        cpp = 2; /* packed 16-bit */
    else if (type == GL_UNSIGNED_SHORT || type == 0x8D61)
        cpp = comp * 2; /* SHORT / HALF_FLOAT_OES */
    else if (type == GL_FLOAT)
        cpp = comp * 4;
    else if (type == 0x84FA)
        cpp = 4; /* UNSIGNED_INT_24_8 */
    else
        cpp = comp;
    if (w < 0)
        w = 0;
    if (h < 0)
        h = 0;
    row = (unsigned)w * cpp;
    pad = (unsigned)unpack_align;
    if (pad < 1)
        pad = 1;
    row = (row + pad - 1u) / pad * pad;
    return row * (unsigned)h;
}

/* ---- generated simple GLES ---- */
#define glActiveTexture glActiveTexture_gen
#define glBindFramebuffer glBindFramebuffer_gen
#define glBindRenderbuffer glBindRenderbuffer_gen
#define glBindTexture glBindTexture_gen
#define glUseProgram glUseProgram_gen
#define glViewport glViewport_gen
#define glScissor glScissor_gen
#define glLinkProgram glLinkProgram_gen
#define glDeleteProgram glDeleteProgram_gen
#include "client_gen.c"
#undef glActiveTexture
#undef glBindFramebuffer
#undef glBindRenderbuffer
#undef glBindTexture
#undef glUseProgram
#undef glViewport
#undef glScissor
#undef glLinkProgram
#undef glDeleteProgram

void glActiveTexture(GLenum texture)
{
    unsigned u = texture >= 0x84C0 ? texture - 0x84C0 : 0;
    if (u < 32)
        active_tex = u;
    glActiveTexture_gen(texture);
}

void glBindFramebuffer(GLenum target, GLuint fb)
{
    if (target == 0x8D40) {
        bound_fb = fb;
        bound_read_fb = fb;
    } else if (target == 0x8CA9)
        bound_fb = fb;
    else if (target == 0x8CA8)
        bound_read_fb = fb;
    glBindFramebuffer_gen(target, fb);
}

void glBindRenderbuffer(GLenum target, GLuint rb)
{
    (void)target;
    bound_rb = rb;
    glBindRenderbuffer_gen(target, rb);
}

void glBindTexture(GLenum target, GLuint texture)
{
    if (active_tex < 32) {
        if (target == GL_TEXTURE_2D)
            tex_bind_2d[active_tex] = texture;
        if (target == GL_TEXTURE_CUBE_MAP ||
            (target >= 0x8515 && target <= 0x851A))
            tex_bind_cube[active_tex] = texture;
    }
    if (target >= 0x8515 && target <= 0x851A)
        target = GL_TEXTURE_CUBE_MAP;
    glBindTexture_gen(target, texture);
}

void glUseProgram(GLuint program)
{
    cur_prog = program;
    glUseProgram_gen(program);
}

void glViewport(GLint x, GLint y, GLsizei w, GLsizei h)
{
    st_viewport[0] = x;
    st_viewport[1] = y;
    st_viewport[2] = w;
    st_viewport[3] = h;
    glViewport_gen(x, y, w, h);
}

void glScissor(GLint x, GLint y, GLsizei w, GLsizei h)
{
    st_scissor[0] = x;
    st_scissor[1] = y;
    st_scissor[2] = w;
    st_scissor[3] = h;
    glScissor_gen(x, y, w, h);
}

#define LOC_N 8192
#define LOC_PROBE 64
#define LOC_NAME 64
struct locent {
    uint32_t prog;
    uint32_t h;
    int32_t loc;
    char name[LOC_NAME];
};
static struct locent ulocs[LOC_N];
static struct locent alocs[LOC_N];

static uint32_t loc_hash(const char *s)
{
    uint32_t h = 2166136261u;
    if (!s)
        return 0;
    while (*s) {
        h ^= (uint8_t)*s++;
        h *= 16777619u;
    }
    return h;
}

static void loc_clear_prog(uint32_t prog)
{
    unsigned i;
    for (i = 0; i < LOC_N; ++i) {
        if (ulocs[i].prog == prog)
            ulocs[i].prog = 0;
        if (alocs[i].prog == prog)
            alocs[i].prog = 0;
    }
}

void glLinkProgram(GLuint program)
{
    loc_clear_prog(program);
    glLinkProgram_gen(program);
}

void glDeleteProgram(GLuint program)
{
    loc_clear_prog(program);
    if (cur_prog == program)
        cur_prog = 0;
    glDeleteProgram_gen(program);
}

static int loc_lookup(struct locent *tab, uint32_t prog, const char *name,
                      int32_t *out)
{
    uint32_t h = loc_hash(name);
    unsigned slot, i;
    size_t nlen;
    if (!name)
        return 0;
    nlen = strlen(name);
    if (nlen >= LOC_NAME)
        return 0;
    slot = h % LOC_N;
    for (i = 0; i < LOC_PROBE; ++i) {
        struct locent *e = &tab[(slot + i) % LOC_N];
        if (e->prog == prog && e->h == h && strcmp(e->name, name) == 0) {
            *out = e->loc;
            return 1;
        }
    }
    return 0;
}

static void loc_store(struct locent *tab, uint32_t prog, const char *name,
                      int32_t loc)
{
    uint32_t h = loc_hash(name);
    unsigned slot, i;
    int empty = -1;
    size_t nlen;
    static int loc_full;
    if (!name)
        return;
    nlen = strlen(name);
    if (nlen >= LOC_NAME)
        return;
    slot = h % LOC_N;
    for (i = 0; i < LOC_PROBE; ++i) {
        struct locent *e = &tab[(slot + i) % LOC_N];
        if (e->prog == prog && e->h == h && strcmp(e->name, name) == 0) {
            e->loc = loc;
            return;
        }
        if (e->prog == 0 && empty < 0)
            empty = (int)i;
    }
    if (empty >= 0) {
        struct locent *e = &tab[(slot + (unsigned)empty) % LOC_N];
        e->prog = prog;
        e->h = h;
        e->loc = loc;
        memset(e->name, 0, sizeof(e->name));
        memcpy(e->name, name, nlen);
        return;
    }
    if (loc_full < 8) {
        fprintf(stderr, "tspgl: loc cache full prog=%u %s\n", prog, name);
        loc_full++;
    }
}

void tspgl_raw_glGenBuffers(int32_t n, uint32_t *ids);
void tspgl_raw_glGenFramebuffers(int32_t n, uint32_t *ids);
void tspgl_raw_glGenRenderbuffers(int32_t n, uint32_t *ids);
void tspgl_raw_glGenTextures(int32_t n, uint32_t *ids);
void tspgl_raw_glGenVertexArrays(int32_t n, uint32_t *ids);
void tspgl_raw_glDeleteBuffers(int32_t n, const uint32_t *ids);
void tspgl_raw_glDeleteFramebuffers(int32_t n, const uint32_t *ids);
void tspgl_raw_glDeleteRenderbuffers(int32_t n, const uint32_t *ids);
void tspgl_raw_glDeleteTextures(int32_t n, const uint32_t *ids);
void tspgl_raw_glDeleteVertexArrays(int32_t n, const uint32_t *ids);

void glGenBuffers(int32_t n, uint32_t *ids)
{
    tspgl_raw_glGenBuffers(n, ids);
}
void glGenFramebuffers(int32_t n, uint32_t *ids)
{
    tspgl_raw_glGenFramebuffers(n, ids);
}
void glGenRenderbuffers(int32_t n, uint32_t *ids)
{
    tspgl_raw_glGenRenderbuffers(n, ids);
}
void glGenTextures(int32_t n, uint32_t *ids)
{
    tspgl_raw_glGenTextures(n, ids);
}
void glGenVertexArrays(int32_t n, uint32_t *ids)
{
    tspgl_raw_glGenVertexArrays(n, ids);
}
void glDeleteBuffers(int32_t n, const uint32_t *ids)
{
    int32_t i;
    tspgl_raw_glDeleteBuffers(n, ids);
    if (!ids)
        return;
    for (i = 0; i < n; ++i) {
        sh_free_id(ids[i]);
        if (ids[i] && ids[i] == bound_array)
            bound_array = 0;
        if (ids[i] && ids[i] == bound_element)
            bound_element = 0;
    }
}
void glDeleteFramebuffers(int32_t n, const uint32_t *ids)
{
    int32_t i;
    tspgl_raw_glDeleteFramebuffers(n, ids);
    if (!ids)
        return;
    for (i = 0; i < n; ++i) {
        if (ids[i] && ids[i] == bound_fb)
            bound_fb = 0;
        if (ids[i] && ids[i] == bound_read_fb)
            bound_read_fb = 0;
    }
}
void glDeleteRenderbuffers(int32_t n, const uint32_t *ids)
{
    int32_t i;
    tspgl_raw_glDeleteRenderbuffers(n, ids);
    if (!ids)
        return;
    for (i = 0; i < n; ++i) {
        if (ids[i] && ids[i] == bound_rb)
            bound_rb = 0;
    }
}
void glDeleteTextures(int32_t n, const uint32_t *ids)
{
    int32_t i, u;
    tspgl_raw_glDeleteTextures(n, ids);
    if (!ids)
        return;
    for (i = 0; i < n; ++i) {
        uint32_t id = ids[i];
        if (!id)
            continue;
        for (u = 0; u < 32; ++u) {
            if (tex_bind_2d[u] == id)
                tex_bind_2d[u] = 0;
            if (tex_bind_cube[u] == id)
                tex_bind_cube[u] = 0;
        }
    }
}
void glDeleteVertexArrays(int32_t n, const uint32_t *ids)
{
    int32_t i;
    tspgl_raw_glDeleteVertexArrays(n, ids);
    if (!ids)
        return;
    for (i = 0; i < n; ++i) {
        struct vao_shadow *v;
        if (!ids[i])
            continue;
        v = find_vao(ids[i], 0);
        if (v) {
            if (v == current_vao) {
                current_vao = &vao0;
                memcpy(attribs, vao0.attrib, sizeof(attribs));
                bound_element = 0;
                bound_vao = 0;
            }
            memset(v, 0, sizeof(*v));
        }
    }
}

void glGenVertexArraysOES(int32_t n, uint32_t *ids)
{
    glGenVertexArrays(n, ids);
}
void glDeleteVertexArraysOES(int32_t n, const uint32_t *ids)
{
    glDeleteVertexArrays(n, ids);
}

/* ---- specials ---- */
const GLubyte *glGetString(GLenum name)
{
    static char vendor[256], renderer[256], version[256], ext[4096];
    static const char *safe_ext =
        "GL_OES_rgb8_rgba8 GL_OES_packed_depth_stencil GL_OES_depth24 "
        "GL_OES_element_index_uint "
        "GL_OES_standard_derivatives GL_OES_texture_npot "
        "GL_OES_texture_float GL_OES_texture_half_float "
        "GL_EXT_texture_format_BGRA8888 GL_EXT_blend_minmax "
        "GL_EXT_discard_framebuffer GL_OES_compressed_ETC1_RGB8_texture "
        "GL_OES_vertex_array_object GL_OES_mapbuffer GL_OES_EGL_image "
        "GL_EXT_texture_format_BGRA8888 GL_EXT_read_format_bgra";
    uint32_t in = name;
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    if (name == 0x1F00) { /* GL_VENDOR */
        memcpy(vendor, "ARM", 4);
        return (const GLubyte *)vendor;
    }
    if (name == 0x1F01) { /* GL_RENDERER */
        memcpy(renderer, "Mali-400 MP", 12);
        return (const GLubyte *)renderer;
    }
    if (name == 0x1F02) { /* GL_VERSION */
        memcpy(version, "OpenGL ES 2.0", 14);
        return (const GLubyte *)version;
    }
    if (name == 0x1F03) { /* GL_EXTENSIONS */
        memset(ext, 0, sizeof(ext));
        memcpy(ext, safe_ext, strlen(safe_ext));
        return (const GLubyte *)ext;
    }
    if (name == 0x8B8C) { /* GL_SHADING_LANGUAGE_VERSION */
        static char sl[32];
        memcpy(sl, "OpenGL ES GLSL ES 1.00", 23);
        return (const GLubyte *)sl;
    }
    tspgl_call(OP_glGetString, &in, 4, buf, sizeof(buf) - 1);
    memcpy(renderer, buf, sizeof(renderer) - 1);
    return (const GLubyte *)renderer;
}

const GLubyte *glGetStringi(GLenum name, GLuint index)
{
    static char one[128];
    static const char *exts[] = {
        "GL_OES_rgb8_rgba8",
        "GL_OES_packed_depth_stencil",
        "GL_OES_depth24",
        "GL_OES_element_index_uint",
        "GL_OES_standard_derivatives",
        "GL_OES_texture_npot",
        "GL_OES_texture_float",
        "GL_OES_texture_half_float",
        "GL_EXT_texture_format_BGRA8888",
        "GL_EXT_blend_minmax",
        "GL_EXT_discard_framebuffer",
        "GL_OES_compressed_ETC1_RGB8_texture",
        "GL_OES_vertex_array_object",
        "GL_OES_mapbuffer",
        "GL_OES_EGL_image",
        "GL_EXT_texture_format_BGRA8888",
        "GL_EXT_read_format_bgra",
        NULL,
    };
    if (name != 0x1F03 || !exts[index])
        return NULL;
    memset(one, 0, sizeof(one));
    memcpy(one, exts[index], strlen(exts[index]));
    return (const GLubyte *)one;
}

static unsigned gl_param_count(GLenum pname)
{
    switch (pname) {
    case 0x0BA2: /* GL_VIEWPORT */
    case 0x0C10: /* GL_SCISSOR_BOX */
    case 0x0C22: /* GL_COLOR_CLEAR_VALUE */
    case 0x0C23: /* GL_COLOR_WRITEMASK */
        return 4;
    case 0x0B12: /* GL_POINT_SIZE_RANGE */
    case 0x0B22: /* GL_LINE_WIDTH_RANGE */
    case 0x0D3A: /* GL_MAX_VIEWPORT_DIMS */
    case 0x846D: /* GL_ALIASED_LINE_WIDTH_RANGE */
    case 0x846E: /* GL_ALIASED_POINT_SIZE_RANGE */
        return 2;
    default:
        return 1;
    }
}

static int gl_limit_pname(GLenum pname)
{
    switch (pname) {
    case 0x0D33: /* MAX_TEXTURE_SIZE */
    case 0x851C: /* MAX_CUBE_MAP_TEXTURE_SIZE */
    case 0x84E8: /* MAX_TEXTURE_IMAGE_UNITS */
    case 0x8872: /* MAX_VERTEX_TEXTURE_IMAGE_UNITS */
    case 0x8B4C: /* MAX_COMBINED_TEXTURE_IMAGE_UNITS */
    case 0x8B4A: /* MAX_VERTEX_UNIFORM_VECTORS */
    case 0x8B4B: /* MAX_FRAGMENT_UNIFORM_VECTORS */
    case 0x8B4D: /* MAX_VARYING_VECTORS */
    case 0x0D3A: /* MAX_VIEWPORT_DIMS */
    case 0x846D: /* ALIASED_LINE_WIDTH_RANGE */
    case 0x846E: /* ALIASED_POINT_SIZE_RANGE */
    case 0x0B12: /* POINT_SIZE_RANGE */
    case 0x80A9: /* MAX_RENDERBUFFER_SIZE? actually 0x84E8 */
    case 0x84E2: /* MAX_3D_TEXTURE_SIZE */
    case 0x8869: /* MAX_VERTEX_ATTRIBS */
    case 0x80AD: /* MAX_ELEMENTS_VERTICES */
    case 0x80AC: /* MAX_ELEMENTS_INDICES */
    case 0x0D3B: /* MAX_TEXTURE_UNITS */
    case 0x8824: /* MAX_COLOR_ATTACHMENTS */
    case 0x8CDF: /* MAX_RENDERBUFFER_SIZE */
    case 0x0D32: /* MAX_PIXEL_MAP_TABLE */
    case 0x8DFB: /* NUM_COMPRESSED_TEXTURE_FORMATS? 0x86A2 */
    case 0x86A2: /* NUM_COMPRESSED_TEXTURE_FORMATS */
        return 1;
    default:
        return 0;
    }
}

static int get_local_int(GLenum pname, GLint *params)
{
    static unsigned geom_logs;
    if (!params)
        return 1;
    switch (pname) {
    case 0x0BA2: /* VIEWPORT */
        memcpy(params, st_viewport, 16);
        if (getenv("POSTAL2_XPORT_DIAG") && geom_logs++ < 8)
            fprintf(stderr, "P2-GEOM get GL_VIEWPORT -> %d,%d,%d,%d\\n", params[0], params[1], params[2], params[3]);
        return 1;
    case 0x0C10: /* SCISSOR_BOX */
        memcpy(params, st_scissor, 16);
        if (getenv("POSTAL2_XPORT_DIAG") && geom_logs++ < 8)
            fprintf(stderr, "P2-GEOM get GL_SCISSOR_BOX -> %d,%d,%d,%d\\n", params[0], params[1], params[2], params[3]);
        return 1;
    case 0x84E0: /* ACTIVE_TEXTURE */
        params[0] = (GLint)(0x84C0 + active_tex);
        return 1;
    case 0x8069: /* TEXTURE_BINDING_2D */
        params[0] = (GLint)tex_bind_2d[active_tex < 32 ? active_tex : 0];
        return 1;
    case 0x8514: /* TEXTURE_BINDING_CUBE_MAP */
        params[0] = (GLint)tex_bind_cube[active_tex < 32 ? active_tex : 0];
        return 1;
    case 0x8CA6: /* FRAMEBUFFER_BINDING / DRAW */
        params[0] = (GLint)bound_fb;
        return 1;
    case 0x8CAA: /* READ_FRAMEBUFFER_BINDING */
        params[0] = (GLint)bound_read_fb;
        return 1;
    case 0x8CA7: /* RENDERBUFFER_BINDING */
        params[0] = (GLint)bound_rb;
        return 1;
    case 0x8B8D: /* CURRENT_PROGRAM */
        params[0] = (GLint)cur_prog;
        return 1;
    case 0x8894: /* ARRAY_BUFFER_BINDING */
        params[0] = (GLint)bound_array;
        return 1;
    case 0x8895: /* ELEMENT_ARRAY_BUFFER_BINDING */
        params[0] = (GLint)bound_element;
        return 1;
    case 0x85B5: /* VERTEX_ARRAY_BINDING */
        params[0] = (GLint)bound_vao;
        return 1;
    case 0x0CF5: /* UNPACK_ALIGNMENT */
        params[0] = unpack_align;
        return 1;
    default:
        return 0;
    }
}

void glGetIntegerv(GLenum pname, GLint *params)
{
    uint32_t in = pname;
    int32_t out[16];
    unsigned n;
    static int logged;
    static struct {
        uint32_t pname;
        int32_t v[4];
        uint8_t n;
        uint8_t ok;
    } caps[24];
    unsigned ci;

    memset(out, 0, sizeof(out));
    if (logged < 10) {
        fprintf(stderr, "tspgl: GetIntegerv pname=0x%x params=%p\n", pname,
                (void *)params);
        logged++;
    }
    if (pname == 0x821B) { /* GL_MAJOR_VERSION */
        if (params)
            params[0] = 2;
        return;
    }
    if (pname == 0x821C) { /* GL_MINOR_VERSION */
        if (params)
            params[0] = 0;
        return;
    }
    if (pname == 0x821D) { /* GL_NUM_EXTENSIONS */
        if (params)
            params[0] = 26;
        return;
    }
    if (get_local_int(pname, params))
        return;
    if (gl_limit_pname(pname)) {
        for (ci = 0; ci < 24; ++ci) {
            if (caps[ci].ok && caps[ci].pname == pname) {
                n = gl_param_count(pname);
                if (params)
                    memcpy(params, caps[ci].v, n * 4);
                return;
            }
        }
    }
    tspgl_call(OP_glGetIntegerv, &in, 4, out, sizeof(out));
    n = gl_param_count(pname);
    if (params)
        memcpy(params, out, n * 4);
    if (gl_limit_pname(pname)) {
        for (ci = 0; ci < 24; ++ci) {
            if (!caps[ci].ok) {
                caps[ci].pname = pname;
                memcpy(caps[ci].v, out, 16);
                caps[ci].n = (uint8_t)n;
                caps[ci].ok = 1;
                break;
            }
        }
    }
}

void glGetFloatv(GLenum pname, GLfloat *params)
{
    uint32_t in = pname;
    float out[16];
    unsigned n = gl_param_count(pname);
    unsigned i;
    int32_t iv[4];
    memset(out, 0, sizeof(out));
    if ((pname == 0x0BA2 || pname == 0x0C10) && params) {
        if (get_local_int(pname, iv)) {
            for (i = 0; i < n && i < 4; ++i)
                params[i] = (GLfloat)iv[i];
            return;
        }
    }
    tspgl_call(OP_glGetFloatv, &in, 4, out, sizeof(out));
    if (params)
        memcpy(params, out, n * 4);
}

void glGetBooleanv(GLenum pname, GLboolean *params)
{
    uint32_t in = pname;
    uint8_t out[16];
    unsigned n = gl_param_count(pname);
    memset(out, 0, sizeof(out));
    tspgl_call(OP_glGetBooleanv, &in, 4, out, sizeof(out));
    if (params)
        memcpy(params, out, n);
}

void glGetShaderiv(GLuint shader, GLenum pname, GLint *params)
{
    uint32_t in[2] = { shader, pname };
    int32_t out = 0;
    tspgl_call(OP_glGetShaderiv, in, 8, &out, 4);
    if (params)
        *params = out;
}

void glGetProgramiv(GLuint program, GLenum pname, GLint *params)
{
    uint32_t in[2] = { program, pname };
    int32_t out = 0;
    tspgl_call(OP_glGetProgramiv, in, 8, &out, 4);
    if (params)
        *params = out;
}

void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length,
                        char *infoLog)
{
    uint32_t in[2] = { shader, (uint32_t)bufSize };
    uint32_t cap = bufSize > 0 ? (uint32_t)bufSize : 1;
    char *tmp = malloc(cap + 8);
    uint32_t n = 0;
    if (!tmp)
        return;
    memset(tmp, 0, cap + 8);
    tspgl_call(OP_glGetShaderInfoLog, in, 8, tmp, cap + 4);
    memcpy(&n, tmp, 4);
    if (infoLog && bufSize > 0)
        memcpy(infoLog, tmp + 4, n < (uint32_t)bufSize ? n : (uint32_t)bufSize);
    if (length)
        *length = (GLsizei)n;
    free(tmp);
}

void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length,
                         char *infoLog)
{
    uint32_t in[2] = { program, (uint32_t)bufSize };
    uint32_t cap = bufSize > 0 ? (uint32_t)bufSize : 1;
    char *tmp = malloc(cap + 8);
    uint32_t n = 0;
    if (!tmp)
        return;
    memset(tmp, 0, cap + 8);
    tspgl_call(OP_glGetProgramInfoLog, in, 8, tmp, cap + 4);
    memcpy(&n, tmp, 4);
    if (infoLog && bufSize > 0)
        memcpy(infoLog, tmp + 4, n < (uint32_t)bufSize ? n : (uint32_t)bufSize);
    if (length)
        *length = (GLsizei)n;
    free(tmp);
}

void glShaderSource(GLuint shader, GLsizei count, const char *const *string,
                    const GLint *length)
{
    uint32_t i;
    uint32_t total = 8;
    uint8_t *buf;
    uint8_t *p;
    if (count < 0)
        count = 0;
    for (i = 0; i < (uint32_t)count; ++i) {
        int32_t len = length ? length[i] : -1;
        if (len < 0)
            len = string && string[i] ? (int32_t)strlen(string[i]) : 0;
        total += 4 + (uint32_t)len;
    }
    buf = malloc(total);
    if (!buf)
        return;
    p = buf;
    memcpy(p, &shader, 4);
    p += 4;
    memcpy(p, &count, 4);
    p += 4;
    for (i = 0; i < (uint32_t)count; ++i) {
        int32_t len = length ? length[i] : -1;
        if (len < 0)
            len = string && string[i] ? (int32_t)strlen(string[i]) : 0;
        memcpy(p, &len, 4);
        p += 4;
        if (len && string && string[i])
            memcpy(p, string[i], (uint32_t)len);
        p += (uint32_t)len;
    }
    tspgl_call(OP_glShaderSource, buf, total, NULL, 0);
    free(buf);
}

void glBindAttribLocation(GLuint program, GLuint index, const char *name)
{
    uint32_t nlen = name ? (uint32_t)strlen(name) + 1 : 1;
    uint8_t *buf = malloc(8 + nlen);
    if (!buf)
        return;
    memcpy(buf, &program, 4);
    memcpy(buf + 4, &index, 4);
    if (name)
        memcpy(buf + 8, name, nlen);
    else
        buf[8] = 0;
    tspgl_call(OP_glBindAttribLocation, buf, 8 + nlen, NULL, 0);
    free(buf);
}

GLint glGetAttribLocation(GLuint program, const char *name)
{
    uint32_t nlen = name ? (uint32_t)strlen(name) + 1 : 1;
    uint8_t *buf;
    int32_t out = -1;
    if (loc_lookup(alocs, program, name, &out))
        return out;
    buf = malloc(4 + nlen);
    if (!buf)
        return -1;
    memcpy(buf, &program, 4);
    if (name)
        memcpy(buf + 4, name, nlen);
    else
        buf[4] = 0;
    tspgl_call(OP_glGetAttribLocation, buf, 4 + nlen, &out, 4);
    free(buf);
    loc_store(alocs, program, name, out);
    return out;
}

GLint glGetUniformLocation(GLuint program, const char *name)
{
    uint32_t nlen = name ? (uint32_t)strlen(name) + 1 : 1;
    uint8_t *buf;
    int32_t out = -1;
    if (loc_lookup(ulocs, program, name, &out))
        return out;
    buf = malloc(4 + nlen);
    if (!buf)
        return -1;
    memcpy(buf, &program, 4);
    if (name)
        memcpy(buf + 4, name, nlen);
    else
        buf[4] = 0;
    tspgl_call(OP_glGetUniformLocation, buf, 4 + nlen, &out, 4);
    free(buf);
    loc_store(ulocs, program, name, out);
    return out;
}

void glBufferData(GLenum target, int32_t size, const void *data, GLenum usage)
{
    uint32_t hdr[3] = { target, (uint32_t)size, usage };
    uint8_t *buf;
    uint32_t extra;
    if (size < 0)
        size = 0;
    extra = (data && size) ? (uint32_t)size : 0;
    buf = malloc(12 + extra);
    if (!buf)
        return;
    memcpy(buf, hdr, 12);
    if (extra)
        memcpy(buf + 12, data, extra);
    tspgl_call(OP_glBufferData, buf, 12 + extra, NULL, 0);
    sh_store(target, (uint32_t)size, usage, data);
    free(buf);
}

void glBufferSubData(GLenum target, int32_t offset, int32_t size,
                     const void *data)
{
    uint32_t hdr[3] = { target, (uint32_t)offset, (uint32_t)size };
    uint8_t *buf;
    if (size < 0)
        size = 0;
    buf = malloc(12 + (uint32_t)size);
    if (!buf)
        return;
    memcpy(buf, hdr, 12);
    if (data && size)
        memcpy(buf + 12, data, (uint32_t)size);
    tspgl_call(OP_glBufferSubData, buf, 12 + (uint32_t)size, NULL, 0);
    sh_sub(target, (uint32_t)offset, (uint32_t)size, data);
    free(buf);
}

void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei w,
                  GLsizei h, GLint border, GLenum format, GLenum type,
                  const void *pixels)
{
    uint32_t hdr[8] = { target, (uint32_t)level, (uint32_t)internalformat,
                        (uint32_t)w, (uint32_t)h, (uint32_t)border, format,
                        type };
    unsigned nb = pixels ? image_bytes(w, h, format, type) : 0;
    uint8_t *buf = malloc(32 + nb);
    if (!buf)
        return;
    memcpy(buf, hdr, 32);
    if (pixels && nb)
        memcpy(buf + 32, pixels, nb);
    if (nb > 4u * 1024u * 1024u)
        fprintf(stderr, "tspgl: TexImage2D %dx%d fmt=0x%x type=0x%x bytes=%u\n",
                (int)w, (int)h, format, type, nb);
    tspgl_call(OP_glTexImage2D, buf, 32 + nb, NULL, 0);
    free(buf);
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoff, GLint yoff,
                     GLsizei w, GLsizei h, GLenum format, GLenum type,
                     const void *pixels)
{
    uint32_t hdr[8] = { target, (uint32_t)level, (uint32_t)xoff, (uint32_t)yoff,
                        (uint32_t)w, (uint32_t)h, format, type };
    unsigned nb = pixels ? image_bytes(w, h, format, type) : 0;
    uint8_t *buf = malloc(32 + nb);
    if (!buf)
        return;
    memcpy(buf, hdr, 32);
    if (pixels && nb)
        memcpy(buf + 32, pixels, nb);
    tspgl_call(OP_glTexSubImage2D, buf, 32 + nb, NULL, 0);
    free(buf);
}

void glCompressedTexImage2D(GLenum target, GLint level, GLenum ifmt, GLsizei w,
                            GLsizei h, GLint border, GLsizei imageSize,
                            const void *data)
{
    uint32_t hdr[7] = { target, (uint32_t)level, ifmt, (uint32_t)w, (uint32_t)h,
                        (uint32_t)border, (uint32_t)imageSize };
    uint8_t *buf;
    if (imageSize < 0)
        imageSize = 0;
    buf = malloc(28 + (uint32_t)imageSize);
    if (!buf)
        return;
    memcpy(buf, hdr, 28);
    if (data && imageSize)
        memcpy(buf + 28, data, (uint32_t)imageSize);
    tspgl_call(OP_glCompressedTexImage2D, buf, 28 + (uint32_t)imageSize, NULL, 0);
    free(buf);
}

void glCompressedTexSubImage2D(GLenum target, GLint level, GLint x, GLint y,
                               GLsizei w, GLsizei h, GLenum format,
                               GLsizei imageSize, const void *data)
{
    uint32_t hdr[8] = { target, (uint32_t)level, (uint32_t)x, (uint32_t)y,
                        (uint32_t)w, (uint32_t)h, format, (uint32_t)imageSize };
    uint8_t *buf;
    if (imageSize < 0)
        imageSize = 0;
    buf = malloc(32 + (uint32_t)imageSize);
    if (!buf)
        return;
    memcpy(buf, hdr, 32);
    if (data && imageSize)
        memcpy(buf + 32, data, (uint32_t)imageSize);
    tspgl_call(OP_glCompressedTexSubImage2D, buf, 32 + (uint32_t)imageSize, NULL,
               0);
    free(buf);
}

void glReadPixels(GLint x, GLint y, GLsizei w, GLsizei h, GLenum format,
                  GLenum type, void *pixels)
{
    uint32_t hdr[6] = { (uint32_t)x, (uint32_t)y, (uint32_t)w, (uint32_t)h,
                        format, type };
    unsigned nb = image_bytes(w, h, format, type);
    if (!pixels || !nb)
        return;
    tspgl_call(OP_glReadPixels, hdr, 24, pixels, nb);
}

static int vao_diag_enabled(void)
{
    const char *v;
    if (vao_diag >= 0)
        return vao_diag;
    v = getenv("POSTAL2_VAO_DIAG");
    vao_diag = v && strcmp(v, "0") != 0;
    return vao_diag;
}

static void vao_diag_log(const char *kind, uint32_t a, uint32_t b,
                         uint32_t c, const void *ptr)
{
    if ((!vao_diag_enabled() && !bound_vao) || vao_diag_calls >= 256)
        return;
    fprintf(stderr,
            "P2-VAO n=%u kind=%s vao=%u array=%u element=%u a=%u b=%u c=%u "
            "ptr=%p\\n", vao_diag_calls++, kind, bound_vao, bound_array,
            bound_element, a, b, c, ptr);
}

void glVertexAttribPointer(GLuint index, GLint size, GLenum type,
                           GLboolean normalized, GLsizei stride,
                           const void *ptr)
{
    uint32_t u[6];
    if (index >= 16)
        return;
    attribs[index].size = size;
    attribs[index].type = type;
    attribs[index].normalized = normalized;
    attribs[index].stride = stride;
    attribs[index].ptr = (uint32_t)(uintptr_t)ptr;
    attribs[index].is_offset = bound_array != 0;
    attribs[index].source_buffer = bound_array;
    if (current_vao)
        current_vao->attrib[index] = attribs[index];
    u[0] = index;
    u[1] = (uint32_t)size;
    u[2] = type;
    u[3] = normalized;
    u[4] = (uint32_t)stride;
    u[5] = attribs[index].is_offset ? attribs[index].ptr : 0xffffffffu;
    vao_diag_log("attrib", index, (uint32_t)size, type, ptr);
    tspgl_call(OP_glVertexAttribPointer, u, 24, NULL, 0);
}

static void send_client_arrays(GLint first, GLsizei count)
{
    unsigned i;
    if (bound_vao)
        return;
    for (i = 0; i < 16; ++i) {
        unsigned stride, start, nb, elem;
        const uint8_t *src;
        uint8_t *buf;
        uint32_t hdr[4];
        if (!attribs[i].enabled || attribs[i].is_offset)
            continue;
        elem = (unsigned)attribs[i].size * type_bytes(attribs[i].type);
        stride = attribs[i].stride ? (unsigned)attribs[i].stride : elem;
        start = (unsigned)first * stride;
        nb = count > 0 ? ((unsigned)(count - 1) * stride + elem) : 0;
        if (!nb)
            continue;
        src = (const uint8_t *)(uintptr_t)attribs[i].ptr;
        buf = malloc(16 + nb);
        if (!buf)
            continue;
        hdr[0] = i;
        hdr[1] = stride;
        hdr[2] = (uint32_t)count;
        hdr[3] = nb;
        memcpy(buf, hdr, 16);
        if (src)
            memcpy(buf + 16, src + start, nb);
        tspgl_call(OP_glUploadAttrib, buf, 16 + nb, NULL, 0);
        free(buf);
    }
}

static int has_client_arrays(void)
{
    unsigned i;
    if (bound_vao)
        return 0;
    for (i = 0; i < 16; ++i) {
        if (attribs[i].enabled && !attribs[i].is_offset)
            return 1;
    }
    return 0;
}

static unsigned max_index_value(GLsizei count, GLenum type, const void *idx)
{
    unsigned i, m = 0;
    if (!idx || count <= 0)
        return 0;
    if (type == GL_UNSIGNED_BYTE) {
        const uint8_t *p = idx;
        for (i = 0; i < (unsigned)count; ++i)
            if (p[i] > m)
                m = p[i];
    } else if (type == GL_UNSIGNED_SHORT) {
        const uint16_t *p = idx;
        for (i = 0; i < (unsigned)count; ++i)
            if (p[i] > m)
                m = p[i];
    } else {
        const uint32_t *p = idx;
        for (i = 0; i < (unsigned)count; ++i)
            if (p[i] > m)
                m = p[i];
    }
    return m;
}

void glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    uint32_t u[3] = { mode, (uint32_t)first, (uint32_t)count };
    int client = has_client_arrays();
    vao_diag_log("draw-arrays", (uint32_t)mode, (uint32_t)first,
                 (uint32_t)count, NULL);
    send_client_arrays(first, count);
    if (client)
        u[1] = 0;
    tspgl_call(OP_glDrawArrays, u, 12, NULL, 0);
}

void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *idx)
{
    uint32_t hdr[4];
    unsigned esize = type_bytes(type);
    int indexed = (bound_element != 0);
    unsigned nb = indexed ? 0 : (unsigned)count * esize;
    uint8_t *buf;
    unsigned verts = (unsigned)count;
    vao_diag_log("draw-elements", (uint32_t)mode, (uint32_t)count,
                 (uint32_t)type, idx);
    if (!indexed && idx)
        verts = max_index_value(count, type, idx) + 1u;
    send_client_arrays(0, (GLsizei)verts);
    hdr[0] = mode;
    hdr[1] = (uint32_t)count;
    hdr[2] = type;
    hdr[3] = indexed ? (uint32_t)(uintptr_t)idx : 0xffffffffu;
    buf = malloc(16 + nb);
    if (!buf)
        return;
    memcpy(buf, hdr, 16);
    if (!indexed && idx && nb)
        memcpy(buf + 16, idx, nb);
    tspgl_call(OP_glDrawElements, buf, 16 + nb, NULL, 0);
    free(buf);
}

void glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count,
                       GLsizei primcount)
{
    GLsizei i;
    for (i = 0; i < primcount; ++i) {
        if (count && count[i] > 0)
            glDrawArrays(mode, first ? first[i] : 0, count[i]);
    }
}

void glMultiDrawArraysEXT(GLenum mode, const GLint *first, const GLsizei *count,
                          GLsizei primcount)
{
    glMultiDrawArrays(mode, first, count, primcount);
}

void glGetProgramBinary(GLuint program, GLsizei bufSize, GLsizei *length,
                        GLenum *binaryFormat, void *binary)
{
    (void)program;
    (void)bufSize;
    (void)binary;
    if (length)
        *length = 0;
    if (binaryFormat)
        *binaryFormat = 0;
}

void glGetProgramBinaryOES(GLuint program, GLsizei bufSize, GLsizei *length,
                           GLenum *binaryFormat, void *binary)
{
    glGetProgramBinary(program, bufSize, length, binaryFormat, binary);
}

void glProgramBinary(GLuint program, GLenum binaryFormat, const void *binary,
                     GLsizei length)
{
    (void)program;
    (void)binaryFormat;
    (void)binary;
    (void)length;
}

void glProgramBinaryOES(GLuint program, GLenum binaryFormat, const void *binary,
                        GLsizei length)
{
    glProgramBinary(program, binaryFormat, binary, length);
}

void glDebugMessageCallback(void *callback, const void *userParam)
{
    (void)callback;
    (void)userParam;
}

void glDebugMessageCallbackKHR(void *callback, const void *userParam)
{
    glDebugMessageCallback(callback, userParam);
}

static void uniform_fv(uint32_t op, GLint loc, GLsizei count, unsigned ncomp,
                       const GLfloat *v)
{
    uint32_t hdr[2] = { (uint32_t)loc, (uint32_t)count };
    unsigned nb = (unsigned)count * ncomp * 4u;
    uint8_t *buf = malloc(8 + nb);
    if (!buf)
        return;
    memcpy(buf, hdr, 8);
    if (v && nb)
        memcpy(buf + 8, v, nb);
    tspgl_call(op, buf, 8 + nb, NULL, 0);
    free(buf);
}

void glUniform1fv(GLint l, GLsizei c, const GLfloat *v)
{
    uniform_fv(OP_glUniform1fv, l, c, 1, v);
}
void glUniform2fv(GLint l, GLsizei c, const GLfloat *v)
{
    uniform_fv(OP_glUniform2fv, l, c, 2, v);
}
void glUniform3fv(GLint l, GLsizei c, const GLfloat *v)
{
    uniform_fv(OP_glUniform3fv, l, c, 3, v);
}
void glUniform4fv(GLint l, GLsizei c, const GLfloat *v)
{
    uniform_fv(OP_glUniform4fv, l, c, 4, v);
}

static void uniform_iv(uint32_t op, GLint loc, GLsizei count, unsigned ncomp,
                       const GLint *v)
{
    uint32_t hdr[2] = { (uint32_t)loc, (uint32_t)count };
    unsigned nb = (unsigned)count * ncomp * 4u;
    uint8_t *buf = malloc(8 + nb);
    if (!buf)
        return;
    memcpy(buf, hdr, 8);
    if (v && nb)
        memcpy(buf + 8, v, nb);
    tspgl_call(op, buf, 8 + nb, NULL, 0);
    free(buf);
}

void glUniform1iv(GLint l, GLsizei c, const GLint *v)
{
    uniform_iv(OP_glUniform1iv, l, c, 1, v);
}
void glUniform2iv(GLint l, GLsizei c, const GLint *v)
{
    uniform_iv(OP_glUniform2iv, l, c, 2, v);
}
void glUniform3iv(GLint l, GLsizei c, const GLint *v)
{
    uniform_iv(OP_glUniform3iv, l, c, 3, v);
}
void glUniform4iv(GLint l, GLsizei c, const GLint *v)
{
    uniform_iv(OP_glUniform4iv, l, c, 4, v);
}

static void uniform_mat(uint32_t op, GLint loc, GLsizei count, unsigned n,
                        GLboolean tr, const GLfloat *v)
{
    uint32_t hdr[3] = { (uint32_t)loc, (uint32_t)count, tr };
    unsigned nb = (unsigned)count * n * n * 4u;
    uint8_t *buf = malloc(12 + nb);
    if (!buf)
        return;
    memcpy(buf, hdr, 12);
    if (v && nb)
        memcpy(buf + 12, v, nb);
    tspgl_call(op, buf, 12 + nb, NULL, 0);
    free(buf);
}

void glUniformMatrix2fv(GLint l, GLsizei c, GLboolean t, const GLfloat *v)
{
    uniform_mat(OP_glUniformMatrix2fv, l, c, 2, t, v);
}
void glUniformMatrix3fv(GLint l, GLsizei c, GLboolean t, const GLfloat *v)
{
    uniform_mat(OP_glUniformMatrix3fv, l, c, 3, t, v);
}
void glUniformMatrix4fv(GLint l, GLsizei c, GLboolean t, const GLfloat *v)
{
    uniform_mat(OP_glUniformMatrix4fv, l, c, 4, t, v);
}

void glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    uint32_t in[2] = { target, pname };
    int32_t out = 0;
    tspgl_call(OP_glGetBufferParameteriv, in, 8, &out, 4);
    if (params)
        *params = out;
}

void *glMapBufferOES(GLenum target, GLenum access)
{
    struct shbuf *s = sh_find(sh_bound(target), 1);
    int32_t gl_size = 0;
    (void)access;
    if (!s)
        return NULL;
    if (s->size == 0) {
        glGetBufferParameteriv(target, 0x8764, &gl_size); /* GL_BUFFER_SIZE */
        if (gl_size > 0)
            s->size = (uint32_t)gl_size;
    }
    if (!s->cpu && s->size) {
        s->cpu = malloc(s->size);
        if (s->cpu)
            memset(s->cpu, 0, s->size);
    }
    if (!s->cpu) {
        fprintf(stderr, "tspgl: MapBuffer NULL target=0x%x id=%u size=%u\n",
                target, sh_bound(target), s->size);
        return NULL;
    }
    s->mapped = 1;
    s->map_off = 0;
    s->map_len = s->size;
    return s->cpu;
}

GLboolean glUnmapBufferOES(GLenum target)
{
    struct shbuf *s = sh_find(sh_bound(target), 0);
    if (!s || !s->mapped)
        return 1;
    if (s->cpu && s->map_len)
        glBufferSubData(target, (int32_t)s->map_off, (int32_t)s->map_len,
                        s->cpu + s->map_off);
    s->mapped = 0;
    return 1;
}

void glGetBufferPointervOES(GLenum target, GLenum pname, void **params)
{
    struct shbuf *s = sh_find(sh_bound(target), 0);
    (void)pname;
    if (params)
        *params = (s && s->mapped) ? s->cpu : NULL;
}

void *glMapBufferRange(GLenum target, int32_t offset, int32_t length,
                       GLbitfield access)
{
    struct shbuf *s = sh_find(sh_bound(target), 0);
    (void)access;
    if (offset < 0)
        offset = 0;
    if (length < 0)
        length = 0;
    if (!s) {
        s = sh_find(sh_bound(target), 1);
        if (!s)
            return NULL;
    }
    if (!s->cpu && s->size) {
        s->cpu = malloc(s->size);
        if (s->cpu)
            memset(s->cpu, 0, s->size);
    }
    if (!s->cpu || (uint32_t)offset >= s->size)
        return NULL;
    if ((uint32_t)offset + (uint32_t)length > s->size)
        length = (int32_t)(s->size - (uint32_t)offset);
    s->mapped = 1;
    s->map_off = (uint32_t)offset;
    s->map_len = (uint32_t)length;
    return s->cpu + offset;
}

void glFlushMappedBufferRange(GLenum target, int32_t offset, int32_t length)
{
    struct shbuf *s = sh_find(sh_bound(target), 0);
    if (!s || !s->mapped || !s->cpu)
        return;
    if (offset < 0)
        offset = 0;
    if (length < 0)
        length = 0;
    glBufferSubData(target, offset, length, s->cpu + offset);
}

void *glMapBuffer(GLenum target, GLenum access)
{
    return glMapBufferOES(target, access);
}

GLboolean glUnmapBuffer(GLenum target)
{
    return glUnmapBufferOES(target);
}

void glBindVertexArray(GLuint array)
{
    uint32_t u = array;
    current_vao = find_vao(array, 1);
    if (current_vao) {
        memcpy(attribs, current_vao->attrib, sizeof(attribs));
        bound_element = current_vao->element_buffer;
    } else {
        bound_element = 0;
    }
    bound_vao = array;
    tspgl_call(OP_glBindVertexArray, &u, 4, NULL, 0);
}

void glBindVertexArrayOES(GLuint array)
{
    glBindVertexArray(array);
}

GLboolean glIsVertexArray(GLuint array)
{
    uint32_t u = array;
    uint32_t out = 0;
    tspgl_call(OP_glIsVertexArray, &u, 4, &out, 4);
    return (GLboolean)out;
}

GLboolean glIsVertexArrayOES(GLuint array)
{
    return glIsVertexArray(array);
}

void glBindBuffer(GLenum target, GLuint buffer)
{
    uint32_t u[2] = { target, buffer };
    if (target == GL_ARRAY_BUFFER)
        bound_array = buffer;
    if (target == GL_ELEMENT_ARRAY_BUFFER) {
        bound_element = buffer;
        if (current_vao)
            current_vao->element_buffer = buffer;
    }
    tspgl_call(OP_glBindBuffer, u, 8, NULL, 0);
}

void glEnableVertexAttribArray(GLuint index)
{
    uint32_t u = index;
    if (index < 16) {
        attribs[index].enabled = 1;
        if (current_vao)
            current_vao->attrib[index].enabled = 1;
    }
    tspgl_call(OP_glEnableVertexAttribArray, &u, 4, NULL, 0);
}

void glDisableVertexAttribArray(GLuint index)
{
    uint32_t u = index;
    if (index < 16) {
        attribs[index].enabled = 0;
        if (current_vao)
            current_vao->attrib[index].enabled = 0;
    }
    tspgl_call(OP_glDisableVertexAttribArray, &u, 4, NULL, 0);
}

void glPixelStorei(GLenum pname, GLint param)
{
    uint32_t u[2] = { pname, (uint32_t)param };
    if (pname == GL_UNPACK_ALIGNMENT)
        unpack_align = param > 0 ? param : 4;
    tspgl_call(OP_glPixelStorei, u, 8, NULL, 0);
}

void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    uint32_t hdr[2] = { target, pname };
    uint8_t buf[8 + 16];
    memcpy(buf, hdr, 8);
    if (params)
        memcpy(buf + 8, params, 16);
    tspgl_call(OP_glTexParameterfv, buf, 8 + 16, NULL, 0);
}

void glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    uint32_t hdr[2] = { target, pname };
    uint8_t buf[8 + 16];
    memcpy(buf, hdr, 8);
    if (params)
        memcpy(buf + 8, params, 16);
    tspgl_call(OP_glTexParameteriv, buf, 8 + 16, NULL, 0);
}

static void attrib_fv(uint32_t op, GLuint index, unsigned n, const GLfloat *v)
{
    uint32_t hdr = index;
    uint8_t buf[4 + 16];
    memcpy(buf, &hdr, 4);
    if (v)
        memcpy(buf + 4, v, n * 4u);
    tspgl_call(op, buf, 4 + n * 4u, NULL, 0);
}

void glVertexAttrib1fv(GLuint i, const GLfloat *v) { attrib_fv(OP_glVertexAttrib1fv, i, 1, v); }
void glVertexAttrib2fv(GLuint i, const GLfloat *v) { attrib_fv(OP_glVertexAttrib2fv, i, 2, v); }
void glVertexAttrib3fv(GLuint i, const GLfloat *v) { attrib_fv(OP_glVertexAttrib3fv, i, 3, v); }
void glVertexAttrib4fv(GLuint i, const GLfloat *v) { attrib_fv(OP_glVertexAttrib4fv, i, 4, v); }

void glInvalidateFramebuffer(GLenum target, GLsizei n, const GLenum *attachments)
{
    (void)target;
    (void)n;
    (void)attachments;
}

void glDiscardFramebufferEXT(GLenum target, GLsizei n, const GLenum *attachments)
{
    glInvalidateFramebuffer(target, n, attachments);
}

static void get_active(uint32_t op, GLuint program, GLuint index, GLsizei bufSize,
                       GLsizei *length, GLint *size, GLenum *type, char *name)
{
    uint32_t in[3] = { program, index, (uint32_t)(bufSize > 0 ? bufSize : 1) };
    uint32_t cap = in[2];
    uint8_t *tmp = malloc(12 + cap + 1);
    int32_t hdr[3];
    if (!tmp)
        return;
    memset(tmp, 0, 12 + cap + 1);
    tspgl_call(op, in, 12, tmp, 12 + cap);
    memcpy(hdr, tmp, 12);
    if (length)
        *length = hdr[0];
    if (size)
        *size = hdr[1];
    if (type)
        *type = (GLenum)hdr[2];
    if (name && bufSize > 0) {
        uint32_t n = (uint32_t)bufSize;
        memcpy(name, tmp + 12, n);
        name[n - 1] = 0;
    }
    free(tmp);
}

void glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize,
                       GLsizei *length, GLint *size, GLenum *type, char *name)
{
    get_active(OP_glGetActiveAttrib, program, index, bufSize, length, size,
               type, name);
}

void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize,
                        GLsizei *length, GLint *size, GLenum *type, char *name)
{
    get_active(OP_glGetActiveUniform, program, index, bufSize, length, size,
               type, name);
}

void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count,
                          GLuint *shaders)
{
    uint32_t in[2] = { program, (uint32_t)maxCount };
    uint32_t tmp[1 + 64];
    uint32_t n;
    memset(tmp, 0, sizeof(tmp));
    tspgl_call(OP_glGetAttachedShaders, in, 8, tmp, sizeof(tmp));
    n = tmp[0];
    if (n > 64)
        n = 64;
    if (count)
        *count = (GLsizei)n;
    if (shaders && maxCount > 0)
        memcpy(shaders, tmp + 1, (n < (uint32_t)maxCount ? n : (uint32_t)maxCount) * 4u);
}

void glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment,
                                           GLenum pname, GLint *params)
{
    uint32_t in[3] = { target, attachment, pname };
    int32_t out = 0;
    tspgl_call(OP_glGetFramebufferAttachmentParameteriv, in, 12, &out, 4);
    if (params)
        *params = out;
}

void glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params)
{
    uint32_t in[2] = { target, pname };
    int32_t out = 0;
    tspgl_call(OP_glGetRenderbufferParameteriv, in, 8, &out, 4);
    if (params)
        *params = out;
}

void glGetShaderPrecisionFormat(GLenum shaderType, GLenum precisionType,
                                GLint *range, GLint *precision)
{
    uint32_t in[2] = { shaderType, precisionType };
    int32_t out[3] = { 0, 0, 0 };
    tspgl_call(OP_glGetShaderPrecisionFormat, in, 8, out, 12);
    if (range) {
        range[0] = out[0];
        range[1] = out[1];
    }
    if (precision)
        *precision = out[2];
}

void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length,
                       char *source)
{
    uint32_t in[2] = { shader, (uint32_t)(bufSize > 0 ? bufSize : 1) };
    uint32_t cap = in[1];
    uint8_t *tmp = malloc(4 + cap);
    uint32_t n = 0;
    if (!tmp)
        return;
    memset(tmp, 0, 4 + cap);
    tspgl_call(OP_glGetShaderSource, in, 8, tmp, 4 + cap);
    memcpy(&n, tmp, 4);
    if (source && bufSize > 0)
        memcpy(source, tmp + 4, n < (uint32_t)bufSize ? n : (uint32_t)bufSize);
    if (length)
        *length = (GLsizei)n;
    free(tmp);
}

void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
    uint32_t in[2] = { target, pname };
    float out[4];
    memset(out, 0, sizeof(out));
    tspgl_call(OP_glGetTexParameterfv, in, 8, out, sizeof(out));
    if (params)
        memcpy(params, out, 4);
}

void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
    uint32_t in[2] = { target, pname };
    int32_t out[4];
    memset(out, 0, sizeof(out));
    tspgl_call(OP_glGetTexParameteriv, in, 8, out, sizeof(out));
    if (params)
        memcpy(params, out, 4);
}

void glGetUniformfv(GLuint program, GLint location, GLfloat *params)
{
    uint32_t in[2] = { program, (uint32_t)location };
    float out[16];
    memset(out, 0, sizeof(out));
    tspgl_call(OP_glGetUniformfv, in, 8, out, sizeof(out));
    if (params)
        memcpy(params, out, sizeof(out));
}

void glGetUniformiv(GLuint program, GLint location, GLint *params)
{
    uint32_t in[2] = { program, (uint32_t)location };
    int32_t out[16];
    memset(out, 0, sizeof(out));
    tspgl_call(OP_glGetUniformiv, in, 8, out, sizeof(out));
    if (params)
        memcpy(params, out, sizeof(out));
}

void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params)
{
    uint32_t in[2] = { index, pname };
    float out[4];
    memset(out, 0, sizeof(out));
    tspgl_call(OP_glGetVertexAttribfv, in, 8, out, sizeof(out));
    if (params)
        memcpy(params, out, pname == 0x8626 ? 16 : 4);
}

void glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params)
{
    uint32_t in[2] = { index, pname };
    int32_t out[4];
    memset(out, 0, sizeof(out));
    tspgl_call(OP_glGetVertexAttribiv, in, 8, out, sizeof(out));
    if (params)
        memcpy(params, out, pname == 0x8626 ? 16 : 4);
}

void glGetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer)
{
    (void)pname;
    if (!pointer)
        return;
    if (index < 16)
        *pointer = (void *)(uintptr_t)attribs[index].ptr;
    else
        *pointer = NULL;
}

void glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryformat,
                    const void *binary, GLsizei length)
{
    (void)count;
    (void)shaders;
    (void)binaryformat;
    (void)binary;
    (void)length;
}

void *glGetProcAddress(const char *name);

/* ---- EGL (local fakes + swap) ---- */
#define EGL_DEFAULT_DISPLAY ((void *)0)
#define EGL_NO_DISPLAY ((void *)0)
#define EGL_NO_CONTEXT ((void *)0)
#define EGL_NO_SURFACE ((void *)0)
#define EGL_SUCCESS 0x3000
#define EGL_OPENGL_ES_API 0x30A0
#define EGL_WIDTH 0x3057
#define EGL_HEIGHT 0x3056
#define EGL_NONE 0x3038
#define EGL_PBUFFER_BIT 0x0001
#define EGL_WINDOW_BIT 0x0004
#define EGL_OPENGL_ES2_BIT 0x0004
#define EGL_OPENGL_ES_BIT 0x0001
#define EGL_SURFACE_TYPE 0x3033
#define EGL_RENDERABLE_TYPE 0x3040
#define EGL_RED_SIZE 0x3024
#define EGL_GREEN_SIZE 0x3023
#define EGL_BLUE_SIZE 0x3022
#define EGL_ALPHA_SIZE 0x3021
#define EGL_DEPTH_SIZE 0x3025
#define EGL_STENCIL_SIZE 0x3026
#define EGL_BUFFER_SIZE 0x3020
#define EGL_COLOR_BUFFER_TYPE 0x303F
#define EGL_RGB_BUFFER 0x308E
#define EGL_NATIVE_VISUAL_ID 0x302E
#define EGL_CONTEXT_CLIENT_VERSION 0x3098

static int egl_err = EGL_SUCCESS;
static int egl_inited;
static void *fake_dpy = (void *)0x10;
static void *fake_cfg = (void *)0x20;
static unsigned egl_next_handle = 0x30;
static void *cur_ctx = EGL_NO_CONTEXT;
static void *cur_draw = EGL_NO_SURFACE;
static void *cur_read = EGL_NO_SURFACE;

static void *egl_new_handle(void)
{
    egl_next_handle += 0x10;
    return (void *)(uintptr_t)egl_next_handle;
}

void *eglGetDisplay(void *native)
{
    (void)native;
    return fake_dpy;
}

unsigned eglGetError(void)
{
    int e = egl_err;
    egl_err = EGL_SUCCESS;
    return (unsigned)e;
}

unsigned eglInitialize(void *dpy, int *maj, int *min)
{
    (void)dpy;
    egl_inited = 1;
    if (maj)
        *maj = 1;
    if (min)
        *min = 4;
    tspgl_call(OP_PING, NULL, 0, NULL, 0);
    return 1;
}

unsigned eglTerminate(void *dpy)
{
    (void)dpy;
    return 1;
}

unsigned eglBindAPI(unsigned api)
{
    (void)api;
    return 1;
}

unsigned eglQueryAPI(void)
{
    return EGL_OPENGL_ES_API;
}

const char *eglQueryString(void *dpy, int name)
{
    (void)dpy;
    if (name == 0x3053) /* EGL_VENDOR */
        return "tsp-glbridge";
    if (name == 0x3054) /* EGL_VERSION */
        return "1.4";
    if (name == 0x3055) /* EGL_EXTENSIONS */
        return "EGL_EXT_client_extensions EGL_EXT_device_base "
               "EGL_EXT_device_enumeration EGL_EXT_device_query "
               "EGL_EXT_platform_base EGL_EXT_platform_device "
               "EGL_KHR_fence_sync EGL_KHR_wait_sync "
               "EGL_KHR_image EGL_KHR_surfaceless_context "
               "EGL_ANDROID_native_fence_sync";
    if (name == 0x308D) /* EGL_CLIENT_APIS */
        return "OpenGL_ES";
    return "";
}

unsigned eglGetConfigs(void *dpy, void **cfgs, int sz, int *n)
{
    (void)dpy;
    if (n)
        *n = 1;
    if (cfgs && sz > 0)
        cfgs[0] = fake_cfg;
    return 1;
}

unsigned eglChooseConfig(void *dpy, const int *attr, void **cfgs, int sz,
                         int *n)
{
    (void)dpy;
    (void)attr;
    if (n)
        *n = 1;
    if (cfgs && sz > 0)
        cfgs[0] = fake_cfg;
    return 1;
}

unsigned eglGetConfigAttrib(void *dpy, void *cfg, int attr, int *value)
{
    (void)dpy;
    (void)cfg;
    if (!value)
        return 0;
    switch (attr) {
    case EGL_RED_SIZE:
    case EGL_GREEN_SIZE:
    case EGL_BLUE_SIZE:
    case EGL_ALPHA_SIZE:
        *value = 8;
        break;
    case EGL_DEPTH_SIZE:
        *value = 16;
        break;
    case EGL_STENCIL_SIZE:
        *value = 8;
        break;
    case EGL_BUFFER_SIZE:
        *value = 32;
        break;
    case EGL_SURFACE_TYPE:
        *value = EGL_WINDOW_BIT | EGL_PBUFFER_BIT;
        break;
    case EGL_RENDERABLE_TYPE:
        *value = EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES_BIT | 0x40;
        break;
    case EGL_COLOR_BUFFER_TYPE:
        *value = EGL_RGB_BUFFER;
        break;
    case EGL_NATIVE_VISUAL_ID:
        *value = 0;
        break;
    default:
        *value = 0;
        break;
    }
    return 1;
}

void *eglCreateWindowSurface(void *dpy, void *cfg, void *win, const int *attr)
{
    (void)dpy;
    (void)cfg;
    (void)win;
    (void)attr;
    return egl_new_handle();
}

void *eglCreatePbufferSurface(void *dpy, void *cfg, const int *attr)
{
    (void)dpy;
    (void)cfg;
    (void)attr;
    return egl_new_handle();
}

unsigned eglDestroySurface(void *dpy, void *surf)
{
    (void)dpy;
    if (cur_draw == surf)
        cur_draw = EGL_NO_SURFACE;
    if (cur_read == surf)
        cur_read = EGL_NO_SURFACE;
    return 1;
}

void *eglCreateContext(void *dpy, void *cfg, void *share, const int *attr)
{
    (void)dpy;
    (void)cfg;
    (void)share;
    (void)attr;
    return egl_new_handle();
}

unsigned eglDestroyContext(void *dpy, void *ctx)
{
    (void)dpy;
    if (cur_ctx == ctx)
        cur_ctx = EGL_NO_CONTEXT;
    return 1;
}

unsigned eglMakeCurrent(void *dpy, void *draw, void *read, void *ctx)
{
    (void)dpy;
    cur_draw = draw;
    cur_read = read;
    cur_ctx = ctx;
    return 1;
}

void *eglGetCurrentContext(void)
{
    return cur_ctx;
}
void *eglGetCurrentSurface(int readdraw)
{
    if (readdraw == 0x305A) /* EGL_READ */
        return cur_read;
    return cur_draw;
}
void *eglGetCurrentDisplay(void)
{
    return cur_ctx ? fake_dpy : EGL_NO_DISPLAY;
}

static int tspgl_dimension(const char *name, int fallback)
{
    const char *text = getenv(name);
    char *end = NULL;
    long value;

    if (!text || !*text)
        return fallback;
    value = strtol(text, &end, 10);
    if (end == text || *end != '\0' || value < 1 || value > 1280)
        return fallback;
    return (int)value;
}

unsigned eglQuerySurface(void *dpy, void *surf, int attr, int *value)
{
    (void)dpy;
    (void)surf;
    if (!value)
        return 0;
    if (attr == EGL_WIDTH)
        *value = tspgl_dimension("TSPGL_WIDTH", 1024);
    else if (attr == EGL_HEIGHT)
        *value = tspgl_dimension("TSPGL_HEIGHT", 768);
    else
        *value = 0;
    return 1;
}

unsigned eglSwapInterval(void *dpy, int interval)
{
    (void)dpy;
    (void)interval;
    return 1;
}

unsigned eglSwapBuffers(void *dpy, void *surf)
{
    (void)dpy;
    (void)surf;
    return tspgl_call(OP_EGL_SWAP, NULL, 0, NULL, 0) == 0;
}

unsigned eglWaitClient(void)
{
    return 1;
}
unsigned eglWaitGL(void)
{
    return 1;
}
unsigned eglWaitNative(int engine)
{
    (void)engine;
    return 1;
}

unsigned eglReleaseThread(void)
{
    return 1;
}

unsigned eglSurfaceAttrib(void *dpy, void *surf, int attr, int value)
{
    (void)dpy;
    (void)surf;
    (void)attr;
    (void)value;
    return 1;
}

static void *fake_dev = (void *)0x51;
static void *fake_sync = (void *)0x61;

unsigned eglQueryDevicesEXT(int max_devices, void **devices, int *num_devices)
{
    if (num_devices)
        *num_devices = 1;
    if (devices && max_devices > 0)
        devices[0] = fake_dev;
    return 1;
}

const char *eglQueryDeviceStringEXT(void *device, int name)
{
    (void)device;
    if (name == 0x3055)
        return "EGL_EXT_device_drm";
    return "";
}

void *eglGetPlatformDisplayEXT(unsigned platform, void *native,
                               const intptr_t *attrib)
{
    (void)platform;
    (void)native;
    (void)attrib;
    return fake_dpy;
}

void *eglGetPlatformDisplay(unsigned platform, void *native,
                            const intptr_t *attrib)
{
    return eglGetPlatformDisplayEXT(platform, native, attrib);
}

void *eglCreateSyncKHR(void *dpy, unsigned type, const intptr_t *attrib)
{
    (void)dpy;
    (void)type;
    (void)attrib;
    return fake_sync;
}

void *eglCreateSync(void *dpy, unsigned type, const intptr_t *attrib)
{
    return eglCreateSyncKHR(dpy, type, attrib);
}

unsigned eglDestroySyncKHR(void *dpy, void *sync)
{
    (void)dpy;
    (void)sync;
    return 1;
}

unsigned eglDestroySync(void *dpy, void *sync)
{
    return eglDestroySyncKHR(dpy, sync);
}

unsigned eglClientWaitSyncKHR(void *dpy, void *sync, unsigned flags,
                              uint64_t timeout)
{
    (void)dpy;
    (void)sync;
    (void)flags;
    (void)timeout;
    return 0x30F6; /* EGL_CONDITION_SATISFIED_KHR */
}

unsigned eglClientWaitSync(void *dpy, void *sync, unsigned flags,
                           uint64_t timeout)
{
    return eglClientWaitSyncKHR(dpy, sync, flags, timeout);
}

unsigned eglWaitSyncKHR(void *dpy, void *sync, unsigned flags)
{
    (void)dpy;
    (void)sync;
    (void)flags;
    return 1;
}

unsigned eglWaitSync(void *dpy, void *sync, unsigned flags)
{
    return eglWaitSyncKHR(dpy, sync, flags);
}

unsigned eglGetSyncAttribKHR(void *dpy, void *sync, int attr, intptr_t *value)
{
    (void)dpy;
    (void)sync;
    if (!value)
        return 0;
    if (attr == 0x30F1) /* EGL_SYNC_STATUS_KHR */
        *value = 0x30F2; /* EGL_SIGNALED_KHR */
    else
        *value = 0;
    return 1;
}

int eglDupNativeFenceFDANDROID(void *dpy, void *sync)
{
    (void)dpy;
    (void)sync;
    return -1;
}

void *eglCreateImageKHR(void *dpy, void *ctx, unsigned target, void *buffer,
                        const intptr_t *attrib)
{
    (void)dpy;
    (void)ctx;
    (void)target;
    (void)buffer;
    (void)attrib;
    return (void *)0x71;
}

unsigned eglDestroyImageKHR(void *dpy, void *image)
{
    (void)dpy;
    (void)image;
    return 1;
}

struct namefn {
    const char *name;
    void *fn;
};

void *eglGetProcAddress(const char *name);

#define FN(n) { #n, (void *)n }

static const struct namefn procs[] = {
    FN(glActiveTexture), FN(glAttachShader), FN(glBindBuffer),
    FN(glBindFramebuffer), FN(glBindRenderbuffer), FN(glBindTexture),
    FN(glBlendColor), FN(glBlendEquation), FN(glBlendEquationSeparate),
    FN(glBlendFunc), FN(glBlendFuncSeparate), FN(glBufferData),
    FN(glBufferSubData), FN(glCheckFramebufferStatus), FN(glClear),
    FN(glClearColor), FN(glClearDepthf), FN(glClearStencil), FN(glColorMask),
    FN(glCompileShader), FN(glCompressedTexImage2D), FN(glCompressedTexSubImage2D),
    FN(glCopyTexImage2D), FN(glCopyTexSubImage2D), FN(glCreateProgram),
    FN(glCreateShader), FN(glCullFace), FN(glDeleteBuffers),
    FN(glDeleteFramebuffers), FN(glDeleteProgram), FN(glDeleteRenderbuffers),
    FN(glDeleteShader), FN(glDeleteTextures), FN(glDepthFunc), FN(glDepthMask),
    FN(glDepthRangef), FN(glDetachShader), FN(glDisable),
    FN(glDisableVertexAttribArray), FN(glDrawArrays), FN(glDrawElements),
    FN(glMultiDrawArrays), FN(glMultiDrawArraysEXT),
    FN(glGetProgramBinary), FN(glGetProgramBinaryOES),
    FN(glProgramBinary), FN(glProgramBinaryOES),
    FN(glDebugMessageCallback), FN(glDebugMessageCallbackKHR),
    FN(glEnable), FN(glEnableVertexAttribArray), FN(glFinish), FN(glFlush),
    FN(glFramebufferRenderbuffer), FN(glFramebufferTexture2D), FN(glFrontFace),
    FN(glGenBuffers), FN(glGenerateMipmap), FN(glGenFramebuffers),
    FN(glGenRenderbuffers), FN(glGenTextures), FN(glGetAttribLocation),
    FN(glGetBooleanv), FN(glGetBufferParameteriv), FN(glGetError),
    FN(glGetFloatv), FN(glGetIntegerv), FN(glGetProgramiv),
    FN(glGetProgramInfoLog), FN(glGetShaderiv), FN(glGetShaderInfoLog),
    FN(glGetString), FN(glGetUniformLocation), FN(glHint), FN(glIsBuffer),
    FN(glIsEnabled), FN(glIsFramebuffer), FN(glIsProgram), FN(glIsRenderbuffer),
    FN(glIsShader), FN(glIsTexture), FN(glLineWidth), FN(glLinkProgram),
    FN(glPixelStorei), FN(glPolygonOffset), FN(glReadPixels),
    FN(glReleaseShaderCompiler), FN(glRenderbufferStorage), FN(glSampleCoverage),
    FN(glScissor), FN(glShaderSource), FN(glStencilFunc),
    FN(glStencilFuncSeparate), FN(glStencilMask), FN(glStencilMaskSeparate),
    FN(glStencilOp), FN(glStencilOpSeparate), FN(glTexImage2D),
    FN(glTexParameterf), FN(glTexParameteri), FN(glTexParameterfv),
    FN(glTexParameteriv), FN(glTexSubImage2D),
    FN(glUniform1f), FN(glUniform2f), FN(glUniform3f), FN(glUniform4f),
    FN(glUniform1i), FN(glUniform2i), FN(glUniform3i), FN(glUniform4i),
    FN(glUniform1fv), FN(glUniform2fv), FN(glUniform3fv), FN(glUniform4fv),
    FN(glUniform1iv), FN(glUniform2iv), FN(glUniform3iv), FN(glUniform4iv),
    FN(glUniformMatrix2fv), FN(glUniformMatrix3fv), FN(glUniformMatrix4fv),
    FN(glUseProgram), FN(glValidateProgram),     FN(glVertexAttrib1f), FN(glVertexAttrib1fv), FN(glVertexAttrib2f),
    FN(glVertexAttrib2fv), FN(glVertexAttrib3f), FN(glVertexAttrib3fv),
    FN(glVertexAttrib4f), FN(glVertexAttrib4fv),
    FN(glVertexAttribPointer), FN(glViewport), FN(glBlitFramebuffer),
    FN(glBindAttribLocation), FN(glMapBufferOES), FN(glUnmapBufferOES),
    FN(glMapBuffer), FN(glUnmapBuffer), FN(glMapBufferRange),
    FN(glFlushMappedBufferRange), FN(glGetBufferPointervOES),
    FN(glGenVertexArrays), FN(glGenVertexArraysOES),
    FN(glDeleteVertexArrays), FN(glDeleteVertexArraysOES),
    FN(glBindVertexArray), FN(glBindVertexArrayOES),
    FN(glIsVertexArray), FN(glIsVertexArrayOES), FN(glGetStringi),
    FN(glGetBufferPointervOES), FN(glMapBufferRange),
    FN(glInvalidateFramebuffer), FN(glDiscardFramebufferEXT),
    FN(glGetActiveAttrib), FN(glGetActiveUniform), FN(glGetAttachedShaders),
    FN(glGetFramebufferAttachmentParameteriv), FN(glGetRenderbufferParameteriv),
    FN(glGetShaderPrecisionFormat), FN(glGetShaderSource),
    FN(glGetTexParameterfv), FN(glGetTexParameteriv),
    FN(glGetUniformfv), FN(glGetUniformiv),
    FN(glGetVertexAttribfv), FN(glGetVertexAttribiv),
    FN(glGetVertexAttribPointerv), FN(glShaderBinary),
    FN(glGetProcAddress),
    FN(eglGetDisplay), FN(eglInitialize), FN(eglTerminate), FN(eglGetError),
    FN(eglChooseConfig), FN(eglGetConfigs), FN(eglGetConfigAttrib),
    FN(eglCreateWindowSurface), FN(eglCreatePbufferSurface),
    FN(eglDestroySurface), FN(eglCreateContext), FN(eglDestroyContext),
    FN(eglMakeCurrent), FN(eglSwapBuffers), FN(eglSwapInterval),
    FN(eglQueryString), FN(eglBindAPI), FN(eglGetProcAddress),
    FN(eglQueryDevicesEXT), FN(eglQueryDeviceStringEXT),
    FN(eglGetPlatformDisplayEXT), FN(eglGetPlatformDisplay),
    FN(eglCreateSyncKHR), FN(eglCreateSync), FN(eglDestroySyncKHR),
    FN(eglDestroySync), FN(eglClientWaitSyncKHR), FN(eglClientWaitSync),
    FN(eglWaitSyncKHR), FN(eglWaitSync), FN(eglGetSyncAttribKHR),
    FN(eglDupNativeFenceFDANDROID), FN(eglCreateImageKHR), FN(eglDestroyImageKHR),
    FN(eglReleaseThread), FN(eglSurfaceAttrib),
    { NULL, NULL }
};

void *eglGetProcAddress(const char *name)
{
    size_t i;
    if (!name)
        return NULL;
    for (i = 0; procs[i].name; ++i) {
        if (strcmp(procs[i].name, name) == 0)
            return procs[i].fn;
    }
    fprintf(stderr, "tspgl: missing %s\n", name);
    return NULL;
}

void *glGetProcAddress(const char *name)
{
    return eglGetProcAddress(name);
}

__attribute__((constructor))
static void tspgl_ctor(void)
{
    tspgl_shared();
    fprintf(stderr, "tspgl: 32-bit GLES client loaded\n");
}
