import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID, CONF_MIN_VALUE, CONF_MAX_VALUE, CONF_STEP
from . import knx_ip_ns, KNXIPComponent, const
DEPENDENCIES = ["knx_ip"]
KNXNumber = knx_ip_ns.class_("KNXNumber", number.Number, cg.Component)
CONFIG_SCHEMA = number.number_schema(KNXNumber).extend({
    cv.GenerateID(): cv.declare_id(KNXNumber),
    cv.GenerateID("knx_id"): cv.use_id(KNXIPComponent),
    cv.Required(const.CONF_COMMAND_GA): cv.string,
    cv.Optional(const.CONF_STATE_GA): cv.string,
}).extend(cv.COMPONENT_SCHEMA)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # Use values from config or defaults
    min_val = config.get(CONF_MIN_VALUE, 0)
    max_val = config.get(CONF_MAX_VALUE, 100)
    step_val = config.get(CONF_STEP, 1)
    await number.register_number(var, config, min_value=min_val, max_value=max_val, step=step_val)
    knx = await cg.get_variable(config["knx_id"])
    cg.add(var.set_knx_component(knx))
    cg.add(var.set_command_ga_id(config[const.CONF_COMMAND_GA]))
    if const.CONF_STATE_GA in config: cg.add(var.set_state_ga_id(config[const.CONF_STATE_GA]))