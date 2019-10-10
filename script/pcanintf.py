# pcanintf.py
#
# $Id: pcanintf.py.rcs 1.5 2014/01/23 07:36:12 mmartens Exp mmartens $
#
# $Log: pcanintf.py.rcs $
# Revision 1.5  2014/01/23 07:36:12  mmartens
# Behandlung von eingehenden J1939-Botschaften hinzu.
#
# Revision 1.4  2013/12/05 15:39:15  mmartens
# Rueckgabewert bei Funktion Write hinzu.
#
# Revision 1.3  2013/11/26 21:26:36  michael
# Funktion Start hinzu, damit Thread.__init__ bei jedem Start aufgerufen wird.
#
# Revision 1.2  2013/11/26 15:42:23  mmartens
# Id- und Log-Kommentare hinzu.
#


from ctypes import *
import canlib

class J1939_Handler :
  def __init__(self, pgn, da, sa, obj):
    self.pgn = pgn
    self.da  = da
    self.sa  = sa
    self.obj = obj

class PcanIntf ():
  def __init__(self, bus, baudrate):
    self.can = bus
    self.handlers = []
    self.J1939handlers = []
    self.can.ReceiveHandler = self.ReceiveHandler
  def ReceiveHandler(self):
    msg = canlib.TCanMessage()
    
    while self.can.Read(msg):
        for handler in self.handlers :
          if msg.ID == handler[0] :  # and msg.MSGTYPE == handler[1]:
            handler[2].HandleMessage(msg)
        if msg.TYPE == 2 :
          # unused  dp  = (msg.ID & 0x01000000) >> 24    # Data Page Bit
          # unused  edp = (msg.ID & 0x02000000) >> 25    # Extended Data Page Bit
          pf  = (msg.ID & 0x00FF0000) >> 16    # PDU Format
          da  = (msg.ID & 0x0000FF00) >> 8     # destination address
          sa  =  msg.ID & 0x000000FF           # Source address
          if pf >= 240 :
            pgn = (msg.ID & 0x03FFFF00) >> 8     # Parameter Group Number of global PGN
          else :
            pgn = (msg.ID & 0x03FF0000) >> 8     # Parameter Group Number of specific PGN

          if pgn == 0xEC00 :                   # BAM/TP init 
            bampgn = msg.DATA[7] << 16 | msg.DATA[6] << 8 | msg.DATA[5]
            # BAM/TP message handling not implemented
          elif pgn == 0xEB00 :                 # BAM/TP sequence data
            seq_number = msg.DATA[0]
            # BAM/TP message handling not implemented 
          else :
            for handler in self.J1939handlers :
              if pgn == handler.pgn and (sa == handler.sa or handler.sa == 0xFF) :
                if pf >= 240 :                  # global pgn
                  handler.obj.HandleMessage(pgn, sa, msg)
                else :
                  if (da == handler.da) or (da == 0xFF) :
                    handler.obj.HandleMessage(pgn, sa, msg)

  def Start(self):
    return self.can.Start()

  def Stop(self):
    return self.can.Stop()
    
  def Write(self, msg):
    return self.can.Write(msg)

  def AddHandler(self, id, type, obj):
    handler = (id, type, obj)
    self.handlers.append(handler)

  def AddJ1939Handler(self, pgn, da, sa, obj):
    handler = J1939_Handler(pgn, da, sa, obj)
    self.J1939handlers.append(handler)
