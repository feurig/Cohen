'''------------------------------------------------------------------------ reset.py
    
    This is a quick reboot mechanism for loading code onto a stock arduino leonardo
    It sends and recieves the Univeral midi sysex id request when it finds the a 
    device it sends the device a sysex to reset it. 
    

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
LGL_BOOTLOADER_CMD     =0x1f


global we_are_done
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

'''
    this is our callback routine for midi messages. 
    It handles sysexes sent to it and the universal sysex id response.
'''
def handleOurSysexes(message, time_stamp):
    global we_are_done
    #print "?"+str(message)+"?"
    if (len(message)>4 and message[0]==SYSEX_BEGIN): #if its a sysex message
        if (message[1]==125 and message[2]==51 and message[3]==51):
            print "DBG:",decodePayload(message)
        elif (message[1]==SYSEX_NON_REAL_TIME and message[3]==SYSEX_GENERAL_INFO
              #and message[4]==SYSEX_IDENTITY_REPLY
              ):
            print 'MANUFACTURER ID: 0x%02X 0x%02X 0x%02X' % (
                   message[5],message[6],message[7])
            print "FAMILY ID      : 0x%02X%02x" % (
                   message[8], message[9])
            print "PRODUCT ID     : 0x%02X%02x" % (
                   message[10], message[11])
            print "VERSION        : 0x%02X%02x%02X%02x" % (
                   message[12],message[13],message[14],message[15])
            if (message[15] == ord('!')) :
                   print "YAY LETS GET LOADED!!!"
                   midi_out.send_message(resetthedevice)
                   we_are_done=True


#012345678
#bxxxyyyzz
#b=sosysex, xxx= my mma id, yyy = client mma id , zz other info

def decodePayload(payload):
#
    return "".join(map(chr, payload[4:-1]))

# main code begins here.
'''
    Monophonic....
    This should poll all avaliable midi devices until we find one that has one of our devices on it.
    eventually this should be expanded to handle multiple devices and understand multiple clients
'''

try:
    midi_in = rtmidi.MidiIn()
    midi_out = rtmidi.MidiOut()
    if len(midi_in.ports):
        print midi_in.ports
        midi_in.callback = handleOurSysexes
        midi_in.ignore_types(False,False,False)
        midi_in.open_port(0)
        midi_out.open_port(0)
        time.sleep(3)
        midi_out.send_message(askfortheids)
    else:
        print "Bailing: No Midi Devices attached!"
        exit()

        

except Exception as e:
    print str(e)
    exit()



while 1:
    time.sleep(1)

    if we_are_done:
        midi_in.close_port()
        midi_out.close_port()
        time.sleep(2)
        exit()
    
    time.sleep(1)
    print "requesting id"

    midi_out.send_message(askfortheids)


"""
    http://www.blitter.com/~russtopia/MIDI/~jglatt/tech/midispec.htm

    0xF0  SysEx
    0x7E  Non-Realtime
    0x7F  The SysEx channel. Could be from 0x00 to 0x7F. Here we set it to "disregard channel".
    0x06  Sub-ID -- General Information
    0x01  Sub-ID2 -- Identity Request
    0xF7  End of SysEx
    Here is the Identity Reply message:
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
"""