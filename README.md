The idea here is to create a lufa based midi framework for use on the stock arduino. 
The goal is twofold 1) to isolate the lower level usb functions from the actual programs
and 2) to create a minimal framework for identifying resetting and loading code onto 
midi devices. 

Packet midi. 
The majority of the usb-midi specification is in simgle 4 byte events or packets. 
The exception to this are sysex messages which are arbitrary in length. 

Because of this the two streams are split out. 




