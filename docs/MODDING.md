# Cara bikin mod Rift

> **Status:** Mod API v1 masih **draft**. Format di bawah ini yang akan dibaca loader Rift (0.6). Kalau ada yang berubah, dokumen ini ikut diupdate.

Satu mod = satu file `.rift` (zip biasa) berisi kode native arm64 yang di-hook ke game.

```
mymod.rift
├── mod.json     info mod
├── mod.so       kode mod (arm64)
└── assets/      opsional: gambar, suara
```

## 1. Siapin Termux

```bash
pkg install clang zip git binutils
```

Ambil library game buat nyari nama fungsi: unzip `lib/arm64-v8a/libcocos2dcpp.so` dari APK Geometry Dash.

## 2. Mulai dari contoh

```bash
cp -r ~/rift-repo/examples/hello-mod ~/mymod
cd ~/mymod
```

## 3. Isi `mod.json`

```json
{
  "id": "kamu.modku",
  "name": "Mod Ku",
  "version": "1.0.0",
  "author": "kamu",
  "description": "Ngapain modnya",
  "api": 1,
  "binary": "mod.so"
}
```

`id` harus unik (huruf kecil, titik, tanpa spasi).

## 4. Tulis kodenya (`main.cpp`)

Mod wajib punya satu fungsi masuk, dipanggil Rift sekali setelah game kebuka:

```cpp
#include "rift.h"

static const RiftApi *R;
static bool (*orig)(void *);

static bool myInit(void *self) {
    bool ok = orig(self);          // jalanin fungsi asli dulu
    R->log("MenuLayer::init kena hook!");
    return ok;
}

extern "C" int rift_mod_init(const RiftApi *api) {
    R = api;
    void *t = api->find_symbol("_ZN9MenuLayer4initEv");
    if (!t) return 1;
    api->hook(t, (void *)myInit, (void **)&orig);
    return 0;                      // 0 = sukses
}
```

API yang tersedia (`sdk/rift.h`): `log`, `find_symbol` (cari fungsi di libcocos2dcpp), `hook`, dan `mod_dir` (folder mod kamu, buat baca assets).

## 5. Cari nama fungsi yang mau di-hook

```bash
nm -D --defined-only libcocos2dcpp.so | grep PlayLayer
```

Nama yang keluar (mangled, misal `_ZN9PlayLayer4initEP11GJGameLevel...`) dipakai di `find_symbol`. Tanda tangan (argumen/return) harus kamu tulis sendiri dengan benar. Kalau salah, game crash.

Hati-hati: struct yang punya copy constructor (contoh `CCSize`, `CCPoint`) dikembalikan lewat memori di ARM64. Struct pembungkusmu harus punya copy constructor juga, kalau tidak game crash.

## 6. Build dan bungkus

```bash
sh build.sh
```

`build.sh` menjalankan:

```bash
clang++ -shared -fPIC -O2 -fno-exceptions -fno-rtti -nostdlib++ -I../../sdk -o mod.so main.cpp
zip -r mymod.rift mod.json mod.so assets
```

## 7. Pasang

Taruh `mymod.rift` di `/sdcard/Rift/mods/` (di Termux: `cp mymod.rift ~/storage/shared/Rift/mods/`). Buka menu Rift di game, tab **Installed Mods**, tekan **Refresh**, lalu nyalain modnya.

## 8. Cek log kalau error

Di launcher: ikon gear, lalu **Salin laporan**. Cari baris yang diawali `Rift`.

## Kirim mod ke katalog

Bikin pull request yang nambahin entri ke `catalog/mods.json` (id, name, author, description, link download `.rift`).
