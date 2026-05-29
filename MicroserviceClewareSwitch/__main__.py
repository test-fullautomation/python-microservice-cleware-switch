#  Copyright 2020-2024 Robert Bosch GmbH
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
# *******************************************************************************
#
# File: __main__.py
#
# Description:
#   Entry point for running MicroserviceClewareSwitch as a package.
#   Usage: python -m MicroserviceClewareSwitch --host localhost --port 5672
#
# *******************************************************************************
import sys
from signal import signal, SIGABRT, SIGILL, SIGINT, SIGSEGV, SIGTERM

from MicroserviceBase import create_transport, create_registry
from .ServiceCleware import ServiceCleware


def main():
   transport = create_transport('rabbitmq', cmd_args=sys.argv[1:],
                                service_name='ServiceCleware')
   registry = create_registry('rabbitmq', cmd_args=sys.argv[1:],
                              service_name='ServiceCleware')
   service = ServiceCleware(transport=transport, registry=registry)

   for sig in (SIGABRT, SIGILL, SIGINT, SIGSEGV, SIGTERM):
      signal(sig, lambda s, f: signal_handler(s, f, service))

   try:
      service.register_service()
      service.serve()
   except KeyboardInterrupt:
      print(" [*] ServiceCleware interrupted.")
   finally:
      try:
         service.unregister_service()
      except Exception:
         pass
      try:
         service.close()
      except Exception:
         pass
      print(" [*] ServiceCleware stopped.")


def signal_handler(sig, frame, obj):
   print("Ctrl+C pressed - Cleaning up...")
   obj.unregister_service()
   obj.close()
   exit(0)


if __name__ == '__main__':
   main()
