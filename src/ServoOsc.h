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

#ifndef __SERVOOSC__H__
#define __SERVOOSC__H__

// Define this to add debug print to Serial
//#define __SO_DBG

#include <Arduino.h>
#include <Servo.h>
#include <math.h>

#ifdef __SO_DBG
// Allows << operator on streams. See:
//  https://gist.github.com/fitzterra/3ac5a09f83e3b116d1c0bf53e2a357bd/
#include <Streaming.h>
#endif // __SO_DBG


// This is the amount of time to wait between updates of the position towards
// the final position - in milliseconds
#define UPDATE_PERIOD 30

//-- Macro for converting from degrees to radians
#ifndef DEG2RAD
  #define DEG2RAD(g) ((g)*M_PI)/180
#endif

/**
 * Servo Oscillator class.
 **/
class ServoOsc {
private:
    Servo servo;        // The servo object
    // ### Member variables
    uint16_t period;    // Total time for one oscillation: milliseconds
    uint8_t amplitude;  // Oscillation amplitude in degrees around offset: 0 - 90
    int8_t offset;      // Offset from 0° for center of oscillation: -90 - 90
    float phase;        // Initial phase in the cycle to start.
    // NOTE: This is in radians and setPhase and the constructor will convert
    //       from degrees to radians when setting this value.
    //       The phase supplied in degrees should ideally be between
    //       (offset-amplitude/2) and (offset+amplitude/2)
    int8_t trim;        // Calibration offset: -x° - +x° from offset
    int8_t pin;         // Pin servo is connected to, -1 if not set
    
    float currPhase;    // The current phase angle (radians)
    float phaseInc;     // By how much to increment phase on every position update
    bool stopped;       // Will be true if the oscillation is stopped
    bool reversed;      // True if the cycle direction should be reversed
    bool attached=false;// True is attached, false otherwise

    float stopAt=0;     // If set, will stop oscillation when currPhase > this value
    uint32_t lastUpdate=0; // Last time the servo position was updated

    // ### Methods
    // Test if it is time for a new update
    bool shouldUpdate();
    // Calculates phaseInc from period and UPDATE_PERIOD
    void calcPhaseInc();

public:
    // Constructor
    ServoOsc(uint16_t p=2000, uint8_t a=45, int8_t o=0, int8_t ph=0,
             int8_t trim=0, int8_t pn=-1, bool atch=false);
    // This method should be called as often as possible, definately more
    // regularly than UPDATE_PERIOD, in order to update the servo position.
    void update();

    // Stop and start
    void stop() {stopped=true;};
    void start() {stopped=false;};

    // Attach to the servo if not already attached
    bool attach(int8_t p=-1);
    // Detach if attached and optionally reset pin to not set
    bool detach(bool resetPin=false);
    // Sets the servo position to a given angle, optionally taking the
    // current trim into account. The angle should be between -90 and 90.
    // Returns false if not attached, true otherwise
    bool positionServo(int8_t angle, bool withTrim=true);
    // Resets the servo to it's start position at offset, taking trim, and
    // phase into account. If attached, it will move the servo even if it is in
    // the stopped state, and will also reset the current position if it has
    // been running. If not attached, all will still be done, except moving
    // the servo.
    void resetToStart();

    // Setters
    void setPeriod(uint16_t p) {period=p; calcPhaseInc();};
    void setOffset(int8_t o) {offset=o;};
    void setAmplitude(uint8_t a) {amplitude=a;};
    void setPhase(int8_t p) {phase=DEG2RAD(p);};
    void setTrim(int8_t t) {trim=t;};
    // Sets the pin to attach to if not currently attached. If the attach arg
    // is true (the default), it will also immediatly attach the servo.
    // Returns true if the pin was set, false if invalid pin or already
    // attached.
    bool setPin(int8_t p, bool attach=true);
    void setReverse(bool r) {reversed=r;};
    // Sets a max number of cycles (can be fractional, i.e. 0.75 cycles) to run
    // before stopping automatically.
    void setCycles(float c) {stopAt = currPhase + 2*M_PI*c;};
    // Getters
    uint16_t getPeriod() {return period;};
    uint8_t getAmplitude() {return amplitude;};
    int8_t getOffset() {return offset;};
    float getPhase() {return phase;};
    float getPhaseStop() {return stopAt;};
    int8_t getTrim() {return trim;};
    int8_t getPin() {return pin;};
    bool getReverse() {return reversed;};
    // These are more for diagnostics and debugging
    float getPhaseInc() {return phaseInc;};
    float getCurrPhase() {return currPhase;};

    // Info
    bool isAttached() {return attached;};
    bool isStopped() {return stopped;};

#ifdef __SO_DBG
    // Prints the current config out to the serial port - for debugging
    void printConf();
#endif // __SO_DBG
};

#endif  // __SERVOOSC__H__
