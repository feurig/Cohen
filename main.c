/*
             

 */

#include "USBMidiTransport.h"
#include <util/delay.h>

/** LUFA MIDI Class driver interface configuration and state information. This structure is
 *  passed to all MIDI Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */

volatile uint8_t myMidiChannel=DEFAULT_MIDI_CHANNEL;
volatile uint8_t myMidiDevice=DEFAULT_MIDI_DEVICE;
volatile uint8_t myMidiCable=DEFAULT_MIDI_CABLE;
volatile uint8_t myMidiID[]={ARDUINO_MMA_VENDOR_1 ,ARDUINO_MMA_VENDOR_2,ARDUINO_MMA_VENDOR_3};
volatile uint8_t mySysexBuffer[MAX_SYSEX_SIZE];
volatile uint8_t mySysexBufferIndex;

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
void initBlinkingLights(void);
void initBlinkingLights(void){
//    DDRB=0xff;
    PORTB=0x0f;
    _delay_ms(1000) ;
    PORTB=0xf0;
    _delay_ms(1500) ;
    PORTB=0x0f;
    _delay_ms(1000) ;
    PORTB=0xcc;
    _delay_ms(1500) ;
    PORTB=0xff;
    _delay_ms(1000) ;
    PORTB=0x00;
    _delay_ms(1500) ;
    
    
}

int main(void)
{     DDRB=0xff;
InitializeUSBMidi();
    
    InitializeUSBMidi();
    
	for (;;)
	{       initBlinkingLights();

		USBMidiEventAvailable();
	}
}

