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
# File: ServiceRegistry.py
#
# Initially created by Nguyen Huynh Tri Cuong (RBVH/ECM51) / Nov 2023
#
# Description:
#   Provide the ServiceRegistry class which anage information for all services 
#   connected to the broker it is connected to.
#
# History:
#
# 24.11.2023 / V 0.1 / Nguyen Huynh Tri Cuong (RBVH/ECM51)
# - Initialize
#
# *******************************************************************************
from ServiceBase import ServiceBase, ResultType, ResponseMessage
from ClewareAccessHelper import ClewareAccessHelper
import time
import pika
import json
import sys
from signal import *


class ServiceCleware(ServiceBase):
   """
Service for controlling Cleware devices.

This class extends ServiceBase to provide functionalities specific to managing and controlling Cleware devices.
   """
   _SERVICE_INFO = {
      'name': 'ServiceCleware',
      'group': 'Switch Boxes',
      'description': 'Service to control Cleware switch box devices.',
      'shortdesc': 'Cleware Controllers',
      'version': '1.0.0',
      'routing_key': 'ServiceClewareKey',
      'tag': '',
      'gui_support': True,
      # Other details
      'methods': []
   }

   def __init__(self, cmd_args=None):
      """
Constructor for the ServiceCleware class.

**Arguments:**

* ``cmd_args``

  / *Condition*: optional / *Type*: list / *Default*: [] /

  Command-line arguments for initializing the ServiceCleware service.

**Returns:**

(*no returns*)
      """
      super(ServiceCleware, self).__init__(cmd_args)
      self.cleware_helper = ClewareAccessHelper()

   def svc_api_get_all_devices_state(self):
      """
Retrieve the state of all Cleware devices.

**Returns:**

  / *Type*: dict /

  A dictionary containing the states of all Cleware devices.
      """
      return self.cleware_helper.get_all_devices_state()

   def svc_api_set_switch(self, device_no, switch_id, state):
      """
Set state for a Cleware device's switch.

**Arguments:**

* ``device_no``

  / *Condition*: required / *Type*: str /

  Cleware device's number.

* ``switch_id``

  / *Condition*: required / *Type*: str /

  Switch number to turn on or off.

* ``state``

  / *Condition*: required / *Type*: str /

  State of swith to set (on/off).

**Returns:**

  / *Type*: int /

  Return ret code, 1 for succeed, 0 for failure.
      """
      ret = self.cleware_helper.set_switch(device_no, switch_id, state)
      self.notify_updates()
      return ret

   def notify_updates(self):
      """
Notify updates to the realtime update channel for Cleware devices.

**Returns:**

(*no returns*)
      """
      connection = pika.BlockingConnection(pika.ConnectionParameters(**self._kw_args))
      channel = connection.channel()

      # Declare an exchange (use 'topic' type for flexible routing)
      exchange_name = 'updates_sw_state'
      channel.exchange_declare(exchange=exchange_name, exchange_type='fanout')

      # Publish updates to the 'updates' topic
      time.sleep(0.05)
      update_info = json.dumps(self.cleware_helper.get_all_devices_state())
      channel.basic_publish(exchange=exchange_name, routing_key='', body=update_info)

      print("Sent to RabbitMQ update info :%s" % update_info)
      connection.close()


def signal_handler(sig, frame, obj):
   """
Handle signals from the operating system.

**Arguments:**

* ``sig``

  / *Condition*: required / *Type*: int /

  The signal number received from the OS.

* ``frame``

  / *Condition*: required / *Type*: frame object /

  The current stack frame.

* ``obj``

  / *Condition*: required / *Type*: object /

  The object that is handling the signal.

**Returns:**

(*no returns*)
   """
   # This function will be called when a SIGINT signal (Ctrl+C) is received
   print("Ctrl+C pressed - Cleaning up...")
   # Perform any necessary cleanup here
   # For example, call the cleanup method of the object
   del obj
   # Exit the program
   exit(0)


if __name__ == '__main__':
   svc = ServiceCleware(sys.argv[1:])

   # Register the signal handler for SIGINT (Ctrl+C)
   for sign in (SIGABRT, SIGILL, SIGINT, SIGSEGV, SIGTERM):
      signal(sign, lambda sig, frame: signal_handler(sig, frame, svc))

   svc.serve()
