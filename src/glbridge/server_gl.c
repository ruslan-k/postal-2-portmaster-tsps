/* GLES specials for the 64-bit PowerVR server. Included from server.c. */

#define GL_ARRAY_BUFFER 0x8892
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_STREAM_DRAW 0x88E0

struct srv_attrib {
    int32_t size;
    uint32_t type;
    uint32_t normalized;
    int32_t stride;
    int is_offset;
    uint32_t source_buffer;
    uint8_t enabled;
};
struct srv_vao {
    uint32_t id;
    uint32_t element_buffer;
    struct srv_attrib attrib[16];
};
static struct srv_vao svao[32];
static struct srv_vao *srv_current_vao;
static struct srv_attrib sattr[16];
static uint32_t srv_current_vao_id;
static uint32_t scratch_vbo[16][3];
static uint8_t scratch_slot[16];
static uint32_t scratch_ibo;
static uint32_t srv_bound_array;
static uint32_t srv_bound_element;

static uint64_t upload_hash(const uint8_t *p, uint32_t n)
{
    uint64_t h = 1469598103934665603ULL;
    uint32_t i;
    for (i = 0; i < n; ++i) {
        h ^= p[i];
        h *= 1099511628211ULL;
    }
    return h;
}

static void classifier_rewrite(char *src, size_t *total)
{
    const char *mode = getenv("POSTAL2_FRAGMENT_CLASSIFIER");
    const char *replacement;
    char *line, *end;
    size_t oldlen, newlen;
    if (!mode || strcmp(mode, "0") == 0 ||
        !strstr(src, "v_colour0") || !strstr(src, "texture2D"))
        return;
    if (strcmp(mode, "solid") == 0)
        replacement = "gl_FragColor = vec4(1.0,0.0,1.0,1.0);";
    else if (strcmp(mode, "texture") == 0)
        replacement = "gl_FragColor = color;";
    else if (strcmp(mode, "colour") == 0)
        replacement = "gl_FragColor = v_colour0;";
    else
        return;
    line = strstr(src, "gl_FragColor");
    end = line ? strchr(line, '\n') : NULL;
    if (!line || !end)
        return;
    oldlen = (size_t)(end - line);
    newlen = strlen(replacement);
    if (newlen > oldlen)
        memmove(line + newlen, end, *total - (size_t)(end - src) + 1u);
    else if (newlen < oldlen)
        memmove(line + newlen, end, *total - (size_t)(end - src) + 1u);
    memcpy(line, replacement, newlen);
    *total = *total - oldlen + newlen;
}

static int need(uint32_t n, uint32_t have)
{
    return have >= n;
}

static const void *stage_blob(const uint8_t *in, uint32_t nbytes, uint32_t off)
{
    if (nbytes <= off)
        return NULL;
    return tspgl_stage(in + off, nbytes - off);
}

static int sh_ident(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') || c == '_';
}

static int sh_has(const char *a, const char *b, const char *n)
{
    size_t nt = strlen(n);
    for (; a + nt <= b; a++)
        if (strncmp(a, n, nt) == 0)
            return 1;
    return 0;
}

static int sh_line_kind(const char *a, const char *b)
{
    const char *eq;
    while (a < b && (*a == ' ' || *a == '\t'))
        a++;
    if (a >= b || *a == '\n' || *a == '\r')
        return 1;
    if (a + 1 < b && a[0] == '/' && a[1] == '/')
        return 1;
    if (*a == '#') {
        if (a + 8 <= b && strncmp(a, "#version", 8) == 0)
            return 2;
        if (a + 10 <= b && strncmp(a, "#extension", 10) == 0 &&
            sh_has(a, b, "framebuffer_fetch"))
            return 3;
        if (a + 10 <= b && strncmp(a, "#extension", 10) == 0 &&
            sh_has(a, b, "GL_IMG_uniform_buffer_object"))
            return 6;
        return 1;
    }
    if (a + 9 <= b && strncmp(a, "precision", 9) == 0 && sh_has(a, b, "float")) {
        const char *semi;

        for (semi = a; semi < b && *semi != ';'; ++semi) {
        }
        if (semi < b) {
            const char *rest = semi + 1;

            while (rest < b && (*rest == ' ' || *rest == '\t' || *rest == '\r'))
                rest++;
            if (rest < b && *rest != '/' && *rest != '\n')
                return 0;
        }
        return 4;
    }
    if (!sh_has(a, b, "gl_LastFragData"))
        return 0;
    for (eq = a; eq < b; eq++)
        if (*eq == '=')
            return 0;
    return 5;
}

static void sh_put(char **buf, size_t *len, size_t *cap, const char *s, size_t n)
{
    if (*len + n + 1 > *cap) {
        size_t nc = *cap ? *cap * 2 : 4096;
        char *nb;
        while (nc < *len + n + 1)
            nc *= 2;
        nb = realloc(*buf, nc);
        if (!nb)
            return;
        *buf = nb;
        *cap = nc;
    }
    memcpy(*buf + *len, s, n);
    *len += n;
    (*buf)[*len] = 0;
}

static int sh_tok(const char *s, const char *line, const char *end,
                  const char *tok, size_t nt)
{
    if ((size_t)(end - s) < nt)
        return 0;
    if (strncmp(s, tok, nt) != 0)
        return 0;
    if (s > line && sh_ident(s[-1]))
        return 0;
    if (s + nt < end && sh_ident(s[nt]))
        return 0;
    return 1;
}

static char *rewrite_gles2_shader(const char *src, size_t slen, int *out_len)
{
    int is_frag, use_fetch, to_es3, strip_img_ubo;
    const char *p, *end;
    char *out = NULL;
    size_t len = 0, cap = 0;
    static int dump;
    const char *last_rep;
    size_t last_n;

    if (!src)
        return NULL;
    end = src + slen;
    is_frag = strstr(src, "gl_FragColor") || strstr(src, "gl_LastFragData") ||
              strstr(src, "gl_FragData");
    use_fetch = strstr(src, "gl_LastFragData") != NULL ||
                strstr(src, "framebuffer_fetch") != NULL;
    strip_img_ubo = sh_has(src, end, "GL_IMG_uniform_buffer_object");
    if (strstr(src, "#version 300") && !strip_img_ubo) {
        *out_len = (int)slen;
        return NULL;
    }
    /*
     * Only promote GLES2 → 300 es when the shader actually uses framebuffer
     * fetch. GOF2 HD packs uniforms on the same line as `precision`, and the
     * NFS rewriter would drop that whole line (main() missing / undeclared
     * varyings). PowerVR still compiles #version 100.
     */
    to_es3 = use_fetch && (have_fb_fetch || have_fb_fetch_nc);

    last_rep = (have_fb_fetch || have_fb_fetch_nc) ? "tspgl_fb" : "vec4(1.0)";
    last_n = strlen(last_rep);

    if (to_es3) {
        sh_put(&out, &len, &cap, "#version 300 es\n", 16);
        if (is_frag && use_fetch) {
            if (have_fb_fetch)
                sh_put(&out, &len, &cap,
                       "#extension GL_EXT_shader_framebuffer_fetch : require\n",
                       53);
            else
                sh_put(&out, &len, &cap,
                       "#extension GL_EXT_shader_framebuffer_fetch_non_coherent : require\n",
                       66);
            sh_put(&out, &len, &cap, "precision highp float;\n", 23);
            if (have_fb_fetch)
                sh_put(&out, &len, &cap,
                       "layout(location=0) inout highp vec4 tspgl_fb;\n", 45);
            else
                sh_put(&out, &len, &cap,
                       "layout(noncoherent, location=0) inout highp vec4 tspgl_fb;\n",
                       58);
        } else if (is_frag) {
            sh_put(&out, &len, &cap, "precision highp float;\n", 23);
            sh_put(&out, &len, &cap,
                   "layout(location=0) out highp vec4 tspgl_fb;\n", 43);
        } else {
            sh_put(&out, &len, &cap, "precision highp float;\n", 23);
        }
    } else if (use_fetch && !(have_fb_fetch || have_fb_fetch_nc)) {
        static int once;
        if (!once) {
            fprintf(stderr,
                    "tspgl-srv: no FB fetch on GPU, LastFragData -> vec4(1.0)\n");
            once = 1;
        }
    } else if (!use_fetch && !strip_img_ubo) {
        *out_len = (int)slen;
        return NULL;
    }

    p = src;
    while (p < end) {
        const char *eol = p;
        int kind;
        while (eol < end && *eol != '\n')
            eol++;
        kind = sh_line_kind(p, eol);
        if (kind == 6) {
            p = (eol < end) ? eol + 1 : eol;
            continue;
        }
        if (to_es3 && kind >= 2 && kind <= 5) {
            p = (eol < end) ? eol + 1 : eol;
            continue;
        }
        if (!to_es3 && (kind == 3 || kind == 5)) {
            p = (eol < end) ? eol + 1 : eol;
            continue;
        }
        {
            const char *q = p;
            while (q < eol) {
                size_t left = (size_t)(eol - q);
                if (to_es3 && left >= 19 &&
                    sh_tok(q, p, eol, "texture2DProjLodEXT", 19)) {
                    sh_put(&out, &len, &cap, "textureProjLod", 14);
                    q += 19;
                    continue;
                }
                if (to_es3 && left >= 16 && sh_tok(q, p, eol, "texture2DProjLod", 16)) {
                    sh_put(&out, &len, &cap, "textureProjLod", 14);
                    q += 16;
                    continue;
                }
                if (to_es3 && left >= 15 && sh_tok(q, p, eol, "texture2DLodEXT", 15)) {
                    sh_put(&out, &len, &cap, "textureLod", 10);
                    q += 15;
                    continue;
                }
                if (to_es3 && left >= 12 && sh_tok(q, p, eol, "texture2DLod", 12)) {
                    sh_put(&out, &len, &cap, "textureLod", 10);
                    q += 12;
                    continue;
                }
                if (to_es3 && left >= 13 && sh_tok(q, p, eol, "texture2DProj", 13)) {
                    sh_put(&out, &len, &cap, "textureProj", 11);
                    q += 13;
                    continue;
                }
                if (to_es3 && left >= 17 && sh_tok(q, p, eol, "textureCubeLodEXT", 17)) {
                    sh_put(&out, &len, &cap, "textureLod", 10);
                    q += 17;
                    continue;
                }
                if (to_es3 && left >= 9 && sh_tok(q, p, eol, "texture2D", 9)) {
                    sh_put(&out, &len, &cap, "texture", 7);
                    q += 9;
                    continue;
                }
                if (to_es3 && left >= 11 && sh_tok(q, p, eol, "textureCube", 11)) {
                    sh_put(&out, &len, &cap, "texture", 7);
                    q += 11;
                    continue;
                }
                if (left >= 15 && sh_tok(q, p, eol, "gl_LastFragData", 15)) {
                    sh_put(&out, &len, &cap, last_rep, last_n);
                    q += 15;
                    if (q < eol && *q == '[') {
                        int depth = 1;
                        q++;
                        while (q < eol && depth) {
                            if (*q == '[')
                                depth++;
                            else if (*q == ']')
                                depth--;
                            q++;
                        }
                    }
                    continue;
                }
                if (to_es3 && is_frag && left >= 12 &&
                    sh_tok(q, p, eol, "gl_FragColor", 12)) {
                    sh_put(&out, &len, &cap, "tspgl_fb", 8);
                    q += 12;
                    continue;
                }
                if (to_es3 && is_frag && left >= 11 &&
                    sh_tok(q, p, eol, "gl_FragData", 11)) {
                    sh_put(&out, &len, &cap, "tspgl_fb", 8);
                    q += 11;
                    if (q < eol && *q == '[') {
                        int depth = 1;
                        q++;
                        while (q < eol && depth) {
                            if (*q == '[')
                                depth++;
                            else if (*q == ']')
                                depth--;
                            q++;
                        }
                    }
                    continue;
                }
                if (to_es3 && left >= 9 && sh_tok(q, p, eol, "attribute", 9)) {
                    sh_put(&out, &len, &cap, "in", 2);
                    q += 9;
                    continue;
                }
                if (to_es3 && left >= 7 && sh_tok(q, p, eol, "varying", 7)) {
                    sh_put(&out, &len, &cap, is_frag ? "in" : "out",
                           is_frag ? 2 : 3);
                    q += 7;
                    continue;
                }
                sh_put(&out, &len, &cap, q, 1);
                q++;
            }
        }
        if (eol < end) {
            sh_put(&out, &len, &cap, "\n", 1);
            p = eol + 1;
        } else
            p = eol;
    }

    if (use_fetch && dump < 3 && out) {
        fprintf(stderr,
                "tspgl-srv: shader rewrite fetch=%d es3=%d bytes=%u\n%.360s\n",
                use_fetch, to_es3, (unsigned)len, out);
        dump++;
    }
    *out_len = (int)len;
    return out;
}

static int tspgl_dispatch_special(uint32_t op, const uint8_t *in, uint32_t nbytes,
                                  uint8_t *out, uint32_t *out_n)
{
    const uint32_t *u = (const uint32_t *)in;
    static int final_dump;
    *out_n = 0;

    switch (op) {
    case OP_glGetString: {
        const uint8_t *(*fn)(uint32_t) = (void *)G.glGetString;
        const uint8_t *s;
        uint32_t n;
        if (!need(4, nbytes) || !fn)
            return -1;
        s = fn(u[0]);
        if (!s)
            s = (const uint8_t *)"";
        n = (uint32_t)strlen((const char *)s) + 1u;
        if (n > 4095)
            n = 4095;
        memcpy(out, s, n);
        out[n] = 0;
        *out_n = n + 1;
        return 0;
    }
    case OP_glGetIntegerv: {
        void (*fn)(uint32_t, int32_t *) = (void *)G.glGetIntegerv;
        if (!need(4, nbytes) || !fn)
            return -1;
        memset(out, 0, 64);
        fn(u[0], (int32_t *)out);
        *out_n = 64;
        return 0;
    }
    case OP_glGetFloatv: {
        void (*fn)(uint32_t, float *) = (void *)G.glGetFloatv;
        if (!need(4, nbytes) || !fn)
            return -1;
        memset(out, 0, 64);
        fn(u[0], (float *)out);
        *out_n = 64;
        return 0;
    }
    case OP_glGetBooleanv: {
        void (*fn)(uint32_t, uint8_t *) = (void *)G.glGetBooleanv;
        if (!need(4, nbytes) || !fn)
            return -1;
        memset(out, 0, 16);
        fn(u[0], out);
        *out_n = 16;
        return 0;
    }
    case OP_glGetShaderiv: {
        void (*fn)(uint32_t, uint32_t, int32_t *) = (void *)G.glGetShaderiv;
        int32_t v = 0;
        if (!need(8, nbytes) || !fn)
            return -1;
        fn(u[0], u[1], &v);
        memcpy(out, &v, 4);
        *out_n = 4;
        return 0;
    }
    case OP_glGetProgramiv: {
        void (*fn)(uint32_t, uint32_t, int32_t *) = (void *)G.glGetProgramiv;
        int32_t v = 0;
        if (!need(8, nbytes) || !fn)
            return -1;
        fn(u[0], u[1], &v);
        memcpy(out, &v, 4);
        *out_n = 4;
        return 0;
    }
    case OP_glGetShaderInfoLog:
    case OP_glGetProgramInfoLog: {
        void (*fn)(uint32_t, int32_t, int32_t *, char *) =
            op == OP_glGetShaderInfoLog ? (void *)G.glGetShaderInfoLog
                                        : (void *)G.glGetProgramInfoLog;
        int32_t cap, len = 0;
        if (!need(8, nbytes) || !fn)
            return -1;
        cap = (int32_t)u[1];
        if (cap < 1)
            cap = 1;
        if ((uint32_t)cap + 4u > TSPGL_MAX_BLOB)
            cap = (int32_t)(TSPGL_MAX_BLOB - 4);
        fn(u[0], cap, &len, (char *)(out + 4));
        if (len < 0)
            len = 0;
        memcpy(out, &len, 4);
        *out_n = 4u + (uint32_t)len;
        return 0;
    }
    case OP_glShaderSource: {
        void (*fn)(uint32_t, int32_t, const char *const *, const int32_t *) =
            (void *)G.glShaderSource;
        uint32_t shader, count, i;
        const uint8_t *p;
        size_t total = 0;
        char *src;
        const char *one;
        int32_t one_len;
        static const char prefix[] = "#version 100\n";
        int have_ver = 0;
        if (!need(8, nbytes) || !fn)
            return -1;
        shader = u[0];
        count = u[1];
        if (count > 64)
            count = 64;
        p = in + 8;
        for (i = 0; i < count; ++i) {
            int32_t len;
            if ((size_t)(p - in) + 4 > nbytes)
                break;
            memcpy(&len, p, 4);
            p += 4;
            if (len < 0)
                len = 0;
            if ((size_t)(p - in) + (size_t)len > nbytes)
                len = (int32_t)(nbytes - (uint32_t)(p - in));
            total += (size_t)len;
            p += (uint32_t)len;
        }
        if (count > 1)
            total += (size_t)(count - 1);
        src = malloc(total + sizeof(prefix) + 128);
        if (!src)
            return -1;
        p = in + 8;
        total = 0;
        for (i = 0; i < count; ++i) {
            int32_t len;
            if ((size_t)(p - in) + 4 > nbytes)
                break;
            memcpy(&len, p, 4);
            p += 4;
            if (len < 0)
                len = 0;
            if ((size_t)(p - in) + (size_t)len > nbytes)
                len = (int32_t)(nbytes - (uint32_t)(p - in));
            memcpy(src + total, p, (size_t)len);
            total += (size_t)len;
            p += (uint32_t)len;
            if (i + 1 < count)
                src[total++] = '\n';
        }
        src[total] = 0;
        {
            static int sourced;
            if (sourced < 8) {
                fprintf(stderr, "tspgl-srv: glShaderSource id=%u bytes=%u\n%.400s\n",
                        shader, (unsigned)total, src);
                sourced++;
            }
        }
        {
            const char *q = src;
            while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n')
                q++;
            if (q[0] == '#' && strncmp(q, "#version", 8) == 0)
                have_ver = 1;
        }
        if (!have_ver) {
            memmove(src + sizeof(prefix) - 1, src, total + 1);
            memcpy(src, prefix, sizeof(prefix) - 1);
            total += sizeof(prefix) - 1;
        }
        {
            int nlen = 0;
            char *rw = rewrite_gles2_shader(src, total, &nlen);
            if (rw) {
                free(src);
                src = rw;
                total = (size_t)nlen;
            }
        }
        classifier_rewrite(src, &total);
        if (gl_diag_enabled() && final_dump < 8) {
            fprintf(stderr, "P2-GL shader-final: id=%u bytes=%u\n%s\n",
                    shader, (unsigned)total, src);
            final_dump++;
        }
        one = src;
        one_len = (int32_t)total;
        fn(shader, 1, &one, &one_len);
        free(src);
        return 0;
    }
    case OP_glBindAttribLocation: {
        void (*fn)(uint32_t, uint32_t, const char *) = (void *)G.glBindAttribLocation;
        if (!need(9, nbytes) || !fn)
            return -1;
        fn(u[0], u[1], (const char *)(in + 8));
        return 0;
    }
    case OP_glGetAttribLocation:
    case OP_glGetUniformLocation: {
        int32_t (*fn)(uint32_t, const char *) =
            op == OP_glGetAttribLocation ? (void *)G.glGetAttribLocation
                                         : (void *)G.glGetUniformLocation;
        int32_t loc;
        if (!need(5, nbytes) || !fn)
            return -1;
        loc = fn(u[0], (const char *)(in + 4));
        memcpy(out, &loc, 4);
        *out_n = 4;
        return 0;
    }
    case OP_glBufferData: {
        void (*fn)(uint32_t, intptr_t, const void *, uint32_t) =
            (void *)G.glBufferData;
        const void *data;
        int32_t size;
        if (!need(12, nbytes) || !fn)
            return -1;
        size = (int32_t)u[1];
        data = (nbytes > 12) ? stage_blob(in, nbytes, 12) : NULL;
        if (getenv("POSTAL2_INDEX_DIAG") && u[0] == GL_ELEMENT_ARRAY_BUFFER &&
            size > 0 && nbytes >= 12u + (uint32_t)size)
            fprintf(stderr, "P2-EBO-DATA target=0x%x size=%d usage=0x%x "
                    "hash=%016llx\\n", u[0], size, u[2],
                    (unsigned long long)upload_hash(in + 12, (uint32_t)size));
        fn(u[0], (intptr_t)size, data, u[2]);
        return 0;
    }
    case OP_glBufferSubData: {
        void (*fn)(uint32_t, intptr_t, intptr_t, const void *) =
            (void *)G.glBufferSubData;
        int32_t size;
        if (!need(12, nbytes) || !fn)
            return -1;
        size = (int32_t)u[2];
        fn(u[0], (intptr_t)(int32_t)u[1], (intptr_t)size,
           nbytes > 12 ? stage_blob(in, nbytes, 12) : NULL);
        return 0;
    }
    case OP_glTexImage2D: {
        void (*fn)(uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t,
                   uint32_t, uint32_t, const void *) = (void *)G.glTexImage2D;
        if (!need(32, nbytes) || !fn)
            return -1;
        fn(u[0], (int32_t)u[1], (int32_t)u[2], (int32_t)u[3], (int32_t)u[4],
           (int32_t)u[5], u[6], u[7], stage_blob(in, nbytes, 32));
        return 0;
    }
    case OP_glTexSubImage2D: {
        void (*fn)(uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t,
                   uint32_t, uint32_t, const void *) = (void *)G.glTexSubImage2D;
        if (!need(32, nbytes) || !fn)
            return -1;
        fn(u[0], (int32_t)u[1], (int32_t)u[2], (int32_t)u[3], (int32_t)u[4],
           (int32_t)u[5], u[6], u[7], stage_blob(in, nbytes, 32));
        return 0;
    }
    case OP_glCompressedTexImage2D: {
        void (*fn)(uint32_t, int32_t, uint32_t, int32_t, int32_t, int32_t,
                   int32_t, const void *) = (void *)G.glCompressedTexImage2D;
        if (!need(28, nbytes) || !fn)
            return -1;
        fn(u[0], (int32_t)u[1], u[2], (int32_t)u[3], (int32_t)u[4],
           (int32_t)u[5], (int32_t)u[6], stage_blob(in, nbytes, 28));
        return 0;
    }
    case OP_glCompressedTexSubImage2D: {
        void (*fn)(uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t,
                   uint32_t, int32_t, const void *) =
            (void *)G.glCompressedTexSubImage2D;
        if (!need(32, nbytes) || !fn)
            return -1;
        fn(u[0], (int32_t)u[1], (int32_t)u[2], (int32_t)u[3], (int32_t)u[4],
           (int32_t)u[5], u[6], (int32_t)u[7], stage_blob(in, nbytes, 32));
        return 0;
    }
    case OP_glReadPixels: {
        void (*fn)(int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t,
                   void *) = (void *)G.glReadPixels;
        int32_t w, h;
        uint32_t nb;
        if (!need(24, nbytes) || !fn)
            return -1;
        w = (int32_t)u[2];
        h = (int32_t)u[3];
        if (w < 0)
            w = 0;
        if (h < 0)
            h = 0;
        nb = (uint32_t)w * (uint32_t)h * 4u;
        if (nb > TSPGL_MAX_BLOB)
            nb = TSPGL_MAX_BLOB;
        fn((int32_t)u[0], (int32_t)u[1], w, h, u[4], u[5], out);
        *out_n = nb;
        return 0;
    }
    case OP_glVertexAttribPointer: {
        void (*fn)(uint32_t, int32_t, uint32_t, uint8_t, int32_t, const void *) =
            (void *)G.glVertexAttribPointer;
        uint32_t index;
        if (!need(24, nbytes) || !fn)
            return -1;
        index = u[0];
        if (index >= 16)
            return 0;
        sattr[index].size = (int32_t)u[1];
        sattr[index].type = u[2];
        sattr[index].normalized = u[3];
        sattr[index].stride = (int32_t)u[4];
        sattr[index].is_offset = (u[5] != 0xffffffffu);
        sattr[index].source_buffer = srv_bound_array;
        if (srv_current_vao)
            memcpy(srv_current_vao->attrib, sattr, sizeof(sattr));
        if (sattr[index].is_offset)
            fn(index, sattr[index].size, sattr[index].type,
               (uint8_t)sattr[index].normalized, sattr[index].stride,
               (const void *)(uintptr_t)u[5]);
        return 0;
    }
    case OP_glUploadAttrib: {
        void (*bind)(uint32_t, uint32_t) = (void *)G.glBindBuffer;
        void (*data)(uint32_t, intptr_t, const void *, uint32_t) =
            (void *)G.glBufferData;
        void (*vap)(uint32_t, int32_t, uint32_t, uint8_t, int32_t, const void *) =
            (void *)G.glVertexAttribPointer;
        void (*gen)(int32_t, uint32_t *) = G.glGenBuffers;
        uint32_t index, nb, slot;
        const char *ring = getenv("POSTAL2_SCRATCH_RING");
        if (!need(16, nbytes) || !bind || !data || !vap || !gen)
            return -1;
        index = u[0];
        nb = u[3];
        if (index >= 16)
            return 0;
        if (nb > nbytes - 16)
            nb = nbytes - 16;
        if (getenv("POSTAL2_FULL_PAYLOAD_DIAG") &&
            (index == 0 || index == 2 || index == 4))
            fprintf(stderr, "P2-ATTR-FULL index=%u bytes=%u hash=%016llx\\n",
                    index, nb,
                    (unsigned long long)upload_hash(in + 16, nb));
        slot = ring && strcmp(ring, "0") != 0 ? scratch_slot[index]++ % 3u : 0u;
        if (!scratch_vbo[index][slot])
            gen(1, &scratch_vbo[index][slot]);
        bind(GL_ARRAY_BUFFER, scratch_vbo[index][slot]);
        data(GL_ARRAY_BUFFER, (intptr_t)nb, stage_blob(in, nbytes, 16),
             GL_STREAM_DRAW);
        vap(index, sattr[index].size ? sattr[index].size : 4, sattr[index].type,
            (uint8_t)sattr[index].normalized, (int32_t)u[1], (const void *)0);
        bind(GL_ARRAY_BUFFER, srv_bound_array);
        return 0;
    }
    case OP_glDrawArrays: {
        void (*fn)(uint32_t, int32_t, int32_t) = (void *)G.glDrawArrays;
        if (!need(12, nbytes) || !fn)
            return -1;
        fn(u[0], (int32_t)u[1], (int32_t)u[2]);
        return 0;
    }
    case OP_glDrawElements: {
        void (*fn)(uint32_t, int32_t, uint32_t, const void *) =
            (void *)G.glDrawElements;
        void (*bind)(uint32_t, uint32_t) = (void *)G.glBindBuffer;
        void (*data)(uint32_t, intptr_t, const void *, uint32_t) =
            (void *)G.glBufferData;
        void (*gen)(int32_t, uint32_t *) = G.glGenBuffers;
        uint32_t nb;
        if (!need(16, nbytes) || !fn)
            return -1;
        if (srv_current_vao && srv_current_vao->element_buffer != 0)
            srv_bound_element = srv_current_vao->element_buffer;
        if (getenv("POSTAL2_INDEX_DIAG"))
            fprintf(stderr, "P2-DRAW-ELEMENTS mode=0x%x count=%u type=0x%x "
                    "index=%x bound_element=%u input_bytes=%u\\n", u[0],
                    u[1], u[2], u[3], srv_bound_element, nbytes);
        if (u[3] == 0xffffffffu) {
            nb = nbytes - 16;
            if (getenv("POSTAL2_FULL_PAYLOAD_DIAG")) {
                const uint8_t *idx = in + 16;
                uint32_t first = nb >= 2 ? idx[0] | ((uint32_t)idx[1] << 8) : 0;
                uint32_t last = nb >= 2 ? idx[nb - 2] | ((uint32_t)idx[nb - 1] << 8) : 0;
                fprintf(stderr, "P2-INDEX-FULL mode=0x%x count=%u type=0x%x "
                        "bytes=%u hash=%016llx first=%u last=%u\\n", u[0],
                        u[1], u[2], nb,
                        (unsigned long long)upload_hash(idx, nb), first, last);
            }
            if (!scratch_ibo && gen)
                gen(1, &scratch_ibo);
            if (bind && data && scratch_ibo) {
                bind(GL_ELEMENT_ARRAY_BUFFER, scratch_ibo);
                data(GL_ELEMENT_ARRAY_BUFFER, (intptr_t)nb,
                     stage_blob(in, nbytes, 16), GL_STREAM_DRAW);
                fn(u[0], (int32_t)u[1], u[2], (const void *)0);
                bind(GL_ELEMENT_ARRAY_BUFFER, srv_bound_element);
                return 0;
            }
        }
        fn(u[0], (int32_t)u[1], u[2], (const void *)(uintptr_t)u[3]);
        return 0;
    }
    case OP_glUniform1fv:
    case OP_glUniform2fv:
    case OP_glUniform3fv:
    case OP_glUniform4fv: {
        void (*fn)(int32_t, int32_t, const float *) =
            op == OP_glUniform1fv   ? (void *)G.glUniform1fv
            : op == OP_glUniform2fv ? (void *)G.glUniform2fv
            : op == OP_glUniform3fv ? (void *)G.glUniform3fv
                                    : (void *)G.glUniform4fv;
        if (!need(8, nbytes) || !fn)
            return -1;
        fn((int32_t)u[0], (int32_t)u[1], (const float *)tspgl_stage(in + 8, nbytes - 8));
        return 0;
    }
    case OP_glUniform1iv:
    case OP_glUniform2iv:
    case OP_glUniform3iv:
    case OP_glUniform4iv: {
        void (*fn)(int32_t, int32_t, const int32_t *) =
            op == OP_glUniform1iv   ? (void *)G.glUniform1iv
            : op == OP_glUniform2iv ? (void *)G.glUniform2iv
            : op == OP_glUniform3iv ? (void *)G.glUniform3iv
                                    : (void *)G.glUniform4iv;
        if (!need(8, nbytes) || !fn)
            return -1;
        fn((int32_t)u[0], (int32_t)u[1], (const int32_t *)tspgl_stage(in + 8, nbytes - 8));
        return 0;
    }
    case OP_glUniformMatrix2fv:
    case OP_glUniformMatrix3fv:
    case OP_glUniformMatrix4fv: {
        void (*fn)(int32_t, int32_t, uint8_t, const float *) =
            op == OP_glUniformMatrix2fv   ? (void *)G.glUniformMatrix2fv
            : op == OP_glUniformMatrix3fv ? (void *)G.glUniformMatrix3fv
                                          : (void *)G.glUniformMatrix4fv;
        if (!need(12, nbytes) || !fn)
            return -1;
        fn((int32_t)u[0], (int32_t)u[1], (uint8_t)u[2],
           (const float *)tspgl_stage(in + 12, nbytes - 12));
        return 0;
    }
    case OP_glGetBufferParameteriv: {
        void (*fn)(uint32_t, uint32_t, int32_t *) =
            (void *)G.glGetBufferParameteriv;
        int32_t v = 0;
        if (!need(8, nbytes) || !fn)
            return -1;
        fn(u[0], u[1], &v);
        memcpy(out, &v, 4);
        *out_n = 4;
        return 0;
    }
    case OP_glBindBuffer: {
        void (*fn)(uint32_t, uint32_t) = (void *)G.glBindBuffer;
        if (!need(8, nbytes) || !fn)
            return -1;
        if (u[0] == GL_ARRAY_BUFFER)
            srv_bound_array = u[1];
        if (u[0] == GL_ELEMENT_ARRAY_BUFFER) {
            srv_bound_element = u[1];
            if (srv_current_vao)
                srv_current_vao->element_buffer = u[1];
        }
        fn(u[0], u[1]);
        return 0;
    }
    case OP_glEnableVertexAttribArray: {
        void (*fn)(uint32_t) = (void *)G.glEnableVertexAttribArray;
        if (!need(4, nbytes) || !fn)
            return -1;
        fn(u[0]);
        return 0;
    }
    case OP_glDisableVertexAttribArray: {
        void (*fn)(uint32_t) = (void *)G.glDisableVertexAttribArray;
        if (!need(4, nbytes) || !fn)
            return -1;
        fn(u[0]);
        return 0;
    }
    case OP_glPixelStorei: {
        void (*fn)(uint32_t, int32_t) = (void *)G.glPixelStorei;
        if (!need(8, nbytes) || !fn)
            return -1;
        fn(u[0], (int32_t)u[1]);
        return 0;
    }
    case OP_glTexParameterfv: {
        void (*fn)(uint32_t, uint32_t, const float *) = (void *)G.glTexParameterfv;
        if (!need(8, nbytes) || !fn)
            return -1;
        fn(u[0], u[1], (const float *)(in + 8));
        return 0;
    }
    case OP_glTexParameteriv: {
        void (*fn)(uint32_t, uint32_t, const int32_t *) =
            (void *)G.glTexParameteriv;
        if (!need(8, nbytes) || !fn)
            return -1;
        fn(u[0], u[1], (const int32_t *)(in + 8));
        return 0;
    }
    case OP_glVertexAttrib1fv:
    case OP_glVertexAttrib2fv:
    case OP_glVertexAttrib3fv:
    case OP_glVertexAttrib4fv: {
        void (*fn)(uint32_t, const float *) =
            op == OP_glVertexAttrib1fv   ? (void *)G.glVertexAttrib1fv
            : op == OP_glVertexAttrib2fv ? (void *)G.glVertexAttrib2fv
            : op == OP_glVertexAttrib3fv ? (void *)G.glVertexAttrib3fv
                                         : (void *)G.glVertexAttrib4fv;
        if (!need(4, nbytes) || !fn)
            return -1;
        fn(u[0], (const float *)(in + 4));
        return 0;
    }
    case OP_glGetActiveAttrib:
    case OP_glGetActiveUniform: {
        void (*fn)(uint32_t, uint32_t, int32_t, int32_t *, int32_t *, uint32_t *,
                   char *) = op == OP_glGetActiveAttrib
                                 ? (void *)G.glGetActiveAttrib
                                 : (void *)G.glGetActiveUniform;
        int32_t bufSize, length = 0, size = 0;
        uint32_t type = 0;
        char *name;
        if (!need(12, nbytes) || !fn)
            return -1;
        bufSize = (int32_t)u[2];
        if (bufSize < 1)
            bufSize = 1;
        if ((uint32_t)bufSize + 12u > TSPGL_MAX_BLOB)
            bufSize = (int32_t)(TSPGL_MAX_BLOB - 12);
        name = (char *)(out + 12);
        memset(name, 0, (uint32_t)bufSize);
        fn(u[0], u[1], bufSize, &length, &size, &type, name);
        memcpy(out, &length, 4);
        memcpy(out + 4, &size, 4);
        memcpy(out + 8, &type, 4);
        *out_n = 12u + (uint32_t)bufSize;
        return 0;
    }
    case OP_glGetAttachedShaders: {
        void (*fn)(uint32_t, int32_t, int32_t *, uint32_t *) =
            (void *)G.glGetAttachedShaders;
        int32_t maxc, count = 0;
        uint32_t shaders[64];
        if (!need(8, nbytes) || !fn)
            return -1;
        maxc = (int32_t)u[1];
        if (maxc > 64)
            maxc = 64;
        if (maxc < 0)
            maxc = 0;
        fn(u[0], maxc, &count, shaders);
        memcpy(out, &count, 4);
        if (count > 0)
            memcpy(out + 4, shaders, (uint32_t)count * 4u);
        *out_n = 4u + (uint32_t)count * 4u;
        return 0;
    }
    case OP_glGetFramebufferAttachmentParameteriv: {
        void (*fn)(uint32_t, uint32_t, uint32_t, int32_t *) =
            (void *)G.glGetFramebufferAttachmentParameteriv;
        int32_t v = 0;
        if (!need(12, nbytes) || !fn)
            return -1;
        fn(u[0], u[1], u[2], &v);
        memcpy(out, &v, 4);
        *out_n = 4;
        return 0;
    }
    case OP_glGetRenderbufferParameteriv: {
        void (*fn)(uint32_t, uint32_t, int32_t *) =
            (void *)G.glGetRenderbufferParameteriv;
        int32_t v = 0;
        if (!need(8, nbytes) || !fn)
            return -1;
        fn(u[0], u[1], &v);
        memcpy(out, &v, 4);
        *out_n = 4;
        return 0;
    }
    case OP_glGetShaderPrecisionFormat: {
        void (*fn)(uint32_t, uint32_t, int32_t *, int32_t *) =
            (void *)G.glGetShaderPrecisionFormat;
        int32_t range[2] = { 0, 0 };
        int32_t prec = 0;
        if (!need(8, nbytes) || !fn)
            return -1;
        fn(u[0], u[1], range, &prec);
        memcpy(out, range, 8);
        memcpy(out + 8, &prec, 4);
        *out_n = 12;
        return 0;
    }
    case OP_glGetShaderSource: {
        void (*fn)(uint32_t, int32_t, int32_t *, char *) =
            (void *)G.glGetShaderSource;
        int32_t cap, len = 0;
        if (!need(8, nbytes) || !fn)
            return -1;
        cap = (int32_t)u[1];
        if (cap < 1)
            cap = 1;
        if ((uint32_t)cap + 4u > TSPGL_MAX_BLOB)
            cap = (int32_t)(TSPGL_MAX_BLOB - 4);
        fn(u[0], cap, &len, (char *)(out + 4));
        memcpy(out, &len, 4);
        *out_n = 4u + (uint32_t)(len > 0 ? len : 0);
        return 0;
    }
    case OP_glGetTexParameterfv: {
        void (*fn)(uint32_t, uint32_t, float *) = (void *)G.glGetTexParameterfv;
        if (!need(8, nbytes) || !fn)
            return -1;
        memset(out, 0, 16);
        fn(u[0], u[1], (float *)out);
        *out_n = 16;
        return 0;
    }
    case OP_glGetTexParameteriv: {
        void (*fn)(uint32_t, uint32_t, int32_t *) = (void *)G.glGetTexParameteriv;
        if (!need(8, nbytes) || !fn)
            return -1;
        memset(out, 0, 16);
        fn(u[0], u[1], (int32_t *)out);
        *out_n = 16;
        return 0;
    }
    case OP_glGetUniformfv: {
        void (*fn)(uint32_t, int32_t, float *) = (void *)G.glGetUniformfv;
        if (!need(8, nbytes) || !fn)
            return -1;
        memset(out, 0, 64);
        fn(u[0], (int32_t)u[1], (float *)out);
        *out_n = 64;
        return 0;
    }
    case OP_glGetUniformiv: {
        void (*fn)(uint32_t, int32_t, int32_t *) = (void *)G.glGetUniformiv;
        if (!need(8, nbytes) || !fn)
            return -1;
        memset(out, 0, 64);
        fn(u[0], (int32_t)u[1], (int32_t *)out);
        *out_n = 64;
        return 0;
    }
    case OP_glGetVertexAttribfv: {
        void (*fn)(uint32_t, uint32_t, float *) = (void *)G.glGetVertexAttribfv;
        if (!need(8, nbytes) || !fn)
            return -1;
        memset(out, 0, 16);
        fn(u[0], u[1], (float *)out);
        *out_n = 16;
        return 0;
    }
    case OP_glGetVertexAttribiv: {
        void (*fn)(uint32_t, uint32_t, int32_t *) = (void *)G.glGetVertexAttribiv;
        if (!need(8, nbytes) || !fn)
            return -1;
        memset(out, 0, 16);
        fn(u[0], u[1], (int32_t *)out);
        *out_n = 16;
        return 0;
    }
    case OP_glBindVertexArray: {
        void (*fn)(uint32_t) = (void *)G.glBindVertexArray;
        uint32_t id;
        if (!need(4, nbytes) || !fn)
            return -1;
        id = u[0];
        if (id < 32) {
            if (svao[id].id == 0)
                svao[id].id = id;
            srv_current_vao_id = id;
            srv_current_vao = &svao[id];
            memcpy(sattr, srv_current_vao->attrib, sizeof(sattr));
            srv_bound_element = srv_current_vao->element_buffer;
        }
        fn(id);
        return 0;
    }
    case OP_glIsVertexArray: {
        uint32_t (*fn)(uint32_t) = (void *)G.glIsVertexArray;
        uint32_t v = 0;
        if (!need(4, nbytes))
            return -1;
        if (fn)
            v = fn(u[0]);
        memcpy(out, &v, 4);
        *out_n = 4;
        return 0;
    }
    default:
        return -1;
    }
}
