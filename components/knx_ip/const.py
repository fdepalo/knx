"""Constants for KNX IP component."""

DOMAIN = "knx_ip"

# Configuration keys
CONF_PHYSICAL_ADDRESS = "physical_address"
CONF_GROUP_ADDRESSES = "group_addresses"
CONF_STATE_GA = "state_ga"
CONF_COMMAND_GA = "command_ga"
CONF_SETPOINT_GA = "setpoint_ga"
CONF_TEMPERATURE_GA = "temperature_ga"
CONF_MODE_GA = "mode_ga"
CONF_ACTION_GA = "action_ga"
CONF_PRESET_COMFORT_GA = "preset_comfort_ga"
CONF_PRESET_ECO_GA = "preset_eco_ga"
CONF_PRESET_AWAY_GA = "preset_away_ga"
CONF_PRESET_SLEEP_GA = "preset_sleep_ga"
CONF_POSITION_GA = "position_ga"
CONF_MOVE_GA = "move_ga"
CONF_STOP_GA = "stop_ga"
CONF_BRIGHTNESS_GA = "brightness_ga"
CONF_SWITCH_GA = "switch_ga"
CONF_INVERT = "invert"
CONF_AUTO_RESET_TIME = "auto_reset_time"
CONF_DPT_TYPE = "dpt_type"
CONF_TIME_BROADCAST_GA = "time_broadcast_ga"
CONF_TIME_BROADCAST_INTERVAL = "time_broadcast_interval"

# IP-specific configuration
CONF_GATEWAY_IP = "gateway_ip"
CONF_GATEWAY_PORT = "gateway_port"
CONF_ROUTING_MODE = "routing_mode"
CONF_MULTICAST_ADDRESS = "multicast_address"

# Default values
DEFAULT_GATEWAY_PORT = 3671
DEFAULT_MULTICAST_ADDRESS = "224.0.23.12"
DEFAULT_ROUTING_MODE = True  # True = routing (multicast), False = tunneling

# DPT Types
DPT_1_001 = "1.001"  # Boolean
DPT_5_001 = "5.001"  # Unsigned 8-bit (0-255)
DPT_5_004 = "5.004"  # Percentage (0-100%)
DPT_9_001 = "9.001"  # Temperature (°C)
DPT_9_004 = "9.004"  # Illuminance (lux)
DPT_9_007 = "9.007"  # Humidity (%)
DPT_16_001 = "16.001"  # Character string
