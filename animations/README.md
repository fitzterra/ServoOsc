Servo Simulations
=================

This directory contains the following to help generate simulations of servo
oscilations:

**`servos.scad`**: OpenSCAD modules for generating a servo and horn.

**`servoSim.scad`**: An OpenSCAD file that uses the `servos.scad` file to create
a servo with horn at a specific angle.

**`makeAnim.sh`**: A shell script that can generate a series of PNGs using
OpenSCAD and the avobe two `.scad` files, with the horn at 5° increment angles
from -90° to +90°. It uses the OpenSCAD command line arguments to set the camera
view, the angle parameters, the desired image size and then exports this to a
PNG image. The same script can then use the generated PNGs to create animated
GIFs.
