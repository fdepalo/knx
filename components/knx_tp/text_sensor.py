import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_ID
from . import knx_tp_ns, KNXTPComponent, const

DEPENDENCIES = ["knx_tp"]

KNXTextSensor = knx_tp_ns.class_("KNXTextSensor", text_sensor.TextSensor, cg.Component)
TextSensorDPT = knx_tp_ns.enum("TextSensorDPT", is_class=True)

DPT_TYPE_16 = TextSensorDPT.DPT_16
DPT_TYPE_10 = TextSensorDPT.DPT_10
DPT_TYPE_11 = TextSensorDPT.DPT_11
DPT_TYPE_19 = TextSensorDPT.DPT_19

DPT_TYPES = {
    "16": DPT_TYPE_16,
    "10": DPT_TYPE_10,
    "11": DPT_TYPE_11,
    "19": DPT_TYPE_19,
}

CONFIG_SCHEMA = text_sensor.text_sensor_schema(KNXTextSensor).extend({
    cv.GenerateID(): cv.declare_id(KNXTextSensor),
    cv.GenerateID("knx_id"): cv.use_id(KNXTPComponent),
    cv.Required(const.CONF_STATE_GA): cv.string,
    cv.Optional(const.CONF_DPT_TYPE, default="16"): cv.enum(DPT_TYPES, upper=False),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)
    knx = await cg.get_variable(config["knx_id"])
    cg.add(var.set_knx_component(knx))
    cg.add(var.set_state_ga_id(config[const.CONF_STATE_GA]))
    cg.add(var.set_dpt_type(config[const.CONF_DPT_TYPE]))