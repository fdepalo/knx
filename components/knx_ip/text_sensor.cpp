#include "text_sensor.h"
#include "dpt.h"
#include "esphome/core/log.h"
#include <cstdio>

namespace esphome { namespace knx_ip {

static constexpr const char* TAG = "knx_ip.text_sensor";

void KNXTextSensor::setup() {
  if (knx_) knx_->register_entity(this);
}

void KNXTextSensor::dump_config() {
  LOG_TEXT_SENSOR("", "KNX Text Sensor", this);
  const char *dpt_name;
  switch (dpt_type_) {
    case TextSensorDPT::DPT_10: dpt_name = "DPT 10.001 (Time of Day)"; break;
    case TextSensorDPT::DPT_11: dpt_name = "DPT 11.001 (Date)"; break;
    case TextSensorDPT::DPT_19: dpt_name = "DPT 19.001 (Date and Time)"; break;
    default: dpt_name = "DPT 16.001 (String)"; break;
  }
  ESP_LOGCONFIG(TAG, "  DPT Type: %s", dpt_name);
}

void KNXTextSensor::on_knx_telegram(const std::string &ga, const std::vector<uint8_t> &data) {
  auto our_ga = knx_->get_group_address(state_ga_id_);
  if (our_ga && our_ga->get_address() == ga) {
    std::string text;
    char buffer[64];

    switch (dpt_type_) {
      case TextSensorDPT::DPT_10: {
        // Time of Day: "14:30:45" or "Mon 14:30:45"
        auto time = DPT::decode_dpt10(data);
        const char *days[] = {"", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
        if (time.day_of_week > 0 && time.day_of_week <= 7) {
          snprintf(buffer, sizeof(buffer), "%s %02d:%02d:%02d",
                   days[time.day_of_week], time.hour, time.minute, time.second);
        } else {
          snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d",
                   time.hour, time.minute, time.second);
        }
        text = buffer;
        break;
      }

      case TextSensorDPT::DPT_11: {
        // Date: "2024-10-20" or "20/10/2024"
        auto date = DPT::decode_dpt11(data);
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d",
                 date.year, date.month, date.day);
        text = buffer;
        break;
      }

      case TextSensorDPT::DPT_19: {
        // Date and Time: "2024-10-20 14:30:45"
        auto dt = DPT::decode_dpt19(data);
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
                 dt.year, dt.month, dt.day, dt.hour, dt.minute, dt.second);
        text = buffer;

        // Add flags info if present
        if (dt.fault || dt.summer_time) {
          std::string flags;
          if (dt.fault) flags += " [FAULT]";
          if (dt.summer_time) flags += " [DST]";
          text += flags;
        }
        break;
      }

      default:
        // DPT 16 - String
        text = DPT::decode_dpt16(data);
        break;
    }

    publish_state(text);
    ESP_LOGD(TAG, "'%s': Received: %s", this->get_name().c_str(), text.c_str());
  }
}

}}