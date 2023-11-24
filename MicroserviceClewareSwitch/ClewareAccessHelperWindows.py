# *******************************************************************************
#
# File: ClewareAccessHelperWindows.py
#
# Initially created by Cuong Nguyen (RBVH/ENG22) / July 2019
#
# Description:
#   The real Cleware Access Helper on Windows.
#
# History:
#
# 22.07.2019 / V 0.1 / Cuong Nguyen
# - Initialize
#
# *******************************************************************************
import time
import os
import inspect
from ctypes import *
from ClewareAccessHelperAbs import ClewareAccessHelperAbs
from Utils import Utils
from ServiceLogger import ServiceLogger


class ClewareAccessHelperWindows(ClewareAccessHelperAbs):
   """
   ClewareAccessHelperWindows acts as the real object on Windows platform in Proxy Pattern
      + This is the wrapper class for the functions provided by USBaccess.dll.
      + The USBaccess.dll is provided by Cleware manufacturer at : http://www.cleware-shop.de/Downloads_EN
   """
   _sPlatform = "windows"
   _sPath = "libs/Windows/USBaccessX64.dll" #./test/libUSBaccessBasic.so

   def __init__(self):
      """
      Constructor of ClewareAccessHelperWindows
      """
      path = inspect.currentframe().f_code.co_filename
      dir_path = os.path.dirname(path)
      self._library = Utils.load_library("%s/%s" % (dir_path, self._sPath))
      self._usb_obj = None
      self.num_device = 0
      self.init_cleware()
      self.open_cleware()


   def __del__(self):
      """
      Destructor of ClewareAccessHelperWindows
      """
      if self._usb_obj:
         self.close_cleware()
         self._library.FCWUnInitObject(self._usb_obj)


   def init_cleware(self):
      self._library.FCWInitObject.restype = POINTER(c_int)
      self._usb_obj = self._library.FCWInitObject()
      self.num_device = 0
      return self._usb_obj

   def open_cleware(self):
      self._library.FCWOpenCleware.argtype = POINTER(c_int)
      self._library.FCWOpenCleware.restype = c_int
      res = self._library.FCWOpenCleware(self._usb_obj)
      self.num_device = res
      return res

   def close_cleware(self):
      self._library.FCWCloseCleware.argtype = POINTER(c_int)
      self._library.FCWCloseCleware.restype = c_int
      res = self._library.FCWCloseCleware(self._usb_obj)
      return res

   def get_handle(self, device_no):
      self._library.FCWGetHandle.argtypes = [POINTER(c_int), c_int]
      self._library.FCWGetHandle.restype = POINTER(c_int)
      res = self._library.FCWGetHandle(self._usb_obj, device_no)
      return res

   def set_value(self, device_no, max_length=1024):
      self._library.FCWSetValue.argtypes = [POINTER(c_int), c_int, c_char_p, c_int]
      self._library.FCWSetValue.restype = c_int

      text_buff = create_string_buffer("", max_length)
      res = self._library.FCWSetValue(self._usb_obj, device_no, text_buff, max_length)
      if res == 0:
         return "FAILED"
      return text_buff.value

   def get_value(self, device_no, max_length=1024):
      self._library.FCWGetValue.argtypes = [POINTER(c_int), c_int, c_char_p, c_int]
      self._library.FCWGetValue.restype = c_int

      text_buff = create_string_buffer("", max_length)
      res = self._library.FCWGetValue(self._usb_obj, device_no, text_buff, max_length)
      if res == 0:
         return "FAILED"
      return text_buff.value

   def set_switch(self, device_no, switch_id, on_off):
      ServiceLogger().log("set SW [%d] of device [%d] to [%d]" % (switch_id, device_no, on_off))
      self._library.FCWSetSwitch.argtypes = [POINTER(c_int), c_int, c_int, c_int]
      self._library.FCWSetSwitch.restype = c_int
      res = self._library.FCWSetSwitch(self._usb_obj, device_no, switch_id, on_off)
      return res

   def get_switch(self, device_no, switch_id):
      self._library.FCWGetSwitch.argtypes = [POINTER(c_int), c_int, c_int]
      self._library.FCWGetSwitch.restype = c_int
      res = self._library.FCWGetSwitch(self._usb_obj, device_no, switch_id)
      return res

   def get_version(self, device_no):
      self._library.FCWGetVersion.argtypes = [POINTER(c_int), c_int]
      self._library.FCWGetVersion.restype = c_int
      res = self._library.FCWGetVersion(self._usb_obj, device_no)
      return res

   def get_usb_type(self, device_no):
      self._library.FCWGetUSBType.argtypes = [POINTER(c_int), c_int]
      self._library.FCWGetUSBType.restype = c_int
      res = self._library.FCWGetUSBType(self._usb_obj, device_no)
      return res

   def get_serial_number(self, device_no):
      self._library.FCWGetSerialNumber.argtypes = [POINTER(c_int), c_int]
      self._library.FCWGetSerialNumber.restype = c_int
      res = self._library.FCWGetSerialNumber(self._usb_obj, device_no)
      return res

   def valid_ser_number(self):
      pass

   def get_hw_version(self, device_no):
      self._library.FCWGetHWversion.argtypes = [POINTER(c_int), c_int]
      self._library.FCWGetHWversion.restype = c_int
      res = self._library.FCWGetHWversion(self._usb_obj, device_no)
      return res

   def is_ampel(self, device_no):
      self._library.FCWIsAmpel.argtypes = [POINTER(c_int), c_int]
      self._library.FCWIsAmpel.restype = c_int
      res = self._library.FCWIsAmpel(self._usb_obj, device_no)
      return res

   def iox(self, device_no, addr, data):
      self._library.FCWIOX.argtypes = [POINTER(c_int), c_int, c_int, c_int]
      self._library.FCWIOX.restype = c_int
      res = self._library.FCWIOX(self._usb_obj, device_no, addr, data)
      return res


if __name__ == '__main__':
   test = ClewareAccessHelperWindows()
   test.set_switch(0, 23, 1)
   # test.set_switch(650937, USBAccessHelperAbs.SWITCH_0, 0)
   # time.sleep(3)
   # test.set_switch(650937, USBAccessHelperAbs.SWITCH_0, 1)
   # time.sleep(3)
   # test.set_switch(650937, USBAccessHelperAbs.SWITCH_0, 0)
