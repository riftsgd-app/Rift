package com.rift.launcher;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.RectF;
import android.graphics.Shader;
import java.io.File;

// gambar kartu Browse, tombol Download / Restart
public class RiftArt5 {
    static void extra(File dir) throws Exception {
        bcard(new File(dir, "rift_bcard.png"));
        pill(new File(dir, "rift_pill_v.png"), 80, 26, true);
        pill(new File(dir, "rift_pill_g.png"), 80, 26, false);
        pill(new File(dir, "rift_btn_restart.png"), 230, 30, true);
    }

    static void bcard(File f) throws Exception {
        int w = Math.round(440 * RiftArt.K), h = Math.round(72 * RiftArt.K);
        Bitmap b = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.scale(RiftArt.K, RiftArt.K);
        RectF r = new RectF(1, 1, 439, 71);
        Paint fill = new Paint(Paint.ANTI_ALIAS_FLAG);
        fill.setColor(0x1CFFFFFF);
        c.drawRoundRect(r, 22, 22, fill);
        Paint st = new Paint(Paint.ANTI_ALIAS_FLAG);
        st.setStyle(Paint.Style.STROKE);
        st.setStrokeWidth(1.6f);
        st.setColor(0x52FFFFFF);
        c.drawRoundRect(r, 22, 22, st);
        Paint ic = new Paint(Paint.ANTI_ALIAS_FLAG);
        ic.setShader(new LinearGradient(14, 14, 58, 58, 0xFF9B8BFF, 0xFF5B34D6, Shader.TileMode.CLAMP));
        c.drawCircle(36, 36, 22, ic);
        RiftArt.logo(c, 36, 36, 15);
        RiftArt.save(b, f);
    }

    static void pill(File f, int pw, int ph, boolean violet) throws Exception {
        int w = Math.round(pw * RiftArt.K), h = Math.round(ph * RiftArt.K);
        Bitmap b = Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.scale(RiftArt.K, RiftArt.K);
        RectF r = new RectF(0.8f, 0.8f, pw - 0.8f, ph - 0.8f);
        float rad = ph / 2f;
        Paint p = new Paint(Paint.ANTI_ALIAS_FLAG);
        if (violet) {
            p.setShader(new LinearGradient(0, 0, pw, ph, 0xFF8A76FF, 0xFF6454F4, Shader.TileMode.CLAMP));
            c.drawRoundRect(r, rad, rad, p);
            Paint g = new Paint(Paint.ANTI_ALIAS_FLAG);
            g.setShader(new LinearGradient(0, 0, 0, ph, 0x48FFFFFF, 0x00FFFFFF, Shader.TileMode.CLAMP));
            c.drawRoundRect(r, rad, rad, g);
        } else {
            p.setColor(0x26FFFFFF);
            c.drawRoundRect(r, rad, rad, p);
            Paint s = new Paint(Paint.ANTI_ALIAS_FLAG);
            s.setStyle(Paint.Style.STROKE);
            s.setStrokeWidth(1.4f);
            s.setColor(0x44FFFFFF);
            c.drawRoundRect(r, rad, rad, s);
        }
        RiftArt.save(b, f);
    }
}
