package com.rift.launcher;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.os.Handler;
import android.os.Looper;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Date;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

final class RiftTools {
    interface UpdateCb { void done(String tag, String url, String err); }

    static File media(Context c) {
        File[] m = c.getExternalMediaDirs();
        return (m != null && m.length > 0) ? m[0] : null;
    }

    static String version(Context c) {
        try {
            PackageInfo p = c.getPackageManager().getPackageInfo(c.getPackageName(), 0);
            return p.versionName;
        } catch (Throwable t) { return "?"; }
    }

    static boolean pref(Context c, String k, boolean d) {
        return c.getSharedPreferences("rift_ui", 0).getBoolean(k, d);
    }

    static void setPref(Context c, String k, boolean v) {
        c.getSharedPreferences("rift_ui", 0).edit().putBoolean(k, v).apply();
    }

    static int modCount(Context c) {
        File m = media(c);
        if (m == null) return 0;
        String[] n = new File(m, "mods").list();
        int k = 0;
        if (n != null) for (String s : n) if (s.endsWith(".rift")) k++;
        return k;
    }

    static boolean safeMode(Context c) {
        File m = media(c);
        return m != null && new File(m, "config/safe.txt").exists();
    }

    static void setSafeMode(Context c, boolean on) {
        File m = media(c);
        if (m == null) return;
        File f = new File(m, "config/safe.txt");
        try {
            if (on) {
                f.getParentFile().mkdirs();
                f.createNewFile();
            } else f.delete();
        } catch (Throwable t) { }
    }

    // ---- data & save ----
    static void walk(File f, String rel, int depth, ArrayList<String> out) {
        if (out.size() > 160 || depth > 5) return;
        File[] kids = f.listFiles();
        if (kids == null) return;
        Arrays.sort(kids);
        SimpleDateFormat df = new SimpleDateFormat("MM-dd HH:mm", Locale.US);
        for (File k : kids) {
            String n = rel + k.getName();
            if (n.startsWith("cache") || n.startsWith("code_cache") || n.startsWith("files/modcache") || n.startsWith("lib")) continue;
            if (k.isDirectory()) {
                out.add(n + "/");
                walk(k, n + "/", depth + 1, out);
            } else if (!k.getName().startsWith("rift_")) {
                out.add(n + "  " + k.length() + "B  " + df.format(new Date(k.lastModified())));
            }
        }
    }

    static String dataReport(Context c) {
        StringBuilder sb = new StringBuilder();
        sb.append("Rift ").append(version(c)).append("\n");
        sb.append("dataDir: ").append(c.getDataDir()).append("\n");
        sb.append("filesDir: ").append(c.getFilesDir()).append("\n");
        File gd = new File("/data/data/com.robtopx.geometryjump");
        sb.append("GD asli dibaca: ").append(gd.canRead()).append("\n\n");
        ArrayList<String> out = new ArrayList<String>();
        walk(c.getDataDir(), "", 0, out);
        for (String s : out) sb.append(s).append("\n");
        File m = media(c);
        if (m != null) {
            sb.append("\nmedia: ").append(m).append("\n");
            String[] b = new File(m, "backups").list();
            sb.append("backup: ").append(b == null ? 0 : b.length).append(" file\n");
        }
        return sb.toString();
    }

    static void zipDir(ZipOutputStream z, File root, File f, String rel) throws Exception {
        File[] kids = f.listFiles();
        if (kids == null) return;
        for (File k : kids) {
            String n = rel + k.getName();
            if (n.startsWith("files/modcache") || n.startsWith("cache") || n.startsWith("code_cache") || n.startsWith("lib")) continue;
            if (k.isDirectory()) zipDir(z, root, k, n + "/");
            else if (!k.getName().startsWith("rift_")) {
                z.putNextEntry(new ZipEntry(n));
                FileInputStream in = new FileInputStream(k);
                byte[] buf = new byte[8192];
                int r;
                while ((r = in.read(buf)) > 0) z.write(buf, 0, r);
                in.close();
                z.closeEntry();
            }
        }
    }

    static String backup(Context c) {
        File m = media(c);
        if (m == null) return "Folder media tidak ada";
        try {
            File dir = new File(m, "backups");
            dir.mkdirs();
            String name = "save-" + new SimpleDateFormat("yyyyMMdd-HHmmss", Locale.US).format(new Date()) + ".zip";
            ZipOutputStream z = new ZipOutputStream(new FileOutputStream(new File(dir, name)));
            zipDir(z, c.getDataDir(), c.getDataDir(), "");
            z.close();
            return "Backup dibuat: backups/" + name;
        } catch (Throwable t) {
            return "Backup gagal: " + t;
        }
    }

    static String restore(Context c) {
        File m = media(c);
        if (m == null) return "Folder media tidak ada";
        File[] list = new File(m, "backups").listFiles();
        if (list == null || list.length == 0) return "Belum ada backup";
        Arrays.sort(list);
        File last = list[list.length - 1];
        try {
            String base = c.getDataDir().getCanonicalPath() + "/";
            ZipInputStream z = new ZipInputStream(new FileInputStream(last));
            ZipEntry e;
            int n = 0;
            byte[] buf = new byte[8192];
            while ((e = z.getNextEntry()) != null) {
                File o = new File(c.getDataDir(), e.getName());
                if (!o.getCanonicalPath().startsWith(base)) continue;
                o.getParentFile().mkdirs();
                FileOutputStream out = new FileOutputStream(o);
                int r;
                while ((r = z.read(buf)) > 0) out.write(buf, 0, r);
                out.close();
                n++;
            }
            z.close();
            return "Dipulihkan " + n + " file dari " + last.getName() + ". Restart Rift.";
        } catch (Throwable t) {
            return "Restore gagal: " + t;
        }
    }

    // ---- log ----
    static String logs() {
        StringBuilder sb = new StringBuilder();
        try {
            Process p = Runtime.getRuntime().exec(new String[] { "logcat", "-d", "-v", "time", "-t", "400" });
            BufferedReader r = new BufferedReader(new InputStreamReader(p.getInputStream()));
            String l;
            ArrayList<String> keep = new ArrayList<String>();
            while ((l = r.readLine()) != null) {
                if (l.contains("Rift") || l.contains("AndroidRuntime") || l.contains("DEBUG") || l.contains("libc")) keep.add(l);
            }
            int from = Math.max(0, keep.size() - 150);
            for (int i = from; i < keep.size(); i++) sb.append(keep.get(i)).append("\n");
        } catch (Throwable t) {
            sb.append("gagal baca log: ").append(t);
        }
        return sb.length() == 0 ? "(log kosong - jalanin game dulu)" : sb.toString();
    }

    // ---- update dari GitHub ----
    static void checkUpdate(final UpdateCb cb) {
        final Handler h = new Handler(Looper.getMainLooper());
        new Thread(new Runnable() {
            public void run() {
                String tag = null, url = null, err = null;
                try {
                    HttpURLConnection con = (HttpURLConnection) new URL("https://api.github.com/repos/riftsgd-app/Rift/releases/latest").openConnection();
                    con.setConnectTimeout(8000);
                    con.setReadTimeout(8000);
                    con.setRequestProperty("Accept", "application/vnd.github+json");
                    con.setRequestProperty("User-Agent", "RiftLauncher");
                    InputStream in = con.getInputStream();
                    StringBuilder sb = new StringBuilder();
                    byte[] buf = new byte[4096];
                    int r;
                    while ((r = in.read(buf)) > 0) sb.append(new String(buf, 0, r, "UTF-8"));
                    in.close();
                    String s = sb.toString();
                    tag = grab(s, "\"tag_name\"");
                    int a = s.indexOf("64bit.apk\""); if (a < 0) a = s.indexOf(".apk\"");
                    if (a > 0) {
                        int st = s.lastIndexOf("\"browser_download_url\"", a);
                        if (st >= 0) url = grab(s.substring(st), "\"browser_download_url\"");
                    }
                    if (tag == null) err = "format balasan aneh";
                } catch (Throwable t) {
                    err = String.valueOf(t);
                }
                final String ft = tag, fu = url, fe = err;
                h.post(new Runnable() { public void run() { cb.done(ft, fu, fe); } });
            }
        }).start();
    }

    static String grab(String s, String key) {
        int i = s.indexOf(key);
        if (i < 0) return null;
        int a = s.indexOf('"', s.indexOf(':', i) + 1);
        int b = a < 0 ? -1 : s.indexOf('"', a + 1);
        return (a < 0 || b < 0) ? null : s.substring(a + 1, b);
    }

    static boolean newer(String tag, String cur) {
        String[] a = tag.replaceAll("[^0-9.]", "").split("\\.");
        String[] b = cur.replaceAll("[^0-9.]", "").split("\\.");
        for (int i = 0; i < 4; i++) {
            int x = i < a.length && a[i].length() > 0 ? Integer.parseInt(a[i]) : 0;
            int y = i < b.length && b[i].length() > 0 ? Integer.parseInt(b[i]) : 0;
            if (x != y) return x > y;
        }
        return false;
    }
}
