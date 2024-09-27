*** Settings ***
Library    QConnectBase.ConnectionManager

*** Variables ***
# Modify to adapt your connection
${CONNECTION_NAME}    TEST_CONN
${DEVICE_ID}          710925                # Cleware device's ID
${HOST}               localhost             # Broker host
${PORT}               5672                  # Broker port
${ROUTING_KEY}        ServiceClewareKey     # Service's routing key

*** Test Cases ***

Test Microservice Connection
   Log To Console    Test alias

   ${config_string}=    catenate  
   ...  {\n
   ...            "address": "${HOST}", \n 
   ...            "port": "${PORT}",\n
   ...            "routing_key": "${ROUTING_KEY}"\n
   ...  }\n

   ${config}=             evaluate        json.loads('''${config_string}''')    json
   Connect             conn_name=${CONNECTION_NAME}
   ...                 conn_type=RabbitmqClient
   ...                 conn_conf=${config}

   ${res}=     Verify        conn_name=${CONNECTION_NAME}
   ...                       send_cmd={ "method": "svc_api_set_switch","args": ["${DEVICE_ID}","16", "on"] }
   ...                       search_pattern=(.*)
   ...                       timeout=30
   
   Log To Console    ${res}[1]

*** Keyword ***
Close Connection
    disconnect  ${CONNECTION_NAME}