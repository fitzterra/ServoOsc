/**
 * Very basic example of running a servo on pin 2 at an oscillation amplitude
 * of 60° around an offset of 20°.
 * 
 * Hint: Uncomment the setReverse() line to mirror the direction.
 **/

#include <ServoOsc.h>

#define SERVO_PIN 2

// Create the oscillator object, using the default values for all settings -
// see ServoOsc.h for defaults
// Another option would be to only declare a pointer to a ServoOsc object here,
// and then use new() in setup() to instantiate an instance.
ServoOsc osc1;

void setup() {
    osc1.setAmplitude(60);
    osc1.setOffset(20);
    osc1.setPeriod(1200);
    //osc1.setReverse(true)
    osc1.attach(SERVO_PIN);
}

void loop() {
    // The update should be called more frequently than the value of
    // UPDATE_PERIOD in ServoOsc.h
    osc1.update();
    delay(10);
}
