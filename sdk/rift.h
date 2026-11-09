// Rift Mod API v3
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct RiftApi {
    int version;                                         // 3
    void (*log)(const char *fmt, ...);                   // tulis ke logcat, tag "Rift"
    void *(*find_symbol)(const char *mangled);           // cari fungsi di libcocos2dcpp.so
    int (*hook)(void *target, void *replacement, void **original);  // 0 = sukses, 4 = sudah di-hook (ditolak)
    const char *mod_dir;                                 // folder mod ini (assets ada di sini)
    // --- v2 ---
    void (*on_tick)(void (*cb)(float dt));               // tiap frame
    void (*on_touch)(void (*cb)(void));                  // tiap jari menyentuh layar
    // --- v3: event level. JANGAN hook PlayLayer sendiri, pakai ini ---
    void (*on_level)(void (*cb)(void *playLayer));       // level baru mulai
    void (*on_leave)(void (*cb)(void));                  // keluar level
    void (*on_death)(void (*cb)(float percent));         // 1x per kematian
    void (*on_win)(void (*cb)(void));                    // level selesai
} RiftApi;

int rift_mod_init(const RiftApi *api);

#ifdef __cplusplus
}
#endif
