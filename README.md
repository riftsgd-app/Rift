# Rift

Mod loader & launcher for **Geometry Dash 2.2** on Android (arm64). Built for versions Geode doesn't support yet. Runs the game *inside* the Rift app with its own separate data, so your original GD install stays untouched.

## Download

Get the latest APK: **[Rift-Launcher.apk](../../releases/latest/download/Rift-Launcher.apk)** (or see the [Releases](../../releases) page).

1. Install Geometry Dash 2.2 (APK) on your phone.
2. Install `Rift-Launcher.apk` and open it.
3. Tap **Launch**. The game starts inside Rift.
4. In the GD main menu, tap the round Rift button (bottom row) to open the **Mod Manager**.

## Mod Manager

- **Installed Mods**: mods found in your mods folder.
- **Browse Mods**: online catalog (see `catalog/mods.json`).

## Make your own mod

Read [`docs/MODDING.md`](docs/MODDING.md) (Indonesian). Starter project: [`examples/hello-mod`](examples/hello-mod). Mod API header: [`sdk/rift.h`](sdk/rift.h).

## Status

Early alpha. The launcher, in-app game launch and Mod Manager UI work. Loading `.rift` mods is being built (Mod API v1 is a draft).

## Repo layout

- `launcher/` Android launcher app (Java + resources)
- `core/` native part injected into the game (C++, hooks)
- `sdk/` header for mod authors
- `examples/` sample mod
- `catalog/` online mod list
