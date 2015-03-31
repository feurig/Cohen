#Overview
The idea here is to create a lufa based midi framework for use on the stock usb capable arduino and other atmega32u4 boards. The goal here is to create a minimal framework for identifying resetting and loading code onto midi devices.


###Breakdown
The majority of the usb-midi specification is in single 4 byte events or packets. The exception to this are sysex messages which are arbitrary in length. 

Because of this the two streams are split out between packets and sysex data. Some of the USB subsystems are interrupt driven. (maple midi zb) and some like lufa are polling based. The data interface must accommodate this. I was originally trying to arrive at some glue where the lower layers can provide hooks for the maintainance tasks and simply provide an interface like this.

        void InitializeUSBMidi(void);
        bool USBMidiEventAvailable (void);
        MIDI_EVENT_PACKET_t GetUSBMidiEvent (void);
        void SendUSBMidiEvent (MIDI_EVENT_PACKET_t);
        bool USBSysexAvaliable (void);
        uint8_t * GetUSBSysex (void);
        void SendUSBSysex (uint8_t *messege);

The remaining details should be as hidden and interchangable as possible.

###MinSysex
A minimal sysex handler, called by either the maintainence tasks or by MidiEventAvailable(), should handle at a minimum the universal sysex id request and response and a (mma) vender specific sysex to jump to a bootloader. This should be expanded to identify the capabilities, architecture, and code loader method, and perhaps a subset of common midi settings (channel,mode etc.) Eventually this sysex subset will form a protocall called "Let's Get Loaded (LGL)"

This handler should also buffer sysex's that are not directed at the target.

###Hardware Abstractions.
Some of the work involved with the LUFA library is to provide feedback for the states of the the usb connection itself. This requires a hardware abstraction for each board supported by the library. In our use case this  would better be served by providing weak functions to be overridden by user functions. 




