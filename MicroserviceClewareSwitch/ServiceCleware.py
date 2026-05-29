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
# File: ServiceCleware.py
#
# Initially created by Nguyen Huynh Tri Cuong (RBVH/ECM51) / Nov 2023
#
# Description:
#   Provide the ServiceCleware class which controls Cleware switch box devices.
#
# History:
#
# 24.11.2023 / V 0.1 / Nguyen Huynh Tri Cuong (RBVH/ECM51)
# - Initialize
#
# *******************************************************************************
from MicroserviceBase import ServiceBase
from .ClewareAccessHelper import ClewareAccessHelper
import time
import json


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
      'methods': [],
      'sample_path': 'resource/sample.robot'
   }

   def __init__(self, transport=None, registry=None):
      """
Constructor for the ServiceCleware class.

**Arguments:**

* ``transport``

  / *Condition*: optional / *Type*: TransportPort /

  A TransportPort implementation for message transport.

* ``registry``

  / *Condition*: optional / *Type*: ServiceRegistryPort /

  A ServiceRegistryPort implementation for service registration.

**Returns:**

(*no returns*)
      """
      super().__init__(transport=transport, registry=registry)
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
      exchange_name = 'updates_sw_state'

      # Declare the fanout exchange via the transport's underlying connection
      channel = self._transport.connection.channel()
      channel.exchange_declare(exchange=exchange_name, exchange_type='fanout')
      channel.close()

      time.sleep(0.05)
      update_info = json.dumps(self.cleware_helper.get_all_devices_state())
      self._transport.publish(exchange=exchange_name, routing_key='', body=update_info)

      print("Sent to RabbitMQ update info :%s" % update_info)
