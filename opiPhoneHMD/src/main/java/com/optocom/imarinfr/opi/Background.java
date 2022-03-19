package com.optocom.imarinfr.opi;

import androidx.annotation.NonNull;

public class Background {
    public int     bgeye;        // eye where to draw the background
    public float   bglum;        // background luminance from 0 to 1
    public float[] bgcol;        // background color
    public int     fixeye;       // eye where to draw the fixation
    public int     fixtype;    // type of target
    public float   fixcx, fixcy; // coordinates of the fixation target center in degrees
    public float   fixsx, fixsy; // size of the fixation target in degrees
    public float   fixtheta;     // rotation of the fixation target in degrees
    public float   fixlum;       // fixation luminance from 0 to 1
    public float[] fixcol;       // fixation color

    public Background() {
        // default data
        bgeye    = 2;
        bglum    = 0.1f;
        bgcol    = new float[]{1.0f, 1.0f, 1.0f, 1.0f};
        fixeye   = 2;
        fixtype  = -1;
        fixcx    = 0;
        fixcy    = 0;
        fixsx    = 0;
        fixsy    = 0;
        fixtheta = 0;
        fixlum   = 0.5f;
        fixcol   = new float[]{0.0f, 1.0f, 0.0f, 1.0f};
    }

    public boolean parseParameters(String[] pars) {
        if(pars.length != 18) return false;
        try {
            bgeye     = Integer.parseInt(pars[0]);
            bglum     = Float.parseFloat(pars[1]);
            bgcol[0]  = Float.parseFloat(pars[2]);
            bgcol[1]  = Float.parseFloat(pars[3]);
            bgcol[2]  = Float.parseFloat(pars[4]);
            bgcol[3]  = Float.parseFloat(pars[5]);
            fixeye    = Integer.parseInt(pars[6]);
            switch(pars[7]) {
                case Stimulus.CIRCLE:
                    fixtype = 0;
                    break;
                case Stimulus.SQUARE:
                    fixtype = 1;
                    break;
                case Stimulus.CROSS:
                    fixtype = 2;
                    break;
                case Stimulus.MALTESE:
                    fixtype = 3;
                    break;
                case Stimulus.ANNULUS:
                    fixtype = 4;
                    break;
                default:
                    fixtype = -1; // defaults to no shape
            }
            fixcx     = Float.parseFloat(pars[8]);
            fixcy     = Float.parseFloat(pars[9]);
            fixsx     = Float.parseFloat(pars[10]);
            fixsy     = Float.parseFloat(pars[11]);
            fixtheta  = Float.parseFloat(pars[12]);
            fixlum    = Float.parseFloat(pars[13]);
            fixcol[0] = Float.parseFloat(pars[14]);
            fixcol[1] = Float.parseFloat(pars[15]);
            fixcol[2] = Float.parseFloat(pars[16]);
            fixcol[3] = Float.parseFloat(pars[17]);
        } catch(NumberFormatException e) { // set all to zero or non-valid values
            return false;
        }
        return isValid();
    }

    public boolean isValid() {
        return bgeye  >= 0    & bgeye  <= 2      &
               fixeye >= 0    & fixeye <= 2      &
               bglum  >= 0    & bglum  <= 1      &
               fixlum >= 0    & fixlum <= 1      &
               bgcol[0] >= 0  & bgcol[0] <= 1    &
               bgcol[1] >= 0  & bgcol[1] <= 1    &
               bgcol[2] >= 0  & bgcol[2] <= 1    &
               bgcol[3] >= 0  & bgcol[3] <= 1    &
               fixcol[0] >= 0 & fixcol[0] <= 1   &
               fixcol[1] >= 0 & fixcol[1] <= 1   &
               fixcol[2] >= 0 & fixcol[2] <= 1   &
               fixcol[3] >= 0 & fixcol[3] <= 1   &
               fixtheta  >= 0  & fixtheta < 360 &
               fixtype  >= -1  & fixtype <= 4;
    }

    @NonNull
    public String toString() {
        return OpiConnection.OPI_SET_BACKGROUND + " " + bgeye + " " + bglum + " (" +
                bgcol[0] + "," + bgcol[1] + "," + bgcol[2] + "," + bgcol[3] + ") " +
                fixtype + fixeye + " " + fixlum + " (" +
                fixcol[0] + " " + fixcol[1] + " " + fixcol[2] + " " + fixcol[3] + ") " +
                fixcx + " " + fixcy + " " + fixsx + " " + fixsy;
    }
}
