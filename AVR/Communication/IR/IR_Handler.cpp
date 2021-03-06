/*
 * IR_Handler.cpp
 *
 *  Created on: 12.05.2016
 *      Author: xasin
 */

#include "IR_Handler.h"

namespace IR {
	IR_LED *led;

	// Function pointer to be executed upon message received
	void (*on_received)();

	// Message variables for: Received message and message length
	uint8_t mLength = 0;
	uint32_t message = 0;

	// Variables for the reading system
	uint8_t readPosition = 0;
	uint8_t readStage = 0;
	uint8_t readChecksum = 0;

	// Variables for the sending system
	uint32_t outputMessage = 0;
	uint8_t outputLength = 0;
	uint8_t outputPosition = 0;
	uint8_t outputStage = 0;
	uint8_t outputChecksum = 0;

	volatile uint8_t *PINx;
	uint8_t pin;

	void init(volatile uint8_t *PORT, uint8_t pin, IR_LED *led, void (*received_funct)()) {
		IR::led = led;

		IR::PINx = (PORT - 2);
		IR::pin = pin;

		*PORT |= (1<< pin); // Initialize Pullup

		on_received = received_funct;
	}

	bool send_8(uint8_t data) {
		if(outputLength != 0)
			return false;

		outputLength = IR_LENGTH_8;
		outputMessage = data;
		outputStage = IR_STAGE_START;

		return true;
	}

	void send() {
		switch(outputStage) {
		case IR_STAGE_IDLE:
			break;

		case IR_STAGE_START:
			led->on();
			if(outputPosition++ == IR_START_LEN) {
				led->off();

				outputStage = IR_STAGE_LENGTH;
				outputPosition = 0;
			}
		break;

		case IR_STAGE_LENGTH:
			led->on();
			if(outputPosition == outputLength) {
				led->off();

				outputChecksum = 	IR_CHECKSUM_START;

				outputPosition = 	0;
				outputStage = 		IR_STAGE_DATA;
			}
			else
				outputPosition++;
		break;

		case IR_STAGE_DATA:
			led->on();

			if((outputMessage & (1<< outputPosition)) == 0)
				led->off();
			else
				outputChecksum++;

			outputPosition++;

			if(outputPosition == (0b100 << outputLength)) { 	//Check if the output position equals the amount of data to be sent.
				outputPosition = 0;
				outputStage = IR_STAGE_CHECKSUM;
			}
		break;


		case IR_STAGE_CHECKSUM:
			led->on();
			if(outputPosition == IR_CHECKSUM_LEN) {
				led->off();

				outputLength = 0;
				outputPosition = 0;
				outputStage = IR_STAGE_IDLE;
			}
			else {
				if((outputChecksum & (1 << outputPosition)) == 0)
					led->off();

				outputPosition++;
			}
		break;
		}
	}

	void receive() {
		switch(readStage) {
		case IR_STAGE_IDLE:
			if((*IR::PINx & (1<< IR::pin)) == 0) {
				readStage = IR_STAGE_START;
				readPosition = 1;
			}
		break;

		case IR_STAGE_START:
			if((*IR::PINx & (1<< IR::pin)) == 0)
				readPosition++;
			else if(readPosition == IR_START_LEN) {
				readPosition = 0;
				readStage = IR_STAGE_LENGTH;
			}
			else {
				readStage = IR_STAGE_IDLE;
			}
		break;

		case IR_STAGE_LENGTH:
			if((*IR::PINx & (1<< IR::pin)) == 0) {
				readPosition++;
			}
			else {
				if((readPosition > 3) || (readPosition == 0))
					readStage = IR_STAGE_IDLE;

				mLength = readPosition;

				readChecksum = IR_CHECKSUM_START;

				readStage = IR_STAGE_DATA;
				readPosition = 0;
			}
		break;

		case IR_STAGE_DATA:
			if((*IR::PINx & (1<< IR::pin)) == 0) {
				message |= (1<< readPosition);
				readChecksum++;
			}
			readPosition++;

			if(readPosition == (0b100 << mLength)) {
				readStage = IR_STAGE_CHECKSUM;
				readPosition = 0;
			}
		break;

		case IR_STAGE_CHECKSUM:

			if((*IR::PINx & (1<< IR::pin)) == 0) {
				readChecksum ^= (1<< readPosition);
			}
			readPosition++;

			if(readPosition == IR_CHECKSUM_LEN) {

				if((readChecksum & ~(0b11111111 << IR_CHECKSUM_LEN)) == 0) {	//Mask all bits except for the ones transmitted via checksum
					(*on_received)();
				}


				readStage = IR_STAGE_IDLE;
				readPosition = 0;
				readChecksum = 0;
				mLength = 0;
				message = 0;
			}
		}
	}

	void update() {
		send();
		receive();
	}
}
