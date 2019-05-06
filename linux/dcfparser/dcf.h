#ifndef DCF_H
#define DCF_H

#include <time.h>
#include <inttypes.h>

/**
	\brief Enqueues incoming data bit from DCF77 for parsing
	
	In order to clear internal buffer, call with duration set to 0.
	
	\param t Pointer to `tm` struct that shall be written upon succesful parsing
	\param state The input state (high/low)
	\param duration The duration of the input state
	\returns A non-zero value if a meaningful frame was parsed
*/
extern uint8_t dcf_push_impulse( struct tm *t, uint8_t state, uint16_t duration );

#endif
