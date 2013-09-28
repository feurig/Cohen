The idea here is to create a lufa based midi framework for use on the stock arduino. 
The goal is twofold 1) to isolate the lower level usb functions from the actual programs
and 2) to create a minimal framework for identifying resetting and loading code onto 
midi devices. 

The majority of the usb-midi specification is in simgle 4 byte events or packets. 
The exception to this are sysex messages which are arbitrary in length. 

Because of this the two streams are split out between packets and sysex data. Some of the USB susbsystems are
interupt driven. (maple midi zb) and some like lufa are polling based. The data interface must accomidate this.
I am trying to arrive at some glue where the upper layers can provide hooks for the maintainance tasks and 
simply provide an interface like this.

bool MidiEventAvaliable ([interface])
void GetMidiEvent ([interface])
void SendMidiEvent ([interface])
bool SysexAvaliable ([intervace])
void GetSysex ([interface])
void SendSysex (uint8_t *messege, [interface]

The remaining details should be as hidden and interchangable as possible.




