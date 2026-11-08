// jaringan + restart lewat JNI (memanggil com.rift.launcher.RiftNet), katalog mod online
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "riftmods.h"
#include "riftnet.h"

extern "C" int __android_log_print(int prio, const char *tag, const char *fmt, ...);
#define LOGN(...) __android_log_print(4, "Rift", __VA_ARGS__)

#ifndef MEDIA_DIR
#define MEDIA_DIR "/storage/emulated/0/Android/media/com.rift.launcher/"
#endif
#ifndef CATALOG_URL
#define CATALOG_URL "https://raw.githubusercontent.com/riftsgd-app/rift/main/catalog/mods.json"
#endif

typedef int (*GetEnvFn)(void *, void **, int);
typedef int (*AttachFn)(void *, void **, void *);
typedef void *(*FindClassFn)(void *, const char *);
typedef void *(*NewGlobalFn)(void *, void *);
typedef void *(*GetMidFn)(void *, void *, const char *, const char *);
typedef void *(*NewStrFn)(void *, const char *);
typedef const char *(*GetCharsFn)(void *, void *, unsigned char *);
typedef void (*RelCharsFn)(void *, void *, const char *);
typedef void (*DelRefFn)(void *, void *);
typedef int (*ExcFn)(void *);
typedef void (*ExcClrFn)(void *);
typedef void *(*CallObjFn)(void *, void *, void *, ...);
typedef void (*CallVoidFn)(void *, void *, void *, ...);

#define JF(env, idx, T) ((T)(*(void ***) (env))[idx])

static void *g_vm, *g_cls, *g_mFetch, *g_mDl, *g_mRestart, *g_mUi, *g_mPoll;

static void *getEnv() {
    if (!g_vm) return nullptr;
    void *env = nullptr;
    GetEnvFn ge = (GetEnvFn)(*(void ***) g_vm)[6];
    if (ge(g_vm, &env, 0x00010006) != 0) {
        AttachFn at = (AttachFn)(*(void ***) g_vm)[4];
        if (at(g_vm, &env, nullptr) != 0) return nullptr;
    }
    return env;
}

extern "C" int JNI_OnLoad(void *vm, void *reserved) {
    g_vm = vm;
    void *env = getEnv();
    if (env) {
        void *c = JF(env, 6, FindClassFn)(env, "com/rift/launcher/RiftNet");
        if (JF(env, 228, ExcFn)(env)) JF(env, 17, ExcClrFn)(env);
        if (c) {
            g_cls = JF(env, 21, NewGlobalFn)(env, c);
            GetMidFn gm = JF(env, 113, GetMidFn);
            g_mFetch = gm(env, g_cls, "fetchText", "(Ljava/lang/String;)Ljava/lang/String;");
            g_mDl = gm(env, g_cls, "download", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
            g_mRestart = gm(env, g_cls, "restart", "()V");
            if (JF(env, 228, ExcFn)(env)) JF(env, 17, ExcClrFn)(env);
            g_mUi = gm(env, g_cls, "ui", "(Ljava/lang/String;)Ljava/lang/String;");
            if (JF(env, 228, ExcFn)(env)) JF(env, 17, ExcClrFn)(env);
            g_mPoll = gm(env, g_cls, "poll", "(Ljava/lang/String;)Ljava/lang/String;");
            if (JF(env, 228, ExcFn)(env)) JF(env, 17, ExcClrFn)(env);
        }
        LOGN("JNI_OnLoad: RiftNet=%d fetch=%d dl=%d restart=%d", g_cls != nullptr, g_mFetch != nullptr, g_mDl != nullptr, g_mRestart != nullptr);
    }
    return 0x00010006;
}

// panggil fungsi Java String -> String (atau String,String -> String); hasil disalin ke out
static bool callStr(void *mid, const char *a, const char *b, char *out, int n) {
    out[0] = 0;
    void *env = getEnv();
    if (!env || !g_cls || !mid) return false;
    NewStrFn ns = JF(env, 167, NewStrFn);
    void *ja = ns(env, a);
    void *jb = b ? ns(env, b) : nullptr;
    CallObjFn call = JF(env, 114, CallObjFn);
    void *res = b ? call(env, g_cls, mid, ja, jb) : call(env, g_cls, mid, ja);
    if (JF(env, 228, ExcFn)(env)) { JF(env, 17, ExcClrFn)(env); res = nullptr; }
    if (res) {
        const char *s = JF(env, 169, GetCharsFn)(env, res, nullptr);
        if (s) { snprintf(out, n, "%s", s); JF(env, 170, RelCharsFn)(env, res, s); }
        JF(env, 23, DelRefFn)(env, res);
    }
    DelRefFn del = JF(env, 23, DelRefFn);
    if (ja) del(env, ja);
    if (jb) del(env, jb);
    return true;
}

void riftnet_ui(const char *cmd) {
    char o[16];
    callStr(g_mUi, cmd, nullptr, o, sizeof o);
}

bool riftnet_poll(char *out, int n) {
    callStr(g_mPoll, "x", nullptr, out, n);
    return out[0] != 0;
}

void riftnet_restart() {
    void *env = getEnv();
    if (!env || !g_cls || !g_mRestart) { LOGN("restart: JNI tidak siap"); return; }
    JF(env, 141, CallVoidFn)(env, g_cls, g_mRestart);
}

// ---- katalog ----
static CatInfo g_cat[32];
static int g_cn;

static void addCat(const char *id, const char *name, const char *author, const char *desc, const char *url) {
    if (g_cn >= 32) return;
    CatInfo *c = &g_cat[g_cn++];
    memset(c, 0, sizeof *c);
    snprintf(c->id, sizeof c->id, "%s", id);
    snprintf(c->name, sizeof c->name, "%s", name);
    snprintf(c->author, sizeof c->author, "%s", author);
    snprintf(c->desc, sizeof c->desc, "%s", desc);
    snprintf(c->url, sizeof c->url, "%s", url);
}

static void builtin() {
    g_cn = 0;
    addCat("pulse.soundboard", "Soundboard", "Pulse", "Trigger custom sounds on jumps and deaths.", "");
    addCat("echo.macropad", "Macro Pad", "echo", "Record and replay inputs for testing sections.", "");
    addCat("nova.iconstudio", "Icon Studio", "Nova", "Recolor and preview every icon in one menu.", "");
    addCat("vex.framestepper", "Frame Stepper", "vex", "Step through any attempt one frame at a time.", "");
    addCat("kiro.practiceplus", "Practice+", "kiro", "Better checkpoints and run stats for practice mode.", "");
    addCat("lumen.neontrails", "Neon Trails", "Lumen", "Glowing trails that follow your icon.", "");
}

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

static void parseCatalog(const char *js) {
    g_cn = 0;
    const char *p = strstr(js, "\"mods\"");
    if (!p) return;
    p = strchr(p, '[');
    if (!p) return;
    while (g_cn < 32) {
        const char *o = strchr(p, '{');
        if (!o) break;
        const char *e = strchr(o, '}');
        if (!e) break;
        size_t len = (size_t)(e - o) + 1;
        if (len < 4000) {
            char tmp[4000];
            memcpy(tmp, o, len);
            tmp[len] = 0;
            char id[64], name[64], author[40], desc[128], url[300];
            jstr(tmp, "id", id, sizeof id);
            jstr(tmp, "name", name, sizeof name);
            jstr(tmp, "author", author, sizeof author);
            jstr(tmp, "description", desc, sizeof desc);
            jstr(tmp, "url", url, sizeof url);
            if (id[0] && name[0]) addCat(id, name, author, desc, url);
        }
        p = e + 1;
    }
}

int cat_fetch() {
    char *buf = (char *) malloc(200000);
    int ok = 0;
    if (buf) {
        if (callStr(g_mFetch, CATALOG_URL, nullptr, buf, 200000) && strlen(buf) > 10) {
            parseCatalog(buf);
            ok = g_cn;
        }
        free(buf);
    }
    if (ok == 0) { LOGN("katalog online gagal, pakai daftar bawaan"); builtin(); }
    return g_cn;
}

int cat_count() { if (g_cn == 0) builtin(); return g_cn; }
CatInfo *cat_get(int i) { cat_count(); return (i >= 0 && i < g_cn) ? &g_cat[i] : nullptr; }

static void destPath(const CatInfo *c, char *out, int n) {
    char id[64];
    int k = 0;
    for (const char *s = c->id; *s && k < 62; s++) {
        char ch = *s;
        bool okc = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '.' || ch == '_' || ch == '-';
        id[k++] = okc ? ch : '_';
    }
    id[k] = 0;
    snprintf(out, n, MEDIA_DIR "mods/%s.rift", id);
}

bool cat_installed(int i) {
    CatInfo *c = cat_get(i);
    if (!c) return false;
    char p[400];
    destPath(c, p, sizeof p);
    if (access(p, F_OK) == 0) return true;
    for (int k = 0; k < mods_count(); k++)
        if (strcmp(mods_get(k)->id, c->id) == 0) return true;
    return false;
}

const char *cat_download(int i) {
    static char err[160];
    CatInfo *c = cat_get(i);
    if (!c || !c->url[0]) return "belum ada file";
    char p[400];
    destPath(c, p, sizeof p);
    char res[200];
    if (!callStr(g_mDl, c->url, p, res, sizeof res)) return "JNI tidak siap";
    if (strcmp(res, "OK") == 0) return nullptr;
    snprintf(err, sizeof err, "%s", res[0] ? res : "gagal");
    return err;
}
