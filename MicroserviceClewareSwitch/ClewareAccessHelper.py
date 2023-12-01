# *******************************************************************************
#
# File: ClewareAccessHelper.py
#
# Initially created by Cuong Nguyen (RBVH/ENG22) / July 2019
#
# Description:
#   A proxy class for divert request to real ClewareAccessHelper depend on platform (Windows/Linux).
#
# History:
#
# 22.07.2019 / V 0.1 / Cuong Nguyen
# - Initialize
#
# *******************************************************************************
import pkgutil
import os
import importlib
import platform
import json
import time
import ast
from Utils import Utils
from ClewareAccessHelperAbs import ClewareAccessHelperAbs
from ServiceLogger import ServiceLogger


class ClewareAccessHelper(ClewareAccessHelperAbs):
   """
   ClewareAccessHelper class acts as a proxy class in Proxy pattern and forward the request to the real helper depend on platform.
   """
   _config_usb_path = r'usb.config'

   __OPEN_CLOSE = {'open': 1,
               'close': 0}

   __STATUS = {'1': 'OK',
               '-1': 'NOK'}

   __ON_OFF = {'on': 1,
               'off': 0}

   def __init__(self):
      """
      Get all supported USB Backend classes and set the real_obj to the instance of the class match with config file
      """
      dir_path = os.path.dirname(os.path.realpath(__file__))
      current_name = os.path.splitext(os.path.basename(__file__))[0]
      for module_loader, name, ispkg in pkgutil.iter_modules([dir_path]):
         if current_name != name:
            try:
               importlib.import_module(name)
            except:
               pass

      supported_usb_access_classes_list = Utils.get_all_descendant_classes(ClewareAccessHelperAbs)
      supported_usb_access_classes_dict = {cls._sPlatform: cls for cls in supported_usb_access_classes_list}

      try:
         self.real_obj = supported_usb_access_classes_dict[platform.system().lower()]()
      except KeyError:
         raise Exception("Service not support '%s' platform" % platform.system().lower())
      except Exception as ex:
         raise ex

      self._dict_config_port = {}
      # self.load_config()

   def load_config(self):
      """
      Load the user config port name for USB access.
      Returns:
         None
      """
      try:
         dir_path = os.path.dirname(os.path.realpath(__file__))
         ServiceLogger().log("Read usb config at %s\\%s" % (dir_path, self._config_usb_path))
         with open("%s\\%s" % (dir_path, self._config_usb_path)) as json_file:
            self._dict_config_port = json.load(json_file)
         self._dict_config_port = {key.lower() if isinstance(key, str) or isinstance(key, unicode) else key: value for key, value in self._dict_config_port.items()}
      except Exception as ex:
         ServiceLogger().log("Unable get Cleware devices config!!! Reason: %s" % ex)

   def get_config(self):
      """
      Get Cleware ports' config.
      Returns:
         Dictionary of ports' setting.
      """
      ServiceLogger().log("get config")
      try:
         dir_path = os.path.dirname(os.path.realpath(__file__))
         ServiceLogger().log("Read usb config at %s\\%s" % (dir_path, self._config_usb_path))
         with open("%s\\%s" % (dir_path, self._config_usb_path)) as json_file:
            self._dict_config_port = json.load(json_file)
            res = self._dict_config_port
      except Exception as ex:
         raise Exception("Oops! Something went wrong! Unable to get the config. Exception: %s" % ex)
      return res

   def set_config(self, config_json):
      """
      Save Cleware ports' config to file.
      Args:
         config_json: data to be saved.

      Returns:
         res: saved result.
      """
      ServiceLogger().log("set config")
      res = "Config saved"
      try:
         dir_path = os.path.dirname(os.path.realpath(__file__))
         ServiceLogger().log("Read usb config at %s\\%s" % (dir_path, self._config_usb_path))
         data = json.loads(config_json)
         with open("%s\\%s" % (dir_path, self._config_usb_path), 'w') as f:
            json.dump(data, f, indent=3)
      except Exception as ex:
         raise Exception("Unable to save config. Exception: %s" % ex)

      return res

   def init_cleware(self):
      return self.real_obj.init_cleware()

   def open_cleware(self):
      return self.real_obj.open_cleware()

   def close_cleware(self):
      return self.real_obj.close_cleware()

   def get_handle(self, device_no):
      return self.real_obj.get_handle(device_no)

   def set_value(self, device_no, max_length=1024):
      return self.real_obj.set_value(device_no, max_length)

   def get_value(self, device_no, max_length=1024):
      return self.real_obj.get_value(device_no, max_length)

   def set_switch(self, device_no, switch_id, state):
      if state.lower() not in self.__ON_OFF:
         return -1

      on_off = self.__ON_OFF[state.lower()]
      if isinstance(switch_id, str):
         switch_id = int(switch_id, 0)
      return self.real_obj.set_switch(int(device_no), switch_id, on_off)

   def set_switch_by_port_name(self, port_name, state):
      res = "STATUS_ %s E_CODE_ %s TIME(ms)_ %s DIGITAL_OUT CHANNEL STATE SET_RET"
      ecode = -13
      millis = int(round(time.time() * 1000))
      try:
         config_dict = ast.literal_eval(port_name.strip())
      except Exception as _ex:
         ServiceLogger().log_debug("exception: %s" %_ex)
         device_no = 0
         switch_id = int(port_name, 0)
      else:
         try:
            device_no = int(config_dict['device'])
            switch_id = int(config_dict['port'], 0)
         except Exception as _ex:
            raise Exception("Unknown device (%s) or port (%s)." % (config_dict['device'], config_dict['port']))

      ServiceLogger().log_debug("device_no: %s  switch_id:%s" % (device_no, switch_id))
      if state.lower() in self.__ON_OFF:
         on_off = self.__ON_OFF[state.lower()]
         status = self.__STATUS[str(self.real_obj.set_switch(device_no, switch_id, on_off))]
      elif state.lower() in self.__OPEN_CLOSE:
         on_off = self.__OPEN_CLOSE[state.lower()]
         status = self.__STATUS[str(self.real_obj.set_switch(device_no, switch_id, on_off))]

      if status == 'OK':
         ecode = 0

      res = res % (status, ecode, millis)
      return res

   def get_switch(self, device_no, switch_id):
      return self.real_obj.get_switch(device_no, switch_id)

   def get_version(self, device_no):
      return self.real_obj.get_version(device_no)

   def get_usb_type(self, device_no):
      return self.real_obj.get_usb_type(device_no)

   def get_serial_number(self, device_no):
      return self.real_obj.get_serial_number(device_no)

   def valid_ser_number(self):
      return self.real_obj.valid_ser_number()

   def get_hw_version(self, device_no):
      return self.real_obj.get_hw_version(device_no)

   def is_ampel(self, device_no):
      return self.real_obj.is_ampel(device_no)

   def iox(self, device_no, addr, data):
      return self.real_obj.iox(device_no)

   def get_all_devices_state(self):
      res = {}
      num_device = self.real_obj.open_cleware()
      if platform.system().lower() == 'linux':
         for device_no in range(0, num_device):
            serial = self.get_serial_number(device_no)
            sw_states = self.real_obj.get_all_sw_state(device_no)
            res[str(serial)] = sw_states
      else:
         # num_device = self.real_obj.open_cleware()
         for device_no in range(0, num_device):
            serial = self.get_serial_number(device_no)
            num_port = 8  # self.get_usb_type(device_no) - ClewareAccessHelperAbs.SWITCH1_DEVICE + 1
            res[str(serial)] = {}
            for port_no in range(0, num_port):
               res[str(serial)].update({str(port_no): self.get_switch(serial, port_no + ClewareAccessHelperAbs.SWITCH_0)})
      return res


if __name__ == '__main__':
   test = ClewareAccessHelper()
   ls = test.get_all_devices_state()
   test.set_switch(0 , ClewareAccessHelperAbs.SWITCH_3, 'on')
   time.sleep(1)
   ls = test.get_all_devices_state()
   print(ls)
   # test.divert("SET SWITCH DEVICE_ 710741 SWITCH_ 0x17 STATE_ Open")
