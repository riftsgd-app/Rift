#pragma once
struct CatInfo {
    char id[64], name[64], author[40], desc[128], url[300];
};
int cat_count();
CatInfo *cat_get(int i);
int cat_fetch();
bool cat_installed(int i);
const char *cat_download(int i);
void riftnet_restart();
