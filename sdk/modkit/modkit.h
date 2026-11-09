// modkit: bantuan kecil buat mod yang jalan di dalam level (PlayLayer).
#pragma once
#include "rift.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

namespace kit {
typedef void *(*LabelFn)(const char *, const char *);
typedef void (*PosFn)(void *, float, float);
typedef void (*ScaleFn)(void *, float);
typedef void (*AddZFn)(void *, void *, int);
typedef void (*SetStrFn)(void *, const char *, bool);
typedef void (*VisFn)(void *, bool);
typedef bool (*InitFn)(void *, void *, bool, bool);
typedef void (*DieFn)(void *, void *, void *);
typedef void (*VFn)(void *);
typedef float (*PctFn)(void *);

struct Handlers {
    void (*onLevel)(void);          // level baru mulai (pl sudah valid)
    void (*onDeath)(float pct);     // 1x per kematian
    void (*onWin)(void);            // level selesai
    void (*onTick)(float dt);       // tiap frame selama di level
    void (*onTouch)(void);          // tiap sentuhan selama di level
    void (*onLeave)(void);          // keluar level
};

static const RiftApi *R;
static Handlers *H;
static LabelFn s_label; static PosFn s_pos; static ScaleFn s_scale; static AddZFn s_addZ;
static SetStrFn s_setStr; static VisFn s_vis;
static PctFn pct;
static void *pl;
static bool safe;
static InitFn o_init; static DieFn o_die; static VFn o_win, o_d0, o_d1;

static void *mk(const char *text, float scale, float x, float y) {
    if (!pl || !s_label || !s_addZ) return nullptr;
    void *l = s_label(text, "bigFont.fnt");
    if (!l) return nullptr;
    if (s_scale) s_scale(l, scale);
    if (s_pos) s_pos(l, x, y);
    s_addZ(pl, l, 100);
    return l;
}
static void set(void *l, const char *s) { if (l && s_setStr) s_setStr(l, s, true); }
static void show(void *l, bool v) { if (l && s_vis) s_vis(l, v); }

static void leave() {
    if (pl && H && H->onLeave) H->onLeave();
    pl = nullptr;
}
static bool gap() {
    struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t);
    double n = t.tv_sec + t.tv_nsec / 1e9; static double l = -10;
    bool r = n - l > 0.5; l = n; return r;
}
static bool hkInit(void *s, void *lv, bool a, bool b) {
    bool r = o_init(s, lv, a, b);
    if (!r) return r;
    pl = s;
    if (H->onLevel) H->onLevel();
    return r;
}
static void hkDie(void *s, void *p, void *o) {
    if (s == pl && H->onDeath) { if (gap()) H->onDeath(pct ? pct(s) : 0.0f); }
    o_die(s, p, o);
}
static void hkWin(void *s) {
    if (s == pl && H->onWin) H->onWin();
    o_win(s);
}
static void hkD0(void *s) { if (s == pl) leave(); o_d0(s); }
static void hkD1(void *s) { if (s == pl) leave(); o_d1(s); }

static void tick(float dt) { if (safe && pl && H->onTick) H->onTick(dt); }
static void touch() { if (safe && pl && H->onTouch) H->onTouch(); }

static void *sym(const char *n) { void *p = R->find_symbol(n); if (!p) R->log("modkit: simbol hilang %s", n); return p; }

static int start(const RiftApi *api, Handlers *h) {
    R = api; H = h;
    s_label = (LabelFn) sym("_ZN7cocos2d13CCLabelBMFont6createEPKcS2_");
    s_pos = (PosFn) sym("_ZN7cocos2d6CCNode11setPositionEff");
    s_scale = (ScaleFn) sym("_ZN7cocos2d6CCNode8setScaleEf");
    s_addZ = (AddZFn) sym("_ZN7cocos2d6CCNode8addChildEPS0_i");
    s_vis = (VisFn) sym("_ZN7cocos2d6CCNode10setVisibleEb");
    s_setStr = (SetStrFn) api->find_symbol("_ZN7cocos2d13CCLabelBMFont9setStringEPKc");
    if (!s_setStr) s_setStr = (SetStrFn) sym("_ZN7cocos2d13CCLabelBMFont9setStringEPKcb");
    pct = (PctFn) sym("_ZN9PlayLayer17getCurrentPercentEv");
    void *i = api->find_symbol("_ZN9PlayLayer4initEP11GJGameLevelbb");
    if (!i) i = api->find_symbol("_ZN9PlayLayer4initEP11GJGameLevelb");
    if (!i) i = sym("_ZN9PlayLayer4initEP11GJGameLevel");
    void *d = sym("_ZN9PlayLayer13destroyPlayerEP12PlayerObjectP10GameObject");
    void *w = sym("_ZN9PlayLayer13levelCompleteEv");
    void *d0 = api->find_symbol("_ZN9PlayLayerD0Ev");
    void *d1 = api->find_symbol("_ZN9PlayLayerD1Ev");
    if (!i || !s_label || !s_addZ || !s_setStr) return 1;
    if (d0 && api->hook(d0, (void *) hkD0, (void **) &o_d0) == 0) safe = true;
    if (d1 && api->hook(d1, (void *) hkD1, (void **) &o_d1) == 0) safe = true;
    if (!safe) { api->log("modkit: tidak ada hook pembersih, mod dimatikan"); return 2; }
    if (d && h->onDeath) api->hook(d, (void *) hkDie, (void **) &o_die);
    if (w && h->onWin) api->hook(w, (void *) hkWin, (void **) &o_win);
    if (api->hook(i, (void *) hkInit, (void **) &o_init) != 0) return 3;
    if (api->version >= 2) {
        if (h->onTick && api->on_tick) api->on_tick(tick);
        if (h->onTouch && api->on_touch) api->on_touch(touch);
    } else api->log("modkit: butuh Rift terbaru (API v2)");
    return 0;
}
}
