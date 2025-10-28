import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_ID, CONF_OUTPUT_ID
from . import knx_tp_ns, KNXTPComponent, const
DEPENDENCIES = ["knx_tp"]
KNXLight = knx_tp_ns.class_("KNXLight", light.LightOutput, cg.Component)
CONFIG_SCHEMA = light.BRIGHTNESS_ONLY_LIGHT_SCHEMA.extend({
    cv.GenerateID(CONF_OUTPUT_ID): cv.declare_id(KNXLight),
    cv.GenerateID("knx_id"): cv.use_id(KNXTPComponent),
    cv.Required(const.CONF_SWITCH_GA): cv.string,
    cv.Optional(const.CONF_BRIGHTNESS_GA): cv.string,
    cv.Optional(const.CONF_STATE_GA): cv.string,
}).extend(cv.COMPONENT_SCHEMA)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_OUTPUT_ID])
    await cg.register_component(var, config)
    await light.register_light(var, config)
    knx = await cg.get_variable(config["knx_id"])
    cg.add(var.set_knx_component(knx))
    cg.add(var.set_switch_ga_id(config[const.CONF_SWITCH_GA]))
    if const.CONF_BRIGHTNESS_GA in config: cg.add(var.set_brightness_ga_id(config[const.CONF_BRIGHTNESS_GA]))
    if const.CONF_STATE_GA in config: cg.add(var.set_state_ga_id(config[const.CONF_STATE_GA]))