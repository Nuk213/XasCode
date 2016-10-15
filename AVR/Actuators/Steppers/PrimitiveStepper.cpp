#include <util/delay.h>

#include "PrimitiveStepper.h"

//Step the motor once into specified direction.
void PrimitiveStepper::step(uint8_t dir) {
	if (dir == 0) {					//Rotate it backwards
		*PORT |= (1 << this->pind);	//Set the direction pin, then activate the stepping pin and deactivate both immediately
		*PORT |= (1 << this->pins);
		*PORT &= ~(1 << this->pins | 1 << this->pind);
		stepsToGo++;				//One step backwards was done; increase the amount of steps to do and decrease the total amount
		currentSteps--;
	} else {						//Rotate it forwards
		*PORT |= (1 << pins);		//Activate the step pin and immediately deactivate
		*PORT &= ~(1 << pins);
		stepsToGo--;				//Decrease the steps to do and increase total amount of steps done.
		currentSteps++;
	}
}

PrimitiveStepper::PrimitiveStepper(volatile uint8_t *P, uint8_t pins, uint8_t pind,
		uint16_t upSpeed) {
	this->PORT = P;
	this->pins = pins;
	this->pind = pind;

	this->updateFrequency = upSpeed;

	*(P - 1) |= (1 << pins | 1 << pind);
}

//Deprecated constructor for the motor stepper. Takes in port pointer, pin and ISR frequency.
PrimitiveStepper::PrimitiveStepper(volatile uint8_t *P, uint8_t pin,
		uint16_t upSpeed) {
	this->PORT = P;
	this->pins = pin;
	this->pind = pin +1;
	this->updateFrequency = upSpeed;

	*(P - 1) |= (3 << pin); 	//Set up the pins to outputs (DDRx address is one below PORTx)
}

//Default constructor for derived classes. Does nothing.
PrimitiveStepper::PrimitiveStepper() {
}

//ISR Routine for the motor, updates it when required.
void PrimitiveStepper::update() {
	if ((stepsToGo <= -1) || (stepsToGo >= 1)) {	//If there are any steps to do at all

		stepBuffer += stepSpeed;	//Increase the "Buffer step" value by the Steps/ISR value

		if (stepBuffer >= 1) {		//Is there a step to be done?
			stepBuffer -= 1;

			if (stepsToGo <= -1)		//Backwards movement
				step(0);
			else if(stepsToGo >= 1)		//Forwards movement
				step(1);
		}
	}
}

//Set the speed of the motor in steps per second.
void PrimitiveStepper::setSpeed(float stepsPerSec) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		stepSpeed = stepsPerSec / updateFrequency;	//Calculate the required steps per ISR call.
	}
}

//Move the stepper motor by the specified amount of steps.
void PrimitiveStepper::move(int32_t steps) {
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		stepsToGo += steps;
	}
}

//Wait for the motor movements to finish.
void PrimitiveStepper::flush() {
	while ((stepsToGo != 0) && (stepSpeed != 0)) {
		_delay_ms(1);
	}
}

//Reset all the values back to 0, except PORT and PIN configurations.
void PrimitiveStepper::reset() {
	stepsToGo = 0;
	currentSteps = 0;
	stepBuffer = 0;
	stepSpeed = 0;
}