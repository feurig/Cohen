/*
             LUFA Library
     Copyright (C) Dean Camera, 2013.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2013  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the MIDI demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "USBMidiTransport.h"

/** LUFA MIDI Class driver interface configuration and state information. This structure is
 *  passed to all MIDI Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MIDI_Device_t USB_MIDI_Interface =
	{
		.Config =
			{
				.StreamingInterfaceNumber = 1,
				.DataINEndpoint           =
					{
						.Address          = MIDI_STREAM_IN_EPADDR,
						.Size             = MIDI_STREAM_EPSIZE,
						.Banks            = 1,
					},
				.DataOUTEndpoint          =
					{
						.Address          = MIDI_STREAM_OUT_EPADDR,
						.Size             = MIDI_STREAM_EPSIZE,
						.Banks            = 1,
					},
			},
	};

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */


void InitializeUSBMidi()
{
	SetupHardware();
	GlobalInterruptEnable();

}

/*
 Functional stub... finished routine should put all incoming packets into a que.
 */
volatile MIDI_EVENT_PACKET_t currentPacket;

MIDI_EVENT_PACKET_t GetUSBMidiEvent (void) { return currentPacket; }

bool USBSysexAvaliable (void){
    return ((mySysexBufferIndex) && (mySysexBuffer[mySysexBufferIndex]==MIDIv1_SYSEX_END ));
}
uint8_t * GetUSBSysex (void) {
    mySysexBuffer[mySysexBufferIndex+1]='\0';
    return mySysexBuffer;
};

void SendUSBSysex (uint8_t *messege) {
    uint8_t len=strlen(messege);
    uint8_t index=0;
    MIDI_EVENT_PACKET_t t;
    t.cable=myMidiCable;
    t.cin=CIN_SYSEX;
    t.midi0=MIDIv1_SYSEX_START;
    if (mySysexBuffer[0]==MIDIv1_SYSEX_START) {
        index++;
    }
    
    if (mySysexBuffer[len]==MIDIv1_SYSEX_END) {
        len--;
    }
    
    if (len-index<1) {
        t.cin=CIN_SYSEX_ENDS_IN_1;
        t.midi0=MIDIv1_SYSEX_END;
        t.midi1=t.midi2='\0';
    } else if (len-index<2) {
        t.cin=CIN_SYSEX_ENDS_IN_2;
        t.midi0=mySysexBuffer[index++]&0x7f;
        t.midi1=MIDIv1_SYSEX_END;
        t.midi2='\0';
    } else if (len-index<3) {
        t.cin=CIN_SYSEX_ENDS_IN_2;
        t.midi0=mySysexBuffer[index++]&0x7f;
        t.midi1=mySysexBuffer[index++]&0x7f;
        t.midi2=MIDIv1_SYSEX_END;
    } else {
        t.cin=CIN_SYSEX;
        t.midi0=mySysexBuffer[index++]&0x7f;
        t.midi1=mySysexBuffer[index++]&0x7f;
        t.midi2=mySysexBuffer[index++]&0x7f;
    }
    MIDI_Device_SendEventPacket(&USB_MIDI_Interface, (MIDI_EventPacket_t *)&t);

    while (index<len) {
        if (len-index<1) {
            t.cin=CIN_SYSEX_ENDS_IN_1;
            t.midi0=MIDIv1_SYSEX_END;
            t.midi1=t.midi2='\0';
        } else if (len-index<2) {
            t.cin=CIN_SYSEX_ENDS_IN_2;
            t.midi0=mySysexBuffer[index++]&0x7f;
            t.midi1=MIDIv1_SYSEX_END;
            t.midi2='\0';
        } else if (len-index<3) {
            t.cin=CIN_SYSEX_ENDS_IN_2;
            t.midi0=mySysexBuffer[index++]&0x7f;
            t.midi1=mySysexBuffer[index++]&0x7f;
            t.midi2=MIDIv1_SYSEX_END;
        } else {
            t.cin=CIN_SYSEX;
            t.midi0=mySysexBuffer[index++]&0x7f;
            t.midi1=mySysexBuffer[index++]&0x7f;
            t.midi2=mySysexBuffer[index++]&0x7f;
        }
        MIDI_Device_SendEventPacket(&USB_MIDI_Interface, (MIDI_EventPacket_t *)&t);

    }
    MIDI_Device_Flush(&USB_MIDI_Interface);
}

bool USBMidiEventAvailable (void)
{
    bool retVal=false;

    if (MIDI_Device_ReceiveEventPacket(&USB_MIDI_Interface, (MIDI_EventPacket_t *)(&currentPacket)))
    {
        if (CIN_IS_SYSEX(currentPacket.cin)) {
            minSysexHandler(currentPacket);
        } else {
            retVal=true;
        }
    }
    MIDI_Device_USBTask(&USB_MIDI_Interface);
    USB_USBTask();
    return retVal;
}

void SendUSBMidiEvent (MIDI_EVENT_PACKET_t t) {
    MIDI_Device_SendEventPacket(&USB_MIDI_Interface, (MIDI_EventPacket_t *) &t);
    MIDI_Device_Flush(&USB_MIDI_Interface);
}


/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
//	LEDs_Init();
	USB_Init();
}


/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{//
//	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
//	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= MIDI_Device_ConfigureEndpoints(&USB_MIDI_Interface);

//	LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	MIDI_Device_ProcessControlRequest(&USB_MIDI_Interface);
}

