// Rift Mod API v1 (draft)
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct RiftApi {
    int version;                                         // 1
    void (*log)(const char *fmt, ...);                   // tulis ke logcat, tag "Rift"
    void *(*find_symbol)(const char *mangled);           // cari fungsi di libcocos2dcpp.so
    int (*hook)(void *target, void *replacement, void **original);  // 0 = sukses
    const char *mod_dir;                                 // folder mod ini (assets ada di sini)
} RiftApi;

// Wajib ada di setiap mod. Dipanggil sekali setelah game kebuka. Return 0 = sukses.
int rift_mod_init(const RiftApi *api);

#ifdef __cplusplus
}
#endif
