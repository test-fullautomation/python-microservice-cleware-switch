#pragma once
#ifndef SERVICECLIENT_H
#define SERVICECOMMUNICATOR_H
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
using namespace std;

class ServiceClient
{
private:
	string mServiceName;
	int mPort;
	string mHost;
    string mRoutingKey;
	amqp_connection_state_t mConn;

public:
    static const string SERVICE_REQUEST_EXCHANGE;
	ServiceClient(string seviceName, string host, int port, string routingKey) {
		mPort = port;
		mHost = host;
		mServiceName = seviceName;
        mRoutingKey = routingKey;
	}

    /**
     * @brief 
     * @return 
    */
    static std::string GenerateUUID();

    int RequestService(std::string jsonData, std::function<void(const std::string&)> callback);
    
};
#endif // SERVICECOMMUNICATOR_H
