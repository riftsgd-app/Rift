package com.rift.launcher;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.res.AssetManager;
import dalvik.system.PathClassLoader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

public class GameLoader {
    static final String GD = "com.robtopx.geometryjump";
    static final String GAME_ACTIVITY = "com.robtopx.geometryjump.GeometryJump";
    private static ClassLoader sLoader;

    static synchronized ClassLoader loader(String gdApk, String selfApk, String nativeDir) {
        if (sLoader == null) {
            String libs = gdApk + "!/lib/arm64-v8a:" + nativeDir;
            sLoader = new PathClassLoader(gdApk + ":" + selfApk, libs,
                    ClassLoader.getSystemClassLoader().getParent());
        }
        return sLoader;
    }

    // siapkan semuanya, balikin Intent buat start game (atau lempar Exception berisi alasan)
    static Intent prepare(Context c) throws Exception {
        ApplicationInfo gd = c.getPackageManager().getApplicationInfo(GD, 0);
        ApplicationInfo me = c.getApplicationInfo();
        File dir = c.getFilesDir();
        // gambar tombol Rift buat hook (dibaca lewat path absolut)
        InputStream in = c.getAssets().open("rift_btn.png");
        FileOutputStream out = new FileOutputStream(new File(dir, "rift_btn.png"));
        byte[] buf = new byte[4096];
        int n;
        while ((n = in.read(buf)) > 0) out.write(buf, 0, n);
        out.close();
        in.close();
        try { RiftArt.generate(dir); } catch (Throwable t) { }
        // cocos baca aset dari sourceDir -> arahkan ke APK GD
        me.sourceDir = gd.sourceDir;
        // aset Java (suara dll): tambahkan APK GD ke resources activity
        try {
            String[] old = me.sharedLibraryFiles;
            int len = old == null ? 0 : old.length;
            String[] nw = new String[len + 1];
            for (int i = 0; i < len; i++) nw[i] = old[i];
            nw[len] = gd.sourceDir;
            me.sharedLibraryFiles = nw;
        } catch (Throwable t) { }
        Intent i = new Intent();
        i.setComponent(new ComponentName(c.getPackageName(), GAME_ACTIVITY));
        i.putExtra("rift.gdapk", gd.sourceDir);
        i.putExtra("rift.selfapk", c.getPackageCodePath());
        i.putExtra("rift.nativedir", me.nativeLibraryDir);
        return i;
    }
}
