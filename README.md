Red Dot Scanner
===============

A laser-based optical 3D scanner that I created for the OpenCV AI Competition 2022. Check out the demo video: https://youtu.be/ZFtsnY-hWn0

In order to make this work, you need:

 *  a LEGO Mindstorms NXT based pan-tilt device (aka "LaserTurret"), where:

     *  a PWM controllable laser module (with a proper resistor) is attached to Port A

     *  the tilt motor is attached to Port B

     *  the pan motor is attached to Port C
 
 *  at least 2 PlayStation 3 Eye cameras

(More info is coming soon)


Building
--------

### Requirements

 *  Visual Studio 2022

 *  Install the Windows App SDK 1.1.5 _Installer_ from https://learn.microsoft.com/en-us/windows/apps/windows-app-sdk/downloads

    For more info: https://learn.microsoft.com/en-us/windows/apps/winui/winui3/create-your-first-winui3-app#unpackaged-create-a-new-project-for-an-unpackaged-c-or-c-winui-3-desktop-app

 *  Install the LEGO NXT Fantom driver from https://www.lego.com/en-us/themes/mindstorms/downloads

 *  Install the WinUSB driver for the PS3Eye camera(s): https://github.com/opentrack/opentrack/wiki/PS3-Eye-open-driver-instructions
    
    (I used [zadig](https://zadig.akeo.ie/) instead of _libusb inf wizard_.)
