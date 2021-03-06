#Overview
The origional idea here was to create a lufa based midi framework for use on the stock usb capable arduino and other atmega32u4 boards. At that time the arduino midi library was restrictively open (hence lufa). The other goal here was to create a minimal framework for identifying resetting and loading code onto midi devices. Since then the arduino midi library has adopted a more usable liscense and the usb stack is also midi friendly. This should be reworked and revisited.


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

This handler should also buffer sysex's that and provide hooks for passing them on if they dont belong to the device
and for handling them belong to the device but arent handled by MinSysex.

