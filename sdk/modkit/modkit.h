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

static void cbLevel(void *p) { pl = p; if (H->onLevel) H->onLevel(); }
static void cbLeave() { if (pl && H->onLeave) H->onLeave(); pl = nullptr; }
static void cbDie(float p) { if (pl && H->onDeath) H->onDeath(p); }
static void cbWin() { if (pl && H->onWin) H->onWin(); }
static void cbTick(float dt) { if (pl && H->onTick) H->onTick(dt); }
static void cbTouch() { if (pl && H->onTouch) H->onTouch(); }

static void *sym(const char *n) { void *p = R->find_symbol(n); if (!p) R->log("modkit: simbol hilang %s", n); return p; }

// mod TIDAK boleh nge-hook PlayLayer sendiri (hook ganda bikin game freeze). Semua lewat event API v3.
static int start(const RiftApi *api, Handlers *h) {
    R = api; H = h;
    if (api->version < 3 || !api->on_level) { api->log("modkit: butuh Rift 1.1.2 atau lebih baru (API v3)"); return 5; }
    s_label = (LabelFn) sym("_ZN7cocos2d13CCLabelBMFont6createEPKcS2_");
    s_pos = (PosFn) sym("_ZN7cocos2d6CCNode11setPositionEff");
    s_scale = (ScaleFn) sym("_ZN7cocos2d6CCNode8setScaleEf");
    s_addZ = (AddZFn) sym("_ZN7cocos2d6CCNode8addChildEPS0_i");
    s_vis = (VisFn) sym("_ZN7cocos2d6CCNode10setVisibleEb");
    s_setStr = (SetStrFn) api->find_symbol("_ZN7cocos2d13CCLabelBMFont9setStringEPKc");
    if (!s_setStr) s_setStr = (SetStrFn) sym("_ZN7cocos2d13CCLabelBMFont9setStringEPKcb");
    pct = (PctFn) sym("_ZN9PlayLayer17getCurrentPercentEv");
    if (!s_label || !s_addZ || !s_setStr) return 1;
    api->on_level(cbLevel);
    api->on_leave(cbLeave);
    if (h->onDeath) api->on_death(cbDie);
    if (h->onWin) api->on_win(cbWin);
    if (h->onTick) api->on_tick(cbTick);
    if (h->onTouch) api->on_touch(cbTouch);
    return 0;
}
}
