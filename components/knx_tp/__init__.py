"""KNX TP component for ESPHome."""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_TRIGGER_ID
from esphome.core import CORE
from esphome.components import uart, time
from esphome import pins, automation
from . import const

CODEOWNERS = ["@fdepalo"]
DEPENDENCIES = ["uart"]
AUTO_LOAD = ["binary_sensor", "switch", "sensor", "climate", "cover", "light", "text_sensor", "number"]

knx_tp_ns = cg.esphome_ns.namespace("knx_tp")
KNXTPComponent = knx_tp_ns.class_("KNXTPComponent", cg.Component, uart.UARTDevice)
GroupAddress = knx_tp_ns.class_("GroupAddress")

# Automation triggers
TelegramTrigger = knx_tp_ns.class_(
    "TelegramTrigger",
    automation.Trigger.template(cg.std_string, cg.std_vector.template(cg.uint8))
)
GroupAddressTrigger = knx_tp_ns.class_(
    "GroupAddressTrigger",
    automation.Trigger.template(cg.std_vector.template(cg.uint8))
)

# Add library dependency for Thelsing KNX stack
# Use Git repository as PlatformIO registry doesn't have macOS ARM builds
cg.add_library("knx", None, "https://github.com/thelsing/knx.git#master")

# Define MASK_VERSION for the entire build
# 0x07B0 = BCU 1, TP1 (Twisted Pair with Bus Coupling Unit 1)
cg.add_build_flag("-DMASK_VERSION=0x07B0")

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

GROUP_ADDRESS_SCHEMA = cv.Schema({
    cv.Required(CONF_ID): cv.declare_id(GroupAddress),
    cv.Required("address"): validate_knx_address,
})

CONF_TIME_ID = "time_id"

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.declare_id(KNXTPComponent),
        cv.Required(const.CONF_PHYSICAL_ADDRESS): validate_knx_address,
        cv.Optional(const.CONF_GROUP_ADDRESSES, default=[]): cv.ensure_list(GROUP_ADDRESS_SCHEMA),
        cv.Optional(const.CONF_SAV_PIN): pins.gpio_input_pin_schema,
        cv.Optional(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
        cv.Optional(const.CONF_TIME_BROADCAST_GA): cv.string,
        cv.Optional(const.CONF_TIME_BROADCAST_INTERVAL, default="60s"): cv.positive_time_period_milliseconds,
        cv.Optional(const.CONF_ON_TELEGRAM): automation.validate_automation({
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(TelegramTrigger),
        }),
        cv.Optional(const.CONF_ON_GROUP_ADDRESS): cv.ensure_list(cv.Schema({
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(GroupAddressTrigger),
            cv.Required(const.CONF_ADDRESS): validate_knx_address,
            cv.Required(automation.CONF_THEN): automation.validate_automation(single=True),
        })),
    })
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_physical_address(config[const.CONF_PHYSICAL_ADDRESS]))

    for ga_config in config[const.CONF_GROUP_ADDRESSES]:
        ga = cg.new_Pvariable(ga_config[CONF_ID])
        cg.add(ga.set_id(str(ga_config[CONF_ID].id)))
        cg.add(ga.set_address(ga_config["address"]))
        cg.add(var.register_group_address(ga))

    # SAV pin configuration (BCU detection)
    if const.CONF_SAV_PIN in config:
        sav_pin = await cg.gpio_pin_expression(config[const.CONF_SAV_PIN])
        cg.add(var.set_sav_pin(sav_pin))

    # Time broadcast configuration
    if CONF_TIME_ID in config:
        time_source = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time_source(time_source))

    if const.CONF_TIME_BROADCAST_GA in config:
        cg.add(var.set_time_broadcast_ga(config[const.CONF_TIME_BROADCAST_GA]))

    if const.CONF_TIME_BROADCAST_INTERVAL in config:
        cg.add(var.set_time_broadcast_interval(config[const.CONF_TIME_BROADCAST_INTERVAL]))

    # Setup on_telegram triggers (generic, for all telegrams)
    for conf in config.get(const.CONF_ON_TELEGRAM, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID])
        # Add lambda that calls trigger when telegram arrives
        cg.add(var.add_on_telegram_callback(
            cg.RawExpression(f"[=](std::string ga, std::vector<uint8_t> data) {{ {trigger}->trigger(ga, data); }}")
        ))
        # Build automation actions
        await automation.build_automation(
            trigger,
            [(cg.std_string, "group_address"), (cg.std_vector.template(cg.uint8), "data")],
            conf,
        )

    # Setup on_group_address triggers (specific GA)
    for ga_conf in config.get(const.CONF_ON_GROUP_ADDRESS, []):
        ga_address = ga_conf[const.CONF_ADDRESS]
        # Convert address string to integer using the same function as C++
        parts = ga_address.split(".")
        area, line, device = int(parts[0]), int(parts[1]), int(parts[2])
        ga_int = ((area & 0x1F) << 11) | ((line & 0x07) << 8) | (device & 0xFF)

        trigger = cg.new_Pvariable(ga_conf[CONF_TRIGGER_ID])
        # Add lambda that calls trigger when GA matches
        cg.add(var.add_on_group_address_callback(
            ga_int,
            cg.RawExpression(f"[=](std::vector<uint8_t> data) {{ {trigger}->trigger(data); }}")
        ))
        # Build automation actions
        await automation.build_automation(
            trigger,
            [(cg.std_vector.template(cg.uint8), "data")],
            ga_conf[automation.CONF_THEN],
        )
