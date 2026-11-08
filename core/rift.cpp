#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
extern "C" int __android_log_print(int prio, const char *tag, const char *fmt, ...);
extern "C" void A64HookFunction(void *const symbol, void *const replace, void **result);
#define LOGI(...) __android_log_print(4, "Rift", __VA_ARGS__)
#define RIFT_DIR "/data/user/0/com.rift.launcher/files/"
#include "riftmods.h"
#include "riftnet.h"

struct Color3B { unsigned char r, g, b; };
struct Size2 { float w, h; Size2() : w(0), h(0) {} Size2(const Size2 &o) : w(o.w), h(o.h) {} };
struct MFP { void *fn; long adj; };   // pointer-to-member-function (ARM64)

typedef bool (*InitFn)(void *);
typedef void *(*LabelCreateFn)(const char *, const char *);
typedef void (*SetPosFn)(void *, float, float);
typedef void (*SetScaleFn)(void *, float);
typedef void (*AddChildFn)(void *, void *);
typedef void (*AddChildZFn)(void *, void *, int);
typedef void (*SetColorFn)(void *, const Color3B &);
typedef void *(*SpriteCreateFn)(const char *);
typedef void *(*MenuCreateFn)();
typedef void *(*NodeCreateFn)();
typedef void *(*ItemCreateFn)(void *, void *, void *, MFP);
typedef void *(*SharedFn)();
typedef Size2 (*WinSizeFn)(void *);
typedef const Size2 *(*CSizeFn)(void *);
typedef void *(*GetChildrenFn)(void *);
typedef unsigned (*CountFn)(void *);
typedef void *(*ObjAtFn)(void *, unsigned);
typedef float (*GetCoordFn)(void *);
typedef void (*SetVisFn)(void *, bool);

static InitFn g_origInit;
static LabelCreateFn g_label;
static SetPosFn g_setPos;
static SetScaleFn g_setScale;
static AddChildFn g_addChild;
static AddChildZFn g_addChildZ;
static SetColorFn g_setColor;
static SpriteCreateFn g_sprite;
static MenuCreateFn g_menu;
static NodeCreateFn g_node;
static ItemCreateFn g_item;
static SharedFn g_shared;
static WinSizeFn g_winSize;
static CSizeFn g_cSize;
static GetChildrenFn g_getChildren;
static CountFn g_childCount, g_arrCount;
static ObjAtFn g_arrAt;
static GetCoordFn g_getX, g_getY;
static SetVisFn g_setVis;

static void *g_layer;
static void *g_overlay;
static float g_winW = 683.0f, g_winH = 320.0f;

static void *mkSprite(const char *name) {
    if (!g_sprite) return nullptr;
    char path[256];
    snprintf(path, sizeof path, RIFT_DIR "%s", name);
    return g_sprite(path);
}

static void closeOverlay();
static void onNoop(void *self, void *sender) {}

static void onClose(void *self, void *sender) {
    closeOverlay();
}

static void addLabel(void *parent, const char *txt, float x, float y, float scale, Color3B col, bool leftAlign) {
    void *l = g_label(txt, "bigFont.fnt");
    if (!l) return;
    if (g_setScale) g_setScale(l, scale);
    if (g_setColor) g_setColor(l, col);
    float px = x;
    if (leftAlign && g_cSize) {
        const Size2 *cs = g_cSize(l);
        if (cs && cs->w > 1.0f && cs->w < 4000.0f) px = x + cs->w * scale / 2.0f;
    }
    g_setPos(l, px, y);
    g_addChild(parent, l);
}

static void *sym2(void *h, const char *a, const char *b);
static void closeOverlay();

struct Pt2 { float x, y; Pt2() : x(0), y(0) {} Pt2(const Pt2 &o) : x(o.x), y(o.y) {} };
typedef Pt2 (*TouchLocFn)(void *);
typedef void *(*AnyObjFn)(void *);
typedef void (*TouchFn)(void *, void *, void *);
typedef void *(*MoveByFn)(float, const Pt2 &);
typedef void *(*Seq2Fn)(void *, void *, void *);
typedef void *(*Act1Fn)(void *);
typedef void *(*RunFn)(void *, void *);
typedef void *(*FadeToFn)(float, unsigned char);

static void *g_winA, *g_winB;
static void *g_cards, *g_empty;
static int g_tab;
static void *g_sprOn[8], *g_sprOff[8];
static SetVisFn g_remFP;
static void *g_bcards, *g_restart;
static bool g_pending, g_catLoaded;
static bool g_catFail[32];
static bool g_open;
static int g_press;
static float g_cx, g_cy, g_sp, g_S;
static TouchLocFn g_touchLoc;
static AnyObjFn g_anyObj;
static TouchFn g_oB, g_oM, g_oE, g_oC;
static MoveByFn g_moveBy;
static Seq2Fn g_seq;
static Act1Fn g_rep, g_ease;
static RunFn g_run;
static FadeToFn g_fadeTo;

static void closeOverlay() {
    g_open = false;
    g_press = 0;
    if (g_overlay && g_setVis) g_setVis(g_overlay, false);
}

static void asciiCopy(char *dst, const char *src, int max) {
    int n = 0;
    for (; src[n] && n < max - 1; n++) {
        unsigned char ch = (unsigned char) src[n];
        dst[n] = (ch >= 32 && ch < 127) ? (char) ch : '?';
    }
    dst[n] = 0;
}

// label bigFont, dipas-in ke kotak maxW x maxH (poin). align: 0 kiri, 1 kanan, 2 tengah
static void fitLabel(void *parent, const char *txt, float x, float y, float maxW, float maxH, int align, Color3B col) {
    if (!g_label || !g_cSize) return;
    char t[160];
    asciiCopy(t, txt, sizeof t);
    void *l = g_label(t, "bigFont.fnt");
    if (!l) return;
    const Size2 *cs = g_cSize(l);
    float w = (cs && cs->w > 1.0f) ? cs->w : 100.0f, h = (cs && cs->h > 1.0f) ? cs->h : 40.0f;
    float s = maxW / w;
    if (maxH / h < s) s = maxH / h;
    if (g_setScale) g_setScale(l, s);
    if (g_setColor) g_setColor(l, col);
    float px = x;
    if (align == 0) px = x + w * s / 2.0f;
    else if (align == 1) px = x - w * s / 2.0f;
    g_setPos(l, px, y);
    g_addChild(parent, l);
}

static float lx(float rx);
static float ly(float ry);

static void showTab() {
    if (!g_setVis) return;
    int n = mods_count();
    if (g_winA) g_setVis(g_winA, g_tab == 0);
    if (g_winB) g_setVis(g_winB, g_tab == 1);
    if (g_cards) g_setVis(g_cards, g_tab == 0);
    if (g_bcards) g_setVis(g_bcards, g_tab == 1);
    if (g_restart) g_setVis(g_restart, g_pending);
    if (g_empty) g_setVis(g_empty, g_tab == 0 && n == 0);
}

static void rebuildCards() {
    if (!g_overlay || !g_node) return;
    if (g_cards) {
        if (g_remFP) g_remFP(g_cards, true);
        else if (g_setVis) g_setVis(g_cards, false);
        g_cards = nullptr;
    }
    int n = mods_scan();
    void *cont = g_node();
    if (!cont) return;
    int shown = n > 6 ? 6 : n;
    Color3B white = {255, 255, 255}, lav = {169, 154, 255}, soft = {205, 205, 222};
    for (int i = 0; i < shown; i++) {
        ModInfo *m = mods_get(i);
        if (!m) continue;
        float dx = 52.0f + (i % 2) * 456.0f, dy = 300.0f + (i / 2) * 82.0f;
        float S = g_S;
        void *on = mkSprite("rift_card_on.png");
        void *off = mkSprite("rift_card_off.png");
        if (on) { g_setScale(on, S); g_setPos(on, lx(dx + 220.0f), ly(dy + 36.0f)); g_addChild(cont, on); }
        if (off) { g_setScale(off, S); g_setPos(off, lx(dx + 220.0f), ly(dy + 36.0f)); g_addChild(cont, off); }
        g_sprOn[i] = on; g_sprOff[i] = off;
        if (g_setVis) { if (on) g_setVis(on, m->enabled); if (off) g_setVis(off, !m->enabled); }
        fitLabel(cont, m->name, lx(dx + 72.0f), ly(dy + 24.0f), 200.0f * g_sp, 26.0f * g_sp, 0, white);
        char by[80];
        snprintf(by, sizeof by, "by %s", m->author[0] ? m->author : "?");
        fitLabel(cont, by, lx(dx + 426.0f), ly(dy + 24.0f), 120.0f * g_sp, 17.0f * g_sp, 1, lav);
        fitLabel(cont, m->desc[0] ? m->desc : "No description", lx(dx + 72.0f), ly(dy + 52.0f), 285.0f * g_sp, 15.0f * g_sp, 0, soft);
    }
    if (n > 0 && !g_pending) fitLabel(cont, "Changes apply after restarting the game", lx(501.5f), ly(553.0f), 460.0f * g_sp, 13.0f * g_sp, 2, lav);
    g_addChild(g_overlay, cont);
    g_cards = cont;
    showTab();
}

static void rebuildBrowse() {
    if (!g_overlay || !g_node) return;
    if (g_bcards) {
        if (g_remFP) g_remFP(g_bcards, true);
        else if (g_setVis) g_setVis(g_bcards, false);
        g_bcards = nullptr;
    }
    int n = cat_count();
    void *cont = g_node();
    if (!cont) return;
    int shown = n > 6 ? 6 : n;
    Color3B white = {255, 255, 255}, lav = {169, 154, 255}, soft = {205, 205, 222};
    for (int i = 0; i < shown; i++) {
        CatInfo *c = cat_get(i);
        if (!c) continue;
        float dx = 52.0f + (i % 2) * 456.0f, dy = 300.0f + (i / 2) * 82.0f;
        void *bc = mkSprite("rift_bcard.png");
        if (bc) { g_setScale(bc, g_S); g_setPos(bc, lx(dx + 220.0f), ly(dy + 36.0f)); g_addChild(cont, bc); }
        fitLabel(cont, c->name, lx(dx + 72.0f), ly(dy + 24.0f), 190.0f * g_sp, 26.0f * g_sp, 0, white);
        char by[80];
        snprintf(by, sizeof by, "by %s", c->author[0] ? c->author : "?");
        fitLabel(cont, by, lx(dx + 426.0f), ly(dy + 24.0f), 110.0f * g_sp, 17.0f * g_sp, 1, lav);
        fitLabel(cont, c->desc[0] ? c->desc : "No description", lx(dx + 72.0f), ly(dy + 52.0f), 262.0f * g_sp, 15.0f * g_sp, 0, soft);
        const char *txt = "Download";
        bool violet = true;
        if (!c->url[0]) { txt = "Soon"; violet = false; }
        else if (cat_installed(i)) { txt = "Installed"; violet = false; }
        else if (g_catFail[i]) { txt = "Retry"; }
        void *pl = mkSprite(violet ? "rift_pill_v.png" : "rift_pill_g.png");
        if (pl) { g_setScale(pl, g_S); g_setPos(pl, lx(dx + 386.0f), ly(dy + 51.0f)); g_addChild(cont, pl); }
        fitLabel(cont, txt, lx(dx + 386.0f), ly(dy + 51.0f), 66.0f * g_sp, 14.0f * g_sp, 2, white);
    }
    g_addChild(g_overlay, cont);
    g_bcards = cont;
    showTab();
}

static void onTabA(void *self, void *sender) { g_tab = 0; showTab(); }
static void onTabB(void *self, void *sender) {
    g_tab = 1;
    if (!g_catLoaded) { g_catLoaded = true; cat_fetch(); rebuildBrowse(); }
    showTab();
}
static void onRefresh(void *self, void *sender) {
    LOGI("Refresh ditekan");
    if (g_tab == 0) rebuildCards();
    else { cat_fetch(); g_catLoaded = true; rebuildBrowse(); }
}

static void doDownload(int i) {
    CatInfo *c = cat_get(i);
    if (!c || !c->url[0] || cat_installed(i)) return;
    const char *err = cat_download(i);
    if (err) { g_catFail[i] = true; LOGI("download %s gagal: %s", c->id, err); }
    else { g_catFail[i] = false; g_pending = true; LOGI("download %s ok", c->id); rebuildCards(); }
    rebuildBrowse();
}

// ---- sentuhan: dicegat langsung di CCTouchDispatcher::touches, tidak tergantung CCMenu ----
static float lx(float rx) { return g_cx + (rx - 501.5f) * g_sp; }
static float ly(float ry) { return g_cy + (294.0f - ry) * g_sp; }
static bool inR(float x, float y, float l, float t, float r, float b, float pad) {
    return x >= lx(l) - pad && x <= lx(r) + pad && y <= ly(t) + pad && y >= ly(b) - pad;
}
static int hitTest(float x, float y) {
    float pad = 6.0f * g_sp;
    float dx = x - lx(930.0f), dy = y - ly(86.0f), rr = 34.0f * g_sp;
    if (dx * dx + dy * dy <= rr * rr) return 1;
    if (inR(x, y, 52, 160, 311, 210, pad)) return 2;
    if (inR(x, y, 326, 160, 537, 210, pad)) return 3;
    if (inR(x, y, 790, 228, 948, 279, pad)) return 4;
    if (g_pending && inR(x, y, 386.5f, 539.0f, 616.5f, 569.0f, 3.0f * g_sp)) return 20;
    if (g_tab == 1) {
        int n = cat_count();
        if (n > 6) n = 6;
        for (int i = 0; i < n; i++)
            if (inR(x, y, 52.0f + (i % 2) * 456.0f + 346.0f, 300.0f + (i / 2) * 82.0f + 38.0f, 52.0f + (i % 2) * 456.0f + 426.0f, 300.0f + (i / 2) * 82.0f + 64.0f, 5.0f * g_sp)) return 30 + i;
    }
    if (g_tab == 0) {
        int n = mods_count();
        if (n > 6) n = 6;
        for (int i = 0; i < n; i++)
            if (inR(x, y, 52.0f + (i % 2) * 456.0f, 300.0f + (i / 2) * 82.0f, 492.0f + (i % 2) * 456.0f, 372.0f + (i / 2) * 82.0f, 0)) return 10 + i;
    }
    if (inR(x, y, 16, 15, 985, 571, 0)) return 5;
    return 6;
}
static void doAction(int a) {
    if (a == 1 || a == 6) closeOverlay();
    else if (a == 2) onTabA(nullptr, nullptr);
    else if (a == 3) onTabB(nullptr, nullptr);
    else if (a == 4) onRefresh(nullptr, nullptr);
    else if (a == 20) riftnet_restart();
    else if (a >= 30) doDownload(a - 30);
    else if (a >= 10) {
        int i = a - 10;
        if (mods_get(i)) {
            mods_toggle(i);
            g_pending = true;
            rebuildCards();
            showTab();
        }
    }
}
static bool handleTouch(void *set, int phase) {
    if (!g_open || !g_anyObj || !g_touchLoc || !set) return false;
    void *t = g_anyObj(set);
    if (!t) return true;
    Pt2 p = g_touchLoc(t);
    int h = hitTest(p.x, p.y);
    if (phase == 0) g_press = h;
    else if (phase == 2) { int pr = g_press; g_press = 0; if (pr == h) doAction(h); }
    else if (phase == 3) g_press = 0;
    return true;
}
static void hkTB(void *s, void *set, void *ev) { if (handleTouch(set, 0)) return; g_oB(s, set, ev); }
static void hkTM(void *s, void *set, void *ev) { if (handleTouch(set, 1)) return; g_oM(s, set, ev); }
static void hkTE(void *s, void *set, void *ev) { if (handleTouch(set, 2)) return; g_oE(s, set, ev); }
static void hkTC(void *s, void *set, void *ev) { if (handleTouch(set, 3)) return; g_oC(s, set, ev); }

static void installExtra(void *h) {
    g_touchLoc = (TouchLocFn) sym2(h, "_ZNK7cocos2d7CCTouch11getLocationEv", "_ZN7cocos2d7CCTouch11getLocationEv");
    g_anyObj   = (AnyObjFn)   sym2(h, "_ZN7cocos2d5CCSet9anyObjectEv", "_ZNK7cocos2d5CCSet9anyObjectEv");
    g_moveBy   = (MoveByFn)   sym2(h, "_ZN7cocos2d8CCMoveBy6createEfRKNS_7CCPointE", nullptr);
    g_seq      = (Seq2Fn)     sym2(h, "_ZN7cocos2d10CCSequence6createEPNS_18CCFiniteTimeActionEz", nullptr);
    g_rep      = (Act1Fn)     sym2(h, "_ZN7cocos2d15CCRepeatForever6createEPNS_16CCActionIntervalE", nullptr);
    g_ease     = (Act1Fn)     sym2(h, "_ZN7cocos2d15CCEaseSineInOut6createEPNS_16CCActionIntervalE", nullptr);
    g_run      = (RunFn)      sym2(h, "_ZN7cocos2d6CCNode9runActionEPNS_8CCActionE", nullptr);
    g_fadeTo   = (FadeToFn)   sym2(h, "_ZN7cocos2d6CCFadeTo6createEfh", nullptr);
    g_remFP    = (SetVisFn)   sym2(h, "_ZN7cocos2d6CCNode27removeFromParentAndCleanupEb", nullptr);
    void *t = dlsym(h, "_ZN7cocos2d17CCTouchDispatcher7touchesEPNS_5CCSetEPNS_7CCEventEj");
    if (!t) LOGI("simbol hilang: CCTouchDispatcher::touches");
    LOGI("extra: touch=%d anim=%d", t != nullptr, g_moveBy != nullptr);
}

static bool g_touchesHooked;
typedef void (*TouchesFn)(void *, void *, void *, unsigned);
static TouchesFn g_oTouches;
static void hkTouches(void *s, void *set, void *ev, unsigned idx) {
    if (g_open && idx < 4) {
        static bool once;
        if (!once) { once = true; LOGI("touch hook aktif"); }
        if (handleTouch(set, (int) idx)) return;
    }
    g_oTouches(s, set, ev, idx);
}
static void hookTouches(void *h) {
    void *t = dlsym(h, "_ZN7cocos2d17CCTouchDispatcher7touchesEPNS_5CCSetEPNS_7CCEventEj");
    if (t && g_touchLoc && g_anyObj) {
        A64HookFunction(t, (void *) hkTouches, (void **) &g_oTouches);
        g_touchesHooked = true;
        LOGI("hook touches terpasang");
    }
}

static void *addHit(void *menu, const char *file, float scale, void (*fn)(void *, void *), float x, float y) {
    void *sp = mkSprite(file);
    if (!sp) return nullptr;
    g_setScale(sp, scale);
    MFP cb = { (void *)fn, 0 };
    void *it = g_item(sp, nullptr, g_layer, cb);
    if (!it) return nullptr;
    g_setPos(it, x, y);
    g_addChild(menu, it);
    return it;
}

static void animOrb(void *o, float dx, float dy, float t, int lo, int hi) {
    if (!o || !g_moveBy || !g_seq || !g_rep || !g_run) return;
    Pt2 a, b;
    a.x = dx; a.y = dy; b.x = -dx; b.y = -dy;
    void *m1 = g_moveBy(t, a);
    void *m2 = g_moveBy(t, b);
    if (!m1 || !m2) return;
    if (g_ease) { m1 = g_ease(m1); m2 = g_ease(m2); }
    void *r = g_rep(g_seq(m1, m2, nullptr));
    if (r) g_run(o, r);
    if (g_fadeTo) {
        void *f1 = g_fadeTo(t * 0.8f, (unsigned char) lo);
        void *f2 = g_fadeTo(t * 0.8f, (unsigned char) hi);
        if (f1 && f2) { void *fr = g_rep(g_seq(f1, f2, nullptr)); if (fr) g_run(o, fr); }
    }
}

static void buildOverlay() {
    if (!g_node || !g_sprite || !g_menu || !g_item || !g_setPos || !g_addChild || !g_setScale) {
        LOGI("overlay: fungsi cocos kurang");
        return;
    }
    void *root = g_node();
    if (!root) return;
    g_cx = g_winW / 2.0f; g_cy = g_winH / 2.0f;
    float cx = g_cx, cy = g_cy;
    float big = g_winW > g_winH ? g_winW : g_winH;
    g_winA = mkSprite("rift_win_a.png");
    g_winB = mkSprite("rift_win_b.png");
    // gambar bisa dimuat dengan content scale 2 -> ukuran poin = piksel / csf
    float csf = 1.0f;
    if (g_winA && g_cSize) {
        const Size2 *c = g_cSize(g_winA);
        if (c && c->w > 100.0f) csf = 1280.0f / c->w;
        if (csf < 1.0f) csf = 1.0f;
        if (csf > 4.0f) csf = 4.0f;
    }
    float F = 0.92f;                         // jendela = 92% tinggi layar
    float S = F * g_winH * csf / 709.5f;     // skala sprite jendela
    g_sp = F * g_winH / 556.0f;              // poin per unit desain
    g_S = S;
    g_tab = 0;
    LOGI("overlay: csf=%.2f S=%.3f sp=%.3f", csf, S, g_sp);
    void *dim = mkSprite("rift_dim.png");
    if (dim) {
        g_setScale(dim, big * csf / 64.0f * 1.4f);
        g_setPos(dim, cx, cy);
        g_addChild(root, dim);
    }
    char nm[32];
    for (int i = 0; i < 3; i++) {
        snprintf(nm, sizeof nm, "rift_orb%d.png", i + 1);
        void *o = mkSprite(nm);
        if (!o) continue;
        g_setScale(o, g_winH * (1.0f + 0.25f * i) * csf / 256.0f);
        float ox = cx + (i == 0 ? -0.30f : (i == 1 ? 0.32f : 0.02f)) * g_winW;
        float oy = cy + (i == 0 ? 0.18f : (i == 1 ? -0.20f : 0.34f)) * g_winH;
        g_setPos(o, ox, oy);
        g_addChild(root, o);
        animOrb(o, (i == 1 ? -1 : 1) * 0.10f * g_winW, (i == 2 ? -1 : 1) * 0.10f * g_winH, 3.2f + i * 0.9f, 70, 200);
    }
    if (g_winA) { g_setScale(g_winA, S); g_setPos(g_winA, cx, cy); g_addChild(root, g_winA); }
    if (g_winB) { g_setScale(g_winB, S); g_setPos(g_winB, cx, cy); g_addChild(root, g_winB); if (g_setVis) g_setVis(g_winB, false); }

    g_empty = mkSprite("rift_empty.png");
    if (g_empty) { g_setScale(g_empty, S); g_setPos(g_empty, cx, cy); g_addChild(root, g_empty); }

    g_restart = g_node();
    if (g_restart) {
        void *rb = mkSprite("rift_btn_restart.png");
        if (rb) { g_setScale(rb, S); g_setPos(rb, lx(501.5f), ly(554.0f)); g_addChild(g_restart, rb); }
        Color3B wh = {255, 255, 255};
        fitLabel(g_restart, "Restart game", lx(501.5f), ly(554.0f), 190.0f * g_sp, 16.0f * g_sp, 2, wh);
        g_addChild(root, g_restart);
        if (g_setVis) g_setVis(g_restart, g_pending);
    }

    void *menu = g_menu();
    if (menu) {
        g_setPos(menu, 0.0f, 0.0f);
        // cadangan kalau hook sentuhan tidak jalan: item menu biasa
        addHit(menu, "rift_hit_close.png", S, onClose, lx(930.0f), ly(86.0f));
        addHit(menu, "rift_hit_tab1.png", S, onTabA, lx(181.5f), ly(185.0f));
        addHit(menu, "rift_hit_tab2.png", S, onTabB, lx(431.5f), ly(185.0f));
        addHit(menu, "rift_hit_refresh.png", S, onRefresh, lx(869.0f), ly(253.5f));
        addHit(menu, "rift_hit_win.png", S * 10.0f, onNoop, cx, cy);
        addHit(menu, "rift_blank.png", big * csf / 8.0f * 1.3f, onClose, cx, cy);
        g_addChild(root, menu);
    }
    if (g_addChildZ) g_addChildZ(g_layer, root, 1000); else g_addChild(g_layer, root);
    g_overlay = root;
    g_open = true;
    g_press = 0;
    rebuildCards();
}

static void onRiftButton(void *self, void *sender) {
    LOGI("Tombol Rift ditekan");
    if (!g_overlay) buildOverlay();
    else {
        if (g_setVis) g_setVis(g_overlay, true);
        g_tab = 0;
        g_open = true;
        g_press = 0;
        rebuildCards();
        if (g_bcards) rebuildBrowse();
        showTab();
    }
}

// cari menu bawah GD: node dengan 4 anak, di tengah layar, sejajar horizontal
static void *findBottomMenu(void *layer, float winW) {
    if (!g_getChildren || !g_arrCount || !g_arrAt || !g_childCount || !g_getX || !g_getY) return nullptr;
    void *arr = g_getChildren(layer);
    if (!arr) return nullptr;
    unsigned n = g_arrCount(arr);
    for (unsigned i = 0; i < n; i++) {
        void *c = g_arrAt(arr, i);
        if (!c) continue;
        if (g_childCount(c) != 4) continue;
        if (fabsf(g_getX(c) - winW / 2.0f) > 3.0f) continue;
        void *ka = g_getChildren(c);
        if (!ka || g_arrCount(ka) != 4) continue;
        float mn = 1e9f, mx = -1e9f, ymin = 1e9f, ymax = -1e9f;
        for (unsigned k = 0; k < 4; k++) {
            void *ch = g_arrAt(ka, k);
            if (!ch) { mn = 1e9f; break; }
            float x = g_getX(ch), y = g_getY(ch);
            if (x < mn) mn = x;
            if (x > mx) mx = x;
            if (y < ymin) ymin = y;
            if (y > ymax) ymax = y;
        }
        if (mn > 1e8f) continue;
        if (ymax - ymin > 2.0f || mx - mn < 60.0f) continue;
        return c;
    }
    return nullptr;
}

// 0 = sukses, >0 = langkah yang gagal
static int addRiftButton(void *layer, float winW, int *mode) {
    *mode = 0;
    if (!g_sprite || !g_item || !g_setPos || !g_addChild || !g_setScale) return 1;
    void *spr = mkSprite("rift_btn_glass.png");
    if (!spr) spr = mkSprite("rift_btn.png");
    if (!spr) return 2;
    MFP cb = { (void *)onRiftButton, 0 };
    void *item = g_item(spr, nullptr, layer, cb);
    if (!item) return 3;
    void *bm = findBottomMenu(layer, winW);
    if (bm) {
        void *ka = g_getChildren(bm);
        float mn = 1e9f, mx = -1e9f, y = 0;
        for (unsigned k = 0; k < 4; k++) {
            void *ch = g_arrAt(ka, k);
            float x = g_getX(ch);
            if (x < mn) mn = x;
            if (x > mx) mx = x;
            y = g_getY(ch);
        }
        float sp = (mx - mn) / 3.0f;
        g_setScale(spr, sp * 0.83f / 104.0f);
        g_setPos(item, mx + sp, y);
        g_addChild(bm, item);
        g_setPos(bm, g_getX(bm) - sp / 2.0f, g_getY(bm));   // tengahin barisnya
        *mode = 1;
        return 0;
    }
    g_setScale(spr, 46.0f / 104.0f);
    void *menu = g_menu ? g_menu() : nullptr;
    if (!menu) return 4;
    g_setPos(menu, 0.0f, 0.0f);
    g_addChild(menu, item);
    g_setPos(item, winW / 2.0f + 143.8f, 40.2f);
    g_addChild(layer, menu);
    return 0;
}

static bool hkMenuInit(void *self) {
    bool ok = g_origInit(self);
    if (!ok) return ok;
    g_layer = self;
    g_overlay = nullptr; g_open = false; g_winA = g_winB = g_cards = g_empty = g_bcards = g_restart = nullptr;
    if (g_shared && g_winSize) {
        void *dir = g_shared();
        if (dir) {
            Size2 s = g_winSize(dir);
            if (s.w > 100.0f && s.h > 100.0f) { g_winW = s.w; g_winH = s.h; }
        }
    }
    int mode = 0;
    int err = addRiftButton(self, g_winW, &mode);
    if (err != 0 && g_label && g_setPos && g_addChild) {
        char text[32];
        snprintf(text, sizeof text, "Rift #%d", err);
        void *label = g_label(text, "bigFont.fnt");
        if (label) {
            if (g_setScale) g_setScale(label, 0.35f);
            g_setPos(label, g_winW - 40.0f, g_winH - 12.0f);
            g_addChild(self, label);
        }
    }
    LOGI("MenuLayer::init selesai, status tombol = %d, mode = %d, win = %.0fx%.0f", err, mode, g_winW, g_winH);
    return ok;
}

static void *sym2(void *h, const char *a, const char *b) {
    void *p = dlsym(h, a);
    if (!p && b) p = dlsym(h, b);
    if (!p) LOGI("simbol hilang: %s", a);
    return p;
}

__attribute__((constructor)) static void rift_main() {
    LOGI("Rift loaded!");
    void *h = dlopen("libcocos2dcpp.so", RTLD_NOW | RTLD_NOLOAD);
    if (!h) h = dlopen("libcocos2dcpp.so", RTLD_NOW);
    if (!h) { LOGI("libcocos2dcpp.so tidak ketemu"); return; }
    g_label       = (LabelCreateFn) sym2(h, "_ZN7cocos2d13CCLabelBMFont6createEPKcS2_", nullptr);
    g_setPos      = (SetPosFn)      sym2(h, "_ZN7cocos2d6CCNode11setPositionEff", nullptr);
    g_setScale    = (SetScaleFn)    sym2(h, "_ZN7cocos2d6CCNode8setScaleEf", nullptr);
    g_addChild    = (AddChildFn)    sym2(h, "_ZN7cocos2d6CCNode8addChildEPS0_", nullptr);
    g_addChildZ   = (AddChildZFn)   sym2(h, "_ZN7cocos2d6CCNode8addChildEPS0_i", nullptr);
    g_setColor    = (SetColorFn)    sym2(h, "_ZN7cocos2d13CCLabelBMFont8setColorERKNS_10_ccColor3BE", nullptr);
    g_sprite      = (SpriteCreateFn)sym2(h, "_ZN7cocos2d8CCSprite6createEPKc", nullptr);
    g_menu        = (MenuCreateFn)  sym2(h, "_ZN7cocos2d6CCMenu6createEv", nullptr);
    g_node        = (NodeCreateFn)  sym2(h, "_ZN7cocos2d6CCNode6createEv", nullptr);
    g_item        = (ItemCreateFn)  sym2(h, "_ZN21CCMenuItemSpriteExtra6createEPN7cocos2d6CCNodeES2_PNS0_8CCObjectEMS3_FvS4_E", nullptr);
    g_shared      = (SharedFn)      sym2(h, "_ZN7cocos2d10CCDirector14sharedDirectorEv", nullptr);
    g_winSize     = (WinSizeFn)     sym2(h, "_ZN7cocos2d10CCDirector10getWinSizeEv", nullptr);
    g_cSize       = (CSizeFn)       sym2(h, "_ZNK7cocos2d6CCNode14getContentSizeEv", nullptr);
    g_getChildren = (GetChildrenFn) sym2(h, "_ZN7cocos2d6CCNode11getChildrenEv", nullptr);
    g_childCount  = (CountFn)       sym2(h, "_ZNK7cocos2d6CCNode16getChildrenCountEv", "_ZN7cocos2d6CCNode16getChildrenCountEv");
    g_arrCount    = (CountFn)       sym2(h, "_ZNK7cocos2d7CCArray5countEv", "_ZN7cocos2d7CCArray5countEv");
    g_arrAt       = (ObjAtFn)       sym2(h, "_ZN7cocos2d7CCArray13objectAtIndexEj", nullptr);
    g_getX        = (GetCoordFn)    sym2(h, "_ZN7cocos2d6CCNode12getPositionXEv", "_ZNK7cocos2d6CCNode12getPositionXEv");
    g_getY        = (GetCoordFn)    sym2(h, "_ZN7cocos2d6CCNode12getPositionYEv", "_ZNK7cocos2d6CCNode12getPositionYEv");
    g_setVis      = (SetVisFn)      sym2(h, "_ZN7cocos2d6CCNode10setVisibleEb", nullptr);
    installExtra(h);
    hookTouches(h);
    void *target = dlsym(h, "_ZN9MenuLayer4initEv");
    if (!target) { LOGI("MenuLayer::init tidak ketemu"); return; }
    A64HookFunction(target, (void *)hkMenuInit, (void **)&g_origInit);
    LOGI("Hook MenuLayer::init terpasang");
    mods_init(h); mods_scan(); mods_load_enabled();
}
