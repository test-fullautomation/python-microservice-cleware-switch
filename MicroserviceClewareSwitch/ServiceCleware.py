from ServiceBase import ServiceBase, ResultType, ResponseMessage
from ClewareAccessHelper import ClewareAccessHelper
import time
import pika
import json
from signal import *


class ServiceCleware(ServiceBase):
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

   def __init__(self, **conn_params):
      super(ServiceCleware, self).__init__(**conn_params)
      self.cleware_helper = ClewareAccessHelper()

   def svc_api_get_all_devices_state(self):
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
   # This function will be called when a SIGINT signal (Ctrl+C) is received
   print("Ctrl+C pressed - Cleaning up...")
   # Perform any necessary cleanup here
   # For example, call the cleanup method of the object
   del obj
   # Exit the program
   exit(0)


if __name__ == '__main__':
   svc = ServiceCleware(host='localhost')

   # Register the signal handler for SIGINT (Ctrl+C)
   for sign in (SIGABRT, SIGILL, SIGINT, SIGSEGV, SIGTERM):
      signal(sign, lambda sig, frame: signal_handler(sig, frame, svc))

   svc.serve()
