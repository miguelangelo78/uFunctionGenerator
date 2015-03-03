#include <avr/io.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define F_CPU 8000000UL 
#include <util/delay.h>

#define DAC_MAX_STEPS pow(2,8)
#define DAC_SIZE 8

const int   DACVAL	 [DAC_SIZE] = {0x80>>7,	0x80>>6, 0x80>>5, 0x80>>4, 0x80>>3, 0x80>>2, 0x80>>1, 0x80};
const float DACTABLE [DAC_SIZE] = {0.01953125f, 0.0390625f, 0.078125, 0.15625f, 0.3125f, 0.625f, 1.25f, 2.5f};

#define VOUT_SET_POS  2
#define VOUT_SET_NEG  1
#define VOUT_SET_NONE 3
#define MAX_VOUT 4.0f

void init(){
	DDRB = 0xFF;
	DDRD = 0xFF;
	PORTD = VOUT_SET_NONE;
}

void write_DAC(uint8_t val){
	PORTB = val;
}

void write_DAC_precise(float volt){
	float accum = 0.0f;
	uint8_t i,j, 
			dac_ctr = 0, 
			dac_res = 0,
			dac_cand[DAC_SIZE]; // Indexes that can match the input voltage
	
	// The voltage is negative, set the circuit to output a negative voltage		
	if(volt<0.0f){
		volt*=-1;
		PORTD = VOUT_SET_NEG;
	}else PORTD = VOUT_SET_POS;
	 
	 // If this happens then the voltage is too low and there is no valid candidate (except for the 1st one):
	if(volt<DACTABLE[0]) { write_DAC(0); return; }
	// If this happens then the voltage is too high and return all the voltages combines
	if(volt>=MAX_VOUT){ write_DAC(0xFF); return; }
	
	// Fill 'dac_cand' list with numbers that fulfill the voltage required
	// And set 'dac_ctr' with how many candidates there are
	char tmp_buff[DAC_SIZE];
	for(i=0;i<DAC_MAX_STEPS;i++){
		// Loop through all combinations of the table and add their combinations into 'accum'.
		itoa(i,tmp_buff,2); // convert i to binary string
		for(j=0;j<strlen(tmp_buff);j++)
			if(tmp_buff[j]=='1'){
				accum+=DACTABLE[j];
				// fill the 'dac_cand' with the indices and set 'dac_ctr' to how many candidates there are:
				dac_cand[dac_ctr++] = j;
			}
			
		// After adding, check if the add result is higher than the voltage required
		if(accum>=volt)	break; // If it is, break out of it, since the array is already filled
		else{ // else restart values and try another combination by incrementing i
			accum=0.0f;
			dac_ctr = 0;
			for(j=0;j<DAC_SIZE;j++) dac_cand[j] = 0;
		}
	}
	
	for(i=0;i<dac_ctr;i++) dac_res |= DACVAL[dac_cand[i]];
	write_DAC(dac_res);
}

int main(void) {
    init();
	
	float i=0.0f;
	while(1) {
		
		write_DAC_precise(i);
		i+=0.1f;
		_delay_ms(1);
		if(i>=MAX_VOUT) i=0.0f;
	}
}