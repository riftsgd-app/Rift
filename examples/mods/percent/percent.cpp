#include "modkit.h"
static void *L; static float acc;
static void lvl() { L = kit::mk("0%", 0.7f, 342.0f, 302.0f); acc = 0; }
static void tk(float dt) {
    acc += dt; if (acc < 0.05f) return; acc = 0;
    if (!L || !kit::pct) return;
    char b[16]; snprintf(b, sizeof b, "%d%%", (int) kit::pct(kit::pl)); kit::set(L, b);
}
static void lv() { L = nullptr; }
static kit::Handlers H = { lvl, nullptr, nullptr, tk, nullptr, lv };
extern "C" int rift_mod_init(const RiftApi *a) { return kit::start(a, &H); }
