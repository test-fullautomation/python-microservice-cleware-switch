# *******************************************************************************
#
# File: CUtils.py
#
# Initially created by Cuong Nguyen (RBVH/ENG22) / July 2019
#
# Description:
#   Provide the utilities for supporting development.
#
# History:
#
# 22.07.2019 / V 0.1 / Cuong Nguyen
# - Initialize
#
# *******************************************************************************
import inspect
import threading
import platform
import json
import collections
from ctypes import *
_LINUX_OS = "linux"
_WINDOWS_OS = "windows"
sPlatform = platform.system().lower()
if sPlatform.startswith(_WINDOWS_OS):
   import win32con
   import win32api


class Singleton(object):  # pylint: disable=R0903
   """
   Class to implement Singleton Design Pattern. This class is used to derive the TTFisClientReal as only a single
   instance of this class is allowed.

   Disabled pyLint Messages:
   R0903:  Too few public methods (%s/%s)
            Used when class has too few public methods, so be sure it's really worth it.

            This base class implements the Singleton Design Pattern required for the TTFisClientReal.
            Adding further methods does not make sense.
   """
   _instance = None
   _lock = threading.Lock()

   def __new__(cls, *args, **kwargs):
      with cls._lock:
         if not cls._instance:
            cls._instance = super(Singleton, cls).__new__(cls, *args, **kwargs)
      return cls._instance


class Utils:
   """
   Class to implement utilities for supporting development.
   """
   LINUX_OS = "linux"
   WINDOWS_OS = "windows"

   def __init__(self):
      """
      Empty constructor.
      """
      pass

   @staticmethod
   def get_all_descendant_classes(cls):
      """
      Get all descendant classes of a class
      Args:
         cls: Input class for finding descendants.

      Returns:
         Array of descendant classes.
      """
      return set(cls.__subclasses__()).union(
         [s for c in cls.__subclasses__() for s in Utils.get_all_sub_classes(c)])

   @staticmethod
   def get_all_sub_classes(cls):
      """
      Get all children classes of a class
      Args:
         cls: Input class for finding children.

      Returns:
         Array of children classes.
      """
      return set(cls.__subclasses__()).union(
         [s for s in cls.__subclasses__()])

   @staticmethod
   def caller_name(skip=2):
      """
      Get a name of a caller in the format module.class.method
      Args:
         skip: specifies how many levels of stack to skip while getting caller
         name. skip=1 means "who calls me", skip=2 "who calls my caller" etc.

      Returns:
         An empty string is returned if skipped levels exceed stack height
      """
      stack = inspect.stack()
      start = 0 + skip
      if len(stack) < start + 1:
         return ''
      parent_frame = stack[start][0]

      name = []
      module = inspect.getmodule(parent_frame)
      # `modname` can be None when frame is executed directly in console
      # TODO(techtonik): consider using __main__
      if module:
         name.append(module.__name__)
      # detect classname
      if 'self' in parent_frame.f_locals:
         # I don't know any way to detect call from the object method
         # XXX: there seems to be no way to detect static method call - it will
         #      be just a function call
         name.append(parent_frame.f_locals['self'].__class__.__name__)
      codename = parent_frame.f_code.co_name
      if codename != '<module>':  # top level usually
         name.append(codename)  # function or a method
      del parent_frame
      return ".".join(name)

   @staticmethod
   def load_library(path, is_stdcall=True):
      """
      Load native library depend on the calling convention.
      Args:
         path: library path.
         is_stdcall: determine if the library's calling convention is stdcall or cdecl.

      Returns:
         Loaded library object.
      """
      try:
         if is_stdcall:
            res_dll = windll.LoadLibrary(path)
         else:
            res_dll = cdll.LoadLibrary(path)
      except Exception as ex:
         res_dll = None
         print("Unable load '%s'. Reason: %s" % (path, ex))
      finally:
         return res_dll

   @staticmethod
   def is_ascii_or_unicode(str_check, codecs=['utf8', 'utf16', 'utf32', 'ascii']):
      """
      Check if the string is ascii or unicode
      Args:
         str_check: string for checking
         codecs: encoding type list

      Returns:
         True : if checked string is ascii or unicode
         False : if checked string is not ascii or unicode
      """
      res = False
      for i in codecs:
         try:
            str_check.decode(i)
            res = True
            break
         except Exception:
            pass
      return res

   @staticmethod
   def get_registry_parameters(service_name, key_name):
      """
      Get service's parameters which stored in registry.
      Args:
         service_name: Name of the service.
         key_name: Name of register key.

      Returns:
         Value of service's parameters
      """
      key = win32api.RegOpenKey(win32con.HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\" + service_name)
      try:
         try:
            (command_line, regtype) = win32api.RegQueryValueEx(key, key_name)
            return command_line
         except:
            pass
      finally:
         key.Close()

      Utils.create_registry_parameters(service_name, key_name, '')
      return ""

   @staticmethod
   def create_registry_parameters(service_name, key_name, parameters):
      """
      Create service's parameters and store it to registry.
      Args:
         service_name: Name of the service.
         key_name: Name of register key.
         parameters: Parameters to be stored.

      Returns:
         None
      """
      new_key = win32api.RegOpenKeyEx(win32con.HKEY_LOCAL_MACHINE, "System\\CurrentControlSet\\Services\\" + service_name, 0, win32con.KEY_ALL_ACCESS)
      try:
         win32api.RegSetValueEx(new_key, key_name, 0, win32con.REG_SZ, parameters)
      except Exception as ex:
         print(ex)
      finally:
         new_key.Close()


class Job(threading.Thread):
   def __init__(self, interval, execute, *args, **kwargs):
      threading.Thread.__init__(self)
      self.daemon = False
      self.stopped = threading.Event()
      self.interval = interval
      self.execute = execute
      self.args = args
      self.kwargs = kwargs

   def stop(self):
      self.stopped.set()
      self.join()

   def run(self):
      while not self.stopped.wait(self.interval.total_seconds()):
         self.execute(*self.args, **self.kwargs)


class ResultType:
   """
   Result Types.
   """
   PASS = "pass"
   FAIL = "fail"
   EXCEPT = "exception"

   def __init__(self):
      pass


class ResponseMessage(object):
   """
   Response message class
   """
   def __init__(self, request="", result=ResultType.PASS, result_data=""):
      self.request = request
      self.result = result
      self.result_data = result_data

   def get_json(self):
      """
      Convert response message to json
      Returns:
         Response message in json format
      """
      return json.dumps(collections.OrderedDict(sorted(self.__dict__.items())))

   def get_data(self):
      """
      Get string data result
      Returns:
         String result
      """
      return self.result_data

   @staticmethod
   def create_from_string(str):
      res = None
      try:
         data = json.loads(str)
         res = ResponseMessage(data['request'], data['result'], data['result_data'])
      except Exception as ex:
         pass
      return res