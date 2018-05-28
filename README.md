Servo Oscillator Library
========================

This library is based on the original Servo Oscillator code by Juan
Gonzalez-Gomez (Obijuan), which is part of his [ArduSnake] project ([on Github]).

Since the Servo Oscillator is so handy in other applications, I have split it
off into it's own library, made a number of changes to make it easier for me,
and hopefully others, to understand and use, and added an extensive interactive
example to show all functionality.

Base functionality
------------------
At it's base, this library allows for a servo to oscillate at a given angle
(amplitude) around a center point.

Here is an animation of 2 example oscillating servos:

Ampl: 90°, Offset: 0°            | Ampl: 60°, Offset: 20°
:-------------------------------:|:--------------------------------:
![Ampl:90, Offs:0][a90o0] <p>**Example 1**</p>  | ![Ampl:60, Offs:20][a60o20] <p>**Example 2**</p>


**Example 1**:  
This one runs at an amplitude of 90° and offset of 0°. One period is 4*90° = 360°

**Example 2**:  
This one has an amplitude of 60° and an offset of 20°. One period is 4*60° = 240°

Features
--------

Some of the main features:

* Run as many servos as available digital pins (only D0 is not available)
* Servos can be attached and detached at run time.
* Servos can be stopped and started - when stopped, it continues to keep the
  cycle position in the background (as long as the update() method is called).
  This is useful on a multi servo walker robot that has a specific gait where
  the synchronization of all servos will be kept even if one or more are stopped
  and then started again later for some reason.
* The amplitude, center offset, speed (period), trim correction and starting
  phase can be controlled for each servo.


Terminology
-----------
* **Oscillation**: Continues rotation of the servo from one end to the opposite
    around the central offset. The full oscillation angle possible is 180°
    (servo dependant) which is virtually mapped from -90° to +90° with 0° being
    the center of oscillation.
* **Offset**: This is the offset from 0° to set as the center of oscillation and
    can be positive or negative. Note that any offset added, reduces the 180°
    arc of movement by the offset amount.
* **Trim**: Some servos do not have a level of precision and setting the offset
    to 0° for example, does not leave the horn exactly center. The trim can be
    used to adjust this position in 1° increments, positive or negative - in
    future, the granularity of this adjustment may be made smaller to allow
    fractions of a degree.
* **Amplitude**: This is the degrees of movement to travel from the offset in
    both positive and negative direction. With an offset of 0°, the max
    amplitude is 90° which will result in a full oscillation arc of 180° (-90°
    to +90°). Note that the amplitude can be greater than 90°, but the
    underlying servo library or the servo itself will ignore angles outside it's
    range. In this case, then the movement reaches the max servo position, it
    would stop, the oscillation calculation would continue with further invalid
    angles sent to the serve until the oscillation max is reached and movement
    direction changes. Only once the angle comes back into range, will the servo
    start moving back toward the offset position.
* **Period**: The amount of time in milliseconds, that one full oscillation cycle
    takes. Note that one full cycle is 4 * the amplitude.
* **Phase**: This is the position within the cycle where the movement starts.
    This is mostly useful when there are two or more servos, oscillating at the
    same time with the same amplitude and period, but one or more needs to start
    at a different positions within the cycle if they are all started together.

Example
-------
With a servo connected to pin 2 of an arduino, the code below will cause that
servo to more or less operate as example 2 above.

```c++
#include <ServoOsc.h>

ServoOsc osc1;

void setup() {
    osc1.setAmplitude(60);
    osc1.setOffset(20);
    osc1.setPeriod(2000);
    osc1.attach(2);
}

void main() {
    osc1.update();
    delay(10);
}
```

[ArduSnake]:  http://www.iearobotics.com/wiki/index.php?title=ArduSnake:amplituderduino_Modular_Snake_Robots_Library
[on Github]: https://github.com/Obijuan/ArduSnake
[a90o0]: animations/servo-A90O0.gif
[a60o20]: animations/servo-A60O20.gif

