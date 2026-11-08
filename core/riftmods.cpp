// pemuat mod .rift: scan folder mods, ekstrak zip, baca mod.json, dlopen mod.so
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "riftmods.h"

extern "C" int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list ap);
extern "C" void A64HookFunction(void *const symbol, void *const replace, void **result);

#ifndef MEDIA_DIR
#define MEDIA_DIR "/storage/emulated/0/Android/media/com.rift.launcher/"
#endif
#ifndef CACHE_DIR
#define CACHE_DIR "/data/user/0/com.rift.launcher/files/modcache/"
#endif

struct RiftApi {
    int version;
    void (*log)(const char *fmt, ...);
    void *(*find_symbol)(const char *mangled);
    int (*hook)(void *target, void *replacement, void **original);
    const char *mod_dir;
};

static void *g_h;
static ModInfo g_mods[MAX_MODS];
static RiftApi g_apis[MAX_MODS];
static int g_n;

static void mlog(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    __android_log_vprint(4, "Rift", fmt, ap);
    va_end(ap);
}
static void *apiFind(const char *n) {
    void *p = g_h ? dlsym(g_h, n) : nullptr;
    if (!p) p = dlsym(RTLD_DEFAULT, n);
    return p;
}
static int apiHook(void *t, void *r, void **o) {
    if (!t || !r) return 1;
    A64HookFunction(t, r, o);
    return 0;
}

static void mkdirs(const char *path) {
    char b[400];
    snprintf(b, sizeof b, "%s", path);
    for (char *p = b + 1; *p; p++) {
        if (*p == '/') { *p = 0; mkdir(b, 0775); *p = '/'; }
    }
    mkdir(b, 0775);
}

static unsigned char *readAll(const char *path, size_t *n) {
    FILE *f = fopen(path, "rb");
    if (!f) return nullptr;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz < 0) { fclose(f); return nullptr; }
    unsigned char *d = (unsigned char *) malloc((size_t) sz + 1);
    if (!d) { fclose(f); return nullptr; }
    size_t r = fread(d, 1, (size_t) sz, f);
    fclose(f);
    d[r] = 0;
    *n = r;
    return d;
}

// ---- zip + zlib (dlopen libz.so) ----
struct ZS {
    const unsigned char *next_in; unsigned avail_in; unsigned long total_in;
    unsigned char *next_out; unsigned avail_out; unsigned long total_out;
    const char *msg; void *state; void *zalloc; void *zfree; void *opaque;
    int data_type; unsigned long adler; unsigned long reserved;
};
typedef int (*InfInit)(ZS *, int, const char *, int);
typedef int (*InfFn)(ZS *, int);
typedef int (*EndFn)(ZS *);
typedef const char *(*ZVer)();
static InfInit z_init;
static InfFn z_inflate;
static EndFn z_end;
static ZVer z_ver;
static bool z_ok;

static bool zlibLoad() {
    if (z_ok) return true;
    void *h = dlopen("libz.so", RTLD_NOW);
    if (!h) return false;
    z_init = (InfInit) dlsym(h, "inflateInit2_");
    z_inflate = (InfFn) dlsym(h, "inflate");
    z_end = (EndFn) dlsym(h, "inflateEnd");
    z_ver = (ZVer) dlsym(h, "zlibVersion");
    z_ok = z_init && z_inflate && z_end && z_ver;
    return z_ok;
}

static bool inflateRaw(const unsigned char *src, unsigned csz, unsigned char *dst, unsigned usz) {
    if (!zlibLoad()) return false;
    ZS s;
    memset(&s, 0, sizeof s);
    s.next_in = src; s.avail_in = csz; s.next_out = dst; s.avail_out = usz;
    if (z_init(&s, -15, z_ver(), (int) sizeof(ZS)) != 0) return false;
    int r = z_inflate(&s, 4);
    z_end(&s);
    return r == 1 && s.total_out == usz;
}

static unsigned u16(const unsigned char *p) { return p[0] | (p[1] << 8); }
static unsigned u32(const unsigned char *p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((unsigned) p[3] << 24); }

static bool extractZip(const char *zip, const char *dest) {
    size_t sz = 0;
    unsigned char *d = readAll(zip, &sz);
    if (!d || sz < 22) { free(d); return false; }
    long e = -1;
    for (long i = (long) sz - 22; i >= 0 && i >= (long) sz - 22 - 65535; i--)
        if (u32(d + i) == 0x06054b50) { e = i; break; }
    if (e < 0) { free(d); return false; }
    unsigned count = u16(d + e + 10), cdoff = u32(d + e + 16);
    const unsigned char *p = d + cdoff;
    bool ok = true;
    for (unsigned k = 0; k < count; k++) {
        if (p + 46 > d + sz || u32(p) != 0x02014b50) { ok = false; break; }
        unsigned method = u16(p + 10), csz = u32(p + 20), usz = u32(p + 24);
        unsigned nl = u16(p + 28), el = u16(p + 30), cl = u16(p + 32), lho = u32(p + 42);
        const unsigned char *next = p + 46 + nl + el + cl;
        char name[300];
        if (nl >= 200 || p + 46 + nl > d + sz) { ok = false; break; }
        memcpy(name, p + 46, nl);
        name[nl] = 0;
        p = next;
        if (nl == 0 || name[nl - 1] == '/' || name[0] == '/' || strstr(name, "..")) continue;
        if ((size_t) lho + 30 > sz || u32(d + lho) != 0x04034b50) { ok = false; break; }
        const unsigned char *data = d + lho + 30 + u16(d + lho + 26) + u16(d + lho + 28);
        if (data + csz > d + sz) { ok = false; break; }
        char out[600];
        snprintf(out, sizeof out, "%s%s", dest, name);
        char dirp[600];
        snprintf(dirp, sizeof dirp, "%s", out);
        char *sl = strrchr(dirp, '/');
        if (sl) { *sl = 0; mkdirs(dirp); }
        unsigned char *buf = nullptr;
        const unsigned char *w = data;
        unsigned wl = csz;
        if (method == 8) {
            buf = (unsigned char *) malloc(usz + 1);
            if (!buf || !inflateRaw(data, csz, buf, usz)) { free(buf); ok = false; break; }
            w = buf; wl = usz;
        } else if (method != 0) { ok = false; break; }
        FILE *f = fopen(out, "wb");
        if (!f) { free(buf); ok = false; break; }
        if (wl) fwrite(w, 1, wl, f);
        fclose(f);
        free(buf);
    }
    free(d);
    return ok;
}

// ---- mod.json (cukup baca "kunci": "nilai") ----
static void jstr(const char *json, const char *key, char *out, int n) {
    out[0] = 0;
    char pat[80];
    snprintf(pat, sizeof pat, "\"%s\"", key);
    const char *p = strstr(json, pat);
    if (!p) return;
    p += strlen(pat);
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p != ':') return;
    p++;
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p != '"') return;
    p++;
    int i = 0;
    while (*p && *p != '"' && i < n - 1) {
        if (*p == '\\' && p[1]) { p++; out[i++] = (*p == 'n') ? ' ' : *p; p++; continue; }
        out[i++] = *p++;
    }
    out[i] = 0;
}

// ---- daftar mod yang dimatikan: config/disabled.txt ----
static bool isDisabled(const char *id) {
    size_t n = 0;
    unsigned char *d = readAll(MEDIA_DIR "config/disabled.txt", &n);
    if (!d) return false;
    bool found = false;
    char *save = nullptr;
    for (char *line = strtok_r((char *) d, "\r\n", &save); line; line = strtok_r(nullptr, "\r\n", &save))
        if (strcmp(line, id) == 0) { found = true; break; }
    free(d);
    return found;
}

static void saveDisabled() {
    FILE *f = fopen(MEDIA_DIR "config/disabled.txt", "wb");
    if (!f) return;
    for (int i = 0; i < g_n; i++)
        if (!g_mods[i].enabled) fprintf(f, "%s\n", g_mods[i].id);
    fclose(f);
}

static int cmpName(const void *a, const void *b) { return strcmp((const char *) a, (const char *) b); }

void mods_init(void *h) {
    g_h = h;
    mkdirs(MEDIA_DIR "mods/");
    mkdirs(MEDIA_DIR "logs/");
    mkdirs(MEDIA_DIR "config/");
    mkdirs(CACHE_DIR);
}

int mods_scan() {
    g_n = 0;
    mkdirs(MEDIA_DIR "mods/");
    mkdirs(CACHE_DIR);
    DIR *dir = opendir(MEDIA_DIR "mods/");
    if (!dir) { mlog("folder mods tidak bisa dibuka"); return 0; }
    static char names[MAX_MODS][128];
    int cnt = 0;
    while (struct dirent *de = readdir(dir)) {
        size_t l = strlen(de->d_name);
        if (l > 5 && l < 120 && strcmp(de->d_name + l - 5, ".rift") == 0 && cnt < MAX_MODS)
            snprintf(names[cnt++], 128, "%s", de->d_name);
    }
    closedir(dir);
    qsort(names, cnt, 128, cmpName);
    for (int i = 0; i < cnt && g_n < MAX_MODS; i++) {
        char zip[300];
        snprintf(zip, sizeof zip, MEDIA_DIR "mods/%s", names[i]);
        struct stat st;
        if (stat(zip, &st) != 0) continue;
        char base[128];
        snprintf(base, sizeof base, "%s", names[i]);
        base[strlen(base) - 5] = 0;
        char dest[400];
        snprintf(dest, sizeof dest, CACHE_DIR "%s-%ld-%ld/", base, (long) st.st_size, (long) st.st_mtime);
        char ok[420];
        snprintf(ok, sizeof ok, "%s.ok", dest);
        if (access(ok, F_OK) != 0) {
            mkdirs(dest);
            if (!extractZip(zip, dest)) { mlog("gagal ekstrak %s", names[i]); continue; }
            FILE *f = fopen(ok, "wb");
            if (f) fclose(f);
        }
        char jp[420], so[420];
        snprintf(jp, sizeof jp, "%smod.json", dest);
        snprintf(so, sizeof so, "%smod.so", dest);
        size_t jn = 0;
        unsigned char *js = readAll(jp, &jn);
        if (!js) { mlog("%s: mod.json tidak ada", names[i]); continue; }
        if (access(so, F_OK) != 0) { mlog("%s: mod.so tidak ada", names[i]); free(js); continue; }
        ModInfo *m = &g_mods[g_n];
        memset(m, 0, sizeof *m);
        jstr((const char *) js, "id", m->id, sizeof m->id);
        jstr((const char *) js, "name", m->name, sizeof m->name);
        jstr((const char *) js, "author", m->author, sizeof m->author);
        jstr((const char *) js, "version", m->version, sizeof m->version);
        jstr((const char *) js, "description", m->desc, sizeof m->desc);
        free(js);
        if (!m->id[0]) snprintf(m->id, sizeof m->id, "%s", base);
        if (!m->name[0]) snprintf(m->name, sizeof m->name, "%s", m->id);
        snprintf(m->dir, sizeof m->dir, "%s", dest);
        m->enabled = !isDisabled(m->id);
        g_n++;
    }
    return g_n;
}

int mods_count() { return g_n; }
ModInfo *mods_get(int i) { return (i >= 0 && i < g_n) ? &g_mods[i] : nullptr; }

bool mods_toggle(int i) {
    ModInfo *m = mods_get(i);
    if (!m) return false;
    m->enabled = !m->enabled;
    saveDisabled();
    return m->enabled;
}

void mods_load_enabled() {
    for (int i = 0; i < g_n; i++) {
        ModInfo *m = &g_mods[i];
        if (!m->enabled || m->loaded) continue;
        char so[420];
        snprintf(so, sizeof so, "%smod.so", m->dir);
        void *h = dlopen(so, RTLD_NOW | RTLD_LOCAL);
        if (!h) { mlog("mod %s: dlopen gagal: %s", m->id, dlerror()); continue; }
        typedef int (*InitFn)(const RiftApi *);
        InitFn init = (InitFn) dlsym(h, "rift_mod_init");
        if (!init) { mlog("mod %s: rift_mod_init tidak ada", m->id); continue; }
        g_apis[i].version = 1;
        g_apis[i].log = (void (*)(const char *, ...)) mlog;
        g_apis[i].find_symbol = apiFind;
        g_apis[i].hook = apiHook;
        g_apis[i].mod_dir = m->dir;
        int r = init(&g_apis[i]);
        m->loaded = (r == 0);
        mlog("mod %s dimuat, init = %d", m->id, r);
    }
}
