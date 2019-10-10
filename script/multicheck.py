# multicheck.py
#
# $Id: multicheck.py.rcs 1.2 2013/11/26 15:40:20 mmartens Exp $
#
# $Log: multicheck.py.rcs $
# Revision 1.2  2013/11/26 15:40:20  mmartens
# Id- und Log-Kommentare hinzu.
#

from pcanintf import *
import canlib

class MultiCheck:
  MultiCheckIDs = (0x5F6, 0x5F7, 0x676, 0x677)
  def __init__(self, number, canbus):
    if number < 1 :
      number = 1
    if number > 4 :
      number = 4
    self.number = number
    self.ID = MultiCheck.MultiCheckIDs[number - 1]
    self.ain = [0, 0, 0, 0]
    self.canbus = canbus
  def Enable(self):
    self.canbus.AddHandler(self.ID, canlib.CAN_MESSAGE_STANDARD, self)
  def SetChannel(self, chn, mode, pinmode, valtype, rate):
    msg = canlib.TCanMessage()
    msg.ID = 0x674
    msg.TYPE = canlib.CAN_MESSAGE_STANDARD
    msg.LEN = 8
    msg.DATA[0] = 0x05
    msg.DATA[1] = self.number << 4 | chn
    if mode in ('0-20mA', '4-20mA'):
      msg.DATA[2] = 0x05
    elif mode in ('0-5V', '0.5-4.5V'):
      msg.DATA[2] = 0x04
    elif mode in ('0-10V', '0-12V'):
      msg.DATA[2] = 0x01
    elif mode in ('F Low'):
      msg.DATA[2] = 0x11
    elif mode in ('F High'):
      msg.DATA[2] = 0x21
    else :
      msg.DATA[2] = 0x01

    if pinmode in ('Namur'):
      msg.DATA[3] = 0x00
    elif pinmode in ('PNP'):
      msg.DATA[3] = 0x01
    elif pinmode in ('Analog'):
      msg.DATA[3] = 0x02
    elif pinmode in ('Hydac'):
      msg.DATA[3] = 0x03
    else :
      msg.DATA[3] = 0x02

    if valtype in ('None'):
      msg.DATA[4] = 0x00
    elif valtype in ('Current'):
      msg.DATA[4] = 0x01
    elif valtype in ('Peak'):
      msg.DATA[4] = 0x02
    elif valtype in ('Average'):
      msg.DATA[4] = 0x03
    else :
      msg.DATA[4] = 0x01

    msg.DATA[5] = 0x00
    msg.DATA[6] = rate & 0x00FF
    msg.DATA[7] = (rate >> 8) & 0X00FF

    result = self.canbus.Write(msg)
#    if result != PCAN_ERROR_OK:
#      result = self.canbus.GetErrorText(result)
#      print result

  def HandleMessage(self, msg):
    self.ain[0] = msg.DATA[0] | msg.DATA[1] << 8
    self.ain[1] = msg.DATA[2] | msg.DATA[3] << 8
    self.ain[2] = msg.DATA[4] | msg.DATA[5] << 8
    self.ain[3] = msg.DATA[6] | msg.DATA[7] << 8
