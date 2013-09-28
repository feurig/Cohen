#Overview
The idea here is to create a lufa based midi framework for use on the stock usb capable arduino. The goal is twofold:

1. to isolate the lower level usb functions from the actual programs

    a. Targeted transports
        1. Lufa
        2. Maple Midi
        3. A neutered version of pjrc's Arduino midi
        4. Microchip's midi code.
        5. Port of MCs midi client to pic32 (dip)
        5. A port of jboones nuvoton cdc code to midi.
        6. YOUR CONTRIBUTION HERE...
    b. Targeted Libraries
        1. Arduino midi v1 (gpl2)
        2. Arruino midi v2 (gpl3)
        3. alex normans stack (gpl3)
        4. A bsd liscensed framework soon to be released.
        5. YOUR LIBRARY HERE….
      

2. To create a minimal framework for identifying resetting and loadib3. code onto midi devices.


###Breakdown
The majority of the usb-midi specification is in single 4 byte events or packets. The exception to this are sysex messages which are arbitrary in length. 

Because of this the two streams are split out between packets and sysex data. Some of the USB subsystems are interrupt driven. (maple midi zb) and some like lufa are polling based. The data interface must accommodate this. I am trying to arrive at some glue where the lower layers can provide hooks for the maintainance tasks and simply provide an interface like this.

    bool MidiEventAvailable ([interface])
    void GetMidiEvent ([interface])
    void SendMidiEvent ([interface])
    bool SysexAvaliable ([intervace])
    void GetSysex ([interface])
    void SendSysex (uint8_t *messege, [interface])

The remaining details should be as hidden and interchangable [^1] as possible.

###MinSysex
A minimal sysex handler, called by either the maintainence tasks or by MidiEventAvailable(), should handle at a minimum the universal sysex id request and response and a (mma) vender specific sysex to jump to a bootloader. This should be expanded to identify the capabilities, architecture, and code loader method, and perhaps a subset of common midi settings (channel,mode etc.) Eventually this sysex subset form a protocall called "Let's Get Loaded (LGL)"

This handler should also buffer sysex's that are not directed at the target or that l with functions which can be overwritten.

### Hardware Abstractions.
Some of the work involved with the LUFA library is to provide feedback for the states of the the usb connection itself. This requires a hardware abstraction for each board supported by the library. In our use case this  would better be served by providing weak functions to be overridden user functions. 

[^1]: because you cant parse it doesnt mean you need to magle it thank your for letting me test you Mau… You are this close to total fail.






