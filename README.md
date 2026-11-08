# Rift

Mod loader and launcher for **Geometry Dash 2.2** on Android. Built for versions that Geode does not support yet (for example GD 2.209). Rift runs the game *inside* the Rift app with its own separate data, so your original GD install stays untouched.

## Download

| Device | File |
| --- | --- |
| 64-bit (arm64, most phones) | [Rift-64bit.apk](https://github.com/riftsgd-app/rift/releases/latest/download/Rift-64bit.apk) |
| 32-bit (armv7) | coming soon (planned for 1.1) |

All versions are on the [Releases](https://github.com/riftsgd-app/rift/releases) page.

1. Install Geometry Dash 2.2 (APK) on your phone.
2. Install `Rift-64bit.apk` and open it.
3. Tap **Launch**. The game starts inside Rift.
4. In the GD main menu, tap the round Rift button (bottom row) to open the **Mod Manager**.

## Features

- Mod Manager inside the game: Installed Mods, Browse Mods, search, download, enable/disable, hold a mod to remove it.
- Settings screen: auto launch, safe mode, update check, save backup/restore, logs.
- Mods are `.rift` files (zip with `mod.json` and `mod.so`). Put them in `Android/media/com.rift.launcher/mods/`.

## Make your own mod

Read [`docs/MODDING.md`](docs/MODDING.md) (Indonesian). Starter project: [`examples/hello-mod`](examples/hello-mod). Mod API header: [`sdk/rift.h`](sdk/rift.h).

## Note

Rift does not include Geometry Dash. You need your own copy of the game installed. Rift is not affiliated with RobTop Games or Geode.
