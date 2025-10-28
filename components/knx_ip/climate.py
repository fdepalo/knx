import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate
from esphome.const import CONF_ID
from . import knx_ip_ns, KNXIPComponent, const

DEPENDENCIES = ["knx_ip"]
KNXClimate = knx_ip_ns.class_("KNXClimate", climate.Climate, cg.Component)

CONFIG_SCHEMA = climate.climate_schema(KNXClimate).extend({
    cv.GenerateID(): cv.declare_id(KNXClimate),
    cv.GenerateID("knx_id"): cv.use_id(KNXIPComponent),
    cv.Required(const.CONF_TEMPERATURE_GA): cv.string,
    cv.Required(const.CONF_SETPOINT_GA): cv.string,
    cv.Optional(const.CONF_MODE_GA): cv.string,
    cv.Optional(const.CONF_ACTION_GA): cv.string,
    cv.Optional(const.CONF_PRESET_COMFORT_GA): cv.string,
    cv.Optional(const.CONF_PRESET_ECO_GA): cv.string,
    cv.Optional(const.CONF_PRESET_AWAY_GA): cv.string,
    cv.Optional(const.CONF_PRESET_SLEEP_GA): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await climate.register_climate(var, config)
    knx = await cg.get_variable(config["knx_id"])
    cg.add(var.set_knx_component(knx))
    cg.add(var.set_temperature_ga_id(config[const.CONF_TEMPERATURE_GA]))
    cg.add(var.set_setpoint_ga_id(config[const.CONF_SETPOINT_GA]))
    if const.CONF_MODE_GA in config:
        cg.add(var.set_mode_ga_id(config[const.CONF_MODE_GA]))
    if const.CONF_ACTION_GA in config:
        cg.add(var.set_action_ga_id(config[const.CONF_ACTION_GA]))
    if const.CONF_PRESET_COMFORT_GA in config:
        cg.add(var.set_preset_comfort_ga_id(config[const.CONF_PRESET_COMFORT_GA]))
    if const.CONF_PRESET_ECO_GA in config:
        cg.add(var.set_preset_eco_ga_id(config[const.CONF_PRESET_ECO_GA]))
    if const.CONF_PRESET_AWAY_GA in config:
        cg.add(var.set_preset_away_ga_id(config[const.CONF_PRESET_AWAY_GA]))
    if const.CONF_PRESET_SLEEP_GA in config:
        cg.add(var.set_preset_sleep_ga_id(config[const.CONF_PRESET_SLEEP_GA]))
