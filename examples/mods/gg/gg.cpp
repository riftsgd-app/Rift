#include "modkit.h"
static void *L; static float t;
static void lvl() { L = kit::mk("GG!", 1.4f, 342.0f, 190.0f); kit::show(L, false); t = 0; }
static void win() { kit::show(L, true); t = 2.5f; }
static void tk(float dt) { if (t > 0) { t -= dt; if (t <= 0) kit::show(L, false); } }
static void lv() { L = nullptr; }
static kit::Handlers H = { lvl, nullptr, win, tk, nullptr, lv };
extern "C" int rift_mod_init(const RiftApi *a) { return kit::start(a, &H); }
