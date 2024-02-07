/**
 * @file ServiceRequest.h
 * @brief Definition of the ServiceRequest class and its concrete subclasses.
 * @author Nguyen Huynh Tri Cuong (MS/EMC51-XC)
 * @date 30th Jan, 2024
 */
#pragma once
#ifndef SERVICEREQUEST_H
#define SERVICEREQUEST_H

#include <string>
#include <stdarg.h>
#include <json/json.h>

/**
 * @brief Abstract class for service requests.
 *
 * This class represents a generic service request. Concrete subclasses should
 * inherit from this class and implement the getRequestJson() method to generate
 * the request data in JSON format.
 */
class ServiceRequest
{
public:
	/**
	 * @brief Constructs a new ServiceRequest object.
	 *
	 * This is the default constructor for the ServiceRequest class.
	 * 
	 */
	ServiceRequest() {};

	/**
	 * @brief Destroys the ServiceRequest object.
	 *
	 * This is the destructor for the ServiceRequest class.
	 *
	 */
	~ServiceRequest() {};

	/**
	 * @brief Converts the service request object to JSON format.
	 *
	 * This method converts the service request object to JSON format.
	 * Subclasses should override this method to provide specific JSON representation
	 * for the corresponding service request.
	 *
	 * @return The service request data in JSON format.
	 */
	virtual Json::Value toJson ()
	{
		Json::Value jsonObject;
		jsonObject[METHOD] = GetServiceApi();
		jsonObject[ARGS] = getArguments();
		return jsonObject;
	}

	/**
	 * @brief Gets the API endpoint for the service request.
	 *
	 * This method returns the API endpoint for the service request.
	 * Subclasses should override this method to provide the specific
	 * API endpoint for their corresponding service requests.
	 *
	 * @return The API endpoint for the service request.
	 */
	virtual std::string GetServiceApi()
	{
		return "Non-Supported Request";
	}

	/**
	 * @brief Converts the service request object to a string representation.
	 *
	 * This method converts the service request object to a string representation.
	 * Subclasses should override this method to provide specific string representation
	 * for the corresponding service request.
	 *
	 * @return The string representation of the service request.
	 */
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

	/**
	 * @brief Gets the arguments for the service API.
	 *
	 * This method returns the arguments for the service API.
	 * Subclasses should override this method to provide the specific
	 * arguments for their corresponding service requests.
	 *
	 * @return The arguments for the service API in JSON format.
	 */
	virtual Json::Value getArguments()
	{
		Json::Value args = Json::nullValue;
		return args;
	}
};


/**
 * @brief Class representing a request to get all device states.
 *
 * This class represents a request to get the state of all devices.
 * It inherits from the ServiceRequest class and provides implementation
 * for generating the request data specific to this request.
 */
class GetAllDeviceStateRequest : public ServiceRequest
{
public:
	/**
     * @brief Gets the API endpoint for the service request.
     * 
     * Overrides the base class method to provide the specific
     * API endpoint for the GetAllDeviceStateRequest.
     * 
     * @return The API endpoint for the service request.
     */
	std::string GetServiceApi()
	{
		return "svc_api_get_all_devices_state";
	}
protected:
	std::string SERVICE_REQUEST_API = "svc_api_get_all_devices_state";

	/**
	 * @brief Gets the arguments for the service API.
	 *
	 * Overrides the base class method to provide the specific
	 * arguments for the GetAllDeviceStateRequest.
	 *
	 * @return The arguments for the service API in JSON format.
	 */
	Json::Value getArguments()
	{
		Json::Value args = Json::nullValue;
		return args;
	}
};


/**
 * @brief Class representing a request to set switch state.
 *
 * This class represents a request to set the state of a switch.
 * It inherits from the ServiceRequest class and provides implementation
 * for generating the request data specific to this request.
 */
class SetSwitchRequest : public ServiceRequest
{
public:
	/**
	 * @brief Enum class representing the state of a switch.
	 */
	enum class SWITCH_STATE
	{
		ON,	/**< Switch is in the ON state. */
		OFF /**< Switch is in the OFF state. */
	};

	/**
	 * @brief Constructs a new SetSwitchRequest object.
	 *
	 * @param device The number of the device.
	 * @param switchNum The switch number.
	 * @param state The state of the switch (ON or OFF).
	 */
	SetSwitchRequest(std::string device, int iSwitchNum, SWITCH_STATE state) : m_sDevice(device), m_iSwitchNum(iSwitchNum), m_iState(state) {};

	/**
	 * @brief Gets the API endpoint for the service request.
	 *
	 * Overrides the base class method to provide the specific
	 * API endpoint for the GetAllDeviceStateRequest.
	 *
	 * @return The API endpoint for the service request.
	 */
	std::string GetServiceApi()
	{
		return "svc_api_set_switch";
	}

private:
	std::string m_sDevice;	// The serial number of the device
	int m_iSwitchNum;		// The switch number
	SWITCH_STATE m_iState;	// The state of the switch

protected:
	std::string SERVICE_REQUEST_API = "svc_api_set_switch";

	/**
	 * @brief Gets the arguments for the service API.
	 *
	 * Overrides the base class method to provide the specific
	 * arguments for the GetAllDeviceStateRequest.
	 *
	 * @return The arguments for the service API in JSON format.
	 */
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