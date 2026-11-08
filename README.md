# Rift

Mod loader and launcher for **Geometry Dash 2.2** on Android. Built for versions that Geode does not support yet (for example GD 2.209). Rift runs the game *inside* the Rift app with its own separate data, so your original GD install stays untouched.

## Download

| Device | File |
| --- | --- |
| 64-bit (arm64-v8a), recommended | [Rift-64bit.apk](../../releases/latest/download/Rift-64bit.apk) |
| 32-bit (armeabi-v7a), experimental | [Rift-32bit.apk](../../releases/latest/download/Rift-32bit.apk) |

Pick the one that matches your Geometry Dash install. They share a package name, so only one can be installed at a time.

## Built-in mods

Turn these on or off in Settings, Rift Mods: FPS Monitor, Attempt Counter, Death Tracker, Best Run Tracker, Session Timer, Level Completion Stats.

## Features

- Mod Manager inside the game: Installed Mods, Browse Mods, search, download, enable/disable, hold a mod to remove it.
- Settings screen: auto launch, safe mode, update check, save backup/restore, logs.
- Mods are `.rift` files (zip with `mod.json` and `mod.so`). Put them in `Android/media/com.rift.launcher/mods/`.

## Make your own mod

Read [`docs/MODDING.md`](docs/MODDING.md) (Indonesian). Starter project: [`examples/hello-mod`](examples/hello-mod). Mod API header: [`sdk/rift.h`](sdk/rift.h).

## Note

Rift does not include Geometry Dash. You need your own copy of the game installed. Rift is not affiliated with RobTop Games or Geode.
