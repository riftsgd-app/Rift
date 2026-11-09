// Rift Mod API v2
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct RiftApi {
    int version;                                         // 2
    void (*log)(const char *fmt, ...);                   // tulis ke logcat, tag "Rift"
    void *(*find_symbol)(const char *mangled);           // cari fungsi di libcocos2dcpp.so
    int (*hook)(void *target, void *replacement, void **original);  // 0 = sukses
    const char *mod_dir;                                 // folder mod ini (assets ada di sini)
    // --- baru di v2 (cek version >= 2 dulu) ---
    void (*on_tick)(void (*cb)(float dt));               // dipanggil tiap frame
    void (*on_touch)(void (*cb)(void));                  // dipanggil tiap jari menyentuh layar
} RiftApi;

int rift_mod_init(const RiftApi *api);

#ifdef __cplusplus
}
#endif
