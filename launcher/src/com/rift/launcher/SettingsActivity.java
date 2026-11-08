package com.rift.launcher;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;
import java.io.File;

public class SettingsActivity extends Activity {
    private LinearLayout list;
    private int delay;

    private void add(View v) {
        list.addView(v, U.lp(this, 6));
        U.fadeIn(v, delay);
        delay += 28;
    }

    private void head(String s) {
        View v = U.header(this, s);
        list.addView(v);
        U.fadeIn(v, delay);
        delay += 28;
    }

    private void toast(String s) {
        Toast.makeText(this, s, Toast.LENGTH_LONG).show();
    }

    private void openMods() {
        File m = RiftTools.media(this);
        String path = m == null ? "Android/media/com.rift.launcher/mods" : "Android/media/com.rift.launcher/mods";
        try {
            Intent i = new Intent(Intent.ACTION_VIEW);
            i.setDataAndType(Uri.parse("content://com.android.externalstorage.documents/document/primary%3AAndroid%2Fmedia%2Fcom.rift.launcher%2Fmods"), "vnd.android.document/directory");
            startActivity(i);
        } catch (Throwable t) {
            toast("Buka file manager, folder: " + path);
        }
    }

    @Override
    protected void onCreate(Bundle b) {
        super.onCreate(b);
        getWindow().setStatusBarColor(Color.parseColor("#1A1D2B"));
        getWindow().setNavigationBarColor(Color.parseColor("#12141C"));
        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setBackground(new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM,
                new int[] { Color.parseColor("#1A1D2B"), Color.parseColor("#12141C") }));

        LinearLayout bar = new LinearLayout(this);
        bar.setGravity(Gravity.CENTER_VERTICAL);
        bar.setPadding(U.dp(this, 10), U.dp(this, 12), U.dp(this, 20), U.dp(this, 8));
        TextView back = U.text(this, "<", 24, U.WHITE, true);
        back.setGravity(Gravity.CENTER);
        back.setPadding(U.dp(this, 14), U.dp(this, 6), U.dp(this, 14), U.dp(this, 6));
        U.press(this, back, Color.TRANSPARENT, 22);
        back.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) { finish(); }
        });
        bar.addView(back);
        bar.addView(U.text(this, "Settings", 22, U.WHITE, true));
        root.addView(bar);

        ScrollView sv = new ScrollView(this);
        list = new LinearLayout(this);
        list.setOrientation(LinearLayout.VERTICAL);
        list.setPadding(U.dp(this, 16), 0, U.dp(this, 16), U.dp(this, 30));
        sv.addView(list);
        root.addView(sv, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 0, 1f));
        setContentView(root);
        build();
    }

    private void build() {
        final Activity a = this;
        head("General");
        add(U.row(this, "Open mods folder", "Taruh file .rift di sini", null, new View.OnClickListener() {
            public void onClick(View v) { openMods(); }
        }));
        add(U.toggle(this, "Black background", "Latar halaman utama hitam pekat", RiftTools.pref(this, "black", false), new U.Toggle() {
            public void on(boolean v) { RiftTools.setPref(a, "black", v); }
        }));

        head("Gameplay");
        add(U.toggle(this, "Automatically launch", "Langsung masuk game saat Rift dibuka", RiftTools.pref(this, "auto", false), new U.Toggle() {
            public void on(boolean v) { RiftTools.setPref(a, "auto", v); }
        }));
        add(U.toggle(this, "Safe mode", "Jalanin game tanpa memuat mod apa pun", RiftTools.safeMode(this), new U.Toggle() {
            public void on(boolean v) { RiftTools.setSafeMode(a, v); toast(v ? "Safe mode nyala (berlaku pas game dibuka lagi)" : "Safe mode mati"); }
        }));

        head("Updates");
        add(U.toggle(this, "Check updates on start", "Kasih tahu kalau ada versi Rift baru", RiftTools.pref(this, "autoupd", true), new U.Toggle() {
            public void on(boolean v) { RiftTools.setPref(a, "autoupd", v); }
        }));
        add(U.row(this, "Check for updates", "Versi sekarang " + RiftTools.version(this), null, new View.OnClickListener() {
            public void onClick(View v) {
                toast("Ngecek...");
                RiftTools.checkUpdate(new RiftTools.UpdateCb() {
                    public void done(String tag, String url, String err) {
                        if (err != null) { toast("Gagal cek: " + err); return; }
                        if (!RiftTools.newer(tag, RiftTools.version(a))) { toast("Sudah versi terbaru (" + tag + ")"); return; }
                        final String u = url != null ? url : "https://github.com/riftsgd-app/Rift/releases/latest";
                        new AlertDialog.Builder(a).setTitle("Update " + tag + " tersedia")
                                .setMessage("Download APK terbaru sekarang?")
                                .setPositiveButton("Download", new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface d, int w) { startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(u))); }
                                }).setNegativeButton("Nanti", null).show();
                    }
                });
            }
        }));

        head("Save data");
        add(U.row(this, "Backup save", "Simpan data game ke folder media/backups", null, new View.OnClickListener() {
            public void onClick(View v) { toast(RiftTools.backup(a)); }
        }));
        add(U.row(this, "Restore latest backup", "Balikin dari backup terakhir", null, new View.OnClickListener() {
            public void onClick(View v) {
                new AlertDialog.Builder(a).setTitle("Restore backup?")
                        .setMessage("Data game sekarang akan ditimpa backup terakhir.")
                        .setPositiveButton("Restore", new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface d, int w) { toast(RiftTools.restore(a)); }
                        }).setNegativeButton("Batal", null).show();
            }
        }));
        add(U.row(this, "Data report", "Lihat file save yang ada (kirim ke aku kalau save hilang)", null, new View.OnClickListener() {
            public void onClick(View v) { U.showText(a, "Data report", RiftTools.dataReport(a)); }
        }));

        head("Advanced");
        add(U.row(this, "View Rift logs", "Log loader dan mod", null, new View.OnClickListener() {
            public void onClick(View v) { U.showText(a, "Rift logs", RiftTools.logs()); }
        }));
        add(U.row(this, "Diagnostics", "Cek Geometry Dash dan lib native", null, new View.OnClickListener() {
            public void onClick(View v) { U.showText(a, "Diagnostics", Probe.run(a).report()); }
        }));

        head("About");
        add(U.row(this, "Rift Launcher", null, RiftTools.version(this), null));
        Probe p = Probe.run(this);
        add(U.row(this, "Geometry Dash", null, p.installed ? p.version : "belum ke-install", null));
        add(U.row(this, "Device", null, Build.MODEL + " (" + Build.SUPPORTED_ABIS[0] + ")", null));
        add(U.row(this, "GitHub", "riftsgd-app/Rift", null, new View.OnClickListener() {
            public void onClick(View v) { startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/riftsgd-app/Rift"))); }
        }));
    }
}
