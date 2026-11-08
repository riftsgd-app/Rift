#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <link.h>
#include <elf.h>
#include <sys/mman.h>
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

struct Rect4 { float x, y, w, h; };
typedef void (*OpacityFn)(void *, unsigned char);
typedef void (*SchedFn)(void *, float);
typedef void *(*SpriteRectFn)(const char *, const Rect4 &);
struct Card { void *node; float cx, cy, birth, cur; bool shown; };

static void *g_winA, *g_winB, *g_cards, *g_bcards, *g_empty, *g_restart, *g_hint;
static void *g_mTopA, *g_mTopB, *g_mBot, *g_dim;
static void *g_orb[3];
static float g_orbX[3], g_orbY[3], g_orbS[3];
static void *g_sprOn[32], *g_sprOff[32];
static SetVisFn g_remFP;
static OpacityFn g_setOp;
static SpriteRectFn g_spriteRect;
static SchedFn g_oSched;
static bool g_tickOK;
static int g_tab, g_press, g_anim, g_pressCard = -1, g_pressTab;
static bool g_open, g_pending, g_catLoaded, g_drag, g_dragArea;
static bool g_catFail[32];
static float g_cx, g_cy, g_sp, g_S, g_csf;
static float g_time, g_animT, g_restartT0 = -100.0f;
static float g_sc[2], g_vel[2], g_maxSc[2];
static float g_dragY0, g_dragSc0, g_lastT, g_dragV;
static Card g_cd[2][32];
static int g_cn2[2];
static TouchLocFn g_touchLoc;
static AnyObjFn g_anyObj;
static TouchFn g_oB, g_oM, g_oE, g_oC;
static char g_query[48];
static int g_map[2][32];
static float g_holdT0, g_pollT;
static int g_holdSlot = -1;
static bool g_holdFired;
static void *g_qlabel;

static float easeOutBack(float t) {
    float c1 = 1.70158f, c3 = c1 + 1.0f, u = t - 1.0f;
    return 1.0f + c3 * u * u * u + c1 * u * u;
}
static void pivotScale(void *n, float cx, float cy, float s) {
    if (!n) return;
    g_setScale(n, s);
    g_setPos(n, cx * (1.0f - s), cy * (1.0f - s));
}
static float lx(float rx) { return g_cx + (rx - 501.5f) * g_sp; }
static float ly(float ry) { return g_cy + (294.0f - ry) * g_sp; }

static void closeOverlay() {
    g_press = 0;
    g_pressCard = -1;
    if (g_tickOK && g_overlay) {
        if (g_anim != 2) { g_anim = 2; g_animT = 0.0f; }
        return;
    }
    g_open = false;
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

static void addZ(void *parent, void *child, int z) {
    if (g_addChildZ) g_addChildZ(parent, child, z); else g_addChild(parent, child);
}

static void showTab() {
    if (!g_setVis) return;
    int n = mods_count();
    if (g_winA) g_setVis(g_winA, g_tab == 0);
    if (g_winB) g_setVis(g_winB, g_tab == 1);
    if (g_mTopA) g_setVis(g_mTopA, g_tab == 0);
    if (g_mTopB) g_setVis(g_mTopB, g_tab == 1);
    if (g_cards) g_setVis(g_cards, g_tab == 0);
    if (g_bcards) g_setVis(g_bcards, g_tab == 1);
    if (g_restart) g_setVis(g_restart, g_pending);
    if (g_hint) g_setVis(g_hint, g_tab == 0);
    if (g_empty) g_setVis(g_empty, g_tab == 0 && n == 0);
}

static void applyScroll(int t) {
    void *node = t ? g_bcards : g_cards;
    if (node) g_setPos(node, 0.0f, g_sc[t] * g_sp);
}

static void dropNode(void **n) {
    if (!*n) return;
    if (g_remFP) g_remFP(*n, true);
    else if (g_setVis) g_setVis(*n, false);
    *n = nullptr;
}

static void refreshHint() {
    dropNode(&g_hint);
    if (!g_overlay || !g_node || g_pending || mods_count() == 0) return;
    void *h = g_node();
    if (!h) return;
    Color3B lav = {169, 154, 255};
    fitLabel(h, "Tap to toggle  -  hold to remove", lx(501.5f), ly(554.0f), 460.0f * g_sp, 13.0f * g_sp, 2, lav);
    addZ(g_overlay, h, 6);
    g_hint = h;
    showTab();
}

static bool hasQ(const char *s) {
    if (!g_query[0]) return true;
    int ql = (int) strlen(g_query);
    for (int i = 0; s && s[i]; i++) {
        int j = 0;
        while (j < ql && s[i + j]) {
            char a = s[i + j], b = g_query[j];
            if (a >= 'A' && a <= 'Z') a += 32;
            if (b >= 'A' && b <= 'Z') b += 32;
            if (a != b) break;
            j++;
        }
        if (j == ql) return true;
    }
    return false;
}
static bool matchQ(const char *a, const char *b, const char *c) { return hasQ(a) || hasQ(b) || hasQ(c); }

static void updateQuery() {
    dropNode(&g_qlabel);
    if (!g_overlay || !g_node || !g_query[0]) return;
    void *h = g_node();
    if (!h) return;
    char t[80];
    snprintf(t, sizeof t, "Filter: %s", g_query);
    Color3B lav = {169, 154, 255};
    fitLabel(h, t, lx(760.0f), ly(253.5f), 300.0f * g_sp, 17.0f * g_sp, 1, lav);
    addZ(g_overlay, h, 6);
    g_qlabel = h;
}

// siapkan kartu ke-i: node sendiri (buat animasi) di dalam wadah
static void *newCard(void *cont, int t, int i, float dx, float dy) {
    void *cn = g_node();
    if (!cn) return nullptr;
    g_addChild(cont, cn);
    Card *c = &g_cd[t][i];
    c->node = cn;
    c->cx = lx(dx + 220.0f);
    c->cy = ly(dy + 36.0f);
    c->birth = g_time + i * 0.04f;
    c->cur = 1.0f;
    c->shown = !g_tickOK;
    if (g_tickOK) { if (g_setVis) g_setVis(cn, false); pivotScale(cn, c->cx, c->cy, 0.8f); }
    return cn;
}

static void finishList(int t, void *cont, int n) {
    g_cn2[t] = n;
    int rows = (n + 1) / 2;
    float mx = 82.0f * rows - 246.0f;
    g_maxSc[t] = mx > 0.0f ? mx : 0.0f;
    if (g_sc[t] > g_maxSc[t]) g_sc[t] = g_maxSc[t];
    if (g_sc[t] < 0.0f) g_sc[t] = 0.0f;
    g_vel[t] = 0.0f;
    addZ(g_overlay, cont, 4);
    if (t) g_bcards = cont; else g_cards = cont;
    applyScroll(t);
    showTab();
}

static void rebuildCards() {
    if (!g_overlay || !g_node) return;
    dropNode(&g_cards);
    int n = mods_scan();
    void *cont = g_node();
    if (!cont) return;
    Color3B white = {255, 255, 255}, lav = {169, 154, 255}, soft = {205, 205, 222};
    int k = 0;
    for (int r = 0; r < n && k < 32; r++) {
        ModInfo *m = mods_get(r);
        if (!m || !matchQ(m->name, m->author, m->desc)) continue;
        int i = k++;
        g_map[0][i] = r;
        float dx = 52.0f + (i % 2) * 456.0f, dy = 300.0f + (i / 2) * 82.0f;
        void *cn = newCard(cont, 0, i, dx, dy);
        if (!cn) continue;
        void *on = mkSprite("rift_card_on.png");
        void *off = mkSprite("rift_card_off.png");
        if (on) { g_setScale(on, g_S); g_setPos(on, lx(dx + 220.0f), ly(dy + 36.0f)); g_addChild(cn, on); }
        if (off) { g_setScale(off, g_S); g_setPos(off, lx(dx + 220.0f), ly(dy + 36.0f)); g_addChild(cn, off); }
        g_sprOn[i] = on; g_sprOff[i] = off;
        if (g_setVis) { if (on) g_setVis(on, m->enabled); if (off) g_setVis(off, !m->enabled); }
        fitLabel(cn, m->name, lx(dx + 72.0f), ly(dy + 24.0f), 200.0f * g_sp, 26.0f * g_sp, 0, white);
        char by[80];
        snprintf(by, sizeof by, "by %s", m->author[0] ? m->author : "?");
        fitLabel(cn, by, lx(dx + 426.0f), ly(dy + 24.0f), 120.0f * g_sp, 17.0f * g_sp, 1, lav);
        fitLabel(cn, m->desc[0] ? m->desc : "No description", lx(dx + 72.0f), ly(dy + 52.0f), 285.0f * g_sp, 15.0f * g_sp, 0, soft);
    }
    finishList(0, cont, k);
    refreshHint();
    updateQuery();
}

static void rebuildBrowse() {
    if (!g_overlay || !g_node) return;
    dropNode(&g_bcards);
    int n = cat_count();
    void *cont = g_node();
    if (!cont) return;
    Color3B white = {255, 255, 255}, lav = {169, 154, 255}, soft = {205, 205, 222};
    int k = 0;
    for (int r = 0; r < n && k < 32; r++) {
        CatInfo *c = cat_get(r);
        if (!c || !matchQ(c->name, c->author, c->desc)) continue;
        int i = k++;
        g_map[1][i] = r;
        float dx = 52.0f + (i % 2) * 456.0f, dy = 300.0f + (i / 2) * 82.0f;
        void *cn = newCard(cont, 1, i, dx, dy);
        if (!cn) continue;
        void *bc = mkSprite("rift_bcard.png");
        if (bc) { g_setScale(bc, g_S); g_setPos(bc, lx(dx + 220.0f), ly(dy + 36.0f)); g_addChild(cn, bc); }
        fitLabel(cn, c->name, lx(dx + 72.0f), ly(dy + 24.0f), 190.0f * g_sp, 26.0f * g_sp, 0, white);
        char by[80];
        snprintf(by, sizeof by, "by %s", c->author[0] ? c->author : "?");
        fitLabel(cn, by, lx(dx + 426.0f), ly(dy + 24.0f), 110.0f * g_sp, 17.0f * g_sp, 1, lav);
        fitLabel(cn, c->desc[0] ? c->desc : "No description", lx(dx + 72.0f), ly(dy + 52.0f), 262.0f * g_sp, 15.0f * g_sp, 0, soft);
        const char *txt = "Download";
        bool violet = true;
        if (!c->url[0]) { txt = "Soon"; violet = false; }
        else if (cat_installed(r)) { txt = "Installed"; violet = false; }
        else if (g_catFail[r]) { txt = "Retry"; }
        void *pl = mkSprite(violet ? "rift_pill_v.png" : "rift_pill_g.png");
        if (pl) { g_setScale(pl, g_S); g_setPos(pl, lx(dx + 386.0f), ly(dy + 51.0f)); g_addChild(cn, pl); }
        fitLabel(cn, txt, lx(dx + 386.0f), ly(dy + 51.0f), 66.0f * g_sp, 14.0f * g_sp, 2, white);
    }
    finishList(1, cont, k);
    updateQuery();
}

static void popCards(int t) {
    for (int i = 0; i < g_cn2[t]; i++) {
        Card *c = &g_cd[t][i];
        c->birth = g_time + i * 0.035f;
        c->shown = !g_tickOK;
        if (g_tickOK && c->node) { if (g_setVis) g_setVis(c->node, false); pivotScale(c->node, c->cx, c->cy, 0.8f); }
    }
}

static void onTabA(void *self, void *sender) {
    if (g_tab == 0) return;
    g_tab = 0;
    popCards(0);
    showTab();
}
static void onTabB(void *self, void *sender) {
    if (g_tab == 1) return;
    g_tab = 1;
    if (!g_catLoaded) { g_catLoaded = true; cat_fetch(); rebuildBrowse(); }
    else popCards(1);
    showTab();
}
static void onRefresh(void *self, void *sender) {
    LOGI("Refresh ditekan");
    if (g_tab == 0) rebuildCards();
    else { cat_fetch(); g_catLoaded = true; rebuildBrowse(); }
}

static void setPending() {
    if (!g_pending) { g_pending = true; g_restartT0 = g_time; }
}

static void doDownload(int i) {
    CatInfo *c = cat_get(i);
    if (!c || !c->url[0] || cat_installed(i)) return;
    const char *err = cat_download(i);
    if (err) { g_catFail[i] = true; LOGI("download %s gagal: %s", c->id, err); }
    else { g_catFail[i] = false; setPending(); LOGI("download %s ok", c->id); rebuildCards(); }
    rebuildBrowse();
}

// ---- sentuhan: dicegat langsung di CCTouchDispatcher::touches, tidak tergantung CCMenu ----
static bool inR(float x, float y, float l, float t, float r, float b, float pad) {
    return x >= lx(l) - pad && x <= lx(r) + pad && y <= ly(t) + pad && y >= ly(b) - pad;
}
static bool inList(float x, float y) {
    float yd = 294.0f - (y - g_cy) / g_sp, xd = 501.5f + (x - g_cx) / g_sp;
    return yd >= 296.0f && yd <= 540.0f && xd >= 40.0f && xd <= 962.0f;
}
static int hitTest(float x, float y) {
    float pad = 6.0f * g_sp;
    float dx = x - lx(930.0f), dy = y - ly(86.0f), rr = 34.0f * g_sp;
    if (dx * dx + dy * dy <= rr * rr) return 1;
    if (inR(x, y, 52, 160, 311, 210, pad)) return 2;
    if (inR(x, y, 326, 160, 537, 210, pad)) return 3;
    if (inR(x, y, 790, 228, 948, 279, pad)) return 4;
    if (g_pending && inR(x, y, 386.5f, 539.0f, 616.5f, 569.0f, 3.0f * g_sp)) return 20;
    if (inR(x, y, 52, 228, 772, 279, pad)) return 7;
    if (inList(x, y)) {
        for (int i = 0; i < g_cn2[g_tab]; i++) {
            float l = 52.0f + (i % 2) * 456.0f, t = 300.0f + (i / 2) * 82.0f - g_sc[g_tab];
            if (g_tab == 0) { if (inR(x, y, l, t, l + 440.0f, t + 72.0f, 0)) return 100 + i; }
            else if (inR(x, y, l + 346.0f, t + 38.0f, l + 426.0f, t + 64.0f, 5.0f * g_sp)) return 200 + i;
        }
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
    else if (a == 7) { char cmd[80]; snprintf(cmd, sizeof cmd, "search:%s", g_query); riftnet_ui(cmd); }
    else if (a >= 200) doDownload(g_map[1][(a - 200) & 31]);
    else if (a >= 100) {
        int i = (a - 100) & 31;
        ModInfo *m = mods_get(g_map[0][i]);
        if (m) {
            mods_toggle(g_map[0][i]);
            if (g_setVis) { if (g_sprOn[i]) g_setVis(g_sprOn[i], m->enabled); if (g_sprOff[i]) g_setVis(g_sprOff[i], !m->enabled); }
            g_cd[0][i].cur = 0.88f;
            setPending();
            refreshHint();
            showTab();
        }
    }
}
static void setPressCard(int h) {
    if (h >= 200) { g_pressCard = h - 200; g_pressTab = 1; }
    else if (h >= 100) { g_pressCard = h - 100; g_pressTab = 0; }
    else g_pressCard = -1;
}
static bool handleTouch(void *set, int phase) {
    if (!g_open || !g_anyObj || !g_touchLoc || !set) return false;
    void *t = g_anyObj(set);
    if (!t) return true;
    if (g_anim == 2) return true;
    Pt2 p = g_touchLoc(t);
    int tb = g_tab;
    if (phase == 0) {
        g_press = hitTest(p.x, p.y);
        g_drag = false;
        g_dragArea = inList(p.x, p.y);
        g_dragY0 = p.y;
        g_dragSc0 = g_sc[tb];
        g_vel[tb] = 0.0f;
        g_lastT = g_time;
        g_dragV = 0.0f;
        setPressCard(g_press);
        g_holdSlot = (g_tab == 0 && g_press >= 100 && g_press < 200) ? g_press - 100 : -1;
        g_holdT0 = g_time;
        g_holdFired = false;
    } else if (phase == 1) {
        if (g_dragArea) {
            float dy = (p.y - g_dragY0) / g_sp;
            if (!g_drag && (dy > 6.0f || dy < -6.0f)) { g_drag = true; g_press = 0; g_holdSlot = -1; setPressCard(0); }
            if (g_drag) {
                float sc = g_dragSc0 + dy, mx = g_maxSc[tb];
                if (sc < 0.0f) sc *= 0.4f;
                else if (sc > mx) sc = mx + (sc - mx) * 0.4f;
                float dt = g_time - g_lastT;
                if (dt > 0.001f) g_dragV = g_dragV * 0.6f + ((sc - g_sc[tb]) / dt) * 0.4f;
                g_sc[tb] = sc;
                g_lastT = g_time;
                applyScroll(tb);
            }
        }
    } else {
        setPressCard(0);
        g_holdSlot = -1;
        if (g_drag) {
            g_drag = false;
            g_press = 0;
            float v = (g_time - g_lastT > 0.12f) ? 0.0f : g_dragV;
            if (v > 2500.0f) v = 2500.0f;
            if (v < -2500.0f) v = -2500.0f;
            g_vel[tb] = v;
        } else if (phase == 2) {
            int pr = g_press, h = hitTest(p.x, p.y);
            g_press = 0;
            if (pr == h) doAction(h);
        } else g_press = 0;
    }
    return true;
}

static void applyRoot(float s, float op) {
    pivotScale(g_overlay, g_cx, g_cy, s);
    if (g_setOp && g_dim) g_setOp(g_dim, (unsigned char) (op * 255.0f));
}

static void tick(float dt) {
    if (dt > 0.1f) dt = 0.1f;
    g_time += dt;
    if (!g_overlay || !g_open) return;
    if (g_anim) {
        g_animT += dt;
        float d = g_anim == 1 ? 0.30f : 0.18f, t = g_animT / d;
        if (t > 1.0f) t = 1.0f;
        if (g_anim == 1) {
            float op = t * 1.6f;
            applyRoot(0.85f + 0.15f * easeOutBack(t), op > 1.0f ? 1.0f : op);
        } else applyRoot(1.0f - 0.08f * t * t, 1.0f - t);
        if (t >= 1.0f) {
            if (g_anim == 2) { g_open = false; if (g_setVis) g_setVis(g_overlay, false); }
            g_anim = 0;
            applyRoot(1.0f, 1.0f);
        }
    }
    for (int i = 0; i < 3; i++) {
        if (!g_orb[i]) continue;
        g_setPos(g_orb[i], g_orbX[i] + sinf(g_time * (0.5f + 0.17f * i) + i * 2.1f) * 0.06f * g_winW,
                 g_orbY[i] + cosf(g_time * (0.4f + 0.13f * i) + i) * 0.08f * g_winH);
        g_setScale(g_orb[i], g_orbS[i] * (1.0f + 0.06f * sinf(g_time * 0.9f + i)));
    }
    int t = g_tab;
    if (!g_drag) {
        float mx = g_maxSc[t];
        if (g_sc[t] < 0.0f || g_sc[t] > mx) {
            float bound = g_sc[t] < 0.0f ? 0.0f : mx, step = dt * 12.0f;
            g_sc[t] += (bound - g_sc[t]) * (step > 1.0f ? 1.0f : step);
            g_vel[t] = 0.0f;
            if (g_sc[t] - bound < 0.2f && bound - g_sc[t] < 0.2f) g_sc[t] = bound;
            applyScroll(t);
        } else if (g_vel[t] > 4.0f || g_vel[t] < -4.0f) {
            g_sc[t] += g_vel[t] * dt;
            g_vel[t] *= expf(-3.2f * dt);
            applyScroll(t);
        } else g_vel[t] = 0.0f;
    }
    for (int i = 0; i < g_cn2[t]; i++) {
        Card *c = &g_cd[t][i];
        if (!c->node) continue;
        float age = g_time - c->birth;
        if (age < 0.0f) continue;
        float ctop = 300.0f + (i / 2) * 82.0f - g_sc[t];
        if (ctop >= 538.0f || ctop + 72.0f <= 298.0f) { if (c->shown && g_setVis) g_setVis(c->node, false); c->shown = false; continue; }
        if (!c->shown) { c->shown = true; if (g_setVis) g_setVis(c->node, true); }
        float pop = age < 0.28f ? 0.8f + 0.2f * easeOutBack(age / 0.28f) : 1.0f;
        float tgt = (g_pressCard == i && g_pressTab == t) ? 0.95f : 1.0f, step = dt * 16.0f;
        c->cur += (tgt - c->cur) * (step > 1.0f ? 1.0f : step);
        pivotScale(c->node, c->cx, c->cy, pop * c->cur);
    }
    if (g_holdSlot >= 0 && !g_drag && !g_holdFired && g_time - g_holdT0 > 0.7f) {
        g_holdFired = true;
        g_press = 0;
        ModInfo *hm = mods_get(g_map[0][g_holdSlot & 31]);
        if (hm) {
            char base[160];
            const char *d = hm->dir;
            int dl = (int) strlen(d);
            while (dl > 0 && d[dl - 1] == '/') dl--;
            int st = dl;
            while (st > 0 && d[st - 1] != '/') st--;
            int len = dl - st;
            if (len > 150) len = 150;
            memcpy(base, d + st, len);
            base[len] = 0;
            for (int q = 0; q < 2; q++) { char *dash = strrchr(base, '-'); if (dash) *dash = 0; }
            char cmd[200];
            snprintf(cmd, sizeof cmd, "del:%s", base);
            riftnet_ui(cmd);
        }
    }
    g_pollT += dt;
    if (g_pollT > 0.3f) {
        g_pollT = 0.0f;
        char r[96];
        if (riftnet_poll(r, sizeof r)) {
            if (r[0] == '=') {
                snprintf(g_query, sizeof g_query, "%s", r + 1);
                rebuildCards();
                if (g_bcards) rebuildBrowse();
            } else if (r[0] == '!') {
                setPending();
                rebuildCards();
            }
        }
    }
    if (g_restart && g_pending) {
        float age = g_time - g_restartT0;
        pivotScale(g_restart, lx(501.5f), ly(554.0f), age < 0.35f ? 0.6f + 0.4f * easeOutBack(age / 0.35f) : 1.0f + 0.025f * sinf(g_time * 4.0f));
    }
}

// ---- save data: catat jalur file yang dibuka game, dan alihkan jalur GD asli ke folder data Rift ----
typedef FILE *(*FopenFn)(const char *, const char *);
typedef int (*OpenFn)(const char *, int, int);
static FopenFn g_oFopen;
static OpenFn g_oOpen;
static int g_fileLog;
static const char *redirPath(const char *p, char *buf, int n) {
    static const char *pre[2] = { "/data/data/com.robtopx.geometryjump/", "/data/user/0/com.robtopx.geometryjump/" };
    for (int i = 0; i < 2; i++) {
        size_t l = strlen(pre[i]);
        if (strncmp(p, pre[i], l) == 0) { snprintf(buf, n, "/data/user/0/com.rift.launcher/%s", p + l); return buf; }
    }
    return p;
}
static bool wantLog(const char *p) {
    return g_fileLog < 80 && (strstr(p, ".dat") || strstr(p, "/data/")) && !strstr(p, "modcache") && !strstr(p, "rift_") && !strstr(p, "/proc/");
}
static FILE *hkFopen(const char *p, const char *m) {
    if (!p) return g_oFopen(p, m);
    char b[600];
    const char *q = redirPath(p, b, sizeof b);
    FILE *f = g_oFopen(q, m);
    if (wantLog(p)) { g_fileLog++; LOGI("fopen %s [%s] -> %s%s", p, m ? m : "?", f ? "ok" : "GAGAL", q != p ? " (dialihkan)" : ""); }
    return f;
}
static int hkOpen(const char *p, int fl, int mode) {
    if (!p) return g_oOpen(p, fl, mode);
    char b[600];
    const char *q = redirPath(p, b, sizeof b);
    int fd = g_oOpen(q, fl, mode);
    if (wantLog(p)) { g_fileLog++; LOGI("open %s -> %s%s", p, fd >= 0 ? "ok" : "GAGAL", q != p ? " (dialihkan)" : ""); }
    return fd;
}
struct GotCtx { unsigned long base; Elf64_Dyn *dyn; };
static int gotCb(struct dl_phdr_info *info, size_t sz, void *d) {
    GotCtx *c = (GotCtx *) d;
    if (!info->dlpi_name || !strstr(info->dlpi_name, "libcocos2dcpp.so")) return 0;
    c->base = (unsigned long) info->dlpi_addr;
    for (int k = 0; k < info->dlpi_phnum; k++)
        if (info->dlpi_phdr[k].p_type == PT_DYNAMIC) c->dyn = (Elf64_Dyn *) (info->dlpi_addr + info->dlpi_phdr[k].p_vaddr);
    return 1;
}
static unsigned long gotAbs(unsigned long v, unsigned long base) { return v < base ? v + base : v; }
static int gotPatch(GotCtx *c, const char *name, void *repl, void **orig) {
    Elf64_Sym *sym = nullptr;
    const char *str = nullptr;
    Elf64_Rela *rel[2] = { nullptr, nullptr };
    unsigned long relsz[2] = { 0, 0 };
    for (Elf64_Dyn *d = c->dyn; d->d_tag != DT_NULL; d++) {
        if (d->d_tag == DT_SYMTAB) sym = (Elf64_Sym *) gotAbs(d->d_un.d_ptr, c->base);
        else if (d->d_tag == DT_STRTAB) str = (const char *) gotAbs(d->d_un.d_ptr, c->base);
        else if (d->d_tag == DT_JMPREL) rel[0] = (Elf64_Rela *) gotAbs(d->d_un.d_ptr, c->base);
        else if (d->d_tag == DT_PLTRELSZ) relsz[0] = d->d_un.d_val;
        else if (d->d_tag == DT_RELA) rel[1] = (Elf64_Rela *) gotAbs(d->d_un.d_ptr, c->base);
        else if (d->d_tag == DT_RELASZ) relsz[1] = d->d_un.d_val;
    }
    if (!sym || !str) return 0;
    int n = 0;
    for (int t = 0; t < 2; t++) {
        if (!rel[t]) continue;
        unsigned long cnt = relsz[t] / sizeof(Elf64_Rela);
        for (unsigned long k = 0; k < cnt; k++) {
            unsigned type = ELF64_R_TYPE(rel[t][k].r_info);
            if (type != 1025 && type != 1026) continue;
            Elf64_Sym *sy = &sym[ELF64_R_SYM(rel[t][k].r_info)];
            if (strcmp(str + sy->st_name, name) != 0) continue;
            void **slot = (void **) (c->base + rel[t][k].r_offset);
            unsigned long pg = (unsigned long) slot & ~4095UL;
            if (mprotect((void *) pg, 4096, PROT_READ | PROT_WRITE) != 0) continue;
            if (!*orig) *orig = *slot;
            *slot = repl;
            n++;
        }
    }
    return n;
}
static void riftFileHook() {
    GotCtx c = { 0, nullptr };
    dl_iterate_phdr(gotCb, &c);
    if (!c.dyn) { LOGI("hook file: libcocos2dcpp tidak ketemu"); return; }
    int a = gotPatch(&c, "fopen", (void *) hkFopen, (void **) &g_oFopen);
    int b = gotPatch(&c, "open", (void *) hkOpen, (void **) &g_oOpen);
    LOGI("hook file (GOT): fopen=%d open=%d", a, b);
}

static void hkTB(void *s, void *set, void *ev) { if (handleTouch(set, 0)) return; g_oB(s, set, ev); }
static void hkTM(void *s, void *set, void *ev) { if (handleTouch(set, 1)) return; g_oM(s, set, ev); }
static void hkTE(void *s, void *set, void *ev) { if (handleTouch(set, 2)) return; g_oE(s, set, ev); }
static void hkTC(void *s, void *set, void *ev) { if (handleTouch(set, 3)) return; g_oC(s, set, ev); }

static void installExtra(void *h) {
    g_touchLoc   = (TouchLocFn)   sym2(h, "_ZNK7cocos2d7CCTouch11getLocationEv", "_ZN7cocos2d7CCTouch11getLocationEv");
    g_anyObj     = (AnyObjFn)     sym2(h, "_ZN7cocos2d5CCSet9anyObjectEv", "_ZNK7cocos2d5CCSet9anyObjectEv");
    g_remFP      = (SetVisFn)     sym2(h, "_ZN7cocos2d6CCNode27removeFromParentAndCleanupEb", nullptr);
    g_setOp      = (OpacityFn)    sym2(h, "_ZN7cocos2d8CCSprite10setOpacityEh", "_ZN7cocos2d9CCNodeRGBA10setOpacityEh");
    g_spriteRect = (SpriteRectFn) sym2(h, "_ZN7cocos2d8CCSprite6createEPKcRKNS_7CCRectE", nullptr);
    LOGI("extra: touch=%d opacity=%d rect=%d", g_touchLoc != nullptr, g_setOp != nullptr, g_spriteRect != nullptr);
}

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
static void hkSched(void *s, float dt) {
    g_oSched(s, dt);
    tick(dt);
}
static void hookTouches(void *h) {
    void *t = dlsym(h, "_ZN7cocos2d17CCTouchDispatcher7touchesEPNS_5CCSetEPNS_7CCEventEj");
    if (t && g_touchLoc && g_anyObj) {
        A64HookFunction(t, (void *) hkTouches, (void **) &g_oTouches);
        LOGI("hook touches terpasang");
    } else LOGI("hook touches GAGAL");
    void *sc = dlsym(h, "_ZN7cocos2d11CCScheduler6updateEf");
    if (sc) {
        A64HookFunction(sc, (void *) hkSched, (void **) &g_oSched);
        g_tickOK = g_oSched != nullptr;
        LOGI("hook scheduler terpasang (animasi aktif)");
    } else LOGI("simbol hilang: CCScheduler::update (animasi mati)");
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

// salinan bagian atas/bawah gambar jendela: nutup kartu yang ke-scroll keluar area daftar
static void *mkMask(const char *file, float y0, float h) {
    if (!g_spriteRect) return nullptr;
    char path[256];
    snprintf(path, sizeof path, RIFT_DIR "%s", file);
    Rect4 r = { 0.0f, y0 / g_csf, 1280.0f / g_csf, h / g_csf };
    void *s = g_spriteRect(path, r);
    if (!s) return nullptr;
    g_setScale(s, g_S);
    g_setPos(s, g_cx, g_cy + (375.0f - (y0 + h / 2.0f)) / g_csf * g_S);
    return s;
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
    float csf = 1.0f;
    if (g_winA && g_cSize) {
        const Size2 *c = g_cSize(g_winA);
        if (c && c->w > 100.0f) csf = 1280.0f / c->w;
        if (csf < 1.0f) csf = 1.0f;
        if (csf > 4.0f) csf = 4.0f;
    }
    float F = 0.92f;
    float S = F * g_winH * csf / 709.5f;
    g_sp = F * g_winH / 556.0f;
    g_S = S;
    g_csf = csf;
    g_tab = 0;
    g_anim = 0;
    LOGI("overlay: csf=%.2f S=%.3f sp=%.3f tick=%d", csf, S, g_sp, (int) g_tickOK);
    g_dim = mkSprite("rift_dim.png");
    if (g_dim) {
        g_setScale(g_dim, big * csf / 64.0f * 1.4f);
        g_setPos(g_dim, cx, cy);
        addZ(root, g_dim, 0);
    }
    char nm[32];
    for (int i = 0; i < 3; i++) {
        snprintf(nm, sizeof nm, "rift_orb%d.png", i + 1);
        void *o = mkSprite(nm);
        if (!o) continue;
        g_orbS[i] = g_winH * (1.0f + 0.25f * i) * csf / 256.0f;
        g_orbX[i] = cx + (i == 0 ? -0.30f : (i == 1 ? 0.32f : 0.02f)) * g_winW;
        g_orbY[i] = cy + (i == 0 ? 0.18f : (i == 1 ? -0.20f : 0.34f)) * g_winH;
        g_setScale(o, g_orbS[i]);
        g_setPos(o, g_orbX[i], g_orbY[i]);
        addZ(root, o, 1);
        g_orb[i] = o;
    }
    if (g_winA) { g_setScale(g_winA, S); g_setPos(g_winA, cx, cy); addZ(root, g_winA, 2); }
    if (g_winB) { g_setScale(g_winB, S); g_setPos(g_winB, cx, cy); addZ(root, g_winB, 2); if (g_setVis) g_setVis(g_winB, false); }
    g_empty = mkSprite("rift_empty.png");
    if (g_empty) { g_setScale(g_empty, S); g_setPos(g_empty, cx, cy); addZ(root, g_empty, 3); }
    g_mTopA = mkMask("rift_win_a.png", 0.0f, 378.0f);
    g_mTopB = mkMask("rift_win_b.png", 0.0f, 378.0f);
    g_mBot = mkMask("rift_win_a.png", 689.0f, 61.0f);
    if (g_mTopA) addZ(root, g_mTopA, 5);
    if (g_mTopB) { addZ(root, g_mTopB, 5); if (g_setVis) g_setVis(g_mTopB, false); }
    if (g_mBot) addZ(root, g_mBot, 5);

    g_restart = g_node();
    if (g_restart) {
        void *rb = mkSprite("rift_btn_restart.png");
        if (rb) { g_setScale(rb, S); g_setPos(rb, lx(501.5f), ly(554.0f)); g_addChild(g_restart, rb); }
        Color3B wh = {255, 255, 255};
        fitLabel(g_restart, "Restart game", lx(501.5f), ly(554.0f), 190.0f * g_sp, 16.0f * g_sp, 2, wh);
        addZ(root, g_restart, 6);
        if (g_setVis) g_setVis(g_restart, g_pending);
    }
    void *menu = g_menu();
    if (menu) {
        g_setPos(menu, 0.0f, 0.0f);
        addHit(menu, "rift_hit_close.png", S, onClose, lx(930.0f), ly(86.0f));
        addHit(menu, "rift_hit_tab1.png", S, onTabA, lx(181.5f), ly(185.0f));
        addHit(menu, "rift_hit_tab2.png", S, onTabB, lx(431.5f), ly(185.0f));
        addHit(menu, "rift_hit_refresh.png", S, onRefresh, lx(869.0f), ly(253.5f));
        addHit(menu, "rift_hit_win.png", S * 10.0f, onNoop, cx, cy);
        addHit(menu, "rift_blank.png", big * csf / 8.0f * 1.3f, onClose, cx, cy);
        addZ(root, menu, 7);
    }
    if (g_addChildZ) g_addChildZ(g_layer, root, 1000); else g_addChild(g_layer, root);
    g_overlay = root;
    g_open = true;
    g_press = 0;
    if (g_tickOK) { g_anim = 1; g_animT = 0.0f; applyRoot(0.85f, 0.0f); }
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
        if (g_tickOK) { g_anim = 1; g_animT = 0.0f; applyRoot(0.85f, 0.0f); }
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
    g_overlay = nullptr; g_open = false; g_winA = g_winB = g_cards = g_empty = g_bcards = g_restart = g_hint = g_mTopA = g_mTopB = g_mBot = g_dim = nullptr; g_anim = 0; g_orb[0] = g_orb[1] = g_orb[2] = nullptr; g_cn2[0] = g_cn2[1] = 0; g_qlabel = nullptr; g_query[0] = 0; g_holdSlot = -1;
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
    riftFileHook();
    void *target = dlsym(h, "_ZN9MenuLayer4initEv");
    if (!target) { LOGI("MenuLayer::init tidak ketemu"); return; }
    A64HookFunction(target, (void *)hkMenuInit, (void **)&g_origInit);
    LOGI("Hook MenuLayer::init terpasang");
    mods_init(h); mods_scan(); mods_load_enabled();
}
