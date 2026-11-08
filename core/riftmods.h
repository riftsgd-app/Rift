#pragma once
#define MAX_MODS 32
struct ModInfo {
    char id[64], name[64], author[40], version[24], desc[128], dir[256];
    bool enabled;
    bool loaded;
};
void mods_init(void *cocosHandle);
int mods_scan();
int mods_count();
ModInfo *mods_get(int i);
void mods_load_enabled();
bool mods_toggle(int i);
