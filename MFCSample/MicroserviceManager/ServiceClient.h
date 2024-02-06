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

class ServiceClient
{
private:
	string mServiceName;
	int mPort;
	string mHost;
    string mRoutingKey;
	amqp_connection_state_t mConn;
    bool mIsConnected;

public:
    static const string SERVICE_REQUEST_EXCHANGE;
	ServiceClient(string seviceName, string host, int port, string routingKey) {
		mPort = port;
		mHost = host;
		mServiceName = seviceName;
        mRoutingKey = routingKey;
        mIsConnected = false;
        mConn = NULL;
	}

    int connect();

    void disconnect();

    /**
     * @brief 
     * @return 
    */
    static std::string GenerateUUID();
    int sendRequest(ServiceRequest* request, std::function<void(const std::string&)> callback);
    
};
#endif // SERVICECOMMUNICATOR_H
