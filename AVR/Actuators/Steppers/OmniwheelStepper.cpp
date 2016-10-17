/*
 * OmniwheelStepper.cpp
 *
 *  Created on: 10.11.2015
 *      Author: xasin
 */

#include "OmniwheelStepper.h"

namespace X3 {

	//Constructor for a translative stepper.
	//It requires the PORT of the motors, the pins (step then direction pin)
	//The difference between ISR Frequency and Calculation Frequency
	//The microstepping of the motor (assumed that it is a standard stepper motor with 200 steps per revolution)
	//The radius of the wheel used, the rotation of the wheel relative to the X-Axis of the robot, and the distance of the wheel to the center.
	OmniwheelStepper::OmniwheelStepper(volatile uint8_t *PORT, uint8_t pins, float ISRPerCal, uint8_t microstepping, float radius, float rotation, float distance, float yRComp)
		: PrimitiveStepper(PORT, pins, 0) {
		//Save these values
		this->ISRPerCal = ISRPerCal;

		//Rotation per Y-Movement compensation value.
		this->yRCompensation = yRComp;

		//Atomic block to ensure that calculation is finished.
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
			//Calculate how many steps per mm the wheel will have to do.
			float stepsPerMM = (200 * microstepping) / (M_PI * 2 * radius);

			//Calculate the "movement factors", i.e. the conversion factor for 1mm of X or Y movement, or one degree of rotation, into steps.
			this->xFact = stepsPerMM * sin(rotation * DEG_TO_RAD);
			this->yFact = -stepsPerMM * cos(rotation * DEG_TO_RAD);
			this->rFact = -(DEG_TO_RAD * distance) * stepsPerMM;
		}
	}

	void OmniwheelStepper::stepBy(float x, float y, float r) {
		//Calculate the nececary steps to go by multiplying the axis with their according conversion value (steps to mm)
		stepsToGo += x *xFact + y *yFact + (r + y*yRCompensation) *rFact;
		stepSpeed = fabs(stepsToGo / ISRPerCal);
	}

	void OmniwheelStepper::stepBy(float x, float y) {
		//Calculate the steps to go by multiplying X and Y movement (in mm)
		//with the according factors (given in steps/MM)
		stepsToGo += x * xFact + y * yFact + (y * yRCompensation) * rFact;
		stepSpeed = fabs(stepsToGo / ISRPerCal);
	}

	void OmniwheelStepper::stepBy(float r) {
		//Calculate the steps to go by multiplying the rotation to do (in degrees)
		//with the steps / degrees precalculated value
		stepsToGo += r * rFact;
		stepSpeed = fabs(stepsToGo / ISRPerCal);
	}
}
