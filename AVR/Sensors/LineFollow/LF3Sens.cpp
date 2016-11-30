/*
 * LF3Sens.cpp
 *
 *  Created on: Jan 3, 2016
 *      Author: xasin
 */

#include "LF3Sens.h"

namespace LF {

Sens3::Sens3(volatile uint8_t *PINx, uint8_t pins) {
	this->PINx = PINx;
	this->pins = pins;
}

//Decide on the next status of the outputs
void Sens3::setStatus(uint8_t sensors) {
	switch(sensors) {
	case 0:
		this->lineStatus = LOST;
		if(lineOffset == LF_LEFT * 2 / 3) 
			lineOffset = LF_LEFT;
		else if(lineOffset == LF_RIGHT * 2 / 3) 
			lineOffset = LF_RIGHT;
	break;

	case 0b100:
		this->lineOffset = LF_LEFT * 2 / 3;
	break;

	case 0b110:
		this->lineStatus = OK;
		this->lineOffset = LF_LEFT / 3;
	break;

	case 0b010:
		this->lineStatus = OK;
		this->lineOffset = 0;
	break;

	case 0b011:
		this->lineStatus = OK;
		this->lineOffset = LF_RIGHT / 3;
	break;

	case 0b001:
		this->lineOffset = LF_RIGHT * 2 / 3;
	break;

	case 0b111:
		this->lineStatus = INTSEC;
		if(this->lineOffset == 0)
			this->lineOffset = INTSEC_DEFAULT_DIR;
	break;

	default:
		this->lineStatus = AMBIGUOUS;
	break;
	}
}

//Update the output values
void Sens3::update() {

	//The sensor outputs give "0" when they are on a line, thusly the inputs have to be inverted
	uint8_t sensors = ~((*this->PINx >> this->pins) | 0b11111000);

	//Check if the sensors have changed. If so, wait for DELAY_VAL Cycles (Debuffing)
	if(sensors != oSens) {
		oSens = sensors;
		uDelay = DELAY_VAL;
	}
	//If the sensors have not changed for a while, update them.
	else if(uDelay == 0)
		setStatus(sensors);
	//Count down.
	else
		uDelay--;
}

}
