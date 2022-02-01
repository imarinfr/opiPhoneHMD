# opiPhoneVR
OPI server on Phone (Android platform). Requires a VR headset that is compatible
with Google Cardboard.

- Server: Android phone
- Client: Desktop

The Server and the Client must be in the same LAN.

## Protocols

    Command                  Description                 
    -----------------------------------------------------------------
    OPI_GET_METRICS          Returns the x and y screen resolution, x and y FOV,
                             and pixel density.

    OPI_SET_BACKGROUND       Sets background color and luminance for left eye,
                             right eye or both. Sets fixation target shape,
                             color, and luminance for left eye, right eye or
                             both.

    OPI_PRESENT              Presents a visual stimulus to the left eye, The
                             right eye or both at a specific (x, y) coordinate
                             with a specific luminance, color, and x and y
                             sizes. The stimulus is presented.

    OPI_CLOSE                Terminates client connection OK | error message    

FOV = Field of view in degrees.

FOV, all positions, and sizes are in degrees of visual.

Luminance and each color channel should be a normalized float value from 0 to 1.
Pixel density is in dots per inch (dpi) and screen resolution are in pixels.

Colors are defined by a 4-element vector for R, G, B channels and the alpha
channel, which allows transparency. All 4 channels are float values from 0 to 1.

The physical dimensions of the phone can be calculated with help of the FOV.
