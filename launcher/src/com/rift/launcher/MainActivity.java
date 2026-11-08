package com.rift.launcher;

import android.app.Activity;
import android.app.Dialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.Intent;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.RippleDrawable;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {
    static final String GD = "com.robtopx.geometryjump";
    static final int VIOLET = Color.parseColor("#7657FF");
    static final int LAVENDER = Color.parseColor("#A99AFF");
    static final int WHITE = Color.parseColor("#E8E9F0");
    static final int DIM = Color.parseColor("#7C819B");
    static final int GREEN = Color.parseColor("#5BE584");
    static final int RED = Color.parseColor("#FF7A90");
    static final int CARD = Color.parseColor("#1F2336");

    private int dp(int v) {
        return (int) (v * getResources().getDisplayMetrics().density + 0.5f);
    }

    private GradientDrawable shape(int color, int radiusDp) {
        GradientDrawable g = new GradientDrawable();
        g.setColor(color);
        g.setCornerRadius(dp(radiusDp));
        return g;
    }

    private TextView text(String s, int sp, int color, boolean bold) {
        TextView t = new TextView(this);
        t.setText(s);
        t.setTextSize(sp);
        t.setTextColor(color);
        if (bold) t.setTypeface(Typeface.DEFAULT_BOLD);
        return t;
    }

    private void press(View v, int color, int radiusDp) {
        GradientDrawable bg = shape(color, radiusDp);
        GradientDrawable mask = shape(Color.WHITE, radiusDp);
        v.setBackground(new RippleDrawable(ColorStateList.valueOf(Color.parseColor("#44FFFFFF")), bg, mask));
    }

    private View line(String label, String value, int state) {
        LinearLayout row = new LinearLayout(this);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(0, dp(9), 0, dp(9));
        int color = state > 0 ? GREEN : (state < 0 ? RED : DIM);
        String mark = state > 0 ? "\u2713" : (state < 0 ? "\u2717" : "\u2022");
        TextView icon = text(mark, 16, color, true);
        icon.setGravity(Gravity.CENTER);
        row.addView(icon, new LinearLayout.LayoutParams(dp(26), ViewGroup.LayoutParams.WRAP_CONTENT));
        TextView l = text(label, 14, WHITE, false);
        row.addView(l, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
        TextView v = text(value, 13, state == 0 ? DIM : color, false);
        v.setGravity(Gravity.RIGHT);
        row.addView(v, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1.2f));
        return row;
    }

    private View divider() {
        View d = new View(this);
        d.setBackgroundColor(Color.parseColor("#2A2F47"));
        d.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, 1));
        return d;
    }

    private void launchGame() {
        try {
            Intent i = GameLoader.prepare(this);
            startActivity(i);
        } catch (Throwable t) {
            Toast.makeText(this, "Gagal jalanin game: " + t, Toast.LENGTH_LONG).show();
        }
    }

    private void showSettings(final Probe p) {
        final Dialog d = new Dialog(this);
        d.requestWindowFeature(Window.FEATURE_NO_TITLE);

        LinearLayout box = new LinearLayout(this);
        box.setOrientation(LinearLayout.VERTICAL);
        box.setPadding(dp(22), dp(22), dp(22), dp(16));
        box.setBackground(shape(CARD, 26));

        box.addView(text("Pengaturan Rift", 22, WHITE, true));
        TextView sub = text("Diagnostik Geometry Dash", 12, LAVENDER, false);
        sub.setPadding(0, dp(2), 0, dp(10));
        box.addView(sub);

        boolean good = p.allGood();
        TextView verdict = text(good ? "\u2713  Siap lanjut ke tahap launcher" : "\u2717  Ada yang perlu dicek", 13, good ? GREEN : RED, true);
        verdict.setPadding(dp(14), dp(9), dp(14), dp(9));
        verdict.setBackground(shape(Color.parseColor(good ? "#1E3A2A" : "#3A2029"), 14));
        box.addView(verdict);

        LinearLayout rows = new LinearLayout(this);
        rows.setOrientation(LinearLayout.VERTICAL);
        rows.setPadding(0, dp(8), 0, 0);
        rows.addView(line("Geometry Dash", p.installed ? "v" + p.version : "belum ke-install", p.installed ? 1 : -1));
        rows.addView(divider());
        rows.addView(line("Jenis APK", p.split ? "split (belum didukung)" : "tunggal", p.split ? -1 : 1));
        rows.addView(divider());
        String libVal = p.soCount == 0 ? "tidak ketemu" : (p.libsReady() ? "dalam APK, siap dimuat" : "terkompres, perlu diekstrak");
        rows.addView(line("Lib native", libVal, p.libsReady() ? 1 : -1));
        rows.addView(divider());
        rows.addView(line("libcocos2dcpp.so", p.hasCocos ? "ada" : "tidak ada", p.hasCocos ? 1 : -1));
        rows.addView(divider());
        rows.addView(line("Kelas Java GD", p.classOk ? "bisa dimuat" : "gagal", p.classOk ? 1 : -1));
        rows.addView(divider());
        rows.addView(line("Akses kode GD", p.ctxOk ? "diizinkan" : "ditolak", p.ctxOk ? 1 : -1));
        rows.addView(divider());
        rows.addView(line("Rift di dalam GD", p.hasRift ? "ya (APK patch)" : "tidak (GD asli)", p.hasRift ? 1 : 0));

        TextView detail = text("sourceDir:\n" + p.sourceDir + "\n\nnativeLibDir:\n" + p.libDir, 10, DIM, false);
        detail.setTypeface(Typeface.MONOSPACE);
        detail.setPadding(0, dp(12), 0, 0);
        rows.addView(detail);

        ScrollView sv = new ScrollView(this);
        sv.addView(rows);
        box.addView(sv, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

        LinearLayout btns = new LinearLayout(this);
        btns.setOrientation(LinearLayout.HORIZONTAL);
        btns.setGravity(Gravity.RIGHT);
        btns.setPadding(0, dp(14), 0, 0);

        TextView copy = text("Salin laporan", 14, LAVENDER, true);
        copy.setGravity(Gravity.CENTER);
        copy.setPadding(dp(18), dp(11), dp(18), dp(11));
        press(copy, Color.parseColor("#262A3D"), 22);
        copy.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                ClipboardManager cm = (ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
                cm.setPrimaryClip(ClipData.newPlainText("Rift", p.report()));
                Toast.makeText(MainActivity.this, "Laporan disalin", Toast.LENGTH_SHORT).show();
            }
        });
        btns.addView(copy);

        TextView close = text("Tutup", 14, Color.WHITE, true);
        close.setGravity(Gravity.CENTER);
        close.setPadding(dp(26), dp(11), dp(26), dp(11));
        press(close, VIOLET, 22);
        close.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                d.dismiss();
            }
        });
        LinearLayout.LayoutParams closeLp = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        closeLp.leftMargin = dp(10);
        btns.addView(close, closeLp);
        box.addView(btns);

        d.setContentView(box);
        Window w = d.getWindow();
        w.setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        w.setLayout((int) (getResources().getDisplayMetrics().widthPixels * 0.92f), ViewGroup.LayoutParams.WRAP_CONTENT);
        d.show();
    }

    @Override
    protected void onCreate(Bundle b) {
        super.onCreate(b);
        getWindow().setStatusBarColor(Color.parseColor("#1E2133"));
        getWindow().setNavigationBarColor(Color.parseColor("#12141C"));

        Probe p = Probe.run(this);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setGravity(Gravity.CENTER);
        root.setPadding(dp(24), dp(24), dp(24), dp(24));
        root.setBackground(new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM,
                new int[] { Color.parseColor("#1E2133"), Color.parseColor("#12141C") }));

        LinearLayout logoRow = new LinearLayout(this);
        logoRow.setOrientation(LinearLayout.HORIZONTAL);
        logoRow.setGravity(Gravity.CENTER_VERTICAL);
        ImageView logo = new ImageView(this);
        logo.setImageResource(getResources().getIdentifier("ic_rift_logo", "drawable", getPackageName()));
        logoRow.addView(logo, new LinearLayout.LayoutParams(dp(76), dp(76)));
        TextView name = text("Rift", 46, WHITE, true);
        name.setPadding(dp(16), 0, 0, 0);
        logoRow.addView(name);
        root.addView(logoRow);

        LinearLayout btnRow = new LinearLayout(this);
        btnRow.setOrientation(LinearLayout.HORIZONTAL);
        btnRow.setGravity(Gravity.CENTER_VERTICAL);
        LinearLayout.LayoutParams rowLp = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        rowLp.topMargin = dp(40);

        TextView launch = text("\u25B6   Launch", 18, Color.WHITE, true);
        launch.setGravity(Gravity.CENTER);
        launch.setPadding(dp(40), dp(16), dp(40), dp(16));
        press(launch, VIOLET, 30);
        launch.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                launchGame();
            }
        });
        btnRow.addView(launch);

        TextView gear = text("\u2699", 26, WHITE, false);
        gear.setGravity(Gravity.CENTER);
        gear.setPadding(dp(16), dp(10), dp(16), dp(10));
        press(gear, Color.parseColor("#262A3D"), 30);
        gear.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                showSettings(Probe.run(MainActivity.this));
            }
        });
        LinearLayout.LayoutParams gearLp = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        gearLp.leftMargin = dp(14);
        btnRow.addView(gear, gearLp);
        root.addView(btnRow, rowLp);

        LinearLayout card = new LinearLayout(this);
        card.setOrientation(LinearLayout.VERTICAL);
        card.setPadding(dp(22), dp(14), dp(22), dp(14));
        card.setBackground(shape(CARD, 22));
        TextView head = text("STATUS", 12, LAVENDER, true);
        head.setPadding(0, 0, 0, dp(2));
        card.addView(head);
        card.addView(line("Geometry Dash", p.installed ? "v" + p.version : "belum ke-install", p.installed ? 1 : -1));
        card.addView(divider());
        card.addView(line("Rift Core", p.hasRift ? "terpasang di GD" : "belum terpasang", p.hasRift ? 1 : 0));
        card.addView(divider());
        card.addView(line("Mode", "jalan di dalam Rift", 0));
        LinearLayout.LayoutParams cardLp = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        cardLp.topMargin = dp(36);
        root.addView(card, cardLp);

        TextView foot = text("Rift Launcher 0.5.12", 12, DIM, false);
        foot.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams footLp = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        footLp.topMargin = dp(28);
        root.addView(foot, footLp);

        setContentView(root);
    }
}
