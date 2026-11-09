#include "modkit.h"
static void *L; static float now, ts[64]; static int n; static float acc;
static void lvl() { L = kit::mk("CPS: 0", 0.4f, 50.0f, 14.0f); n = 0; now = 0; acc = 0; }
static void touch() { if (n < 64) ts[n++] = now; }
static void tk(float dt) {
    now += dt; acc += dt; if (acc < 0.1f) return; acc = 0;
    int k = 0; for (int i = 0; i < n; i++) if (now - ts[i] <= 1.0f) ts[k++] = ts[i];
    n = k;
    char b[24]; snprintf(b, sizeof b, "CPS: %d", n); kit::set(L, b);
}
static void lv() { L = nullptr; }
static kit::Handlers H = { lvl, nullptr, nullptr, tk, touch, lv };
extern "C" int rift_mod_init(const RiftApi *a) { return kit::start(a, &H); }
