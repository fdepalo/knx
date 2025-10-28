import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover
from esphome.const import CONF_ID
from . import knx_ip_ns, KNXIPComponent, const
DEPENDENCIES = ["knx_ip"]
KNXCover = knx_ip_ns.class_("KNXCover", cover.Cover, cg.Component)
CONFIG_SCHEMA = cover.cover_schema(KNXCover).extend({
    cv.GenerateID(): cv.declare_id(KNXCover),
    cv.GenerateID("knx_id"): cv.use_id(KNXIPComponent),
    cv.Required(const.CONF_MOVE_GA): cv.string,
    cv.Optional(const.CONF_POSITION_GA): cv.string,
    cv.Optional(const.CONF_STOP_GA): cv.string,
}).extend(cv.COMPONENT_SCHEMA)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)
    knx = await cg.get_variable(config["knx_id"])
    cg.add(var.set_knx_component(knx))
    cg.add(var.set_move_ga_id(config[const.CONF_MOVE_GA]))
    if const.CONF_POSITION_GA in config: cg.add(var.set_position_ga_id(config[const.CONF_POSITION_GA]))
    if const.CONF_STOP_GA in config: cg.add(var.set_stop_ga_id(config[const.CONF_STOP_GA]))