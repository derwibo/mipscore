# ecc.py
#
# $Id:$
#
# $Log:$

import canlib
from pcanintf import *
from threading import Thread, Lock
from ctypes import *

class ECC (Thread):
  ECC_Node_IDs = (0x4B, 0x4A, 0x49, 0x48)
  ECC_PA_Ports = ('PA1', 'PB1', 'PA2', 'PB2', 'PA3', 'PB3', 'PA4', 'PB4')
  ECC_PE_Ports = ('PE1', 'PE2', 'PE3', 'PE4')
  ECC_FQ_Ports = ('FQ1', 'FQ2')
  ECC_SE_Ports = ('SE1')
  ECC_CE_Ports = ('CE1', 'CE2', 'CE3')
  def __init__(self, number, controllerid, canbus):
    Thread.__init__(self, group=None, target=None, name=None, args=())
    if number < 1 :
      number = 1
    if number > 4 :
      number = 4
    self.number = number
    self.controllerid = controllerid
    self.NodeID = ECC.ECC_Node_IDs[number - 1]

    self.TxPDO1_id = 0x180 + self.NodeID
    self.TxPDO2_id = 0x280 + self.NodeID

    self.RxPDO1_id = 0x200 + self.NodeID
    self.RxPDO2_id = 0x300 + self.NodeID

    self.TxSDO_id  = 0x580 + self.NodeID
    self.RxSDO_id  = 0x600 + self.NodeID

    self.TxNMT_id  = 0x680 + self.NodeID

    self.SE1 = 0
    self.CE1 = 0
    self.CE2 = 0
    self.AN1 = 0
    self.FQ1 = 0
    self.FQ2 = 0
    self.FQ1diag = 0
    self.FQ2diag = 0

    self.PA1 = 0
    self.PA2 = 0
    self.PA3 = 0
    self.PA4 = 0
    self.PB1 = 0
    self.PB2 = 0
    self.PB3 = 0
    self.PB4 = 0

    
    self.canbus = canbus
    self.timer = None
    self.run_thread = 0

    self.RxPDO_msg = canlib.TCanMessage()
    self.RxPDO_msg.ID = self.RxPDO1_id
    self.RxPDO_msg.TYPE = canlib.CAN_MESSAGE_STANDARD
    self.RxPDO_msg.LEN = 8

    self.RxSDO_msg = canlib.TCanMessage()
    self.RxSDO_msg.ID = self.RxSDO_id
    self.RxSDO_msg.TYPE = canlib.CAN_MESSAGE_STANDARD
    self.RxSDO_msg.LEN = 8

  def Enable(self):
    self.canbus.AddHandler(self.TxPDO1_id, canlib.CAN_MESSAGE_STANDARD, self)
    self.canbus.AddHandler(self.TxPDO2_id, canlib.CAN_MESSAGE_STANDARD, self)

  def SetDeviceState(self, state):
    NMT_msg = canlib.TCanMessage()
    NMT_msg.ID = 0x00
    NMT_msg.TYPE = canlib.CAN_MESSAGE_STANDARD
    NMT_msg.LEN = 2
    if state == 'Operational' :
      NMT_msg.DATA[0] = 0x01
    elif state == 'Stop' :
      NMT_msg.DATA[0] = 0x02
    elif state == 'Pre-Operational' :
      NMT_msg.DATA[0] = 0x80
    else :
      NMT_msg.DATA[0] = 0x80

    NMT_msg.DATA[1] = self.NodeID
#    NMT_msg.DATA[1] = 0x00
    
    result = self.canbus.Write(NMT_msg)
    if not result :
      print 'ECC CAN Write Error'

  def SetPortMode(self, port, mode):
    if port in ECC.ECC_PA_Ports :
      self.RxSDO_msg.DATA[0] = 0x2F
      self.RxSDO_msg.DATA[1] = 0x00 + ECC.ECC_PA_Ports.index(port)
      self.RxSDO_msg.DATA[2] = 0x20
      self.RxSDO_msg.DATA[3] = 0x01
      if mode == 'SA' :
        self.RxSDO_msg.DATA[4] = 0x02
      elif mode == 'PA' :
        self.RxSDO_msg.DATA[4] = 0x04
      elif mode == 'PWM' :
        self.RxSDO_msg.DATA[4] = 0x08
      self.RxSDO_msg.LEN = 8
    elif port in ECC.ECC_FQ_Ports :
      self.RxSDO_msg.DATA[0] = 0x2F
      self.RxSDO_msg.DATA[1] = 0x00
      self.RxSDO_msg.DATA[2] = 0x22
      self.RxSDO_msg.DATA[3] = 0x02 + ECC.ECC_FQ_Ports.index(port)
      self.RxSDO_msg.DATA[4] = mode
      self.RxSDO_msg.LEN = 8
    elif port == 'SE1' :
      self.RxSDO_msg.DATA[0] = 0x2F
      self.RxSDO_msg.DATA[1] = 0x00
      self.RxSDO_msg.DATA[2] = 0x22
      self.RxSDO_msg.DATA[3] = 0x01
      self.RxSDO_msg.DATA[4] = mode
      self.RxSDO_msg.LEN = 8
    elif port == 'CE3' :
      self.RxSDO_msg.DATA[0] = 0x2F
      self.RxSDO_msg.DATA[1] = 0x00
      self.RxSDO_msg.DATA[2] = 0x22
      self.RxSDO_msg.DATA[3] = 0x06
      self.RxSDO_msg.DATA[4] = mode
      self.RxSDO_msg.LEN = 8
    else :
      return

    result = self.canbus.Write(self.RxPDO_msg)
    if not result :
      print 'ECC CAN Write Error'

  def SetPortParam(self, port, index, value):
    if port in ECC.ECC_PA_Ports :
      portindex = ECC.ECC_PA_Ports.index(port)
      self.RxSDO_msg.DATA[0] = 0x2B
      self.RxSDO_msg.DATA[1] = 0x00 + portindex
      self.RxSDO_msg.DATA[2] = 0x20
      self.RxSDO_msg.DATA[3] = index
      self.RxSDO_msg.DATA[4] = value
      self.RxSDO_msg.DATA[5] = value >> 8
      self.RxSDO_msg.LEN = 8
    else :
      return

    result = self.canbus.Write(self.RxPDO_msg)
    if not result :
      print 'ECC CAN Write Error'

  def EnablePort(self, port, enable):
    if port in ECC.ECC_PA_Ports :
      portindex = ECC.ECC_PA_Ports.index(port)
      self.RxSDO_msg.DATA[0] = 0x2F
      self.RxSDO_msg.DATA[1] = 0x00 + portindex
      self.RxSDO_msg.DATA[2] = 0x20
      self.RxSDO_msg.DATA[3] = 0x10
      if enable in (1, 'Enable') :
        self.RxSDO_msg.DATA[4] = 0x0
      else :
        self.RxSDO_msg.DATA[4] = 0x1
      self.RxSDO_msg.LEN = 8
    else :
      return

    result = self.canbus.Write(self.RxPDO_msg)
    if not result :
      print 'ECC CAN Write Error'

  def HandleMessage(self, msg) :
    if msg.ID == self.TxPDO1_id :
      self.SE1 = msg.DATA[0] | msg.DATA[1] << 8
      self.CE1 = msg.DATA[2] | msg.DATA[3] << 8
      self.CE2 = msg.DATA[4] | msg.DATA[5] << 8
      self.AN1 = msg.DATA[6] | msg.DATA[7] << 8

    elif msg.ID == self.TxPDO2_id :
      self.FQ1 = msg.DATA[0] | msg.DATA[1] << 8
      self.FQ2 = msg.DATA[2] | msg.DATA[3] << 8
      self.FQ1diag = msg.DATA[4] | msg.DATA[5] << 8
      self.FQ2diag = msg.DATA[6] | msg.DATA[7] << 8

    elif msg.ID == self.TxSDO_id :
      if msg.DATA[0] == 0x60 :
      # Sucessful write response
        print 'ECA Write Operation successful'

      elif msg.DATA[0] == 0x43 :
      # Sucessfull read response
        print 'ECA Read Operation successful'

#      elif msg.DATA[0] == 0x80 :
      # abort operation

#    elif msg.id == self.TxNMT_id :



  def run(self):
    print "Starting ECC Thread"
    self.timer = windll.kernel32.CreateWaitableTimerA(None, 0, None)
    self.run_thread = 1
    duetime = c_longlong(-100)
    windll.kernel32.SetWaitableTimer(self.timer, byref(duetime), 100, None, None, 0)
    while True:
      ret = windll.kernel32.WaitForSingleObject(self.timer, 10000)
      if self.run_thread == 0 :
        break

      self.RxPDO_msg.ID = self.RxPDO1_id
      self.RxPDO_msg.DATA[0] = self.PA1
      self.RxPDO_msg.DATA[1] = self.PA1 >> 8
      self.RxPDO_msg.DATA[2] = self.PB1
      self.RxPDO_msg.DATA[3] = self.PB1 >> 8
      self.RxPDO_msg.DATA[4] = self.PA2
      self.RxPDO_msg.DATA[5] = self.PA2 >> 8
      self.RxPDO_msg.DATA[6] = self.PB2
      self.RxPDO_msg.DATA[7] = self.PB2 >> 8

      result = self.canbus.Write(self.RxPDO_msg)
      if not result :
        print 'ECC CAN RxPDO1 Write Error'

      self.RxPDO_msg.ID = self.RxPDO2_id
      self.RxPDO_msg.DATA[0] = self.PA3
      self.RxPDO_msg.DATA[1] = self.PA3 >> 8
      self.RxPDO_msg.DATA[2] = self.PB3
      self.RxPDO_msg.DATA[3] = self.PB3 >> 8
      self.RxPDO_msg.DATA[4] = self.PA4
      self.RxPDO_msg.DATA[5] = self.PA4 >> 8
      self.RxPDO_msg.DATA[6] = self.PB4
      self.RxPDO_msg.DATA[7] = self.PB4 >> 8

      result = self.canbus.Write(self.RxPDO_msg)
      if not result :
        print 'ECC CAN RxPDO2 Write Error'

    windll.kernel32.CloseHandle(self.timer)
    self.timer = None

  def Start(self):
    Thread.__init__(self, group=None, target=None, name=None, args=())
    self.SetDeviceState('Operational')
    self.start()
  def Stop(self):
    print "Stopping Receive Thread"
    duetime = c_longlong(-10)
    self.run_thread = 0
    windll.kernel32.SetWaitableTimer(self.timer, byref(duetime), 0, None, None, 0)
    self.SetDeviceState('Pre-Operational')
