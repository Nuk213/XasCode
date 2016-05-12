/*
 * IR_Handler.h
 *
 *  Created on: 12.05.2016
 *      Author: xasin
 */

#ifndef LIBCODE_COMMUNICATION_IR_IR_HANDLER_H_
#define LIBCODE_COMMUNICATION_IR_IR_HANDLER_H_

#include <avr/io.h>

#include "IR_LED.h"

#define IR_LENGTH_8			1
#define IR_LENGTH_16		2
#define IR_LENGTH_32		3

#define IR_START_LEN		4

#define IR_STAGE_IDLE 		0
#define IR_STAGE_START		1
#define IR_STAGE_LENGTH		2
#define IR_STAGE_DATA		3
#define IR_STAGE_CHECKSUM	4

namespace IR {
	extern void (*on_received)();

	extern uint8_t mLen;
	extern uint32_t message;

	void init(volatile uint8_t *PORT, uint8_t pin, IR_LED *led);

	void update();
}


#endif /* LIBCODE_COMMUNICATION_IR_IR_HANDLER_H_ */