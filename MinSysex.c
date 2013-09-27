//
//  MinSysex.c
//  LibMaple4Midi
//
//  Created by Donald D Davis on 4/11/13.
//  Copyright (c) 2013 Suspect Devices. All rights reserved.
//  Modified BSD Liscense 
/*
 0xF0  SysEx
 0x7E  Non-Realtime
 0x7F  The SysEx channel. Could be from 0x00 to 0x7F.
 Here we set it to "disregard channel".
 0x06  Sub-ID -- General Information
 0x01  Sub-ID2 -- Identity Request
 0xF7  End of SysEx
---- response
 0xF0  SysEx
 0x7E  Non-Realtime
 0x7F  The SysEx channel. Could be from 0x00 to 0x7F.
 Here we set it to "disregard channel".
 0x06  Sub-ID -- General Information
 0x02  Sub-ID2 -- Identity Reply
 0xID  Manufacturer's ID
 0xf1  The f1 and f2 bytes make up the family code. Each
 0xf2  manufacturer assigns different family codes to his products.
 0xp1  The p1 and p2 bytes make up the model number. Each
 0xp2  manufacturer assigns different model numbers to his products.
 0xv1  The v1, v2, v3 and v4 bytes make up the version number.
 0xv2
 0xv3
 0xv4
 0xF7  End of SysEx
*/
#define STANDARD_ID_RESPONSE_LENGTH 6

//#include <libmaple/usb_midi_device.h>
//#include <libmaple/nvic.h>
//#include <libmaple/delay.h>
#include <MinSysex.h>
//#include <wirish/wirish.h>
#include <avr/wdt.h>
#include <util/delay.h>

/********************************* ACHTUNG! ignores usbmidi cable ********************************/
const MIDI_EVENT_PACKET_t standardIDResponse[]={
    {   CIN_SYSEX,
        DEFAULT_MIDI_CABLE,
        MIDIv1_SYSEX_START,
        USYSEX_NON_REAL_TIME,
        USYSEX_ALL_CHANNELS},
    {   CIN_SYSEX,
        DEFAULT_MIDI_CABLE,
        USYSEX_GENERAL_INFO,
        USYSEX_GI_ID_RESPONSE,
        ARDUINO_MMA_VENDOR_1},
    {   CIN_SYSEX,
        DEFAULT_MIDI_CABLE,
        ARDUINO_MMA_VENDOR_2, // extended ID
        ARDUINO_MMA_VENDOR_3, // extended ID
        1}, // family #1
    {   CIN_SYSEX,
        DEFAULT_MIDI_CABLE,
        2, // family #2
        1, // part   #1
        2}, // part   #2
    {   CIN_SYSEX,
        DEFAULT_MIDI_CABLE,
        0, // version 1
        0, // version 2
        1}, // version 3
    {   CIN_SYSEX_ENDS_IN_2,
        DEFAULT_MIDI_CABLE,
        '!', // lgl compatible
        MIDIv1_SYSEX_END,
        0}
};

typedef enum  {NOT_IN_SYSEX=0,COULD_BE_MY_SYSEX,YUP_ITS_MY_SYSEX,ITS_NOT_MY_SYSEX} sysexStates;
volatile MIDI_EVENT_PACKET_t sysexBuffer[MAX_LGL_SYSEX_SIZE];
volatile sysexStates sysexState;
volatile int sysexFinger=0;

/* 
 0xF0  SysEx
 0x??  ARDUINO_MMA_VENDOR_1
 0x??  ARDUINO_MMA_VENDOR_2

 0x??  ARDUINO_MMA_VENDOR_3
 0x10  LGL_DEVICE_NUMBER
 0xLE  CMD: REBOOT

 0xf7  EOSysEx
*/
#define STACK_TOP 0x20000800
#define EXC_RETURN 0xFFFFFFF9
#define DEFAULT_CPSR 0x61000000
#define RESET_DELAY 100000

void dealWithItQuickly(void);
/* -----------------------------------------------------------------------------dealWithItQuickly()
 * Note: at this point we have established that the sysex belongs to us.
 * So we need to respond to any generic requests like information requests.
 * We also need to handle requests which are meant for us. At the moment this is just the 
 * reset request. 
 * 
 */
void dealWithItQuickly(){
    LEDs_ToggleLEDs(LEDS_LED2);
    switch (sysexBuffer[0].midi1) {

        case USYSEX_NON_REAL_TIME:
            switch (sysexBuffer[1].midi0) {
                case USYSEX_GENERAL_INFO:
                    if (sysexBuffer[1].midi1==USYSEX_GI_ID_REQUEST) {
                        for (uint8_t i=0;i<STANDARD_ID_RESPONSE_LENGTH;i++)
                            MIDI_Device_SendEventPacket(&USB_MIDI_Interface, (MIDI_EventPacket_t *)standardIDResponse+i);
                        MIDI_Device_Flush(&USB_MIDI_Interface);
                    }
                    
                    break;
            }
        case USYSEX_REAL_TIME:
            break;
        case ARDUINO_MMA_VENDOR_1:
            if (sysexBuffer[1].midi2==LGL_RESET_CMD) {
                Jump_To_Bootloader();
 
            }
    
        default:
            // userland sysex copy the sysex to the buffer if its not handled by us....
            mySysexBufferIndex=0;
            for (uint8_t i=0; i<sysexFinger; i++) {
                mySysexBuffer[mySysexBufferIndex++]=sysexBuffer[i].midi0;
                if ((sysexBuffer[i].cin == CIN_SYSEX_ENDS_IN_1)
                    ||(sysexBuffer[i].cin == CIN_1BYTE)
                    ) continue;
                mySysexBuffer[mySysexBufferIndex++]=sysexBuffer[i].midi1;
                if ((sysexBuffer[i].cin == CIN_SYSEX_ENDS_IN_2)
                    ||(sysexBuffer[i].cin == CIN_PROGRAM_CHANGE)
                    ||(sysexBuffer[i].cin == CIN_CHANNEL_PRESSURE)
                    ||(sysexBuffer[i].cin == CIN_2BYTE_SYS_COMMON)
                    ) continue;
                mySysexBuffer[mySysexBufferIndex++]=sysexBuffer[i].midi2;
            }
            
            break;
    }
    ;//turn the led on?
}

/* -----------------------------------------------------------------------------LglSysexHandler()
 * The idea here is to identify which Sysex's belong to us and deal with them.
 */
void minSysexHandler(MIDI_EVENT_PACKET_t e) {
    
    if (!CIN_IS_SYSEX(e.cin)) {
        return;
    }
    
    if ((sysexState==YUP_ITS_MY_SYSEX) && (sysexFinger>MAX_SYSEX_SIZE)){
        sysexState=ITS_NOT_MY_SYSEX;
        // move the data here to the sysex buffer....
    }

    switch (e.cin) {
        case CIN_SYSEX: // sysex starts or continues.
            switch (sysexState) {
                case NOT_IN_SYSEX : // new sysex.
                    sysexFinger=0;
                    if (e.midi0 == MIDIv1_SYSEX_START) {
                        if (e.midi1==USYSEX_REAL_TIME
                            ||e.midi1==USYSEX_NON_REAL_TIME) {
                            if ((e.midi2==myMidiChannel)
                                ||(e.midi2==USYSEX_ALL_CHANNELS)
                                ) {
                                sysexState=YUP_ITS_MY_SYSEX;
                                sysexBuffer[sysexFinger++]=e;
                                                            }
                        } else if ((e.midi1==myMidiID[0])
                                   && (e.midi2==myMidiID[1])
                                   ){
                            
                            sysexState=COULD_BE_MY_SYSEX;
                            sysexBuffer[sysexFinger++]=e;
                        }
                    }
                    break;
                case COULD_BE_MY_SYSEX:
                    if (e.midi0==myMidiID[2]) {
                        sysexState=YUP_ITS_MY_SYSEX;
                        sysexBuffer[sysexFinger++]=e;
                    } else {
                        sysexState=ITS_NOT_MY_SYSEX;
                        sysexBuffer[sysexFinger++]=e;
                        //move previous data to sysex
                        mySysexBufferIndex=0;
                        for (uint8_t i=0; i<sysexFinger; i++) {
                            mySysexBuffer[mySysexBufferIndex++]=sysexBuffer[i].midi0;
                            if ((sysexBuffer[i].cin == CIN_SYSEX_ENDS_IN_1)
                                ||(sysexBuffer[i].cin == CIN_1BYTE)
                                ) continue;
                            mySysexBuffer[mySysexBufferIndex++]=sysexBuffer[i].midi1;
                            if ((sysexBuffer[i].cin == CIN_SYSEX_ENDS_IN_2)
                                ||(sysexBuffer[i].cin == CIN_PROGRAM_CHANGE)
                                ||(sysexBuffer[i].cin == CIN_CHANNEL_PRESSURE)
                                ||(sysexBuffer[i].cin == CIN_2BYTE_SYS_COMMON)
                                ) continue;
                            mySysexBuffer[mySysexBufferIndex++]=sysexBuffer[i].midi2;
                        }
                        
                    }
                    break;
                case YUP_ITS_MY_SYSEX:
                    sysexBuffer[sysexFinger++]=e;
                    break;
                case ITS_NOT_MY_SYSEX:
                    // if the buffer is full DTMFA
                    if ((mySysexBufferIndex+3)<MAX_SYSEX_SIZE) {
                        mySysexBufferIndex=0;
                        sysexState=NOT_IN_SYSEX;
                    } else {
                        mySysexBuffer[mySysexBufferIndex++]=e.midi0;
                        mySysexBuffer[mySysexBufferIndex++]=e.midi1;
                        mySysexBuffer[mySysexBufferIndex++]=e.midi2;
                     }
                    
                default:
                    break;                    
            }            
            break;
        case CIN_SYSEX_ENDS_IN_1:
        case CIN_SYSEX_ENDS_IN_2:
        case CIN_SYSEX_ENDS_IN_3:
            sysexBuffer[sysexFinger++]=e;
            if (sysexState==YUP_ITS_MY_SYSEX) {
                dealWithItQuickly(); // its our sysex and we will cry if we want to
                LEDs_ToggleLEDs(LEDS_LED2);
            } else {
                mySysexBuffer[mySysexBufferIndex++]=e.midi0;
                if (e.cin != CIN_SYSEX_ENDS_IN_1) {
                    mySysexBuffer[mySysexBufferIndex++]=e.midi1;
                } else if (e.cin == CIN_SYSEX_ENDS_IN_3) {
                    mySysexBuffer[mySysexBufferIndex++]=e.midi2;
                }
            }
            
            //move sysex data to buffer...
            sysexState=NOT_IN_SYSEX;
            break;
        default:
            return;
    }
    return;
}

void Jump_To_Bootloader(void)
{
    // If USB is used, detach from the bus and reset it
    USB_Disable(); // Disable all interrupts
    cli();
    // Wait for the USB detachment to register on the host
    Delay_MS(500);
    *(uint16_t *)0x0800 = 0x7777;
    wdt_enable(WDTO_250MS);
    for (;;);
}

void Software_Reset(void)
{
    // If USB is used, detach from the bus and reset it
    USB_Disable(); // Disable all interrupts
    cli();
    // Wait for the USB detachment to register on the host
    Delay_MS(500);
    *(uint16_t *)0x0800 = 0;
    wdt_enable(WDTO_250MS);
    for (;;);
}