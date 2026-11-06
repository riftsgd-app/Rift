package com.rift.launcher;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.RippleDrawable;
import android.os.Bundle;
import android.view.Gravity;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;
import java.io.File;

public class MainActivity extends Activity {
    static final String GD = "com.robtopx.geometryjump";
    static final int VIOLET = Color.parseColor("#7657FF");
    static final int LAVENDER = Color.parseColor("#A99AFF");
    static final int WHITE = Color.parseColor("#E8E9F0");
    static final int DIM = Color.parseColor("#6F7590");

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

    private View line(String label, String value, boolean ok) {
        LinearLayout row = new LinearLayout(this);
        row.setOrientation(LinearLayout.HORIZONTAL);
        row.setPadding(0, dp(10), 0, 0);
        TextView l = text(label, 15, DIM, false);
        TextView r = text(value, 15, ok ? WHITE : Color.parseColor("#FF7A90"), false);
        r.setGravity(Gravity.RIGHT);
        row.addView(l, new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1f));
        row.addView(r, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        return row;
    }

    private void launchGame() {
        Intent i = getPackageManager().getLaunchIntentForPackage(GD);
        if (i == null) {
            Toast.makeText(this, "Geometry Dash belum ke-install", Toast.LENGTH_SHORT).show();
            return;
        }
        startActivity(i);
    }

    private void showSettings(String libDir) {
        String msg = Probe.run(this);
        new AlertDialog.Builder(this).setTitle("Pengaturan Rift").setMessage(msg).setPositiveButton("OK", null).show();
    }

    @Override
    protected void onCreate(Bundle b) {
        super.onCreate(b);
        getWindow().setStatusBarColor(Color.parseColor("#1E2133"));
        getWindow().setNavigationBarColor(Color.parseColor("#12141C"));

        String gdVersion = null;
        String libDir = null;
        boolean riftInGd = false;
        try {
            PackageInfo p = getPackageManager().getPackageInfo(GD, 0);
            gdVersion = p.versionName;
            libDir = p.applicationInfo.nativeLibraryDir;
            riftInGd = new File(libDir, "libRift.so").exists();
        } catch (Exception e) {
        }
        final String fLibDir = libDir;

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
        int resId = getResources().getIdentifier("ic_rift_logo", "drawable", getPackageName());
        logo.setImageResource(resId);
        logoRow.addView(logo, new LinearLayout.LayoutParams(dp(76), dp(76)));
        TextView name = text("Rift", 46, WHITE, true);
        name.setPadding(dp(16), 0, 0, 0);
        logoRow.addView(name);
        root.addView(logoRow);

        LinearLayout btnRow = new LinearLayout(this);
        btnRow.setOrientation(LinearLayout.HORIZONTAL);
        btnRow.setGravity(Gravity.CENTER_VERTICAL);
        LinearLayout.LayoutParams rowLp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
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
                showSettings(fLibDir);
            }
        });
        LinearLayout.LayoutParams gearLp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        gearLp.leftMargin = dp(14);
        btnRow.addView(gear, gearLp);
        root.addView(btnRow, rowLp);

        LinearLayout card = new LinearLayout(this);
        card.setOrientation(LinearLayout.VERTICAL);
        card.setPadding(dp(22), dp(18), dp(22), dp(18));
        card.setBackground(shape(Color.parseColor("#1F2336"), 22));
        card.addView(text("STATUS", 12, LAVENDER, true));
        card.addView(line("Geometry Dash", gdVersion != null ? "v" + gdVersion : "belum ke-install", gdVersion != null));
        card.addView(line("Rift Core", riftInGd ? "terpasang di GD" : "belum terpasang", riftInGd));
        card.addView(line("Mode", "sementara: buka GD terpasang", true));
        LinearLayout.LayoutParams cardLp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        cardLp.topMargin = dp(36);
        root.addView(card, cardLp);

        TextView foot = text("Rift Launcher 0.2", 12, DIM, false);
        foot.setGravity(Gravity.CENTER);
        LinearLayout.LayoutParams footLp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        footLp.topMargin = dp(28);
        root.addView(foot, footLp);

        setContentView(root);
    }
}
