# *******************************************************************************
#
# File: ClewareAccessConstants.py
#
# Initially created by Cuong Nguyen (RBVH/ENG22) / July 2019
#
# Description:
#   Define constants for Cleware Access Service.
#
# History:
#
# 22.07.2019 / V 0.1 / Cuong Nguyen
# - Initialize
#
# *******************************************************************************

# Service's supported commands list

# Commands without parameter
GET_VERSION_CMD = 'GET VERSION'                 # Get service version command.
GET_DEVICES_STATE_CMD = 'GET DEVICES STATE'     # Get device state command.
GET_CONFIG_CMD = 'GET CONFIG'                   # Get config content command.

# Commands with parameters
SET_CONFIG_CMD = 'SET CONFIG'             # Set config content command.
SET_CONFIG_CMD_ARGS = '%s'
SET_CONFIG_CMD_ARGS_FILTER = "(.*)"       # Filter for getting parameters for 'Set config content' command.

GET_USB_TYPE_CMD = 'GET USB TYPE'         # Get Cleware box's type command.
GET_USB_TYPE_CMD_ARGS = '%s'
GET_USB_TYPE_CMD_ARGS_FILTER = "(.*)"     # Filter for getting parameters for 'Get Cleware box's type' command.

SET_SW_BY_PORT_NAME_CMD = 'DIGITAL_OUT CHANNEL STATE SET'            # Set cleware switch by config name command.
SET_SW_BY_PORT_NAME_CMD_ARGS = 'CH_NAME_ %s STATE_ %s'
SET_SW_BY_PORT_NAME_CMD_ARGS_FILTER = 'CH_NAME_.(.*).STATE_.(.*)'    # Filter for getting parameters for 'Set cleware switch by config name' command.
SET_SW_BY_PORT_NAME_CMD_ALIAS_FILTER = 'CH_NAME_.(.*).STATE_.*'

SET_SW_CMD = 'SET SWITCH'                                            # Set cleware switch command.
SET_SW_CMD_ARGS = 'DEVICE_ %s SWITCH_ %s STATE_ %s'
SET_SW_CMD_ARGS_FILTER = 'DEVICE_.(.*).SWITCH_.(.*).STATE_.(.*)'     # Filter for getting parameters for 'Set cleware switch' command.

GET_SW_CMD = 'GET SWITCH'                                            # Get cleware switch command.
GET_SW_CMD_ARGS = 'DEVICE_ %s SWITCH_ %s'
GET_SW_CMD_ARGS_FILTER = 'DEVICE_.(.*).SWITCH_.(.*)'                 # Filter for getting parameters for 'Get cleware switch' command.

