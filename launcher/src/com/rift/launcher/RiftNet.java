package com.rift.launcher;

import android.app.AlarmManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Process;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

// dipanggil dari kode native (JNI): download file, ambil teks, restart game
public class RiftNet {
    static HttpURLConnection open(String url) throws Exception {
        HttpURLConnection c = (HttpURLConnection) new URL(url).openConnection();
        c.setConnectTimeout(5000);
        c.setReadTimeout(10000);
        c.setInstanceFollowRedirects(true);
        c.setRequestProperty("User-Agent", "Rift-Launcher");
        return c;
    }

    // balikin isi teks, atau "" kalau gagal
    public static String fetchText(String url) {
        try {
            HttpURLConnection c = open(url);
            if (c.getResponseCode() != 200) return "";
            InputStream in = c.getInputStream();
            ByteArrayOutputStream o = new ByteArrayOutputStream();
            byte[] b = new byte[8192];
            int n;
            while ((n = in.read(b)) > 0) {
                o.write(b, 0, n);
                if (o.size() > 190000) break;
            }
            in.close();
            return new String(o.toByteArray(), "UTF-8");
        } catch (Throwable t) {
            return "";
        }
    }

    // balikin "OK" kalau berhasil, selain itu pesan error
    public static String download(String url, String dest) {
        try {
            HttpURLConnection c = open(url);
            int code = c.getResponseCode();
            if (code != 200) return "HTTP " + code;
            File part = new File(dest + ".part");
            File parent = part.getParentFile();
            if (parent != null) parent.mkdirs();
            InputStream in = c.getInputStream();
            FileOutputStream out = new FileOutputStream(part);
            byte[] b = new byte[16384];
            long total = 0;
            int n;
            while ((n = in.read(b)) > 0) {
                out.write(b, 0, n);
                total += n;
                if (total > 64L * 1024 * 1024) {
                    out.close();
                    part.delete();
                    return "file terlalu besar";
                }
            }
            out.close();
            in.close();
            File f = new File(dest);
            if (f.exists()) f.delete();
            if (!part.renameTo(f)) return "gagal simpan file";
            return "OK";
        } catch (Throwable t) {
            return "" + t.getClass().getSimpleName() + ": " + t.getMessage();
        }
    }

    // tutup game; coba buka lagi launcher Rift sebentar lagi
    public static void restart() {
        try {
            Context c = (Context) Class.forName("android.app.ActivityThread").getMethod("currentApplication").invoke(null);
            Intent i = c.getPackageManager().getLaunchIntentForPackage(c.getPackageName());
            if (i != null) {
                i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
                PendingIntent pi = PendingIntent.getActivity(c, 7, i, PendingIntent.FLAG_IMMUTABLE | PendingIntent.FLAG_CANCEL_CURRENT);
                AlarmManager am = (AlarmManager) c.getSystemService("alarm");
                if (am != null) am.set(AlarmManager.RTC, System.currentTimeMillis() + 800, pi);
            }
        } catch (Throwable t) {
        }
        new Thread(new Runnable() {
            public void run() {
                try { Thread.sleep(200); } catch (Throwable t) { }
                Process.killProcess(Process.myPid());
                System.exit(0);
            }
        }).start();
    }
}
