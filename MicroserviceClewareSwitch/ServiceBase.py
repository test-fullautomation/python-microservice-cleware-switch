import abc
import pika
import json
import collections
import zipfile
import os
import base64


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


class ServiceBase:
   _SERVICE_INFO = {
      'name': 'ServiceBase',
      'description': 'A template for services.',
      'shortdesc': '',
      'group': '',
      'tag': '',
      'version': '1.0.0',
      'routing_key': '',
      'gui_support': False,
      # Other details
      'methods': []
   }

   def __init__(self, **conn_params):
      self.connection = None
      self._kw_args = conn_params
      self.name = self._SERVICE_INFO['name']
      self._api_dict = self.get_svc_api_methods_dict()
      self._SERVICE_INFO['methods'] = list(self._api_dict.keys())
      self.connect_broker(**conn_params)
      self.register_service()

   def close(self):
      self.connection.close()

   def __del__(self):
      self.unregister_service()
      self.close()

   def connect_broker(self, **kwargs):
      self.connection = pika.BlockingConnection(pika.ConnectionParameters(**kwargs))

   def serve(self):
      channel = self.connection.channel()

      channel.exchange_declare(exchange='services_request', exchange_type='direct')
      channel.queue_declare(queue=self.name)

      # Purge the queue
      channel.queue_purge(queue=self.name)
      print(f"Queue '{self.name}' purged")

      # Bind the queue to the exchange with a routing key
      channel.queue_bind(exchange='services_request', queue=self.name, routing_key=self._SERVICE_INFO['routing_key'])

      channel.basic_qos(prefetch_count=1)
      channel.basic_consume(queue=self.name, on_message_callback=self.on_request)

      print(" [x] Awaiting RPC requests")
      channel.start_consuming()

   def register_service(self):
      exchange_name = 'service_information'
      channel = self.connection.channel()
      channel.exchange_declare(exchange=exchange_name, exchange_type='topic')

      service_info = {
         'info': self._SERVICE_INFO,
         'state': 'on'
      }

      # Ensure the queue is durable and named to be reused
      queue_name = 'service_infor_queue'
      channel.queue_declare(queue=queue_name, durable=True)

      # Bind the queue to specific routing keys
      channel.queue_bind(exchange=exchange_name, queue=queue_name, routing_key='service.information')

      channel.basic_publish(
         exchange=exchange_name,
         routing_key='service.information',
         body=json.dumps(service_info),
         properties=pika.BasicProperties(
            delivery_mode=2,  # Make message persistent
         )
      )

      print(" [x] Registered service to Registry Service")

   def unregister_service(self):
      exchange_name = 'service_information'
      channel = self.connection.channel()
      channel.exchange_declare(exchange=exchange_name, exchange_type='topic')

      service_info = {
         'info': self._SERVICE_INFO,
         'state': 'off'
      }

      # Ensure the queue is durable and named to be reused
      queue_name = 'service_infor_queue'
      channel.queue_declare(queue=queue_name, durable=True)

      # Bind the queue to specific routing keys
      channel.queue_bind(exchange=exchange_name, queue=queue_name, routing_key='service.information')

      channel.basic_publish(
         exchange=exchange_name,
         routing_key='service.information',
         body=json.dumps(service_info),
         properties=pika.BasicProperties(
            delivery_mode=2,  # Make message persistent
         )
      )

      print(" [x] Unregistered service from Registry Service")

   def get_svc_api_methods_dict(self):
      methods = {method: getattr(self, method) for method in dir(self) if method.startswith('svc_api') and callable(getattr(self, method))}
      if not self._SERVICE_INFO['gui_support']:
         del methods['svc_api_get_gui_files']
      return methods

   def svc_api_get_version(self):
      return self._SERVICE_INFO['version']

   def svc_api_get_gui_files(self):
      # Compress the files into a ZIP file
      file_content = None
      if self._SERVICE_INFO['gui_support']:
         zip_file_path = 'files.zip'
         with zipfile.ZipFile(zip_file_path, 'w') as zipf:
            for root, dirs, files in os.walk('GUIs'):
               for file in files:
                  zipf.write(os.path.join(root, file), os.path.relpath(os.path.join(root, file), 'GUIs'))

         with open(zip_file_path, 'rb') as file:
            file_content = file.read()

         os.remove(zip_file_path)

      return file_content

   def on_request(self, ch, method, props, body):
      """

      :param ch:
      :param method:
      :param props:
      :param body:
      :return:
      """
      response = "Non-supported request"
      result_type = ResultType.FAIL
      request_api = ""
      try:
         if isinstance(body, bytes):
            body = json.loads(body.decode('utf-8'))

         request_api = body['method']
         if body['method'] in self._api_dict:
            if body['args'] is None:
               response = self._api_dict[body['method']]()
            else:
               response = self._api_dict[body['method']](*body['args'])
            result_type = ResultType.PASS
      except Exception as ex:
         result_type = ResultType.EXCEPT
         response = str(ex)

      if isinstance(response, bytes):
         # Convert bytes data to base64 encoded string
         response = base64.b64encode(response).decode('utf-8')

      resp = ResponseMessage(request_api, result_type, response)
      # print(props.reply_to)
      ch.basic_publish(exchange='',
                       routing_key=props.reply_to,
                       properties=pika.BasicProperties(correlation_id=props.correlation_id),
                       body=resp.get_json())
      ch.basic_ack(delivery_tag=method.delivery_tag)


if __name__ == '__main__':
   svc = ServiceBase(host='localhost')
   svc.serve()
