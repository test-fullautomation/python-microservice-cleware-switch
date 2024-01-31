#include "pch.h"
#include "ServiceClient.h"

const string ServiceClient::SERVICE_REQUEST_EXCHANGE = "services_request";

/**
 * @brief 
 * 
 * @return 
*/
string ServiceClient::GenerateUUID()
{
    // Use a random device and a uniform distribution to generate random bytes
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    // Generate 16 random bytes (UUID size)
    std::vector<uint8_t> bytes(16);
    for (int i = 0; i < 16; ++i) {
        bytes[i] = static_cast<uint8_t>(dis(gen));
    }

    // Set the version (4) and variant (2) bits as required by UUID specification
    bytes[6] = (bytes[6] & 0x0F) | 0x40; // version 4
    bytes[8] = (bytes[8] & 0x3F) | 0x80; // variant 2

    // Format the bytes as a string in the standard UUID format
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0');
    for (const auto& byte : bytes) {
        ss << std::setw(2) << static_cast<int>(byte);
    }

    return ss.str();
}

int ServiceClient::RequestService(std::string jsonData, std::function<void(const std::string&)> callback) {
    amqp_connection_state_t conn = amqp_new_connection();
    amqp_socket_t* socket = amqp_tcp_socket_new(conn);
    if (!socket) {
        // Handle socket creation failure
        std::cerr << "Error creating TCP socket" << std::endl;
        return 0;  // or handle the error appropriately
    }

    ////amqp_socket_open(socket, mHost.c_str(), mPort);
    //int status = amqp_socket_open(socket, mHost.c_str(), mPort);
    int status = amqp_socket_open(socket, "localhost", 5672);
    if (status != AMQP_STATUS_OK) {
        // Handle socket open failure
        std::cerr << "Error opening socket: " << amqp_error_string2(status) << std::endl;
        return -1;  // or handle the error appropriately
    }
    amqp_login(conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN,
        "guest", "guest");

    amqp_channel_open(conn, 1);
    //amqp_channel_close(conn, 1, AMQP_REPLY_SUCCESS);
    amqp_rpc_reply_t reply = amqp_get_rpc_reply(conn);
    if (reply.reply_type == AMQP_RESPONSE_LIBRARY_EXCEPTION) {
        string test(amqp_error_string2(reply.library_error));
        printf("Exception: %s", amqp_error_string2(reply.library_error));
        std::cerr << "Library exception: " << amqp_error_string2(reply.library_error) << std::endl;
    }
    else {
        std::cerr << "Other error type: " << reply.reply_type << std::endl;
    }

    // Declare an exclusive queue
    amqp_queue_declare_ok_t_* tmp_queue = amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 1, 1, amqp_empty_table);

    // Get the queue name
    amqp_bytes_t queueName = tmp_queue->queue;//amqp_queue_declare(conn, 1, amqp_empty_bytes, 0, 0, 1, 0, amqp_empty_table)->queue;

    string correlationId = GenerateUUID();

    std::cout << " [x] Requesting Cleware Service with data: " << jsonData << std::endl;

    // Consume the response from the exclusive queue
    amqp_basic_consume(conn, 1, queueName, amqp_empty_bytes, 0, 1, 0, amqp_empty_table);
    // Prepare the message properties
    amqp_basic_properties_t properties;
    properties._flags = AMQP_BASIC_REPLY_TO_FLAG | AMQP_BASIC_CORRELATION_ID_FLAG;
    properties.reply_to = queueName;
    const char* correlationIdString = correlationId.c_str();
    properties.correlation_id = amqp_cstring_bytes(correlationIdString);
    // Send the request to the service using routing key 
    amqp_basic_publish( conn, 
                        1, 
                        amqp_cstring_bytes(SERVICE_REQUEST_EXCHANGE.c_str()), 
                        amqp_cstring_bytes(mRoutingKey.c_str()), 
                        0,  
                        0, 
                        &properties,
                        amqp_cstring_bytes(jsonData.c_str()));

    amqp_rpc_reply_t res;
    amqp_envelope_t envelope;

    // Consume a message
    res = amqp_consume_message(conn, &envelope, NULL, 0);

    // Check if the message was consumed successfully
    if (res.reply_type == AMQP_RESPONSE_NORMAL) {
        // Access the message properties
        amqp_basic_properties_t* properties = &envelope.message.properties;

        // Check if properties are available
        if (properties != NULL) {
            // Access the correlation_id
            std::string correlationId((char*)properties->correlation_id.bytes, properties->correlation_id.len);

            // Check if the correlation_id matches
            if (correlationId == correlationId) {
                // Access the message body
                std::string result((char*)envelope.message.body.bytes, envelope.message.body.len);

                // Handle the result
                std::cout << " [.] Got response: " << result << std::endl;
                callback(result);
            }
        }

        // Destroy the envelope
        amqp_destroy_envelope(&envelope);

        // Close the connection after processing the message
        amqp_connection_close(conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(conn);
    }
    else {
        // Handle the error (res.reply_type contains the error type)
        std::cerr << "Error consuming message: " << res.library_error << std::endl;
    }

    // Continue with the rest of your code...

    return 1;
}