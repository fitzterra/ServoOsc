/**
 * Interactive demo for using the ServoOsc library.
 *
 * (c) Tom Coetser (Fitzterra), May 2018
 * GPL License
 *
 * The demo requires the serial command parser by Pedro Tiago Pereira available
 * here: https://github.com/ppedro74/Arduino-SerialCommands.git
 * or directy installable from the Arduino IDE. This is a very conveninet
 * library for creating a serial command protocol to control things.
 *
 * It also requires the Streaming.h lib by Mikal Hart that makes it much easier
 * to write to a serial stream using the '<<' operator. This library is not
 * available via the Arduino IDE, but I have made it available here: 
 * https://gist.github.com/fitzterra/3ac5a09f83e3b116d1c0bf53e2a357bd/
 *
 * Connect one or more servos to any digital pin (except D0), upload the sketch
 * and open a serial monitor at the default 115200 baud (adjust this in setup()
 * if needed).
 * From the serial monitor send a question mark '?' followed by CRLF (enter
 * key, or set the Arduino serial monitor to send CRLF) to get a menu.
 * From here you can attach to any of the connected servos (commands takes
 * spaces between arguments and is terminated by CRLF)
 *
 * Terminology
 * ===========
 * Oscillation: Continues rotation of the servo from one end to the opposite
 *     around the central offset. The full oscillation angle possible is 180°
 *     (servo dependant) which is virtually mapped from -90° to +90° with 0°
 *     being the center of oscillation.
 * Offset: This is the offset from 0° to set as the center of oscillation and
 *     can be positive or negative. Note that any offset added, reduces the
 *     180° arc of movement by the offset amount.
 * Trim: Some servos do not have a level of precision and setting the offset to
 *     0° for example, does not leave the horn exactly center. The trim can be
 *     used to adjust this position in 1° increments, positive or negative - in
 *     future, the granularity of this adjustment may be made smaller to allow
 *     fractions of a degree.
 * Amplitude: This is the degrees of movement to travel from the offset in both
 *     positive and negative direction. With an offset of 0°, the max amplitude
 *     is 90° which will result in a full oscillation arc of 180° (-90° to
 *     +90°). Note that the amplitude can be greater than 90°, but the
 *     underlying servo library or the servo itself will ignore angles outside
 *     it's range. In this case, then the movement reaches the max servo
 *     position, it would stop, the oscillation calculation would continue with
 *     further invalid angles sent to the serve until the oscillation max is
 *     reached and movement direction changes. Only once the angle comes back
 *     into range, will the servo start moving back toward the offset position.
 * Period: The amount of time in milliseconds, that one full oscillation cycle
 *     takes. Note that one full cycle is 4 * the amplitude.
 * Phase: This is the position within the cycle where the movement starts. This
 *     is mostly useful when there are two or more servos, oscillating at the
 *     same time with the same amplitude and period, but one or more needs to
 *     start at a different positions within the cycle if they are all started
 *     together.
 *
 **/
#include <SerialCommands.h>
#include <Streaming.h>
#include "ServoOsc.h"

//-- Oscillator with default parameters
ServoOsc *osc;

//-- Serial command buffer, handler and command callbacks
#define BUFFLEN 10
char cmdBuff[BUFFLEN];
// Set up a serial command parser on the Serial stream, using cmdBuff as the
// command buffer, expecting '\r' as command terminator and a space as
// command/args seperator.
SerialCommands cmdParser(&Serial, cmdBuff, BUFFLEN, "\r\n", " ");

/**
 * Serial Command Default Handler.
 *
 * This function gets called when no other command matches.
 */
void cInvalid(SerialCommands* sender, const char* cmd) {
	*(sender->GetSerial()) << F("ERR: ") << F("Unrecognized command [") << cmd
                           << F("]\nTry ? for help.\n");
}

/**
 * Serial Command Handler to show help menu
 */
void cHelp(SerialCommands *sender) {
    Serial << "\n" <<
    F("Available commands:\n") <<
    F("?\t- Show this help\n") <<
    F("??\t- Show status\n") <<
    F("n pn [a]- Set pin. Optionally also attach.\n") <<
    F("a [pn]\t- Attach servo. Pin is optional if not set yet.\n") <<
    F("d [r]\t- Dettach servo. Optionally reset pin.\n") <<
    F("A ampl\t- Set amplitude (0 - 180 degrees)\n") << 
    F("O offs\t- Set offset (-90 - +90 degrees)\n") << 
    F("P prd\t- Set period in milliseconds)\n") << 
    F("p phase\t- Set starting phase (-90 - +90 degrees)\n") << 
    F("t trim\t- Set trim angle (-90 - +90 degrees)\n") << 
    F("c cycl\t- Set the number of cycles (can be fract i.e. 0.75) to run\n") << 
    F("m\t- Reverse/mirror the angle/direction\n") << 
    F("r\t- Run oscillation if stopped\n") <<
    F("s\t- Stop oscillation if playing/running\n") <<
    F("S a [0]\t- Direct set servo angle (-90 - +90), optional do not adjust for trim\n") <<
    F("R\t- Reset servo to start pos (incl. offset, trim, phase)\n");
}
// Setup callback
SerialCommand cHelp_cb("?", cHelp);

/**
 * Serial Command Handler to show current status
 */
void cStatus(SerialCommands *sender) {
    Serial << "\n" <<
        F("Status:\n") <<
        F("Pin: ") << osc->getPin() <<
        F("   \tAttached: ") << (osc->isAttached() ? F("Yes") : F("No")) <<
        F("\nStopped: ") << (osc->isStopped() ? F("Yes") : F("No")) <<
        F("\tAmplitude: ") << osc->getAmplitude() << F("\nOffset: ") << osc->getOffset() <<
        F("\tPeriod: ") << osc->getPeriod() << F("\nTrim: ") << osc->getTrim() <<
        F(" \tPhase: ") << osc->getPhase() <<
        F("\nPhase inc: ") << osc->getPhaseInc() << F("\tCurr phase: ") << osc->getCurrPhase() << endl <<
        F("Phse stop: ") << osc->getPhaseStop() << endl <<
        endl;
}
// Setup callback
SerialCommand cStatus_cb("??", cStatus);

/**
 * Serial Command Handler to set the oscillator servo pin and optionally attach.
 *
 * Command: "n pin [a]"
 *
 * where pin is the pin number to set and if a is supplied also attach. 
 */
void cSetPin(SerialCommands* sender) {
    // Can only attach if detached now
    if(osc->isAttached()) {
		*(sender->GetSerial()) << F("ERR: ") << F("already attached to pin ")
                               << osc->getPin() << endl;
        return;
    }
    // Get a pointer to the pin argument string
	char* argP = sender->Next();
	if (argP == NULL) {
		*(sender->GetSerial()) << F("ERR: ") << F("pin number expected.\n");
		return;
	}
    // Convert the pin string to an integer
	int pin = atoi(argP);

    // See if we have to attach
    bool attach = false;
	argP = sender->Next();
	if (argP != NULL) {
        if (argP[0] == 'a')
            attach = true;
        else {
            *(sender->GetSerial()) << F("ERR: ") << F("Invalid command. Try ?.\n");
            return;
        }
    }
    // Set pin
    bool res = osc->setPin(pin, attach);
    if(!res)
        *(sender->GetSerial()) << F("Error setting pin to ") << pin << endl;
    else
        *(sender->GetSerial()) << F("Servo pin set to ") << osc->getPin() << endl;
}
// Setup callback
SerialCommand cSetPin_cb("n", cSetPin);

/**
 * Serial Command Handler to attach the oscillator servo, optionally supplying
 * a pin to attach to if a pin has not been set yet.
 *
 * Command: "a [pin]"
 *
 * where pin, if supplied, is the pin to attach to
 */
void cAttach(SerialCommands* sender) {
    // Can only attach if detached now
    if(osc->isAttached()) {
		*(sender->GetSerial()) << F("ERR: ") << F("already attached to pin ")
                               << osc->getPin() << endl;
        return;
    }
    // Preset that we will not supply a pin
    int pin = -1;
    // Get a pointer to the pin argument string
	char* argP = sender->Next();
	if (argP != NULL) {
        // Convert the pin string to an integer
        pin = atoi(argP);
	}
    // Attach
    bool res = osc->attach(pin);
    if(!res)
        *(sender->GetSerial()) << F("Error attaching to servo. Pin set?") << endl;
    else
        *(sender->GetSerial()) << F("Servo attached to pin ") << osc->getPin() << endl;
}
// Setup callback
SerialCommand cAttach_cb("a", cAttach);

/**
 * Serial Command Handler to detach the oscillator servo.
 *
 * Command: "d"
 */
void cDetachPin(SerialCommands* sender) {
    // Can only detach if attached now
    if(!osc->isAttached()) {
		*(sender->GetSerial()) << F("ERR: ") << F("not attached\n");
        return;
    }
    // See if we have to reset the pin
    bool rst = false;
	char *argP = sender->Next();
	if (argP != NULL) {
        if (argP[0] == 'r')
            rst = true;
        else {
            *(sender->GetSerial()) << F("ERR: ") << F("Invalid command. Try ?.\n");
            return;
        }
    }
    // Detach
    osc->detach(rst);
    *(sender->GetSerial()) << F("Servo detached\n");
}
// Setup callback
SerialCommand cDetachPin_cb("d", cDetachPin);

/**
 * Serial Command Handler to set the oscillation amplitude.
 *
 * Command: "A amplitude"
 *
 * where amplitude is in degrees from 0 to 180
 */
void cAmplitude(SerialCommands* sender) {
    // Get a pointer to the amplitude argument string
	char* argP = sender->Next();
	if (argP == NULL) {
		*(sender->GetSerial()) << F("ERR: ") << F("amplitude expected.\n");
		return;
	}
    // Convert the amplitude string to an integer
	int amplitude = atoi(argP);
	if (amplitude<0 || amplitude>180) {
		*(sender->GetSerial()) << F("ERR: ") << F("amplitude must be >=0 and <=180.\n");
		return;
	}
    // Attach
    osc->setAmplitude(amplitude);
    *(sender->GetSerial()) << F("Oscillation amplitude set to ") << amplitude << endl;

}
// Setup callback
SerialCommand cAmplitude_cb("A", cAmplitude);

/**
 * Serial Command Handler to set the oscillation offset.
 *
 * Command: "O angle"
 *
 * where angle is in degrees from -90 to 90
 */
void cOffset(SerialCommands* sender) {
    // Get a pointer to the offset argument string
	char* argP = sender->Next();
	if (argP == NULL) {
		*(sender->GetSerial()) << F("ERR: ") << F("offset expected.\n");
		return;
	}
    // Convert the offset string to an integer
	int offset = atoi(argP);
	if (offset<-90 || offset>90) {
		*(sender->GetSerial()) << F("ERR: ") << F("offset must be >=-90 and <=90.\n");
		return;
	}
    // Attach
    osc->setOffset(offset);
    *(sender->GetSerial()) << F("Oscillation offset set to ") << offset << endl;

}
// Setup callback
SerialCommand cOffset_cb("O", cOffset);

/**
 * Serial Command Handler to set the oscillation period.
 *
 * Command: "P period"
 *
 * where period is in milliseconds
 */
void cPeriod(SerialCommands* sender) {
    // Get a pointer to the period argument string
	char* argP = sender->Next();
	if (argP == NULL) {
		*(sender->GetSerial()) << F("ERR: ") << F("period expected.\n");
		return;
	}
    // Convert the period string to an integer
	long period = atoi(argP);
	if (period<0) {
		*(sender->GetSerial()) << F("ERR: ") << F("period must be >0.\n");
		return;
	}
    // Attach
    osc->setPeriod(period);
    *(sender->GetSerial()) << F("Oscillation period set to ") << period << endl;
}
// Setup callback
SerialCommand cPeriod_cb("P", cPeriod);

/**
 * Serial Command Handler to set the oscillation phase.
 *
 * Command: "p phase"
 *
 * where phase is between -90° and 90° ?????? is it not 0 - 180??
 */
void cPhase(SerialCommands* sender) {
    // Get a pointer to the phase argument string
	char* argP = sender->Next();
	if (argP == NULL) {
		*(sender->GetSerial()) << F("ERR: ") << F("phase expected.\n");
		return;
	}
    // Convert the phase string to an integer
	int phase = atoi(argP);
	if (phase<-90 || phase>90) {
		*(sender->GetSerial()) << F("ERR: ") << F("phase must be >=-90 and <=90.\n");
		return;
	}
    // Attach
    osc->setPhase(phase);
    *(sender->GetSerial()) << F("Oscillation phase set to ") << phase << endl;
}
// Setup callback
SerialCommand cPhase_cb("p", cPhase);

/**
 * Serial Command Handler to set the trim correction.
 *
 * Command: "t trim"
 *
 * where trim is between -90° and 90°
 */
void cTrim(SerialCommands* sender) {
    // Get a pointer to the trim argument string
	char* argP = sender->Next();
	if (argP == NULL) {
		*(sender->GetSerial()) << F("ERR: ") << F("trim expected.\n");
		return;
	}
    // Convert the trim string to an integer
	int trim = atoi(argP);
	if (trim<-90 || trim>90) {
		*(sender->GetSerial()) << F("ERR: ") << F("trim must be >=-90 and <=90.\n");
		return;
	}
    // Set the trim
    osc->setTrim(trim);
    *(sender->GetSerial()) << F("Trim correction set to ") << trim << endl;
}
// Setup callback
SerialCommand cTrim_cb("t", cTrim);

/**
 * Serial Command Handler to set the number of cycles to run.
 *
 * Command: "t cycl"
 *
 * where cycl is a positive, optional fractional value
 */
void cCycles(SerialCommands* sender) {
    // Get a pointer to the cycle argument string
	char* argP = sender->Next();
	if (argP == NULL) {
		*(sender->GetSerial()) << F("ERR: ") << F("cycles expected.\n");
		return;
	}
    // Convert the cycles string to a float
	float cycle = atof(argP);
	if (cycle<0.0) {
		*(sender->GetSerial()) << F("ERR: ") << F("cycle must be >=0.0\n");
		return;
	}
    // Set the cycles
    osc->setCycles(cycle);
    *(sender->GetSerial()) << F("Cycles set to ") << cycle << endl;
}
// Setup callback
SerialCommand cCycles_cb("c", cCycles);

/**
 * Serial Command Handler to reverse the servo angle
 *
 * Command: "m"
 */
void cReverse(SerialCommands* sender) {
    // Toggle the reverse state
    osc->setReverse(!osc->getReverse());
    *(sender->GetSerial()) << F("Servo direction reversed\n");

}
// Setup callback
SerialCommand cReverse_cb("m", cReverse);

/**
 * Serial Command Handler to run the oscillator servo if stopped.
 *
 * Command: "r"
 */
void cRun(SerialCommands* sender) {
    // Can only run if stopped now
    if(!osc->isStopped()) {
		*(sender->GetSerial()) << F("ERR: ") << F("not running.\n");
        return;
    }
    // Detach
    osc->start();
    *(sender->GetSerial()) << F("Servo running\n");

}
// Setup callback
SerialCommand cRun_cb("r", cRun);

/**
 * Serial Command Handler to stop the oscillator servo if running.
 *
 * Command: "s"
 */
void cStop(SerialCommands* sender) {
    // Can only stopp if running now
    if(osc->isStopped()) {
		*(sender->GetSerial()) << F("ERR: ") << F("already stopped.\n");
        return;
    }
    // Detach
    osc->stop();
    *(sender->GetSerial()) << F("Servo stopped\n");

}
// Setup callback
SerialCommand cStop_cb("s", cStop);

/**
 * Serial Command Handler to set the servo angle directly
 *
 * Command: "S angle [0/1]"
 *
 * where angle is between -90° and 90° and optioanlly followed by 0 (do not
 * take trim into account) or 1 (adjust for trim)
 */
void cPositionServo(SerialCommands* sender) {
    // Get a pointer to the angle argument string
	char* argP = sender->Next();
	if (argP == NULL) {
		*(sender->GetSerial()) << F("ERR: ") << F("angle expected.\n");
		return;
	}
    // Convert the angle string to an integer
	int angle = atoi(argP);
	if (angle<-90 || angle>90) {
		*(sender->GetSerial()) << F("ERR: ") << F("angle must be >=-90 and <=90.\n");
		return;
	}
    // See if we got the trim adjust arg too
    bool trimAdjust = true;
	argP = sender->Next();
	if (argP != NULL) {
        switch (argP[0]) {
            case '0':
                trimAdjust = false;
                break;
            case '1':
                trimAdjust = true;
            default:
                *(sender->GetSerial()) << F("ERR: ") << F("trim adjust must be 0 or 1.\n");
                return;
        }
    }
    // set position
    bool res = osc->positionServo(angle, trimAdjust);
    if(!res) {
        *(sender->GetSerial()) << F("ERR: ") << F("Setting position failed. Attached?\n");
    } else {
        *(sender->GetSerial()) << F("Servo positioned to ") << angle << F(" with") << (trimAdjust ? F(" ") : F("out ")) << F("trim adjustment.") << endl;
    }
}
// Setup callback
SerialCommand cPositionServo_cb("S", cPositionServo);

/**
 * Serial Command Handler to reset the servo to it's start position
 *
 * Command: "R"
 */
void cResetServo(SerialCommands* sender) {
    osc->resetToStart();
    *(sender->GetSerial()) << F("Servo reset.\n");
}
// Setup callback
SerialCommand cResetServo_cb("R", cResetServo);


void setup() {
    Serial.begin(115200);
    Serial << "ready... Enter ? for help\n";

    osc = new ServoOsc();

    // Set the default command handler, and add the callback commands
    cmdParser.SetDefaultHandler(cInvalid);
    cmdParser.AddCommand(&cHelp_cb);
    cmdParser.AddCommand(&cStatus_cb);
    cmdParser.AddCommand(&cSetPin_cb);
    cmdParser.AddCommand(&cAttach_cb);
    cmdParser.AddCommand(&cDetachPin_cb);
    cmdParser.AddCommand(&cAmplitude_cb);
    cmdParser.AddCommand(&cOffset_cb);
    cmdParser.AddCommand(&cPeriod_cb);
    cmdParser.AddCommand(&cPhase_cb);
    cmdParser.AddCommand(&cTrim_cb);
    cmdParser.AddCommand(&cCycles_cb);
    cmdParser.AddCommand(&cReverse_cb);
    cmdParser.AddCommand(&cRun_cb);
    cmdParser.AddCommand(&cStop_cb);
    cmdParser.AddCommand(&cPositionServo_cb);
    cmdParser.AddCommand(&cResetServo_cb);
}

void loop() {
    cmdParser.ReadSerial();

    if(osc->isAttached())
        osc->update();
}


