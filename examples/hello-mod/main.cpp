#include "rift.h"

static const RiftApi *R;
static bool (*orig)(void *);

static bool myInit(void *self) {
    bool ok = orig(self);
    R->log("hello-mod: MenuLayer::init kena hook!");
    return ok;
}

extern "C" int rift_mod_init(const RiftApi *api) {
    R = api;
    void *t = api->find_symbol("_ZN9MenuLayer4initEv");
    if (!t) return 1;
    api->hook(t, (void *)myInit, (void **)&orig);
    return 0;
}
