package com.rift.launcher;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import dalvik.system.PathClassLoader;
import java.io.File;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public class Probe {
    static final String GD = "com.robtopx.geometryjump";

    boolean installed;
    String version = "-";
    String sourceDir = "-";
    String libDir = "-";
    boolean split;
    boolean apkReadable;
    long apkMB;
    int soCount;
    int soStored;
    boolean hasCocos;
    boolean hasRift;
    boolean classOk;
    String classErr = "";
    boolean ctxOk;
    String ctxErr = "";
    String error = "";

    boolean libsReady() {
        return soCount > 0 && soStored == soCount;
    }

    boolean allGood() {
        return installed && !split && hasCocos && libsReady() && classOk && ctxOk;
    }

    static Probe run(Context c) {
        Probe p = new Probe();
        try {
            ApplicationInfo ai = c.getPackageManager().getApplicationInfo(GD, 0);
            PackageInfo pi = c.getPackageManager().getPackageInfo(GD, 0);
            p.installed = true;
            p.version = pi.versionName == null ? "?" : pi.versionName;
            p.sourceDir = ai.sourceDir;
            p.libDir = ai.nativeLibraryDir;
            p.split = ai.splitSourceDirs != null && ai.splitSourceDirs.length > 0;
            File apk = new File(ai.sourceDir);
            p.apkReadable = apk.canRead();
            p.apkMB = apk.length() / 1048576;
            try {
                ZipFile zf = new ZipFile(ai.sourceDir);
                try {
                    Enumeration<? extends ZipEntry> en = zf.entries();
                    while (en.hasMoreElements()) {
                        ZipEntry e = en.nextElement();
                        String n = e.getName();
                        if (n.startsWith("lib/arm64-v8a/") && n.endsWith(".so")) {
                            p.soCount++;
                            if (e.getMethod() == ZipEntry.STORED) p.soStored++;
                            if (n.endsWith("/libcocos2dcpp.so")) p.hasCocos = true;
                            if (n.endsWith("/libRift.so")) p.hasRift = true;
                        }
                    }
                } finally {
                    zf.close();
                }
            } catch (Throwable t) {
                p.error = "zip: " + t;
            }
            try {
                PathClassLoader cl = new PathClassLoader(ai.sourceDir, ai.nativeLibraryDir, c.getClassLoader());
                cl.loadClass(GD + ".GeometryJump");
                p.classOk = true;
            } catch (Throwable t) {
                p.classErr = String.valueOf(t);
            }
            try {
                c.createPackageContext(GD, Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
                p.ctxOk = true;
            } catch (Throwable t) {
                p.ctxErr = String.valueOf(t);
            }
        } catch (Throwable e) {
            p.error = String.valueOf(e);
        }
        return p;
    }

    String report() {
        StringBuilder sb = new StringBuilder();
        sb.append("Rift Launcher 0.4 - laporan\n");
        sb.append("GD terpasang: ").append(installed).append(" (v").append(version).append(")\n");
        sb.append("Split APK: ").append(split).append("\n");
        sb.append("APK: ").append(apkMB).append(" MB, bisa dibaca=").append(apkReadable).append("\n");
        sb.append("Lib arm64: ").append(soCount).append(" file, tanpa kompres=").append(soStored).append("\n");
        sb.append("libcocos2dcpp.so di APK: ").append(hasCocos).append("\n");
        sb.append("libRift.so di APK: ").append(hasRift).append("\n");
        sb.append("Kelas GeometryJump: ").append(classOk ? "OK" : classErr).append("\n");
        sb.append("createPackageContext: ").append(ctxOk ? "OK" : ctxErr).append("\n");
        sb.append("sourceDir: ").append(sourceDir).append("\n");
        sb.append("nativeLibDir: ").append(libDir).append("\n");
        if (error.length() > 0) sb.append("Error: ").append(error).append("\n");
        sb.append(crashLog());
        return sb.toString();
    }

    static String crashLog() {
        StringBuilder o = new StringBuilder("\n--- log (logcat) ---\n");
        try {
            Process pr = Runtime.getRuntime().exec(new String[]{"logcat", "-d", "-t", "3000", "-v", "brief"});
            java.io.BufferedReader r = new java.io.BufferedReader(new java.io.InputStreamReader(pr.getInputStream()));
            java.util.ArrayList<String> keep = new java.util.ArrayList<String>();
            String l;
            while ((l = r.readLine()) != null) {
                if (l.contains("AndroidRuntime") || l.contains("Rift") || l.contains("DEBUG") || l.contains("Fatal")
                        || l.contains("signal") || l.contains("UnsatisfiedLink") || l.contains("backtrace")
                        || l.contains("  #") || l.contains("Exception") || l.contains("linker")) keep.add(l);
            }
            int from = Math.max(0, keep.size() - 120);
            for (int i = from; i < keep.size(); i++) o.append(keep.get(i)).append("\n");
            if (keep.size() == 0) o.append("(kosong)\n");
        } catch (Throwable t) {
            o.append("gagal baca log: ").append(t).append("\n");
        }
        return o.toString();
    }
}
