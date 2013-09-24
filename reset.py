#!/usr/bin/env python
'''------------------------------------------------------------------------ upload.py
    
    This is a quick reboot mechanism for loading code onto a stock arduino leonardo
    It sends and recieves the Univeral midi sysex id request when it finds the a 
    device it sends the device a sysex to reset it. Then it waits to see where the 
    new device and uploads the hex file to that port.

 Copyright (c) 2013 Donald Delmar Davis, Suspect Devices

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify,
 merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be included in all copies
 or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

'''

import rtmidi_python as rtmidi
import time
import sys
import serial
import serial.tools.list_ports
from subprocess import call

# this stuff should probably go in a constants file.
SYSEX_BEGIN             =0xf0
SYSEX_NON_REAL_TIME     =0x7e
SYSEX_NON_REAL          =0x7f
SYSEX_ALL_CHANNELS      =0x7f
SYSEX_GENERAL_INFO      =0x06
SYSEX_IDENTITY_REQUEST  =0x01
SYSEX_IDENTITY_REPLY    =0x02
SYSEX_END_SYSEX         =0xF7


ARDUINO_MMA_VENDOR_1   =0x7D
ARDUINO_MMA_VENDOR_2   =0x23
ARDUINO_MMA_VENDOR_3   =0x41

LGL_RESET_CMD          =0x1e
#LGL_BOOTLOADER_CMD     =0x1f


we_are_done=False

askfortheids=[SYSEX_BEGIN,SYSEX_NON_REAL_TIME,SYSEX_ALL_CHANNELS,
              SYSEX_GENERAL_INFO,SYSEX_IDENTITY_REQUEST,SYSEX_END_SYSEX]

resetthedevice=[SYSEX_BEGIN,
                ARDUINO_MMA_VENDOR_1,
                ARDUINO_MMA_VENDOR_2,
                ARDUINO_MMA_VENDOR_3,
                0x0a, #device/channel is currently ignored...
                LGL_RESET_CMD,
                SYSEX_END_SYSEX]

# main code begins here.
'''
    Monophonic....
    This should poll all avaliable midi ports and channels until we find one that has one of
    our devices on it.
    eventually this should be expanded to handle multiple devices and understand multiple clients
'''
we_are_done = False
try:
    serial_ports_before=serial.tools.list_ports.comports()
    midi_in = rtmidi.MidiIn()
    midi_out = rtmidi.MidiOut()
    if len(midi_in.ports):
        print midi_in.ports
        midi_in.ignore_types(False,False,False) # do not ignore sysexs or any other messeges.
        midi_in.open_port(0)
        midi_out.open_port(0)
        midi_out.send_message(askfortheids)

        while not we_are_done:
            message, delta_time = midi_in.get_message()
            if message:
                if (len(message)>14 and message[0]==SYSEX_BEGIN): #if its a sysex message
                    if (message[1]==SYSEX_NON_REAL_TIME and message[3]==SYSEX_GENERAL_INFO
                        and message[4]==SYSEX_IDENTITY_REPLY ):
                        print 'MANUFACTURER ID: 0x%02X 0x%02X 0x%02X' % (
                               message[5],message[6],message[7])
                        print "FAMILY ID      : 0x%02X%02x" % (
                               message[8], message[9])
                        print "PRODUCT ID     : 0x%02X%02x" % (
                               message[10], message[11])
                        print "VERSION        : 0x%02X%02x%02X%02x" % (
                               message[12],message[13],message[14],message[15])
                        if (message[15] == ord('!')) :
                            print "OK! LETS GET LOADED!!!"
                            we_are_done=True
                            midi_out.send_message(resetthedevice)
                            we_are_done=True
                            midi_in.close_port()
                            midi_out.close_port()
                            new_serial_ports=list(set(serial.tools.list_ports.comports())-set(serial_ports_before))
                            
                            # the code below should have a timeout in case the os decides not to enumerate the
                            # new serial port. Which happens on mountain lion more than it should.
                            # wait for the bootloader to show up as a serial port.
                            while len(new_serial_ports) < 1 :
                                new_serial_ports=list(set(serial.tools.list_ports.comports())-set(serial_ports_before))
                            print new_serial_ports[0][0]

                            # Now we can call avrdude with the new serial port. 
#exit()
    else:
        print "Bailing: No Midi Devices attached!"
        exit()

        

except Exception as e:
    print str(e)
    exit()

