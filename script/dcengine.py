# dcengine.py
#
# Receive CAN-Messages from a Diesel Engine

from PCANBasic import *

class DCEngine:
  DCEngineIDs = (0x00)
  def __init__(self, controllerid, canbus):

    self.controllerid = controllerid

    self.ETC1_id    = 0x0CF00203

    self.TSC_ENG_id = 0x0C000000 + controllerid
    self.ESS_id     = 0x0C000000 + controllerid
    self.EEC1_id    = 0x0CF00400
    self.EEC2_id    = 0x0CF00300
    self.EEC3_id    = 0x18FEDF00
    self.CCVS_id    = 0x18FEF100
    self.EET_id     = 0x18FEEE00
    self.EFL_P_id   = 0x18FEEF00
    self.CFG_E_id   = 0x18FEE300
    self.DM1_id     = 0x18FECA00

    self.TSC1_ER_id = 0x0C000F00 + controllerid
    self.CFG_ER_id  = 0x18FEE10F
    self.ERC1_ER_id = 0x18F0000F

    self.speedsetting = 0
    self.speed = 0.0
    self.torque = 0
    
    self.canbus = canbus

  def Enable(self):
    self.canbus.AddHandler(self.EEC1_id, PCAN_MESSAGE_EXTENDED, self)

  def HandleMessage(self, msg):
    if msg.ID == self.EEC1_id :
      self.speed = 0.125 * (msg.DATA[5] | msg.DATA[6] << 8)
      self.torque = msg.DATA[1]

