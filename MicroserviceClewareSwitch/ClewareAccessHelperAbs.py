 # *******************************************************************************
#
# File: ClewareAccessHelperAbs.py
#
# Initially created by Cuong Nguyen (RBVH/ENG22) / July 2019
#
# Description:
#   An abstract class for implementing the real Helper depend on platform.
#
# History:
#
# 22.07.2019 / V 0.1 / Cuong Nguyen
# - Initialize
#
# *******************************************************************************
import abc
from abc import ABCMeta


class ClewareAccessHelperAbs(object):
   """
   ClewareAccessHelperAbs acts as a abstract class for the real Helper in Proxy Pattern.
   """
   __metaclass__ = ABCMeta
   _sPlatform = "NotSupport"

   SWITCH_0 = 0x10
   SWITCH_1 = 0x11
   SWITCH_2 = 0x12
   SWITCH_3 = 0x13
   SWITCH_4 = 0x14
   SWITCH_5 = 0x15
   SWITCH_6 = 0x16
   SWITCH_7 = 0x17
   SWITCH_8 = 0x18
   SWITCH_9 = 0x19
   SWITCH_10 = 0x1a
   SWITCH_11 = 0x1b
   SWITCH_12 = 0x1c
   SWITCH_13 = 0x1d
   SWITCH_14 = 0x1e
   SWITCH_15 = 0x1f

   SWITCH1_DEVICE = 0x08
   SWITCH2_DEVICE = 0x09
   SWITCH3_DEVICE = 0x0a
   SWITCH4_DEVICE = 0x0b
   SWITCH5_DEVICE = 0x0c
   SWITCH6_DEVICE = 0x0d
   SWITCH7_DEVICE = 0x0e
   SWITCH8_DEVICE = 0x0f


   def __init__(self):
      pass

   @abc.abstractmethod
   def init_cleware(self):
      pass

   @abc.abstractmethod
   def open_cleware(self):
      pass

   @abc.abstractmethod
   def close_cleware(self):
      pass

   @abc.abstractmethod
   def set_value(self):
      pass

   @abc.abstractmethod
   def get_value(self):
      pass

   @abc.abstractmethod
   def set_switch(self):
      pass

   @abc.abstractmethod
   def get_switch(self):
      pass

   @abc.abstractmethod
   def get_handle(self):
      pass

   @abc.abstractmethod
   def get_version(self):
      pass

   @abc.abstractmethod
   def get_usb_type(self):
      pass

   @abc.abstractmethod
   def get_usb_type(self):
      pass

   @abc.abstractmethod
   def get_serial_number(self):
      pass

   @abc.abstractmethod
   def valid_ser_number(self):
      pass

   @abc.abstractmethod
   def get_hw_version(self):
      pass

   @abc.abstractmethod
   def is_ampel(self):
      pass

   @abc.abstractmethod
   def iox(self):
      pass