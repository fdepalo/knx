import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_ID
from . import knx_tp_ns, KNXTPComponent, const

DEPENDENCIES = ["knx_tp"]

KNXBinarySensor = knx_tp_ns.class_("KNXBinarySensor", binary_sensor.BinarySensor, cg.Component)

CONF_KNX_ID = "knx_id"

CONFIG_SCHEMA = binary_sensor.binary_sensor_schema(KNXBinarySensor).extend({
    cv.GenerateID(): cv.declare_id(KNXBinarySensor),
    cv.GenerateID(CONF_KNX_ID): cv.use_id(KNXTPComponent),
    cv.Required(const.CONF_STATE_GA): cv.string,
    cv.Optional(const.CONF_INVERT, default=False): cv.boolean,
    cv.Optional(const.CONF_AUTO_RESET_TIME): cv.positive_time_period_milliseconds,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    """Generate code for KNX Binary Sensor."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await binary_sensor.register_binary_sensor(var, config)
    
    # Get the KNX component
    knx = await cg.get_variable(config[CONF_KNX_ID])
    cg.add(var.set_knx_component(knx))
    
    # Set the group address ID
    cg.add(var.set_state_ga_id(config[const.CONF_STATE_GA]))
    
    # Optional parameters
    if const.CONF_INVERT in config:
        cg.add(var.set_invert(config[const.CONF_INVERT]))
    
    if const.CONF_AUTO_RESET_TIME in config:
        cg.add(var.set_auto_reset_time(config[const.CONF_AUTO_RESET_TIME]))
