package com.optocom.imarinfr.opi;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.opengl.GLSurfaceView;

public class Renderer implements GLSurfaceView.Renderer {

    private final long nativeApp;
    private final GLSurfaceView glView;

    private Background bg = new Background();
    private Stimulus stim = new Stimulus();
    private int step = 0;

    private boolean trialRunning = false;
    private long startTime = 0;
    private long timeSinceOnset;
    private long responseTime = 0;
    public Renderer(long mainNativeApp, GLSurfaceView mainGlView) {
        nativeApp = mainNativeApp;
        glView = mainGlView;
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {
        nativeOnSurfaceCreated(nativeApp);
    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int width, int height) {
        nativeSetScreenParams(nativeApp, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        // draw frame
        nativeOnDrawFrame(nativeApp, bg.bgeye, bg.bglum, bg.bgcol, bg.fixeye, bg.fixtype,
                bg.fixcx, bg.fixcy, bg.fixsx, bg.fixsy, bg.fixtheta, bg.fixlum, bg.fixcol,
                stim.eye[step], stim.type[step], stim.cx[step], stim.cy[step], stim.sx[step],
                stim.sy[step], stim.theta[step], stim.lum[step], stim.col[step]);
    }

    public void changeBackground(Background newbg) {
        bg = newbg;
        glView.requestRender(); // update background
    }

    public void presentStimulus(Stimulus newstim) {
        stim = newstim;
        long t0, dt;
        startTime      = System.currentTimeMillis();
        timeSinceOnset = System.currentTimeMillis() - startTime;
        trialRunning = true;

        for(int i = 0; i < stim.nsteps; i++) {
            step = i;
            glView.requestRender(); // render stimulus
            t0 = System.currentTimeMillis();
            dt = 0;
            while(trialRunning && dt < stim.tstep[step]) {
                dt = System.currentTimeMillis() - t0;
            }
        }
        // wait until minimum presentation time has passed
        while(timeSinceOnset < stim.d) {
            timeSinceOnset = System.currentTimeMillis() - startTime;
        }
        // clean stimulus
        stim = new Stimulus();
        step = 0;
        glView.requestRender();
        // while there is no response (aka trial still running),
        // wait to extinguish response window
        while(trialRunning && timeSinceOnset < stim.w) {
            timeSinceOnset = System.currentTimeMillis() - startTime;
        }
        trialRunning = false;
    }

    public void onTriggerEvent() {
        long minResponseTime = 100;
        if(trialRunning & timeSinceOnset > minResponseTime & timeSinceOnset < stim.w) {
            responseTime = timeSinceOnset;
            // wait at least until minimum stimulus presentation time is reached
            while(timeSinceOnset < stim.d) {
                timeSinceOnset = System.currentTimeMillis() - startTime;
            }
            trialRunning = false;
        }
    }

    public boolean trialRunning() {
        return trialRunning;
    }

    public long responseTime() {
        return responseTime;
    }

    private native void nativeOnSurfaceCreated(long nativeApp);
    private native void nativeSetScreenParams(long nativeApp, int width, int height);
    private native void nativeOnDrawFrame(long nativeApp, int bgeye, float bglum, float[] bgcol,
                                          int fixeye, int fixtype, float fixcx, float fixcy,
                                          float fixsx, float fixsy, float fixtheta, float fixlum,
                                          float[] fixcol, int steye, int sttype, float cx,
                                          float cy, float sx, float sy,float sttheta,
                                          float stlum, float[] stcol);
}
