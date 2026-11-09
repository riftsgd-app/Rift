#include "modkit.h"
static const char *Q[] = { "Skill issue", "Hampir!", "Sekali lagi...", "Santai aja", "Jangan nyerah", "Pasti bisa", "Coba lagi!", "Ini baru warm-up" };
static void *L; static float t; static int c;
static void lvl() { L = kit::mk("", 0.5f, 342.0f, 40.0f); kit::show(L, false); t = 0; }
static void die(float) { kit::set(L, Q[c++ % 8]); kit::show(L, true); t = 1.2f; }
static void tk(float dt) { if (t > 0) { t -= dt; if (t <= 0) kit::show(L, false); } }
static void lv() { L = nullptr; }
static kit::Handlers H = { lvl, die, nullptr, tk, nullptr, lv };
extern "C" int rift_mod_init(const RiftApi *a) { return kit::start(a, &H); }
