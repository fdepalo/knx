import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID
from . import knx_tp_ns, KNXTPComponent, const

DEPENDENCIES = ["knx_tp"]

KNXSwitch = knx_tp_ns.class_("KNXSwitch", switch.Switch, cg.Component)

CONF_KNX_ID = "knx_id"

CONFIG_SCHEMA = switch.switch_schema(KNXSwitch).extend({
    cv.GenerateID(): cv.declare_id(KNXSwitch),
    cv.GenerateID(CONF_KNX_ID): cv.use_id(KNXTPComponent),
    cv.Required(const.CONF_COMMAND_GA): cv.string,
    cv.Optional(const.CONF_STATE_GA): cv.string,
    cv.Optional(const.CONF_INVERT, default=False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    """Generate code for KNX Switch."""
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await switch.register_switch(var, config)
    
    # Get the KNX component
    knx = await cg.get_variable(config[CONF_KNX_ID])
    cg.add(var.set_knx_component(knx))
    
    # Set the command group address
    cg.add(var.set_command_ga_id(config[const.CONF_COMMAND_GA]))
    
    # Optional state feedback group address
    if const.CONF_STATE_GA in config:
        cg.add(var.set_state_ga_id(config[const.CONF_STATE_GA]))
    
    # Optional invert
    if const.CONF_INVERT in config:
        cg.add(var.set_invert(config[const.CONF_INVERT]))
