/**
 * This library is based on the original Servo Oscillator code by Juan
 * Gonzalez-Gomez (Obijuan), which is part of his ArduSnake project.
 * 
 * ArduSnake:  http://www.iearobotics.com/wiki/index.php?title=ArduSnake:amplituderduino_Modular_Snake_Robots_Library
 * and: https://github.com/Obijuan/ArduSnake
 *
 * Since the Servo Oscillator is so handy in other applications, I have split
 * it off into it's own library, make a number of changes to make it easier for
 * me to understand and use, and added an extensive interactive example to show
 * all functionality.
 *
 * (c) Juan Gonzalez-Gomez (Obijuan), Dec 2011
 * GPL License
 *
 * (c) Tom Coetser (Fitzterra), May 2018
 * GPL License
 **/

#include "ServoOsc.h"

// Constructor
ServoOsc::ServoOsc(uint16_t p, uint8_t a, int8_t o, int8_t ph, int8_t tr, uint8_t pn):
    period(p),
    amplitude(a),
    offset(o),
    phase(DEG2RAD(ph)),
    trim(tr),
    pin(pn),
    currPhase(0.0),
    stopped(false),
    reversed(false) {

    // We need to calculate the phase increment wherever the period changes
    calcPhaseInc();
    // Attach the servo if possible
    attach();

#ifdef __SO_DBG
    printConf();
#endif // __SO_DBG
}

// Tests if it is time for the next position update
bool ServoOsc::shouldUpdate() {
    // Local millis() value of the last update
    static uint32_t lastUpdate = 0;
    uint32_t now = millis();

    if((now - lastUpdate)> UPDATE_PERIOD) {
        lastUpdate = now;
        return true;
    }

    return false;
}

// Calculates phaseInc from period and UPDATE_PERIOD
void ServoOsc::calcPhaseInc() {
    // Calculate the number of samples that will be taken given the total
    // period for one cycle, and how often we do updates.
    float numSamples = (float)period / UPDATE_PERIOD;
    // Calculate by how many radians to move the servo on every update
    phaseInc = 2*M_PI / numSamples;

#ifdef __SO_DBG
    Serial << F("New phase inc: ") << phaseInc << endl;
#endif // __SO_DBG
}

// Attach to the servo if not already attached.
//
// Optionally supply the pin to attach to.
// If not attached and p==0 (default if not supplied), it will attach to the
// previously set pin.
// If p!=0, it will attach to that pin, and make it the new default pin.
//
// Returns true if new attachment, else false
bool ServoOsc::attach(uint8_t p) {
    // Return failure if we are already attached
    if (pin!=0)
        return false;
    // Update the new pin?
    pin = p!=0 ? p : pin;
    // Return failure if we do not have a valid pin
    if(pin==0)
        return false;

    servo.attach(pin);
#ifdef __SO_DBG
    Serial << "Servo is attached? " << servo.attached() << endl;
#endif // __SO_DBG
    return true;
}

// Detach from the servo if attached.
// If not attached, returns false, else return true after detach.
bool ServoOsc::detach() {
    // Return failure if we are currently detached
    if (pin==0)
        return false;
    // Detach and update the pin
    servo.attach(pin);
    pin = 0;

    return true;
}

// Set servo poistion if attached
bool ServoOsc::positionServo(int8_t a, bool t) {
    // Indicate error if not attached
    if (pin == 0) return false;

    // Adjust angle
    a += t ? trim : 0;
    if(reversed)
        a = -a;
    // Set the angle
    servo.write(a+90);

    return true;
}

// Reset the servo to it's starting position
void ServoOsc::resetToStart() {
    // Reset the current phase
    currPhase = 0;
    // Calc the servo poistion as done by update() - this may be
    // refactored into a single calc function in future.
    float pos = round(amplitude * sin(currPhase + phase) + offset);
    if (reversed)
        pos = -pos;

    // Postion the servo
    positionServo(pos, true);
}

// Servo position update
void ServoOsc::update() {
    // Can't do anything if the servo is not attached
    if(pin == 0)
        return;

    // Return if we're not due for the next update
    if(!shouldUpdate())
        return;

    // If the oscillator is not stopped, calculate the servo position
    if (!stopped) {
        // Sample the sine function and set the servo pos
        float pos = round(amplitude * sin(currPhase + phase) + offset);
        if (reversed)
            pos = -pos;

         servo.write(pos + 90 + trim);
#ifdef __SO_DBG
         Serial << "new pos: " << (pos + 90 + trim) << "\n";
#endif //__SO_DBG
     }

    // Increment the phase
    // It is always increased, even when the oscillator is stop
    // so that the coordination is always kept
    currPhase = currPhase + phaseInc;
}

#ifdef __SO_DBG
// Print config
void ServoOsc::printConf() {
    Serial << "Config:\n" <<
        "period: " << period <<
        "\tamplitude: " << amplitude <<
        "\toffset: " << offset <<
        "\ntrim: " << trim <<
        "\t\tphase: " << phase <<
        "\tpin: " << pin <<
        endl;
}
#endif // __SO_DBG

