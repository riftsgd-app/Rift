#include "modkit.h"
static void *L; static int bk[10], last, tot;
static void draw() {
    int top = 0; for (int i = 1; i < 10; i++) if (bk[i] > bk[top]) top = i;
    char b[96];
    if (!tot) snprintf(b, sizeof b, "Death spots: -");
    else snprintf(b, sizeof b, "Last: %d%%\nWorst: %d-%d%% (x%d)", last, top * 10, top * 10 + 9, bk[top]);
    kit::set(L, b);
}
static void lvl() { for (int i = 0; i < 10; i++) bk[i] = 0; tot = 0; last = 0; L = kit::mk("Death spots: -", 0.3f, 612.0f, 262.0f); }
static void die(float p) { int k = (int) (p / 10.0f); if (k < 0) k = 0; if (k > 9) k = 9; bk[k]++; tot++; last = (int) p; draw(); }
static void lv() { L = nullptr; }
static kit::Handlers H = { lvl, die, nullptr, nullptr, nullptr, lv };
extern "C" int rift_mod_init(const RiftApi *a) { return kit::start(a, &H); }
