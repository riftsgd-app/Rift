# Rift

Mod loader untuk Geometry Dash Android (arm64), dibuat sepenuhnya di Termux.

- `core/` : `libRift.so`, hook ke `libcocos2dcpp.so` (MenuLayer::init, tombol Rift)
- `launcher/` : Rift Launcher, app terpisah ala Geode
- `assets/` : gambar tombol Rift

Palet: Obsidian #171923, Electric Violet #7657FF, Lavender #A99AFF, Cloud White #E8E9F0

Repo ini TIDAK berisi file Geometry Dash (APK, lib, aset) milik RobTop.
Hook memakai And64InlineHook (MIT): https://github.com/Rprop/And64InlineHook
