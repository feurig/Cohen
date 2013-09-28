The idea here is to create a lufa based midi framework for use on the stock arduino. The goal is twofold 1) to isolate the lower level usb functions from the actual programs and 2) to create a minimal framework for identifying resetting and loading code onto midi devices. 


The majority of the usb-midi specification is in single 4 byte events or packets. The exception to this are sysex messages which are arbitrary in length. 

Because of this the two streams are split out between packets and sysex data. Some of the USB subsystems are interrupt driven. (maple midi zb) and some like lufa are polling based. The data interface must accommodate this. I am trying to arrive at some glue where the lower layers can provide hooks for the maintainance tasks and simply provide an interface like this.

    bool MidiEventAvaliable ([interface])
    void GetMidiEvent ([interface])
    void SendMidiEvent ([interface])
    bool SysexAvaliable ([intervace])
    void GetSysex ([interface])
    void SendSysex (uint8_t *messege, [interface])

The remaining details should be as hidden and interchangeable as possible.


A minimal sysex handler called by either the maintainence tasks or by GetMidiEvent should handle at a minimum the universal sysex id request and response and a (mma) vender specific sysex to jump to a bootloader. This should beexpanded to identify the capabilities, architecture, and code loader method, and perhaps a subset of common midi settings (channel,mode etc.) 

This handler should also buffer sysex's that are not directed at the target or that the handler does not deal with. 



