#include "../PluginStructs/P140_data_struct.h"

#ifdef USES_P140

# include "../Commands/ExecuteCommand.h"

P140_data_struct::P140_data_struct(struct EventStruct *event) {
  _events = 1 == P140_SEND_EVENTS;
}

P140_data_struct::~P140_data_struct() {}

bool P140_data_struct::plugin_read(struct EventStruct *event) {
  return false;
}

bool P140_data_struct::plugin_write(struct EventStruct *event, String& string) {
  const String cmd = parseString(string, 1);

  if (equals(cmd, F("cardkb"))) {
    const String subcmd = parseString(string, 2);

    if (equals(subcmd, F("events")) && ((0 == event->Par2) || (1 == event->Par2))) {
      P140_SEND_EVENTS = event->Par2;
      _events          = 1 == event->Par1;
      return true;
    }
  }
  return false;
}

bool P140_data_struct::plugin_get_config_value(struct EventStruct *event,
                                               String            & string) {
  const String cmd = parseString(string, 1);

  if (equals(cmd, F("buffer"))) {
    if (_inCounter < P140_INPUT_BUFFER_SIZE) {
      _buffer[_inCounter] = 0; // terminate string
    }
    const String tmp(_buffer);
    string = tmp;
    return true;
  }
  return false;
}

bool P140_data_struct::plugin_ten_per_second(struct EventStruct *event) {
  bool result = false;
  bool update = false;
  bool ok;
  const uint8_t inChar = I2C_read8(P140_I2C_ADDR, &ok);

  if (isprint(inChar))
  {
    if (_inCounter < P140_INPUT_BUFFER_SIZE) { // add char to string if it still fits
      _buffer[_inCounter++] = inChar;
    }
    update = true;
    # if P140_DEBUG

    if (_inCounter < P140_INPUT_BUFFER_SIZE) {
      _buffer[_inCounter] = 0; // terminate string
    }
    addLog(LOG_LEVEL_INFO, strformat(F("CardKB: Key %d (0x%02X) '%c' command: %s"), inChar, inChar, inChar, _buffer));
    # endif // if P140_DEBUG
  } else

  if ((inChar == '\b') && (_inCounter > 0)) // Correct a typo using BackSpace
  {
    _buffer[--_inCounter] = 0; // shorten input
    update                = true;
    # if P140_DEBUG
    addLog(LOG_LEVEL_INFO, strformat(F("CardKB: Backspace : %s"), _buffer));
    # endif // if P140_DEBUG
  } else
  if ((inChar == '\r') || (inChar == '\n')) { // CR or LF completes command
    // Ignore empty command
    if (_inCounter != 0) {
      _buffer[_inCounter] = 0;                // keyboard data completed
      const String cmd(_buffer);

      if (logl(LOG_LEVEL_INFO)) {
        addLog(LOG_LEVEL_INFO, strformat(F("CardKB: Execute: %s"), _buffer));
      }

      // Act like we entered a command via serial
      ExecuteCommand_all({ EventValueSource::Enum::VALUE_SOURCE_SERIAL, std::move(cmd) }, true);
      _inCounter = 0;
      _buffer[0] = 0; // keyboard data processed, clear buffer
      update     = true;
      result     = true;
    }
  } else
  if (inChar != 0) {
    update = true;
    # if P140_DEBUG
    addLog(LOG_LEVEL_INFO, strformat(F("CardKB: Key: %d (0x%02X) (ignored)"), inChar, inChar));
    # endif // if P140_DEBUG
  }


  if (update) {
    UserVar.setFloat(event->TaskIndex, 0, inChar);
    UserVar.setFloat(event->TaskIndex, 1, _inCounter);

    if (_events) {
      sendData(event);
    }
  }
  return result;
}

#endif // ifdef USES_P140
