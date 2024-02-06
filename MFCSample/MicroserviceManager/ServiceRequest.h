#pragma once
#ifndef SERVICEREQUEST_H
#define SERVICEREQUEST_H

#include <string>
#include <stdarg.h>
#include <json/json.h>

class  ServiceRequest
{
public:
	ServiceRequest() {};
	~ServiceRequest() {};

	std::string formatString(const char* format, ...)
	{
		const int MAX_BUFF = 1024;
		char buffer[MAX_BUFF] = { 0 };
		va_list argprt;
		va_start(argprt, format);
		if (vsnprintf(buffer, MAX_BUFF - 1, format, argprt) == -1)
			buffer[MAX_BUFF - 1] = 0;
		return buffer;
	}

	virtual Json::Value toJson ()
	{
		Json::Value jsonObject;
		jsonObject[METHOD] = GetServiceApi();
		jsonObject[ARGS] = getArguments();
		return jsonObject;
	}

	virtual std::string GetServiceApi()
	{
		return "Non-Supported Request";
	}

	virtual std::string toString()
	{
		Json::Value jsonObject = this->toJson();
		Json::StreamWriterBuilder builder;
		std::string jsonString = Json::writeString(builder, jsonObject);
		return jsonString;
	}

protected:
	std::string SERVICE_REQUEST_API = "Non-Supported Request";
	const std::string METHOD = "method";
	const std::string ARGS = "args";
	virtual Json::Value getArguments()
	{
		Json::Value args = Json::nullValue;
		return args;
	}
};

class GetAllDeviceStateRequest : public ServiceRequest
{
public:
	std::string GetServiceApi()
	{
		return "svc_api_get_all_devices_state";
	}
protected:
	std::string SERVICE_REQUEST_API = "svc_api_get_all_devices_state";
	Json::Value getArguments()
	{
		Json::Value args = Json::nullValue;
		return args;
	}
};

class SetSwitchRequest : public ServiceRequest
{
public:
	enum class SWITCH_STATE
	{
		ON,
		OFF
	};

	SetSwitchRequest(std::string device, int iSwitchNum, SWITCH_STATE state) : m_sDevice(device), m_iSwitchNum(iSwitchNum), m_iState(state) {};
	std::string GetServiceApi()
	{
		return "svc_api_set_switch";
	}

private:
	std::string m_sDevice;
	int m_iSwitchNum;
	SWITCH_STATE m_iState;

protected:
	std::string SERVICE_REQUEST_API = "svc_api_set_switch";
	Json::Value getArguments()
	{
		Json::Value args;
		args.append(m_sDevice.c_str());
		args.append(m_iSwitchNum);
		std::string state = m_iState == SWITCH_STATE::ON ? "on" : "off";
		args.append(state.c_str());
		return args;
	}
};

#endif // SERVICEREQUEST_H