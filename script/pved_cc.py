# pved_cc.py
#
# $Id: pved_cc.py.rcs 1.4 2014/01/23 07:32:06 mmartens Exp mmartens $
#
# $Log: pved_cc.py.rcs $
# Revision 1.4  2014/01/23 07:32:06  mmartens
# Adressierung auf J1939 und PGNs umgestellt.
# Funktion zur Aenderung der Node-ID hinzu.
#
# Revision 1.3  2013/12/10 09:58:55  mmartens
# Variablen fuer flow und flowmode hinzu.
#
# Revision 1.2  2013/11/26 21:28:08  michael
# Datei umbenannt.
# Thread-Funktion ergaenzt.
#
# Revision 1.1  2013/11/26 16:41:16  mmartens
# Initial revision
#

from PCANBasic import *
from threading import Thread, Lock
from ctypes import *
import time

class PVED_CC (Thread):
  PVED_Node_IDs = (0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F)
  AVC_PGNs  = (0xFE30, 0xFE31, 0xFE32, 0xFE33, 0xFE34, 0xFE35, 0xFE36, 0xFE37, 0xFE38, 0xFE39, 0xFE3A, 0xFE3B, 0xFE3C, 0xFE3D, 0xFE3E, 0xFE3F)
  AVEF_PGNs = (0xFE10, 0xFE11, 0xFE12, 0xFE13, 0xFE14, 0xFE15, 0xFE16, 0xFE17, 0xFE18, 0xFE19, 0xFE1A, 0xFE1B, 0xFE1C, 0xFE1D, 0xFE1E, 0xFE1F)
  
  def __init__(self, number, controllerid, canbus):
    Thread.__init__(self, group=None, target=None, name=None, args=())
    if number < 1 :
      number = 1
    if number > 16 :
      number = 16
    self.number = number
    self.controllerid = controllerid
    self.NodeID = PVED_CC.PVED_Node_IDs[number - 1]

    self.AVC_PGN    = PVED_CC.AVC_PGNs[number - 1]
    self.AVEF_PGN   = PVED_CC.AVEF_PGNs[number - 1]
    self.DM1_id     = 0x18FECA00 + self.NodeID

    self.extendflow  = 0
    self.retractflow = 0
    self.state       = 0
    self.spoolpos    = 0.0

    self.fmode = 0
    self.flow = 0
    
    self.canbus = canbus
    self.timer = None
    self.run_thread = 0
    self.avc_lock = Lock()
    self.AVC_msg = TPCANMsg()
    self.AVC_msg.ID = 0x0C000000 + (self.AVC_PGN << 8) + self.controllerid
    self.AVC_msg.MSGTYPE = PCAN_MESSAGE_EXTENDED
    self.AVC_msg.LEN = 8
    self.AVC_msg.DATA[0] = 0
    self.AVC_msg.DATA[1] = 0
    self.AVC_msg.DATA[2] = 0
    self.AVC_msg.DATA[3] = 0
    self.AVC_msg.DATA[4] = 0
    self.AVC_msg.DATA[5] = 0
    self.AVC_msg.DATA[6] = 0
    self.AVC_msg.DATA[7] = 0

    self.PDM_msg = TPCANMsg()
    self.PDM_msg.ID = 0x0CCB0000 + (self.NodeID << 8) + self.controllerid
    self.PDM_msg.MSGTYPE = PCAN_MESSAGE_EXTENDED
    self.PDM_msg.LEN = 8
    self.PDM_msg.DATA[0] = 0
    self.PDM_msg.DATA[1] = 0
    self.PDM_msg.DATA[2] = 0
    self.PDM_msg.DATA[3] = 0
    self.PDM_msg.DATA[4] = 0
    self.PDM_msg.DATA[5] = 0
    self.PDM_msg.DATA[6] = 0
    self.PDM_msg.DATA[7] = 0

  def Enable(self):
    self.canbus.AddJ1939Handler(self.AVEF_PGN, 0, 0xFF, self)

  def SetFlow(self, mode, flow):
    self.avc_lock.acquire()
    if mode in (0, 'Blocked'):
      self.fmode = 0
      self.flow = 0
    elif mode in (1, 'Extend'):
      self.fmode = 1
      self.flow = flow
    elif mode in (2, 'Retract'):
      self.fmode = 2
      self.flow = flow
    elif mode in (3, 'Floating'):
      self.fmode = 3
      self.flow = 0
    elif mode in (10, 'Manual'):
      self.fmode = 0xA
      self.flow = 0
    else :
      self.fmode = 0
      self.flow = 0
    self.avc_lock.release()

    if self.run_thread == 1 :
      duetime = c_longlong(-10)
      windll.kernel32.SetWaitableTimer(self.timer, byref(duetime), 500, None, None, 0)

  def SendPDM(self, dd, value):
    self.PDM_msg.DATA[3] = dd
    self.PDM_msg.DATA[4] = value
    result = self.canbus.Write(self.PDM_msg)
    if result != PCAN_ERROR_OK:
      result = self.canbus.GetErrorText(result)
      print result

  def SetRamp(self, ramptype, value):
    if ramptype in (1, 'RampUpExtend'):
      self.PDM_msg.DATA[3] = 0x68
      self.PDM_msg.DATA[4] = value
    elif ramptype in (2, 'RampDownExtend'):
      self.PDM_msg.DATA[3] = 0x69
      self.PDM_msg.DATA[4] = value
    elif ramptype in (3, 'RampUpRetract'):
      self.PDM_msg.DATA[3] = 0x6A
      self.PDM_msg.DATA[4] = value
    elif ramptype in (4, 'RampDownRetract'):
      self.PDM_msg.DATA[3] = 0x6B
      self.PDM_msg.DATA[4] = value
    else :
      return
    result = self.canbus.Write(self.PDM_msg)
    if result != PCAN_ERROR_OK:
      result = self.canbus.GetErrorText(result)
      print result

  def HandleMessage(self, pgn, sa, msg):
    if pgn == self.AVEF_PGN :
      self.extendflow  = msg.DATA[0] - 125
      self.retractflow = msg.DATA[1] - 125
      self.state       = msg.DATA[2]
      self.spoolpos    = 7.0 / 125.0 * (msg.DATA[6] - 125)
    if msg.ID == self.DM1_id :
      print 'PVED-CC Error'

  def run(self):
    print "Starting PVED Thread"
    self.timer = windll.kernel32.CreateWaitableTimerA(None, 0, None)
    self.run_thread = 1
    duetime = c_longlong(-100)
    windll.kernel32.SetWaitableTimer(self.timer, byref(duetime), 500, None, None, 0)
    while True:
      ret = windll.kernel32.WaitForSingleObject(self.timer, 10000)
      if self.run_thread == 0 :
        break
      self.avc_lock.acquire()
      self.AVC_msg.DATA[2] = self.fmode
      self.AVC_msg.DATA[0] = self.flow
      self.avc_lock.release()
      result = self.canbus.Write(self.AVC_msg)
      if result != PCAN_ERROR_OK:
        print 'PVED-CC CAN Write Error'

    windll.kernel32.CloseHandle(self.timer)
    self.timer = None

  def Start(self):
    Thread.__init__(self, group=None, target=None, name=None, args=())
    self.start()
  def Stop(self):
    print "Stopping Receive Thread"
    duetime = c_longlong(-10)
    self.run_thread = 0
    windll.kernel32.SetWaitableTimer(self.timer, byref(duetime), 0, None, None, 0)

def PVED_CC_ChangeNodeID(canbus, OldNodeId, NewNodeID):
  msg = TPCANMsg()

  msg.ID = 0x1CEF00C8 + (OldNodeId << 8)
  msg.MSGTYPE = PCAN_MESSAGE_EXTENDED
  msg.LEN = 8
  msg.DATA[0] = 0xF9
  msg.DATA[1] = 0x20
  msg.DATA[2] = 0x09
  msg.DATA[3] = 0x00
  msg.DATA[4] = 0x01
  msg.DATA[5] = 0x00
  msg.DATA[6] = 0x09
  msg.DATA[7] = 0x50

  result = canbus.Write(msg)
  if result != PCAN_ERROR_OK:
    print 'PVED-CC CAN Write Error'
    return 1
  time.sleep(0.1)

  msg.DATA[0] = 0xF9
  msg.DATA[1] = 0x61
  msg.DATA[2] = 0x73
  msg.DATA[3] = 0x73
  msg.DATA[4] = 0x77
  msg.DATA[5] = 0x6F
  msg.DATA[6] = 0x72
  msg.DATA[7] = 0x64

  result = canbus.Write(msg)
  if result != PCAN_ERROR_OK:
    print 'PVED-CC CAN Write Error'
    return 2
  time.sleep(0.1)

  msg.DATA[0] = 0xF9
  msg.DATA[1] = 0x31
  msg.DATA[2] = 0x97
  msg.DATA[3] = 0xFF
  msg.DATA[4] = 0xFF
  msg.DATA[5] = 0xFF
  msg.DATA[6] = 0xFF
  msg.DATA[7] = 0xFF

  result = canbus.Write(msg)
  if result != PCAN_ERROR_OK:
    print 'PVED-CC CAN Write Error'
    return 3
  time.sleep(0.1)

  msg.DATA[0] = 0xF9
  msg.DATA[1] = 0x20
  msg.DATA[2] = 0x0B
  msg.DATA[3] = 0x00
  msg.DATA[4] = 0x33
  msg.DATA[5] = 0x00
  msg.DATA[6] = 0x02
  msg.DATA[7] = NewNodeID

  result = canbus.Write(msg)
  if result != PCAN_ERROR_OK:
    print 'PVED-CC CAN Write Error'
    return 4
  time.sleep(0.1)

  msg.DATA[0] = 0xF9
  msg.DATA[1] = 0x00
  msg.DATA[2] = 0x00
  msg.DATA[3] = 0x00
  msg.DATA[4] = 0x00
  msg.DATA[5] = 0x00
  msg.DATA[6] = 0x00
  msg.DATA[7] = 0x00

  result = canbus.Write(msg)
  if result != PCAN_ERROR_OK:
    print 'PVED-CC CAN Write Error'
    return 5

  return 0
