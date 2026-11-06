package com.rift.launcher;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import dalvik.system.PathClassLoader;
import java.io.File;
import java.util.Arrays;

public class Probe {
    static final String GD = "com.robtopx.geometryjump";

    static String run(Context c) {
        StringBuilder sb = new StringBuilder();
        try {
            ApplicationInfo ai = c.getPackageManager().getApplicationInfo(GD, 0);
            sb.append("sourceDir:\n").append(ai.sourceDir).append("\n\n");
            String[] sp = ai.splitSourceDirs;
            sb.append("Split APK: ").append(sp == null ? "tidak" : sp.length + " file").append("\n");
            File apk = new File(ai.sourceDir);
            sb.append("APK bisa dibaca: ").append(apk.canRead()).append(" (").append(apk.length() / 1048576).append(" MB)\n\n");
            sb.append("nativeLibDir:\n").append(ai.nativeLibraryDir).append("\n");
            File ld = new File(ai.nativeLibraryDir);
            String[] names = ld.list();
            sb.append("Isi lib: ").append(names == null ? "TIDAK BISA DIBACA" : Arrays.toString(names)).append("\n");
            File cc = new File(ld, "libcocos2dcpp.so");
            sb.append("libcocos2dcpp.so: ada=").append(cc.exists()).append(" bisa dibaca=").append(cc.canRead()).append("\n");
            File rift = new File(ld, "libRift.so");
            sb.append("libRift.so di GD: ").append(rift.exists()).append("\n\n");
            try {
                PathClassLoader cl = new PathClassLoader(ai.sourceDir, ai.nativeLibraryDir, c.getClassLoader());
                Class<?> k = cl.loadClass(GD + ".GeometryJump");
                sb.append("Kelas GeometryJump: ketemu\nSuper: ").append(k.getSuperclass().getName()).append("\n");
            } catch (Throwable t) {
                sb.append("Kelas GeometryJump: GAGAL ").append(t).append("\n");
            }
            try {
                c.createPackageContext(GD, Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
                sb.append("createPackageContext: OK\n");
            } catch (Throwable t) {
                sb.append("createPackageContext: GAGAL ").append(t).append("\n");
            }
        } catch (Throwable e) {
            sb.append("GD belum ke-install atau error: ").append(e);
        }
        return sb.toString();
    }
}
