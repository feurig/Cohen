//
//  lgl_min_sysex.h
//  LibMaple4Midi
//
//  Created by Donald D Davis on 4/11/13.
//  Copyright (c) 2013 Suspect Devices. All rights reserved.
//
#ifndef __LGL_MIN_SYSEX_H__
#define __LGL_MIN_SYSEX_H__ 1


#include "MidiSpecs.h"
#include "USB_MIDI.h"
//#include "LGL.h"

#define ARDUINO_MMA_VENDOR_1   0x7D
#define ARDUINO_MMA_VENDOR_2   0x23
#define ARDUINO_MMA_VENDOR_3   0x41

// move to LGL.h
#define LGL_RESET_CMD           0x1e
#define LGL_BOOTLOADER_CMD      0x1f

#define DEFAULT_MIDI_CHANNEL    0x0A
#define DEFAULT_MIDI_DEVICE     0x0A
#define DEFAULT_MIDI_CABLE      0x00

#define MAX_LGL_SYSEX_SIZE ((16+1)/4)
#define MAX_SYSEX_SIZE (128)

// eventually all of this should be in a place for settings which can be written to flash.
extern volatile uint8_t myMidiChannel;
extern volatile uint8_t myMidiDevice;
extern volatile uint8_t myMidiCable;
extern volatile uint8_t myMidiID[];
extern USB_ClassInfo_MIDI_Device_t USB_MIDI_Interface;
extern volatile uint8_t mySysexBuffer[];
extern volatile uint8_t mySysexBufferIndex;



void LglSysexHandler(uint32_t  *midiBufferRx,uint32_t *rx_offset,uint32_t *n_unread_bytes);
void minSysexHandler(MIDI_EVENT_PACKET_t e);
void Jump_To_Bootloader(void);
#endif