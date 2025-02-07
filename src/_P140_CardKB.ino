#include "_Plugin_Helper.h"
#ifdef USES_P140

// #######################################################################################################
// ########################## Plugin 140: Gases - SCD4x CO2, Humidity, Temperature #######################
// #######################################################################################################

/**
 * 2025-02-06 tonhuisman: Start plugin for M5Stack CardKB I2C mini keyboard
 **/
# define PLUGIN_140
# define PLUGIN_ID_140          140
# define PLUGIN_NAME_140        "Input - CardKB I2C"
# define PLUGIN_VALUENAME1_140  "Key"
# define PLUGIN_VALUENAME2_140  "Length"

# include "./src/PluginStructs/P140_data_struct.h"

boolean Plugin_140(uint8_t function, struct EventStruct *event, String& string)
{
  boolean success = false;

  switch (function)
  {
    case PLUGIN_DEVICE_ADD:
    {
      auto& dev = Device[++deviceCount];
      dev.Number         = PLUGIN_ID_140;
      dev.Type           = DEVICE_TYPE_I2C;
      dev.VType          = Sensor_VType::SENSOR_TYPE_DUAL;
      dev.ValueCount     = 2;
      dev.SendDataOption = true;

      break;
    }

    case PLUGIN_GET_DEVICENAME:
    {
      string = F(PLUGIN_NAME_140);

      break;
    }

    case PLUGIN_GET_DEVICEVALUENAMES:
    {
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[0], PSTR(PLUGIN_VALUENAME1_140));
      strcpy_P(ExtraTaskSettings.TaskDeviceValueNames[1], PSTR(PLUGIN_VALUENAME2_140));

      break;
    }

    case PLUGIN_I2C_HAS_ADDRESS:
    {
      success = event->Par1 == P140_I2C_ADDR;
      break;
    }

    # if FEATURE_I2C_GET_ADDRESS
    case PLUGIN_I2C_GET_ADDRESS:
    {
      event->Par1 = P140_I2C_ADDR;
      success     = true;
      break;
    }
    # endif // if FEATURE_I2C_GET_ADDRESS

    case PLUGIN_SET_DEFAULTS:
    {
      ExtraTaskSettings.TaskDeviceValueDecimals[0] = 0;
      ExtraTaskSettings.TaskDeviceValueDecimals[1] = 0;
      break;
    }

    case PLUGIN_WEBFORM_LOAD:
    {
      addFormCheckBox(F("Send event on keypress"), F("events"), P140_SEND_EVENTS == 1);
      P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));
      String buffer               = F("buffer"); // Fetch buffer content

      if ((nullptr != P140_data) && P140_data->plugin_get_config_value(event, buffer)) {
        addFormSubHeader(F("Buffer"));
        addRowLabel(F("Current buffer content"));
        addHtml(buffer);
      }
      success = true;
      break;
    }

    case PLUGIN_WEBFORM_SAVE:
    {
      P140_SEND_EVENTS = isFormItemChecked(F("events")) ? 1 : 0;
      success          = true;
      break;
    }

    case PLUGIN_INIT:
    {
      initPluginTaskData(event->TaskIndex, new (std::nothrow) P140_data_struct(event));
      P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));

      success = (nullptr != P140_data);

      break;
    }

    case PLUGIN_READ:
    {
      P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P140_data) {
        success = P140_data->plugin_read(event);
      }

      break;
    }

    case PLUGIN_WRITE:
    {
      P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P140_data) {
        success = P140_data->plugin_write(event, string);
      }

      break;
    }

    case PLUGIN_GET_CONFIG_VALUE:
    {
      P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P140_data) {
        success = P140_data->plugin_get_config_value(event, string);
      }

      break;
    }

    case PLUGIN_TEN_PER_SECOND:
    {
      P140_data_struct *P140_data = static_cast<P140_data_struct *>(getPluginTaskData(event->TaskIndex));

      if (nullptr != P140_data) {
        success = P140_data->plugin_ten_per_second(event);
      }

      break;
    }
  }
  return success;
}

#endif // USES_P140
