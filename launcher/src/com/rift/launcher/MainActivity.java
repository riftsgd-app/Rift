package com.rift.launcher;

import android.animation.ObjectAnimator;
import android.animation.ValueAnimator;
import android.app.Activity;
import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.RippleDrawable;
import android.content.res.ColorStateList;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.view.Gravity;
import android.view.View;
import android.view.ViewGroup;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.widget.FrameLayout;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {
    private LinearLayout status;
    private TextView banner;
    private String updateUrl;

    private void launchGame() {
        try {
            Intent i = GameLoader.prepare(this);
            startActivity(i);
        } catch (Throwable t) {
            Toast.makeText(this, "Gagal jalanin game: " + t, Toast.LENGTH_LONG).show();
        }
    }

    private View statusRow(String label, String value, int color) {
        LinearLayout row = new LinearLayout(this);
        row.setGravity(Gravity.CENTER_VERTICAL);
        row.setPadding(0, U.dp(this, 11), 0, U.dp(this, 11));
        View dot = new View(this);
        dot.setBackground(U.shape(this, color, 6));
        row.addView(dot, new LinearLayout.LayoutParams(U.dp(this, 8), U.dp(this, 8)));
        TextView l = U.text(this, label, 14, U.WHITE, false);
        l.setPadding(U.dp(this, 12), 0, 0, 0);
        row.addView(l, new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
        TextView v = U.text(this, value, 14, color, false);
        v.setGravity(Gravity.RIGHT);
        row.addView(v);
        return row;
    }

    private void refresh() {
        Probe p = Probe.run(this);
        status.removeAllViews();
        TextView h = U.text(this, "STATUS", 12, U.LAVENDER, true);
        h.setLetterSpacing(0.12f);
        status.addView(h);
        status.addView(statusRow("Geometry Dash", p.installed ? "v" + p.version : "belum ke-install", p.installed ? U.GREEN : U.RED));
        int n = RiftTools.modCount(this);
        status.addView(statusRow("Mods", n + " terpasang", n > 0 ? U.GREEN : U.DIM));
        boolean safe = RiftTools.safeMode(this);
        status.addView(statusRow("Safe mode", safe ? "aktif" : "mati", safe ? U.RED : U.DIM));
        status.addView(statusRow("Rift Core", "jalan di dalam Rift", U.GREEN));
    }

    private LinearLayout pill(String t, int sp, int color, boolean solid) {
        LinearLayout b = new LinearLayout(this);
        b.setGravity(Gravity.CENTER);
        b.addView(U.text(this, t, sp, solid ? Color.WHITE : U.WHITE, true));
        U.press(this, b, color, 28);
        return b;
    }

    @Override
    protected void onCreate(Bundle sb) {
        super.onCreate(sb);
        final boolean black = RiftTools.pref(this, "black", false);
        getWindow().setStatusBarColor(Color.TRANSPARENT);
        getWindow().setNavigationBarColor(Color.parseColor(black ? "#000000" : "#12141C"));
        getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);

        FrameLayout frame = new FrameLayout(this);
        frame.setBackground(new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM,
                black ? new int[] { Color.BLACK, Color.BLACK } : new int[] { Color.parseColor("#1D2033"), Color.parseColor("#10121A") }));

        View glow = new View(this);
        GradientDrawable gd = new GradientDrawable();
        gd.setGradientType(GradientDrawable.RADIAL_GRADIENT);
        gd.setGradientRadius(U.dp(this, 260));
        gd.setColors(new int[] { Color.parseColor("#557657FF"), Color.parseColor("#007657FF") });
        glow.setBackground(gd);
        FrameLayout.LayoutParams glp = new FrameLayout.LayoutParams(U.dp(this, 520), U.dp(this, 520), Gravity.TOP | Gravity.CENTER_HORIZONTAL);
        glp.topMargin = -U.dp(this, 150);
        frame.addView(glow, glp);
        ObjectAnimator pulse = ObjectAnimator.ofFloat(glow, "alpha", 0.55f, 1f);
        pulse.setDuration(3200);
        pulse.setRepeatCount(ValueAnimator.INFINITE);
        pulse.setRepeatMode(ValueAnimator.REVERSE);
        pulse.setInterpolator(new AccelerateDecelerateInterpolator());
        pulse.start();

        LinearLayout col = new LinearLayout(this);
        col.setOrientation(LinearLayout.VERTICAL);
        col.setGravity(Gravity.CENTER_HORIZONTAL);
        col.setPadding(U.dp(this, 24), U.dp(this, 56), U.dp(this, 24), U.dp(this, 28));
        int d = 0;

        ImageView logo = new ImageView(this);
        logo.setImageResource(getResources().getIdentifier("ic_rift_logo", "drawable", getPackageName()));
        col.addView(logo, new LinearLayout.LayoutParams(U.dp(this, 96), U.dp(this, 96)));
        ObjectAnimator fl = ObjectAnimator.ofFloat(logo, "translationY", 0f, -U.dp(this, 6), 0f);
        fl.setDuration(3600);
        fl.setRepeatCount(ValueAnimator.INFINITE);
        fl.setInterpolator(new AccelerateDecelerateInterpolator());
        fl.start();
        TextView name = U.text(this, "Rift", 44, U.WHITE, true);
        name.setPadding(0, U.dp(this, 8), 0, 0);
        col.addView(name);
        TextView sub = U.text(this, "Mod loader for Geometry Dash", 13, U.LAVENDER, false);
        col.addView(sub);
        U.fadeIn(logo, d); U.fadeIn(name, d += 60); U.fadeIn(sub, d += 40);

        banner = U.text(this, "", 13, Color.WHITE, true);
        banner.setGravity(Gravity.CENTER);
        banner.setPadding(U.dp(this, 16), U.dp(this, 10), U.dp(this, 16), U.dp(this, 10));
        banner.setBackground(U.shape(this, Color.parseColor("#3A2F7A"), 16));
        banner.setVisibility(View.GONE);
        banner.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(updateUrl != null ? updateUrl : "https://github.com/riftsgd-app/Rift/releases/latest")));
            }
        });
        col.addView(banner, U.lp(this, 18));

        LinearLayout launch = pill("Launch", 20, U.VIOLET, true);
        GradientDrawable lg = new GradientDrawable(GradientDrawable.Orientation.LEFT_RIGHT, new int[] { Color.parseColor("#7657FF"), Color.parseColor("#9D7DFF") });
        lg.setCornerRadius(U.dp(this, 30));
        GradientDrawable lm = new GradientDrawable();
        lm.setColor(Color.WHITE);
        lm.setCornerRadius(U.dp(this, 30));
        launch.setBackground(new RippleDrawable(ColorStateList.valueOf(Color.parseColor("#55FFFFFF")), lg, lm));
        launch.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) { launchGame(); }
        });
        col.addView(launch, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, U.dp(this, 60)));
        ((LinearLayout.LayoutParams) launch.getLayoutParams()).topMargin = U.dp(this, 30);
        U.fadeIn(launch, d += 60);

        LinearLayout two = new LinearLayout(this);
        LinearLayout mods = pill("Mods folder", 15, U.CARD2, false);
        LinearLayout set = pill("Settings", 15, U.CARD2, false);
        mods.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                try {
                    Intent i = new Intent(Intent.ACTION_VIEW);
                    i.setDataAndType(Uri.parse("content://com.android.externalstorage.documents/document/primary%3AAndroid%2Fmedia%2Fcom.rift.launcher%2Fmods"), "vnd.android.document/directory");
                    startActivity(i);
                } catch (Throwable t) {
                    Toast.makeText(MainActivity.this, "Folder: Android/media/com.rift.launcher/mods", Toast.LENGTH_LONG).show();
                }
            }
        });
        set.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) { startActivity(new Intent(MainActivity.this, SettingsActivity.class)); }
        });
        LinearLayout.LayoutParams a1 = new LinearLayout.LayoutParams(0, U.dp(this, 52), 1f);
        a1.rightMargin = U.dp(this, 6);
        LinearLayout.LayoutParams a2 = new LinearLayout.LayoutParams(0, U.dp(this, 52), 1f);
        a2.leftMargin = U.dp(this, 6);
        two.addView(mods, a1);
        two.addView(set, a2);
        col.addView(two, U.lp(this, 12));
        U.fadeIn(two, d += 60);

        status = new LinearLayout(this);
        status.setOrientation(LinearLayout.VERTICAL);
        status.setPadding(U.dp(this, 20), U.dp(this, 16), U.dp(this, 20), U.dp(this, 8));
        status.setBackground(U.shape(this, U.CARD, 22));
        col.addView(status, U.lp(this, 20));
        U.fadeIn(status, d += 60);

        TextView foot = U.text(this, "Rift Launcher " + RiftTools.version(this), 12, U.DIM, false);
        foot.setPadding(0, U.dp(this, 22), 0, 0);
        col.addView(foot);
        U.fadeIn(foot, d + 60);

        ScrollView sv = new ScrollView(this);
        sv.setFillViewport(true);
        sv.addView(col);
        frame.addView(sv);
        setContentView(frame);
        refresh();

        if (RiftTools.pref(this, "autoupd", true)) {
            RiftTools.checkUpdate(new RiftTools.UpdateCb() {
                public void done(String tag, String url, String err) {
                    if (err != null || tag == null) return;
                    try {
                        if (RiftTools.newer(tag, RiftTools.version(MainActivity.this))) {
                            updateUrl = url;
                            banner.setText("Update " + tag + " tersedia - ketuk buat download");
                            banner.setVisibility(View.VISIBLE);
                            U.fadeIn(banner, 0);
                        }
                    } catch (Throwable t) { }
                }
            });
        }
        if (sb == null && RiftTools.pref(this, "auto", false)) {
            Toast.makeText(this, "Auto launch...", Toast.LENGTH_SHORT).show();
            new Handler().postDelayed(new Runnable() {
                public void run() { launchGame(); }
            }, 1400);
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (status != null) refresh();
    }
}
