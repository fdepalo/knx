"""KNX IP component for ESPHome."""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.core import CORE
from esphome.components import time
from . import const

CODEOWNERS = ["@fdepalo"]
DEPENDENCIES = ["network"]  # Requires WiFi or Ethernet, not UART
AUTO_LOAD = ["binary_sensor", "switch", "sensor", "climate", "cover", "light", "text_sensor", "number"]

knx_ip_ns = cg.esphome_ns.namespace("knx_ip")
KNXIPComponent = knx_ip_ns.class_("KNXIPComponent", cg.Component)
GroupAddress = knx_ip_ns.class_("GroupAddress")

# Add library dependency for Thelsing KNX stack
# Same library as knx_tp, just different MASK_VERSION
cg.add_library("knx", None, "https://github.com/thelsing/knx.git#master")

# Define MASK_VERSION for IP devices
# 0x57B0 = IP device (vs 0x07B0 for TP)
cg.add_build_flag("-DMASK_VERSION=0x57B0")

# Include stub header for KNX library examples
import os
stub_header = os.path.join(os.path.dirname(__file__), "knx_stubs.h")
cg.add_build_flag(f'-include "{stub_header}"')

def validate_knx_address(value):
    """Validate KNX address format (e.g., 1.2.3 or 1/2/3)."""
    value = cv.string(value)
    value = value.replace("/", ".")
    parts = value.split(".")
    if len(parts) != 3:
        raise cv.Invalid("KNX address must have 3 parts (e.g., 1.2.3)")
    try:
        main = int(parts[0])
        middle = int(parts[1])
        sub = int(parts[2])
        if not (0 <= main <= 31 and 0 <= middle <= 7 and 0 <= sub <= 255):
            raise ValueError
    except ValueError:
        raise cv.Invalid("Invalid KNX address format")
    return f"{main}.{middle}.{sub}"

def validate_ip_address(value):
    """Validate IP address format."""
    # Simple validation - ESPHome has better validators we could use
    value = cv.string(value)
    parts = value.split(".")
    if len(parts) != 4:
        raise cv.Invalid("IP address must have 4 parts")
    try:
        for part in parts:
            num = int(part)
            if not (0 <= num <= 255):
                raise ValueError
    except ValueError:
        raise cv.Invalid("Invalid IP address format")
    return value

GROUP_ADDRESS_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(GroupAddress),
    cv.Required("address"): validate_knx_address,
})

CONF_TIME_ID = "time_id"

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.declare_id(KNXIPComponent),
        cv.Required(const.CONF_PHYSICAL_ADDRESS): validate_knx_address,
        cv.Optional(const.CONF_GROUP_ADDRESSES, default=[]): cv.ensure_list(GROUP_ADDRESS_SCHEMA),

        # IP-specific configuration
        cv.Optional(const.CONF_GATEWAY_IP): validate_ip_address,
        cv.Optional(const.CONF_GATEWAY_PORT, default=const.DEFAULT_GATEWAY_PORT): cv.port,
        cv.Optional(const.CONF_ROUTING_MODE, default=const.DEFAULT_ROUTING_MODE): cv.boolean,
        cv.Optional(const.CONF_MULTICAST_ADDRESS, default=const.DEFAULT_MULTICAST_ADDRESS): validate_ip_address,

        # Time broadcast (optional, like knx_tp)
        cv.Optional(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
        cv.Optional(const.CONF_TIME_BROADCAST_GA): cv.string,
        cv.Optional(const.CONF_TIME_BROADCAST_INTERVAL, default="60s"): cv.positive_time_period_milliseconds,
    })
    .extend(cv.COMPONENT_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    cg.add(var.set_physical_address(config[const.CONF_PHYSICAL_ADDRESS]))

    # IP-specific configuration
    if const.CONF_GATEWAY_IP in config:
        cg.add(var.set_gateway_ip(config[const.CONF_GATEWAY_IP]))

    cg.add(var.set_gateway_port(config[const.CONF_GATEWAY_PORT]))
    cg.add(var.set_routing_mode(config[const.CONF_ROUTING_MODE]))
    cg.add(var.set_multicast_address(config[const.CONF_MULTICAST_ADDRESS]))

    # Group addresses
    for ga_config in config[const.CONF_GROUP_ADDRESSES]:
        ga = cg.new_Pvariable(ga_config[CONF_ID])
        cg.add(ga.set_id(str(ga_config[CONF_ID].id)))
        cg.add(ga.set_address(ga_config["address"]))
        cg.add(var.register_group_address(ga))

    # Time broadcast configuration
    if CONF_TIME_ID in config:
        time_component = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time_source(time_component))

    if const.CONF_TIME_BROADCAST_GA in config:
        cg.add(var.set_time_broadcast_ga(config[const.CONF_TIME_BROADCAST_GA]))

    if const.CONF_TIME_BROADCAST_INTERVAL in config:
        cg.add(var.set_time_broadcast_interval(config[const.CONF_TIME_BROADCAST_INTERVAL]))
