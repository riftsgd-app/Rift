#include <dlfcn.h>
#include <stdio.h>
#include "syms.h"
extern "C" int __android_log_print(int prio, const char *tag, const char *fmt, ...);
extern "C" void A64HookFunction(void *const symbol, void *const replace, void **result);
#define LOGI(...) __android_log_print(4, "Rift", __VA_ARGS__)

#ifndef SYM_MENUINIT
#define SYM_MENUINIT "_ZN9MenuLayer4initEv"
#endif
#ifndef SYM_LABEL
#define SYM_LABEL "_ZN7cocos2d13CCLabelBMFont6createEPKcS2_"
#endif
#ifndef SYM_SETSTR
#define SYM_SETSTR "_ZN7cocos2d13CCLabelBMFont9setStringEPKc"
#endif
#ifndef SYM_SETPOS
#define SYM_SETPOS "_ZN7cocos2d6CCNode11setPositionEff"
#endif
#ifndef SYM_SETSCALE
#define SYM_SETSCALE "_ZN7cocos2d6CCNode8setScaleEf"
#endif
#ifndef SYM_ADDCHILD
#define SYM_ADDCHILD "_ZN7cocos2d6CCNode8addChildEPS0_"
#endif
#ifndef SYM_SETCOLOR
#define SYM_SETCOLOR "_ZN7cocos2d13CCLabelBMFont8setColorERKNS_10_ccColor3BE"
#endif
#ifndef SYM_SPRITE
#define SYM_SPRITE "_ZN7cocos2d8CCSprite6createEPKc"
#endif
#ifndef SYM_MENU
#define SYM_MENU "_ZN7cocos2d6CCMenu6createEv"
#endif
#ifndef SYM_ITEM
#define SYM_ITEM "_ZN20CCMenuItemSpriteExtra6createEPN7cocos2d6CCNodeES2_PNS0_8CCObjectEMS3_FvS4_E"
#endif
#ifndef SYM_SHARED
#define SYM_SHARED "_ZN7cocos2d10CCDirector14sharedDirectorEv"
#endif
#ifndef SYM_WINSIZE
#define SYM_WINSIZE "_ZN7cocos2d10CCDirector10getWinSizeEv"
#endif
#ifndef SYM_CSIZE
#define SYM_CSIZE "_ZNK7cocos2d6CCNode14getContentSizeEv"
#endif

struct Color3B { unsigned char r, g, b; };
struct Size2 { float w, h; };
struct MFP { void *fn; long adj; };   // pointer-to-member-function (ABI ARM64)

typedef bool (*InitFn)(void *);
typedef void *(*LabelCreateFn)(const char *, const char *);
typedef void (*SetStrFn)(void *, const char *);
typedef void (*SetPosFn)(void *, float, float);
typedef void (*SetScaleFn)(void *, float);
typedef void (*AddChildFn)(void *, void *);
typedef void (*SetColorFn)(void *, const Color3B &);
typedef void *(*SpriteCreateFn)(const char *);
typedef void *(*MenuCreateFn)();
typedef void *(*ItemCreateFn)(void *, void *, void *, MFP);
typedef void *(*SharedFn)();
typedef Size2 (*WinSizeFn)(void *);
typedef const Size2 *(*CSizeFn)(void *);

static InitFn g_origInit;
static LabelCreateFn g_label;
static SetStrFn g_setStr;
static SetPosFn g_setPos;
static SetScaleFn g_setScale;
static AddChildFn g_addChild;
static SetColorFn g_setColor;
static SpriteCreateFn g_sprite;
static MenuCreateFn g_menu;
static ItemCreateFn g_item;
static SharedFn g_shared;
static WinSizeFn g_winSize;
static CSizeFn g_cSize;
static void *g_statusLabel;

static void onRiftButton(void *self, void *sender) {
    LOGI("Tombol Rift ditekan");
    if (g_statusLabel && g_setStr) g_setStr(g_statusLabel, "Rift: belum ada mod dimuat");
}

// kembalikan 0 kalau sukses, angka > 0 = langkah yang gagal
static int addRiftButton(void *menuLayer, float winW) {
    if (!g_sprite || !g_menu || !g_item || !g_setPos || !g_addChild) return 1;
    void *spr = g_sprite("rift_btn.png");
    if (!spr) return 2;
    if (g_setScale && g_cSize) {
        const Size2 *cs = g_cSize(spr);
        if (cs && cs->w > 1.0f && cs->w < 4096.0f) g_setScale(spr, 46.0f / cs->w);
    }
    MFP cb = { (void *)onRiftButton, 0 };
    void *item = g_item(spr, nullptr, menuLayer, cb);
    if (!item) return 3;
    void *menu = g_menu();
    if (!menu) return 4;
    g_setPos(menu, 0.0f, 0.0f);
    g_addChild(menu, item);
    g_setPos(item, winW / 2.0f + 140.0f, 44.0f);
    g_addChild(menuLayer, menu);
    return 0;
}

static bool hkMenuInit(void *self) {
    bool ok = g_origInit(self);
    if (!ok) return ok;
    float winW = 683.0f, winH = 320.0f;
    if (g_shared && g_winSize) {
        void *dir = g_shared();
        if (dir) { Size2 s = g_winSize(dir); if (s.w > 100.0f && s.h > 100.0f) { winW = s.w; winH = s.h; } }
    }
    int err = addRiftButton(self, winW);
    char text[64];
    if (err == 0) snprintf(text, sizeof text, "Rift 0.1");
    else snprintf(text, sizeof text, "Rift 0.1 - tombol gagal #%d", err);
    if (g_label && g_setPos && g_addChild) {
        void *label = g_label(text, "bigFont.fnt");
        if (label) {
            if (g_setScale) g_setScale(label, 0.5f);
            if (g_setColor) { Color3B c = {0x76, 0x57, 0xFF}; g_setColor(label, c); }
            g_setPos(label, 130.0f, winH - 40.0f);
            g_addChild(self, label);
            g_statusLabel = label;
        }
    }
    LOGI("MenuLayer::init selesai, status tombol = %d", err);
    return ok;
}

__attribute__((constructor)) static void rift_main() {
    LOGI("Rift loaded!");
    void *h = dlopen("libcocos2dcpp.so", RTLD_NOW | RTLD_NOLOAD);
    if (!h) h = dlopen("libcocos2dcpp.so", RTLD_NOW);
    if (!h) { LOGI("libcocos2dcpp.so tidak ketemu"); return; }
    g_label    = (LabelCreateFn)dlsym(h, SYM_LABEL);
    g_setStr   = (SetStrFn)     dlsym(h, SYM_SETSTR);
    g_setPos   = (SetPosFn)     dlsym(h, SYM_SETPOS);
    g_setScale = (SetScaleFn)   dlsym(h, SYM_SETSCALE);
    g_addChild = (AddChildFn)   dlsym(h, SYM_ADDCHILD);
    g_setColor = (SetColorFn)   dlsym(h, SYM_SETCOLOR);
    g_sprite   = (SpriteCreateFn)dlsym(h, SYM_SPRITE);
    g_menu     = (MenuCreateFn) dlsym(h, SYM_MENU);
    g_item     = (ItemCreateFn) dlsym(h, SYM_ITEM);
    g_shared   = (SharedFn)     dlsym(h, SYM_SHARED);
    g_winSize  = (WinSizeFn)    dlsym(h, SYM_WINSIZE);
    g_cSize    = (CSizeFn)      dlsym(h, SYM_CSIZE);
    void *target = dlsym(h, SYM_MENUINIT);
    if (!target) { LOGI("MenuLayer::init tidak ketemu"); return; }
    A64HookFunction(target, (void *)hkMenuInit, (void **)&g_origInit);
    LOGI("Hook MenuLayer::init terpasang");
}
