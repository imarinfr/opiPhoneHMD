package com.optocom.imarinfr.opi;

public class Stimulus {
    public static final String CIRCLE = "circle";
    public static final String SQUARE = "square";
    public static final String CROSS = "cross";
    public static final String MALTESE = "maltese";
    public static final String ANNULUS = "annulus";

    public int   nsteps;   // number of steps in the presentation
    public int[] eye;      // eye where to draw the stimulus in each step
    public int[] type;     // type of stimulus to present in each step
    public float[] cx, cy; // coordinates of the stimulus center in each step in degrees
    public float[] sx, sy; // size of the stimulus in this each in degrees
    public float[] theta;  // rotation of the stimulus in this each in degrees
    public long[] tstep;   // step duration in ms
    public float[] lum;    // step luminance from 0 to 1
    public float[][] col;  // step color
    public long  d;        // total stimulus duration in ms
    public long  w;        // response window in ms

    public Stimulus() {
        nsteps = 1;
        d = 0;
        w = 0;
        initStepParams(nsteps);
    }

    public void initStepParams(int n) {
        eye    = new int[n];
        type   = new int[n];
        cx     = new float[n];
        cy     = new float[n];
        sx     = new float[n];
        sy     = new float[n];
        theta  = new float[n];
        tstep  = new long[n];
        lum    = new float[n];
        col    = new float[n][4];
    }

    public int steps() {
        return nsteps;
    }

    public boolean parseGlobalPars(String[] pars) {
        if(pars.length != 3) return false;
        try {
            nsteps = Integer.parseInt(pars[0]); // number of stimulus steps
            d      = Long.parseLong(pars[1]);   // stimulus duration
            w      = Long.parseLong(pars[2]);   // response window
            // prepare to receive detail
            initStepParams(nsteps);
        } catch(NumberFormatException e) { // set all to zero or non-valid values
            return false;
        }
        return areGlobalParsValid();
    }

    public boolean parseStepPars(int step, String[] pars) {
        if(pars.length != 13) return false;
        try {
            eye[step]    = Integer.parseInt(pars[0]);
            type[step]   = parseTypes(pars[1]);
            cx[step]     = Float.parseFloat(pars[2]);
            cy[step]     = Float.parseFloat(pars[3]);
            sx[step]     = Float.parseFloat(pars[4]);
            sy[step]     = Float.parseFloat(pars[5]);
            theta[step]  = Float.parseFloat(pars[6]);
            tstep[step]  = Long.parseLong(pars[7]);
            lum[step]    = Float.parseFloat(pars[8]);
            col[step][0] = Float.parseFloat(pars[9]);
            col[step][1] = Float.parseFloat(pars[10]);
            col[step][2] = Float.parseFloat(pars[11]);
            col[step][3] = Float.parseFloat(pars[12]);
        } catch(NumberFormatException e) { // set all to zero or non-valid values
            return false;
        }
        return areStepParsValid(step);
    }

    public int parseTypes(String type) {
        switch (type) {
            case CIRCLE:
                return 0;
            case SQUARE:
                return 1;
            case CROSS:
                return 2;
            case MALTESE:
                return 3;
            case ANNULUS:
                return 4;
            default:
                return -1;
        }
    }

    public boolean areGlobalParsValid() {
        return (nsteps > 0 && d > 0 && w > d);
    }

    public boolean areStepParsValid(int step) {
        return eye[step] >= 0 && eye[step] <= 2 &&
                !(lum[step] < 0) && !(lum[step] > 1) &&
                !(theta[step] < 0) && !(theta[step] >= 360) &&
                type[step] >= -1 && type[step] <= 4 &&
                tstep[step] > 0 &&
                !(col[step][0] < 0) && !(col[step][0] > 1) &&
                !(col[step][1] < 0) && !(col[step][1] > 1) &&
                !(col[step][2] < 0) && !(col[step][2] > 1) &&
                !(col[step][3] < 0) && !(col[step][3] > 1);
    }
}
