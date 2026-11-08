package com.rift.launcher;

import android.app.Activity;
import android.app.Dialog;
import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.graphics.Typeface;
import android.graphics.drawable.ColorDrawable;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.RippleDrawable;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

final class U {
    static final int VIOLET = Color.parseColor("#7657FF");
    static final int LAVENDER = Color.parseColor("#A99AFF");
    static final int WHITE = Color.parseColor("#E8E9F0");
    static final int DIM = Color.parseColor("#8A8FAB");
    static final int GREEN = Color.parseColor("#5BE584");
    static final int RED = Color.parseColor("#FF7A90");
    static final int CARD = Color.parseColor("#1F2336");
    static final int CARD2 = Color.parseColor("#262B42");

    interface Toggle { void on(boolean v); }

    static int dp(Context c, int v) {
        return (int) (v * c.getResources().getDisplayMetrics().density + 0.5f);
    }

    static GradientDrawable shape(Context c, int color, int r) {
        GradientDrawable g = new GradientDrawable();
        g.setColor(color);
        g.setCornerRadius(dp(c, r));
        return g;
    }

    static TextView text(Context c, String s, int sp, int color, boolean bold) {
        TextView t = new TextView(c);
        t.setText(s);
        t.setTextSize(sp);
        t.setTextColor(color);
        if (bold) t.setTypeface(Typeface.DEFAULT_BOLD);
        return t;
    }

    static void press(Context c, View v, int color, int r) {
        v.setBackground(new RippleDrawable(ColorStateList.valueOf(Color.parseColor("#44FFFFFF")), shape(c, color, r), shape(c, Color.WHITE, r)));
        v.setOnTouchListener(new View.OnTouchListener() {
            public boolean onTouch(View x, MotionEvent e) {
                int a = e.getActionMasked();
                if (a == MotionEvent.ACTION_DOWN) x.animate().scaleX(0.96f).scaleY(0.96f).setDuration(90).start();
                else if (a == MotionEvent.ACTION_UP || a == MotionEvent.ACTION_CANCEL) x.animate().scaleX(1f).scaleY(1f).setDuration(160).start();
                return false;
            }
        });
    }

    static LinearLayout.LayoutParams lp(Context c, int top) {
        LinearLayout.LayoutParams l = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        l.topMargin = dp(c, top);
        return l;
    }

    static View header(Context c, String s) {
        TextView t = text(c, s.toUpperCase(), 12, LAVENDER, true);
        t.setLetterSpacing(0.12f);
        t.setPadding(dp(c, 6), dp(c, 22), 0, dp(c, 6));
        return t;
    }

    static LinearLayout rowBase(Context c) {
        LinearLayout r = new LinearLayout(c);
        r.setOrientation(LinearLayout.HORIZONTAL);
        r.setGravity(Gravity.CENTER_VERTICAL);
        r.setPadding(dp(c, 18), dp(c, 14), dp(c, 18), dp(c, 14));
        return r;
    }

    static LinearLayout titles(Context c, String title, String sub) {
        LinearLayout v = new LinearLayout(c);
        v.setOrientation(LinearLayout.VERTICAL);
        v.addView(text(c, title, 15, WHITE, false));
        if (sub != null) {
            TextView s = text(c, sub, 12, DIM, false);
            s.setPadding(0, dp(c, 2), 0, 0);
            v.addView(s);
        }
        return v;
    }

    // baris yang bisa ditekan
    static View row(Context c, String title, String sub, String right, View.OnClickListener l) {
        LinearLayout r = rowBase(c);
        r.addView(titles(c, title, sub), new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
        if (right != null) {
            TextView t = text(c, right, 14, LAVENDER, false);
            t.setGravity(Gravity.RIGHT);
            r.addView(t);
        }
        press(c, r, CARD, 18);
        if (l != null) r.setOnClickListener(l);
        return r;
    }

    static View toggle(final Context c, String title, String sub, boolean on, final Toggle cb) {
        LinearLayout r = rowBase(c);
        r.addView(titles(c, title, sub), new LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f));
        final Switch sw = new Switch(c);
        int[][] st = new int[][] { new int[] { android.R.attr.state_checked }, new int[] {} };
        sw.setThumbTintList(new ColorStateList(st, new int[] { Color.WHITE, Color.parseColor("#9A9FB8") }));
        sw.setTrackTintList(new ColorStateList(st, new int[] { VIOLET, Color.parseColor("#3A3F58") }));
        sw.setChecked(on);
        sw.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton b, boolean v) { cb.on(v); }
        });
        r.addView(sw);
        press(c, r, CARD, 18);
        r.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) { sw.toggle(); }
        });
        return r;
    }

    static void fadeIn(View v, int delay) {
        v.setAlpha(0f);
        v.setTranslationY(dp(v.getContext(), 18));
        v.animate().alpha(1f).translationY(0f).setStartDelay(delay).setDuration(380).start();
    }

    // dialog teks panjang + tombol salin
    static void showText(final Activity a, String title, final String body) {
        final Dialog d = new Dialog(a);
        d.requestWindowFeature(Window.FEATURE_NO_TITLE);
        LinearLayout box = new LinearLayout(a);
        box.setOrientation(LinearLayout.VERTICAL);
        box.setPadding(dp(a, 20), dp(a, 20), dp(a, 20), dp(a, 14));
        box.setBackground(shape(a, CARD, 24));
        box.addView(text(a, title, 20, WHITE, true));
        TextView t = text(a, body, 11, WHITE, false);
        t.setTypeface(Typeface.MONOSPACE);
        t.setTextIsSelectable(true);
        t.setPadding(0, dp(a, 10), 0, dp(a, 10));
        ScrollView sv = new ScrollView(a);
        sv.addView(t);
        box.addView(sv, new LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, dp(a, 360)));
        LinearLayout btns = new LinearLayout(a);
        btns.setGravity(Gravity.RIGHT);
        TextView copy = text(a, "Salin", 14, LAVENDER, true);
        copy.setPadding(dp(a, 18), dp(a, 10), dp(a, 18), dp(a, 10));
        press(a, copy, CARD2, 20);
        copy.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                ClipboardManager cm = (ClipboardManager) a.getSystemService(Context.CLIPBOARD_SERVICE);
                cm.setPrimaryClip(ClipData.newPlainText("Rift", body));
                Toast.makeText(a, "Disalin", Toast.LENGTH_SHORT).show();
            }
        });
        TextView close = text(a, "Tutup", 14, Color.WHITE, true);
        close.setPadding(dp(a, 24), dp(a, 10), dp(a, 24), dp(a, 10));
        press(a, close, VIOLET, 20);
        close.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) { d.dismiss(); }
        });
        btns.addView(copy);
        LinearLayout.LayoutParams cl = new LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        cl.leftMargin = dp(a, 10);
        btns.addView(close, cl);
        box.addView(btns);
        d.setContentView(box);
        Window w = d.getWindow();
        w.setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        w.setLayout((int) (a.getResources().getDisplayMetrics().widthPixels * 0.94f), ViewGroup.LayoutParams.WRAP_CONTENT);
        d.show();
    }
}
