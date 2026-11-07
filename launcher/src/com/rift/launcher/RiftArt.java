package com.rift.launcher;

import android.graphics.Bitmap;
import android.graphics.BlurMaskFilter;
import android.graphics.Canvas;
import android.graphics.LinearGradient;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.RadialGradient;
import android.graphics.RectF;
import android.graphics.Shader;
import android.graphics.Typeface;
import java.io.File;
import java.io.FileOutputStream;

// gambar menu Rift di dalam game: jendela "Mod Manager" bergaya glassmorphism
public class RiftArt {
    // semua koordinat desain memakai skala 1003 x 588, lalu dikali K ke piksel gambar
    static final float K = 1280f / 1003f;

    static void save(Bitmap b, File f) throws Exception {
        FileOutputStream o = new FileOutputStream(f);
        b.compress(Bitmap.CompressFormat.PNG, 100, o);
        o.close();
        b.recycle();
    }

    static Paint paint() {
        return new Paint(Paint.ANTI_ALIAS_FLAG);
    }

    static void blank(File f, int w, int h) throws Exception {
        save(Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888), f);
    }

    static void generate(File dir) throws Exception {
        window(new File(dir, "rift_win_a.png"), false);
        window(new File(dir, "rift_win_b.png"), true);
        button(new File(dir, "rift_btn_glass.png"));
        blank(new File(dir, "rift_blank.png"), 8, 8);
        blank(new File(dir, "rift_hit_win.png"), 128, 75);
        blank(new File(dir, "rift_hit_tab1.png"), Math.round(259 * K), Math.round(50 * K));
        blank(new File(dir, "rift_hit_tab2.png"), Math.round(211 * K), Math.round(50 * K));
        blank(new File(dir, "rift_hit_refresh.png"), Math.round(158 * K), Math.round(51 * K));
        blank(new File(dir, "rift_hit_close.png"), Math.round(52 * K), Math.round(52 * K));
        RiftArt2.extra(dir);
    }

    static void text(Canvas c, String s, float x, float y, float size, int color, boolean bold, float spacing, Paint.Align al) {
        Paint p = paint();
        p.setColor(color);
        p.setTextSize(size);
        p.setTextAlign(al);
        p.setTypeface(Typeface.create(bold ? "sans-serif" : "sans-serif-medium", bold ? Typeface.BOLD : Typeface.NORMAL));
        p.setLetterSpacing(spacing);
        c.drawText(s, x, y, p);
    }

    static void pill(Canvas c, float l, float t, float r, float b, boolean active) {
        RectF rf = new RectF(l, t, r, b);
        float rad = (b - t) / 2f;
        if (active) {
            Paint p = paint();
            p.setShader(new LinearGradient(l, t, r, b, 0xFF7C67FF, 0xFF6454F4, Shader.TileMode.CLAMP));
            c.drawRoundRect(rf, rad, rad, p);
            Paint g = paint();
            g.setShader(new LinearGradient(0, t, 0, b, 0x40FFFFFF, 0x00FFFFFF, Shader.TileMode.CLAMP));
            c.drawRoundRect(rf, rad, rad, g);
        } else {
            Paint p = paint();
            p.setColor(0x1CFFFFFF);
            c.drawRoundRect(rf, rad, rad, p);
            Paint s = paint();
            s.setStyle(Paint.Style.STROKE);
            s.setStrokeWidth(1.6f);
            s.setColor(0x52FFFFFF);
            c.drawRoundRect(rf, rad, rad, s);
        }
    }

    static void logo(Canvas c, float cx, float cy, float half) {
        float k = half / 26f;
        c.save();
        c.translate(cx, cy);
        c.scale(k, k);
        c.translate(-56, -56);
        Path l = new Path();
        l.moveTo(53, 30); l.lineTo(33.5f, 43); l.lineTo(33.5f, 69); l.lineTo(53, 82);
        l.lineTo(53, 64); l.lineTo(43, 56); l.lineTo(53, 48); l.close();
        Path rr = new Path();
        rr.moveTo(59, 30); rr.lineTo(78.5f, 43); rr.lineTo(78.5f, 69); rr.lineTo(59, 82);
        rr.lineTo(59, 64); rr.lineTo(69, 56); rr.lineTo(59, 48); rr.close();
        Paint lp = paint();
        lp.setShader(new LinearGradient(33, 30, 53, 82, 0xFF9B6BFF, 0xFF5B34D6, Shader.TileMode.CLAMP));
        c.drawPath(l, lp);
        Paint rp = paint();
        rp.setShader(new LinearGradient(59, 30, 79, 82, 0xFFD9CEFF, 0xFFB7A6FF, Shader.TileMode.CLAMP));
        c.drawPath(rr, rp);
        c.restore();
    }

    static void window(File f, boolean browse) throws Exception {
        int W = 1280, H = 750;
        Bitmap b = Bitmap.createBitmap(W, H, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        c.scale(K, K);
        RectF r = new RectF(16, 15, 985, 571);
        Paint sh = paint();
        sh.setColor(0x80000000);
        sh.setMaskFilter(new BlurMaskFilter(14, BlurMaskFilter.Blur.NORMAL));
        c.drawRoundRect(new RectF(18, 22, 983, 578), 52, 52, sh);
        Path clip = new Path();
        clip.addRoundRect(r, 52, 52, Path.Direction.CW);
        c.save();
        c.clipPath(clip);
        Paint bg = paint();
        bg.setShader(new LinearGradient(16, 15, 985, 571, new int[] {0xFF5B4FC4, 0xFF383A57, 0xFF2A2D3A},
                new float[] {0f, 0.5f, 1f}, Shader.TileMode.CLAMP));
        c.drawRect(r, bg);
        Paint g1 = paint();
        g1.setShader(new RadialGradient(120, 60, 430, 0x808A7DFF, 0x008A7DFF, Shader.TileMode.CLAMP));
        c.drawRect(r, g1);
        Paint g2 = paint();
        g2.setShader(new RadialGradient(985, 590, 340, 0x9A7471B0, 0x007471B0, Shader.TileMode.CLAMP));
        c.drawRect(r, g2);
        Paint top = paint();
        top.setShader(new LinearGradient(0, 15, 0, 170, 0x26FFFFFF, 0x00FFFFFF, Shader.TileMode.CLAMP));
        c.drawRect(r, top);
        c.restore();
        Paint br = paint();
        br.setStyle(Paint.Style.STROKE);
        br.setStrokeWidth(2.4f);
        br.setShader(new LinearGradient(16, 15, 985, 571, new int[] {0xB0FFFFFF, 0x48FFFFFF, 0x90FFFFFF},
                new float[] {0f, 0.5f, 1f}, Shader.TileMode.CLAMP));
        c.drawRoundRect(r, 52, 52, br);

        // header
        logo(c, 91, 90, 37);
        text(c, "RIFT", 152, 91, 39, 0xFFFFFFFF, true, 0.06f, Paint.Align.LEFT);
        text(c, "MOD MANAGER", 152, 121, 19, 0xFFA99AFF, false, 0.07f, Paint.Align.LEFT);
        Paint cf = paint();
        cf.setShader(new LinearGradient(0, 62, 0, 110, 0x55FFFFFF, 0x2AFFFFFF, Shader.TileMode.CLAMP));
        c.drawCircle(930, 86, 24, cf);
        Paint cr = paint();
        cr.setStyle(Paint.Style.STROKE);
        cr.setStrokeWidth(2.2f);
        cr.setColor(0xB0FFFFFF);
        c.drawCircle(930, 86, 23, cr);
        Paint x = paint();
        x.setStyle(Paint.Style.STROKE);
        x.setStrokeWidth(4f);
        x.setStrokeCap(Paint.Cap.ROUND);
        x.setColor(0xFFFFFFFF);
        c.drawLine(921, 77, 939, 95, x);
        c.drawLine(939, 77, 921, 95, x);

        // tab
        pill(c, 52, 160, 311, 210, !browse);
        pill(c, 326, 160, 537, 210, browse);
        text(c, "Installed Mods", 181.5f, 193.5f, 24, browse ? 0xFFB5A9FF : 0xFFFFFFFF, false, 0f, Paint.Align.CENTER);
        text(c, "Browse Mods", 431.5f, 193.5f, 24, browse ? 0xFFFFFFFF : 0xFFB5A9FF, false, 0f, Paint.Align.CENTER);

        // pencarian + refresh
        pill(c, 52, 228, 771, 279, false);
        text(c, "Search mods...", 87, 261, 22, 0x73FFFFFF, false, 0f, Paint.Align.LEFT);
        pill(c, 790, 228, 948, 279, true);
        text(c, "Refresh", 869, 261.5f, 23, 0xFFFFFFFF, false, 0f, Paint.Align.CENTER);

        // isi
        if (!browse) {
            text(c, "Belum ada mod terpasang", 501, 428, 27, 0xCCFFFFFF, false, 0f, Paint.Align.CENTER);
            text(c, "Taruh file .rift di folder mods lalu tekan Refresh", 501, 466, 20, 0xFFB5A9FF, false, 0f, Paint.Align.CENTER);
        } else {
            RiftArt3.browse(c);
        }
        save(b, f);
    }

    static void button(File f) throws Exception {
        int S = 112;
        Bitmap b = Bitmap.createBitmap(S, S, Bitmap.Config.ARGB_8888);
        Canvas c = new Canvas(b);
        float cx = 56, cy = 56, R = 52;
        Paint sh = paint();
        sh.setColor(0x55000000);
        sh.setMaskFilter(new BlurMaskFilter(4, BlurMaskFilter.Blur.NORMAL));
        c.drawCircle(cx, cy + 2, R, sh);
        Paint fill = paint();
        fill.setShader(new LinearGradient(14, 8, 98, 104, 0xEEB6A6FF, 0xEE6A45F2, Shader.TileMode.CLAMP));
        c.drawCircle(cx, cy, R, fill);
        Path clip = new Path();
        clip.addCircle(cx, cy, R, Path.Direction.CW);
        c.save();
        c.clipPath(clip);
        Paint gloss = paint();
        gloss.setShader(new LinearGradient(0, 4, 0, 60, 0x88FFFFFF, 0x00FFFFFF, Shader.TileMode.CLAMP));
        c.drawOval(new RectF(10, 4, 102, 62), gloss);
        c.restore();
        Paint rim = paint();
        rim.setStyle(Paint.Style.STROKE);
        rim.setStrokeWidth(3.5f);
        rim.setShader(new LinearGradient(8, 8, 104, 104, 0xFFFFFFFF, 0x66FFFFFF, Shader.TileMode.CLAMP));
        c.drawCircle(cx, cy, R - 1.5f, rim);
        Path l = new Path();
        l.moveTo(53, 30); l.lineTo(33.5f, 43); l.lineTo(33.5f, 69); l.lineTo(53, 82);
        l.lineTo(53, 64); l.lineTo(43, 56); l.lineTo(53, 48); l.close();
        Path rr = new Path();
        rr.moveTo(59, 30); rr.lineTo(78.5f, 43); rr.lineTo(78.5f, 69); rr.lineTo(59, 82);
        rr.lineTo(59, 64); rr.lineTo(69, 56); rr.lineTo(59, 48); rr.close();
        Paint lp = paint();
        lp.setColor(0xFFFFFFFF);
        c.drawPath(l, lp);
        Paint rp = paint();
        rp.setColor(0xFFE2DBFF);
        c.drawPath(rr, rp);
        save(b, f);
    }
}
