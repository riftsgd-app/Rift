package com.rift.launcher;

import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.Typeface;

// daftar mod di tab Browse Mods
public class RiftArt3 {
    static final String[][] MODS = {
        {"Soundboard", "Pulse", "Trigger custom sounds on jumps and deaths."},
        {"Macro Pad", "echo", "Record and replay inputs for testing sections."},
        {"Icon Studio", "Nova", "Recolor and preview every icon in one menu."},
        {"Frame Stepper", "vex", "Step through any attempt one frame at a time."},
        {"Practice+", "kiro", "Better checkpoints and run stats for practice mode."},
        {"Neon Trails", "Lumen", "Glowing trails that follow your icon."},
    };

    static void fit(Canvas c, String s, float x, float y, float size, float maxW, int color, boolean bold, Paint.Align al) {
        Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);
        p.setTypeface(Typeface.create(bold ? "sans-serif" : "sans-serif-medium", bold ? Typeface.BOLD : Typeface.NORMAL));
        p.setTextSize(size);
        while (p.measureText(s) > maxW && size > 9) {
            size -= 0.5f;
            p.setTextSize(size);
        }
        RiftArt.text(c, s, x, y, size, color, bold, 0f, al);
    }

    static void browse(Canvas c) {
        for (int i = 0; i < MODS.length; i++) {
            float x = 52 + (i % 2) * 456;
            float y = 300 + (i / 2) * 82;
            RectF r = new RectF(x, y, x + 440, y + 72);
            Paint f = new Paint(Paint.ANTI_ALIAS_FLAG);
            f.setColor(0x1CFFFFFF);
            c.drawRoundRect(r, 22, 22, f);
            Paint s = new Paint(Paint.ANTI_ALIAS_FLAG);
            s.setStyle(Paint.Style.STROKE);
            s.setStrokeWidth(1.6f);
            s.setColor(0x52FFFFFF);
            c.drawRoundRect(r, 22, 22, s);
            Paint ic = new Paint(Paint.ANTI_ALIAS_FLAG);
            ic.setShader(new LinearGradient(x + 14, y + 14, x + 58, y + 58, 0xFF9B8BFF, 0xFF5B34D6, Shader.TileMode.CLAMP));
            c.drawCircle(x + 36, y + 36, 22, ic);
            RiftArt.text(c, MODS[i][0].substring(0, 1), x + 36, y + 45, 26, 0xFFFFFFFF, true, 0f, Paint.Align.CENTER);
            fit(c, MODS[i][0], x + 72, y + 31, 22, 230, 0xFFFFFFFF, true, Paint.Align.LEFT);
            fit(c, "by " + MODS[i][1], x + 426, y + 31, 16, 120, 0xFFA99AFF, false, Paint.Align.RIGHT);
            fit(c, MODS[i][2], x + 72, y + 57, 15, 354, 0xB3FFFFFF, false, Paint.Align.LEFT);
        }
    }
}
