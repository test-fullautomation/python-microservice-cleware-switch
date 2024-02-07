/**
 * @file ServiceClient.h
 * @brief Definition of the ServiceClient class.
 * @author Nguyen Huynh Tri Cuong (MS/EMC51-XC)
 * @date 30th Jan, 2024.
 */
#pragma once
#ifndef SERVICECLIENT_H
#define SERVICECLIENT_H
#include <random>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <string>
#include <functional>
#include <amqp.h>
#include <amqp_framing.h>
#include <amqp_tcp_socket.h>
#include <json/json.h>
#include "ServiceRequest.h"

using namespace std;

/**
 * @brief Class that represents a service client.
 *
 * This class contains information about a service and provides methods
 * to connect to, disconnect from, and send requests to the service.
*/
class ServiceClient
{
private:
	/**
	 * @brief The service name.
	*/
	string mServiceName;

	/**
	 * @brief The RabbitMQ broker port.
	*/
	int mPort;

	/**
	 * @brief The RabbitMQ broker host.
	*/
	string mHost;

    /**
     * @brief The routing key of the service.
    */
    string mRoutingKey;

	/**
	 * @brief The amqp connection object.
	*/
	amqp_connection_state_t mConn;

    /**
     * @brief The connection state of this service client.
    */
    bool mIsConnected;

public:
    /**
     * @brief The constant store the Exchange name for requesting service serving.
    */
    static const string SERVICE_REQUEST_EXCHANGE;

    /**
     * @brief Constructs a new ServiceClient object.
     *
     * @param serviceName The name of the service.
     * @param host The RabbitMQ broker host.
     * @param port The RabbitMQ broker port.
     * @param routingKey The routing key to access the service.
     */
	ServiceClient(string seviceName, string host, int port, string routingKey) {
		mPort = port;
		mHost = host;
		mServiceName = seviceName;
        mRoutingKey = routingKey;
        mIsConnected = false;
        mConn = NULL;
	}

    /**
     * @brief Connects to the service.
     *
     * @return 1 if connection is successful, otherwise an error code.
     */
    int connect();

    /**
     * @brief Disconnects from the service.
     */
    void disconnect();

    /**
     * @brief Generates a UUID (Universally Unique Identifier).
     *
     * @return A string representing the generated UUID.
     */
    static std::string GenerateUUID();

    /**
     * @brief Sends a request to the service and invokes the callback function with the response.
     *
     * @param request Pointer to the service request object.
     * @param callback Callback function to be invoked with the response.
     *                 The response will be passed as a parameter to the callback function.
     *                 The callback function should have the signature: void(const std::string&).
     *                 The response is represented as a string.
     *
     * @return 1 if the request is successfully sent, otherwise an error code.
     */
    int sendRequest(ServiceRequest* request, std::function<void(const std::string&)> callback);
    
};
#endif // SERVICECOMMUNICATOR_H
