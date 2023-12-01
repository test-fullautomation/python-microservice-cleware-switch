import logging
import logging.handlers
import datetime
import os
import platform
import inspect
import configparser
from Utils import Utils, Singleton
_LINUX_OS = "linux"
_WINDOWS_OS = "windows"
sPlatform = platform.system().lower()
if sPlatform.startswith(_LINUX_OS):
   os.environ['TMLAutoTestPath'] = '/opt/bosch/tml/tml_framework'
   os.environ['TMLMicroservicePath'] = '/opt/bosch/tml/microservice'

LOG_LEVEL = {
      'notset' : logging.NOTSET,
      'debug' : logging.DEBUG,
      'info': logging.INFO,
      'warning': logging.WARNING,
      'error': logging.ERROR,
      'critical': logging.CRITICAL,
   }


class ServiceLogger(Singleton):
   """
   Logger class.
   """
   _indent = 0
   service_logger = None

   def __init__(self):
      Singleton.__init__(self)

   @staticmethod
   def set_logger(service_name):
      ServiceLogger.service_logger = logging.getLogger(service_name)
      conf = ServiceLogger.get_config_file()
      if conf:
         config = configparser.ConfigParser()
         config.read(conf)
         try:
            level = config['debug']['level']
         except Exception as _ex:
            log_lv = logging.INFO
         else:
            log_lv = LOG_LEVEL[level.lower()]

         try:
            tml_log_path = config['debug']['path']
         except Exception as _ex:
            tml_log_path = os.environ['TMLMicroservicePath'] + "/logfiles"

         if not tml_log_path:
            tml_log_path = os.environ['TMLMicroservicePath'] + "/logfiles"

         if not os.path.exists(tml_log_path):
            os.makedirs(tml_log_path)

         log_file = "%s/%s.log" % (tml_log_path, service_name)

         # Sets the threshold for this logger to lvl. Logging messages which are less severe than lvl will be ignored.
         ServiceLogger.service_logger.setLevel(log_lv)

         # Sets rotation parameters of disk log files
         # https://docs.python.org/3.4/library/logging.handlers.html#rotatingfilehandler
         handler = logging.handlers.RotatingFileHandler(log_file, maxBytes=10485760, backupCount=2)

         # Sets format of record in log file
         formatter = logging.Formatter('%(asctime)s - %(levelname)-8s %(message)s', '%d-%m-%Y %H:%M:%S')
         handler.setFormatter(formatter)

         # Adds the specified handler to logger "MyLogger"
         ServiceLogger.service_logger.addHandler(handler)

   def __del__(self):
      pass

   @staticmethod
   def get_config_file(skip=0):
      """
      Get a name of a caller in the format module.class.method
      Args:
         skip: specifies how many levels of stack to skip while getting caller
         name. skip=1 means "who calls me", skip=2 "who calls my caller" etc.

      Returns:
         An empty string is returned if skipped levels exceed stack height
      """
      res = os.environ.get('TMLAutoTestPath') + "/external_tools/MicroservicesSystem/loggerConfig.ini"
      stack = inspect.stack()
      start = 0 + skip
      if len(stack) < start + 1:
         return ''
      parent_frame = stack[start][0]
      module = inspect.getmodule(parent_frame)
      if module:
         dir_path = os.path.dirname(os.path.abspath(module.__file__))
         config_file = dir_path + "/loggerConfig.ini"
         if os.path.exists(config_file):
            res = config_file
         elif not os.path.exists(res):
            res = None

      del parent_frame
      return res

   @staticmethod
   def log(msg):
      ServiceLogger.log_info(msg)

   @staticmethod
   def log_info(msg):
      if ServiceLogger.service_logger:
         ServiceLogger.service_logger.info("[%s] %s" % (Utils.caller_name(), msg))

   @staticmethod
   def log_debug(msg):
      if ServiceLogger.service_logger:
         ServiceLogger.service_logger.debug("[%s] %s" % (Utils.caller_name(), msg))

   @staticmethod
   def log_warning(msg):
      if ServiceLogger.service_logger:
         ServiceLogger.service_logger.warning("[%s] %s" % (Utils.caller_name(), msg))

   @staticmethod
   def log_error(msg):
      if ServiceLogger.service_logger:
         ServiceLogger.service_logger.error("[%s] %s" % (Utils.caller_name(), msg))

   @staticmethod
   def log_critical(msg):
      if ServiceLogger.service_logger:
         ServiceLogger.service_logger.critical("[%s] %s" % (Utils.caller_name(), msg))


if __name__ == '__main__':
   ServiceLogger().set_logger("test")
   ServiceLogger().log("test")