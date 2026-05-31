# CYD_Mandelbrot
A Mandelbrot-App for a CYD-display (cheap yellow display, bought may 2026)

This App is for a CYD-uinit. Its a common name for cheap yellow PCBs that carry a display and an ESP32-processor.

The Source is for Arduino-IDE. Choose "ESP32 Dev Module" as unit.

Install the "TFT_eSPI by Bodmer" as driver for the display.
Copy the "User_Setup.h"-file to your Library-folder, into folder TFT_eSPI.

compile, upload, have fun...

When all works, you will see a startup-screen for Mandelbrot Maker.

Then, the complete mandelbrot-set is calculated.

Usage after maxndelbrot is created:

You have eight pads on your display to tap on:

Left-top : Press short to start to choose a new part of the set, click on the midpint of the image-area you want to see zoomed.
            Press long (more than one second) to restore the standard-mandelbrot-set. The itaration-setting is not resetted.
            
Left-bottom : Press short to zoom Into the set without changing the midpoint
              Press long (mor than one second) to increase Iteration-value. Every second adds 10 units to the iteration-value

Right-bottom ; Press short to zoom out
                Press long to decrease Iteration-value

Mid-top : Scroll image up, long press for more scrolling
Mid-bottom : Scroll image down, long press for more scrolling 
Left-mid : Scroll image left, long press for more scrolling
Right-mid : Scroll image right, long press for more scrolling

Have Phun!
