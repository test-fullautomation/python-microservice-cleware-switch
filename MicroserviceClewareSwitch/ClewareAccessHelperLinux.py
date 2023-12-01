# *******************************************************************************
#
# File: ClewareAccessHelperLinux.py
#
# Initially created by Cuong Nguyen (RBVH/ENG22) / July 2019
#
# Description:
#   The real Cleware Access Helper on Linux.
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


class ClewareAccessHelperLinux(ClewareAccessHelperAbs):
   """
   ClewareAccessHelperLinux acts as the real object on Linux platform in Proxy Pattern
      + This is the wrapper class for the functions provided by C/C++ source for Linux.
      + The C/C++ source for Linux is provided by Cleware manufacturer at : http://www.cleware-shop.de/Downloads_EN
      + The C/C++ source for Linux is modified by Cuong Nguyen (RBVH/ENG22) for support python to load dynamic library on Linux.
   """
   _sPlatform = "linux"

   _sPath = "libs/Linux/USBAccessLinux.so"  # ./test/libUSBaccessBasic.so

   def __init__(self):
      """
      Constructor of ClewareAccessHelperLinux
      """
      path = inspect.currentframe().f_code.co_filename
      dir_path = os.path.dirname(path)
      self._library = Utils.load_library("%s/%s" % (dir_path, self._sPath), is_stdcall=False)
      self._usb_obj = None
      self.init_cleware()
      self.open_cleware()

   def __del__(self):
      """
      Destructor of ClewareAccessHelperLinux
      """
      if self._usb_obj:
         self.close_cleware()
         self._library.FCWUnInitObject(self._usb_obj)

   def init_cleware(self):
      self._library.FCWInitObject.restype = POINTER(c_int)
      self._usb_obj = self._library.FCWInitObject()
      return self._usb_obj

   def open_cleware(self):
      self._library.FCWOpenCleware.argtype = POINTER(c_int)
      self._library.FCWOpenCleware.restype = c_int
      res = self._library.FCWOpenCleware(self._usb_obj)
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
   
   def get_all_sw_state(self, device_no):
      self._library.FCWGetAllSwitchState.argtypes = [POINTER(c_int), c_int]
      self._library.FCWGetAllSwitchState.restype = POINTER(c_int)
      array_ptr = self._library.FCWGetAllSwitchState(self._usb_obj, device_no)
      array = [array_ptr[i] for i in range(8)] 
      result_dict = {str(i): array[i] for i in range(8)}
      self._library.free(array_ptr)
      return result_dict

if __name__ == '__main__':
   try:
      test = ClewareAccessHelperLinux()
   except Exception as ex:
      print (ex)

   print (test.get_serial_number(0))

   test.set_switch(710925, ClewareAccessHelperAbs.SWITCH_0, 0)
   time.sleep(3)
   test.set_switch(710925, ClewareAccessHelperAbs.SWITCH_0, 1)
   time.sleep(3)
   test.set_switch(710925, ClewareAccessHelperAbs.SWITCH_0, 0)