package com.optocom.imarinfr.opi;

import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class Renderer implements GLSurfaceView.Renderer {

    private final long nativeApp;
    private final GLSurfaceView glView;

    private Background bg = new Background();
    private Stimulus stim = new Stimulus();
    private int step = 0;

    private boolean canClick = false;
    private boolean clicked = false;
    private long startTime = 0;
    private long timeSinceOnset = 0;
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

    public void presentStimulus(Stimulus newStim) {
        long minResponseTime = 100;

        stim = newStim;
        long w = stim.w;
        canClick = false;
        clicked = false;
        timeSinceOnset = 0;
        responseTime = 0;
        // render stimulus and manage response
        new Thread(this::renderStimulus).start();
        startTime = System.currentTimeMillis();
        // block until we can respond, then open response window
        while(timeSinceOnset < minResponseTime)
            timeSinceOnset = System.currentTimeMillis() - startTime;
        canClick = true;
        // keep open until either there has been a response
        // or allotted response time is over
        while(!clicked && timeSinceOnset < w)
            timeSinceOnset = System.currentTimeMillis() - startTime;
        canClick = false;
    }

    private void renderStimulus() {
        long t0, dt;
        for(int i = 0; i < stim.nsteps; i++) {
            step = i;
            glView.requestRender(); // render stimulus
            t0 = System.currentTimeMillis();
            dt = 0;
            while(dt < stim.tstep[step]) {
                dt = System.currentTimeMillis() - t0;
            }
            // if we got a valid response and minimum presentation
            // time is over, clean stimulus
            if(clicked && System.currentTimeMillis() - startTime > stim.d)
                break;
        }
        // if responded clean stimulus
        stim = new Stimulus();
        step = 0;
        glView.requestRender();
    }

    public void onTriggerEvent() {
        // if can respond
        if(canClick) {
            responseTime = timeSinceOnset;
            clicked = true;
            canClick = false;
        }
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
