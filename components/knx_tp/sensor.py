import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID, CONF_TYPE
from . import knx_tp_ns, KNXTPComponent, const

DEPENDENCIES = ["knx_tp"]

KNXSensor = knx_tp_ns.class_("KNXSensor", sensor.Sensor, cg.Component)
KNXSensorType = knx_tp_ns.enum("KNXSensorType")

SENSOR_TYPES = {
    "temperature": KNXSensorType.KNX_SENSOR_TYPE_TEMPERATURE,
    "humidity": KNXSensorType.KNX_SENSOR_TYPE_HUMIDITY,
    "brightness": KNXSensorType.KNX_SENSOR_TYPE_BRIGHTNESS,
    "pressure": KNXSensorType.KNX_SENSOR_TYPE_PRESSURE,
    "percentage": KNXSensorType.KNX_SENSOR_TYPE_PERCENTAGE,
    "angle": KNXSensorType.KNX_SENSOR_TYPE_ANGLE,
    "generic_1byte": KNXSensorType.KNX_SENSOR_TYPE_GENERIC_1BYTE,
    "generic_2byte": KNXSensorType.KNX_SENSOR_TYPE_GENERIC_2BYTE,
    "generic_4byte": KNXSensorType.KNX_SENSOR_TYPE_GENERIC_4BYTE,
}

CONF_KNX_ID = "knx_id"

CONFIG_SCHEMA = sensor.sensor_schema(KNXSensor).extend({
    cv.GenerateID(): cv.declare_id(KNXSensor),
    cv.GenerateID(CONF_KNX_ID): cv.use_id(KNXTPComponent),
    cv.Required(const.CONF_STATE_GA): cv.string,
    cv.Optional(CONF_TYPE, default="generic_2byte"): cv.enum(SENSOR_TYPES, lower=True),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    """Generate code for KNX Sensor."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await sensor.register_sensor(var, config)
    
    # Get the KNX component
    knx = await cg.get_variable(config[CONF_KNX_ID])
    cg.add(var.set_knx_component(knx))
    
    # Set the group address and sensor type
    cg.add(var.set_state_ga_id(config[const.CONF_STATE_GA]))
    cg.add(var.set_sensor_type(config[CONF_TYPE]))
