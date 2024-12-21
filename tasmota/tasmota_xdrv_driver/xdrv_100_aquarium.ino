#ifdef USE_AQUARIUM_LIGHTING
#ifdef USE_LIGHT

#define XDRV_100 100
#include "include/tasmota_types.h"  // Access to TSettings and AquariumLightSettings
#include <math.h>                   // For sine functions

bool init_success = false;

/*********************************************************************************************\
 * Commands
\*********************************************************************************************/
const char CommandStrs[] PROGMEM = "|"  // No Prefix
  "UpdateSunriseTime|" 
  "UpdatePeakBrightness|"
  "UpdateRampShape|"
  "UpdateRampTime|"
  "UpdatePeakHoldTime|"
  "UpdateSteepness|"
  "UpdateUpdateFrequency|"
  "ToggleOverride|"
  "UpdateOverrideColor|"
  "UpdateOverride";

void (* const CommandFuncs[])(void) PROGMEM = {
  &CmdUpdateSunriseTime,
  &CmdUpdatePeakBrightness,
  &CmdUpdateRampShape,
  &CmdUpdateRampTime,
  &CmdUpdatePeakHoldTime,
  &CmdUpdateSteepness,
  &CmdUpdateUpdateFrequency,
  &CmdToggleOverride,
  &CmdUpdateOverrideColor,
  &CmdUpdateOverride
};

void CmdUpdateSunriseTime(void) {
    uint8_t hour, minute;
    if (sscanf(XdrvMailbox.data, "%hhu:%hhu", &hour, &minute) == 2) {
        if (hour < 24 && minute < 60) {
            Settings->free_eb0.aquarium_light_settings.sunrise_hour = hour;
            Settings->free_eb0.aquarium_light_settings.sunrise_minute = minute;
            SettingsSave(0);
            Response_P(PSTR("{\"sunrise_time\":\"%02d:%02d\"}"), hour, minute);
        } else {
            ResponseCmndChar(PSTR("Invalid sunrise time. Use: 0-23,0-59"));
        }
    } else {
        ResponseCmndChar(PSTR("Syntax: UpdateSunriseTime <hour>,<minute>"));
    }
}

void CmdUpdatePeakBrightness(void) {
    uint8_t r, g, b, w;
    if (sscanf(XdrvMailbox.data, "%hhu,%hhu,%hhu,%hhu", &r, &g, &b, &w) == 4) {
        if (r <= 255 && g <= 255 && b <= 255 && w <= 255) {
            Settings->free_eb0.aquarium_light_settings.peak_brightness_color[0] = r;
            Settings->free_eb0.aquarium_light_settings.peak_brightness_color[1] = g;
            Settings->free_eb0.aquarium_light_settings.peak_brightness_color[2] = b;
            Settings->free_eb0.aquarium_light_settings.peak_brightness_color[3] = w;
            SettingsSave(0);
            Response_P(PSTR("{\"color\": \"%d,%d,%d,%d\"}"), r, g, b, w);
        } else {
            ResponseCmndChar(PSTR("Invalid color values. Use: 0-255 for each channel."));
        }
    } else {
        ResponseCmndChar(PSTR("Syntax: UpdatePeakBrightness <R>,<G>,<B>,<W>"));
    }
}

void CmdUpdateRampShape(void) {
    uint8_t shape;
    if (sscanf(XdrvMailbox.data, "%hhu", &shape) == 1) {
        if (shape > 4) {
            ResponseCmndChar(PSTR("Invalid ramp shape selected. Select 0-4."));
        } else {
            Settings->free_eb0.aquarium_light_settings.ramp_shape = shape;
            SettingsSave(0);
            Response_P(PSTR("{\"ramp_shape\": \"%d\"}"), shape);
        }
    } else {
        ResponseCmndChar(PSTR("Syntax: UpdateRampShape <0-4>"));
    }
}

void CmdUpdateRampTime(void) {
    uint8_t time;
    if (sscanf(XdrvMailbox.data, "%hhu", &time) == 1) {
        if (time > 1440) {
            ResponseCmndChar(PSTR("Maximum ramp time is 1440 minutes."));
        } else {
            Settings->free_eb0.aquarium_light_settings.ramp_time = time;
            SettingsSave(0);
            Response_P(PSTR("{\"ramp_time\": \"%d\"}"), time);
        }
    } else {
        ResponseCmndChar(PSTR("Syntax: UpdateRampTime <0-1440>"));
    }
}

void CmdUpdatePeakHoldTime(void) {
    uint8_t time;
    if (sscanf(XdrvMailbox.data, "%hhu", &time) == 1) {
        if (time > 1440) {
            ResponseCmndChar(PSTR("Maximum peak hold time is 1440 minutes."));
        } else {
            Settings->free_eb0.aquarium_light_settings.peak_hold_time = time;
            SettingsSave(0);
            Response_P(PSTR("{\"peak_hold_time\": \"%d\"}"), time);
        }
    } else {
        ResponseCmndChar(PSTR("Syntax: UpdatePeakHoldTime <0-1440>"));
    }
}

uint16_t ConvertDecimal(int decimal) {
    if (decimal % 10 == decimal) { // If the decimal part is only one digit
        return decimal * 100;
    } else if (decimal % 100 == decimal) { // If the decimal is only two digits long
        return decimal * 10;
    }

    return decimal;
}

void CmdUpdateSteepness(void) {
    uint8_t steepness_whole = 0;
    int steepness_decimal = 0;
    uint16_t scaled_steepness = 0;

    // Try parsing with a decimal point
    uint8_t result = sscanf(XdrvMailbox.data, "%hhu.%3d", &steepness_whole, &steepness_decimal);

    if (result == 2) { // Case: Value with a decimal
        if (steepness_decimal < 0 || steepness_decimal > 999) {
            ResponseCmndChar(PSTR("Syntax error: Invalid decimal part (0-999)."));
            return;
        }
    } else if (result == 1) { // Case: Value without a decimal
        steepness_decimal = 0; // Default decimal part to 0
    } else {
        ResponseCmndChar(PSTR("Syntax: UpdateSteepness <0.000-65.000>"));
        return;
    }
    
    scaled_steepness = (uint16_t)(steepness_whole * 1000 + ConvertDecimal(steepness_decimal));
    if (scaled_steepness <= 65000) {
        Settings->free_eb0.aquarium_light_settings.steepness = scaled_steepness;
        SettingsSave(0);
        Response_P(PSTR("{\"steepness\": \"%d.%03d\"}"), steepness_whole, ConvertDecimal(steepness_decimal));
    } else {
        ResponseCmndChar(PSTR("Steepness value out of range. Max: 65.000."));
    }
}


void CmdUpdateUpdateFrequency(void) {
    uint8_t setting;
    if (sscanf(XdrvMailbox.data, "%hhu", &setting) == 1) {
        if (setting > 3) {
            ResponseCmndChar(PSTR("Invalid update frequency setting selected. Select 0-3."));
        } else {
            Settings->free_eb0.aquarium_light_settings.update_frequency = setting;
            SettingsSave(0);
            Response_P(PSTR("{\"update_frequency_setting\": \"%d\"}"), setting);
        }
    } else {
        ResponseCmndChar(PSTR("Syntax: UpdateUpdateFrequency <0-3>"));
    }
}

void CmdToggleOverride(void) {
  uint8_t override_status = Settings->free_eb0.aquarium_light_settings.override_enabled;
  Settings->free_eb0.aquarium_light_settings.override_enabled = !override_status;

  Response_P(PSTR("{\"state\":\"%s\"}"),
    Settings->free_eb0.aquarium_light_settings.override_enabled ? "ON" : "OFF");
}

void CmdUpdateOverrideColor(void) {
    uint8_t r, g, b, w;
    if (sscanf(XdrvMailbox.data, "%hhu,%hhu,%hhu,%hhu", &r, &g, &b, &w) == 4) {
        if (r <= 255 && g <= 255 && b <= 255 && w <= 255) {
            Settings->free_eb0.aquarium_light_settings.override_color[0] = r;
            Settings->free_eb0.aquarium_light_settings.override_color[1] = g;
            Settings->free_eb0.aquarium_light_settings.override_color[2] = b;
            Settings->free_eb0.aquarium_light_settings.override_color[3] = w;
            SettingsSave(0);
            Response_P(PSTR("{\"color\": %d,%d,%d,%d}"), r, g, b, w);
        } else {
            ResponseCmndChar(PSTR("Invalid input. RGBW: 0-255 each."));
        }
    } else {
        ResponseCmndChar(PSTR("Syntax: UpdateOverrideColor <R>,<G>,<B>,<W>"));
    }
}

void CmdUpdateOverride(void) {
    JsonParser parser((char*) XdrvMailbox.data);
    JsonParserObject root = parser.getRootObject();
    if (!root) {
        AddLog(LOG_LEVEL_ERROR, PSTR("JSON Parse Error"));
        return;
    }

    JsonParserToken state = root[PSTR("state")];
    if (state) {
        const char* state_val = state.getStr();
        if (strcmp(state_val, "ON") == 0) {
            Settings->free_eb0.aquarium_light_settings.override_enabled = 1;
        } else if (strcmp(state_val, "OFF") == 0) {
            Settings->free_eb0.aquarium_light_settings.override_enabled = 0;
        } else {
            AddLog(LOG_LEVEL_ERROR, PSTR("Invalid state value: %s"), state_val);
            return;
        }
    }

    JsonParserObject color = root[PSTR("color")];
    if (color) {
        uint8_t r = color[PSTR("r")].getUInt();
        uint8_t g = color[PSTR("g")].getUInt();
        uint8_t b = color[PSTR("b")].getUInt();
        uint8_t w = color[PSTR("w")].getUInt();

        if (r <= 255 && g <= 255 && b <= 255 && w <= 255) {
            Settings->free_eb0.aquarium_light_settings.override_color[0] = r;
            Settings->free_eb0.aquarium_light_settings.override_color[1] = g;
            Settings->free_eb0.aquarium_light_settings.override_color[2] = b;
            Settings->free_eb0.aquarium_light_settings.override_color[3] = w;
            AddLog(LOG_LEVEL_INFO, PSTR("Override color updated: R=%u, G=%u, B=%u, W=%u"), r, g, b, w);
        } else {
            AddLog(LOG_LEVEL_ERROR, PSTR("Invalid color values: R=%u, G=%u, B=%u, W=%u"), r, g, b, w);
        }
    }
    const char* override_state =  Settings->free_eb0.aquarium_light_settings.override_enabled ? "ON" : "OFF";
    Response_P(PSTR("{\"state\":\"%s\",\"color\":{\"r\":%d,\"g\":%d,\"b\":%d,\"w\":%d}}"),
      override_state,
      Settings->free_eb0.aquarium_light_settings.override_color[0],
      Settings->free_eb0.aquarium_light_settings.override_color[1],
      Settings->free_eb0.aquarium_light_settings.override_color[2],
      Settings->free_eb0.aquarium_light_settings.override_color[3]
    );
}

/*********************************************************************************************\
 * Tasmota Functions
\*********************************************************************************************/
void AquariumInit() {
  AddLog(LOG_LEVEL_INFO, PSTR("Initializing aquarium light driver"));
  // Any necessary initialization logic goes here...

  init_success = true;
}

uint32_t GetMillisecondsSinceMidnight() {
    uint32_t current_millis = millis() % 1000;
    uint32_t ms = ((RtcTime.hour * 3600UL) + (RtcTime.minute * 60) + RtcTime.second) * 1000;
    ms += current_millis; 
    return ms;
}

uint32_t Get_t(uint32_t i, uint32_t ramp_time_ms, uint32_t peak_hold_time_ms) {
    if (i <= ramp_time_ms) {
        AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Inside ramp up phase"));
        return i;
    } else if (i > ramp_time_ms && i <= ramp_time_ms + peak_hold_time_ms) {
        AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Inside peak hold phase"));
        return i;
    } else if (i > ramp_time_ms + peak_hold_time_ms && i < 2 * ramp_time_ms + peak_hold_time_ms) {
        AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Inside ramp down phase"));
        return 2 * ramp_time_ms + peak_hold_time_ms - i;
    } else {
        AddLog(LOG_LEVEL_ERROR, PSTR("Ramp index outside of ramp schedule"));
        return 2 * ramp_time_ms + peak_hold_time_ms;
    }
}

float RampSine(uint32_t t, uint32_t ramp_time_ms) {
    float coeff = M_PI / ((float)ramp_time_ms * 2.0f);

    return sinf(coeff * (float) t);
}

float RampSCurve(uint32_t t, uint32_t ramp_time_ms, float steepness) {
    float t_scaled = (float)t / (float)ramp_time_ms;
    float x = pow(t_scaled, steepness);

    return x / (x + pow(1 - t_scaled, steepness)); 
}

float RampLinear(uint32_t t, uint32_t ramp_time_ms) {
    return (float)t / (float)ramp_time_ms;
}

// Static accumulators for fractional parts of each color channel
static float current_R = 0.0f;
static float current_G = 0.0f;
static float current_B = 0.0f;
static float current_W = 0.0f;

void UpdateChannelsSmooth(uint8_t peak_color[4], float brightness_factor, float alpha) {
    // Calculate the target float values for each channel
    uint8_t new_R = (uint8_t)(peak_color[0] * brightness_factor);
    uint8_t new_G = (uint8_t)(peak_color[1] * brightness_factor);
    uint8_t new_B = (uint8_t)(peak_color[2] * brightness_factor);
    uint8_t new_W = (uint8_t)(peak_color[3] * brightness_factor);

    // Smoothly approach new values:
    current_R += alpha * ((float)new_R - current_R);
    current_G += alpha * ((float)new_G - current_G);
    current_B += alpha * ((float)new_B - current_B);
    current_W += alpha * ((float)new_W - current_W);

    uint8_t channels[5] = {
        (uint8_t)current_R, 
        (uint8_t)current_G, 
        (uint8_t)current_B, 
        (uint8_t)current_W, 
        0
    };

    light_controller.changeChannels(channels, false);
    AddLog(LOG_LEVEL_DEBUG, PSTR("Aquarium lights set to RGBW(%d,%d,%d,%d)"), (uint8_t)current_R, (uint8_t)current_G, (uint8_t)current_B, (uint8_t)current_W);
}


// Updates aquarium brightness based on settings and time
void UpdateAquariumBrightness() {
    uint32_t ms_since_midnight = GetMillisecondsSinceMidnight();

    // Convert sunrise and sunset times to milliseconds since midnight
    uint32_t sunrise_time_ms = (Settings->free_eb0.aquarium_light_settings.sunrise_hour * 3600000UL) +
                               (Settings->free_eb0.aquarium_light_settings.sunrise_minute * 60000UL);

    uint32_t ramp_time_ms = Settings->free_eb0.aquarium_light_settings.ramp_time * 60000UL;
    uint32_t peak_hold_time_ms = Settings->free_eb0.aquarium_light_settings.peak_hold_time * 60000UL;
    float steepness = (float)Settings->free_eb0.aquarium_light_settings.steepness / 1000.0f;
    uint32_t sunset_time_ms = sunrise_time_ms + (2 * ramp_time_ms) + peak_hold_time_ms;

    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("sunrise_time_ms: %d"), sunrise_time_ms);
    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("ramp_time_ms: %d"), ramp_time_ms);
    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("peak_hold_time_ms: %d"), peak_hold_time_ms);
    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("sunset_time_ms: %d"), sunset_time_ms);
    AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("steepness: %d.%03d"), (int)steepness, (int)((steepness - (int)steepness) * 1000));

    // Handle override
    if (Settings->free_eb0.aquarium_light_settings.override_enabled) {
        uint8_t* override_color = Settings->free_eb0.aquarium_light_settings.override_color;
        uint8_t channels[5] = {override_color[0], override_color[1], override_color[2], override_color[3], override_color[3]};
        light_controller.changeChannels(channels, true);
        AddLog(LOG_LEVEL_DEBUG, PSTR("Aquarium override enabled: Setting color RGBW(%d,%d,%d,%d)"),
               override_color[0], override_color[1], override_color[2], override_color[3]);
        return;
    }

    // Check if current time is outside sunrise-sunset window
    if (ms_since_midnight < sunrise_time_ms || ms_since_midnight > sunset_time_ms) {
        // Turn lights off
        uint8_t channels[5] = { 0, 0, 0, 0, 0 };
        light_controller.changeChannels(channels, true);
        AddLog(LOG_LEVEL_DEBUG, PSTR("Aquarium lights off: Outside sunrise-sunset window"));
        return;
    }

    uint8_t* peak_color = Settings->free_eb0.aquarium_light_settings.peak_brightness_color;
    uint32_t i = ms_since_midnight - sunrise_time_ms;
    
    // If inside peak hold phase...
    if (i > ramp_time_ms && i <= ramp_time_ms + peak_hold_time_ms) {
        // Set channels to peak brightness
        uint8_t channels[5] = {peak_color[0], peak_color[1], peak_color[2], peak_color[3], 0};
        light_controller.changeChannels(channels, false);
        AddLog(LOG_LEVEL_DEBUG, PSTR("Holding at peak brightness."));
        return;
    }

    uint32_t t = Get_t(i, ramp_time_ms, peak_hold_time_ms);
    float brightness_factor;

    switch (Settings->free_eb0.aquarium_light_settings.ramp_shape) {
        case 0:
            brightness_factor = RampSine(t, ramp_time_ms);
            break;
        case 1:
            brightness_factor = RampSCurve(t, ramp_time_ms, steepness);
            break;
        case 2:
            brightness_factor = RampLinear(t, ramp_time_ms);
            break;
        default:
            AddLog(LOG_LEVEL_ERROR, PSTR("Undefined ramp shape chosen"));
            brightness_factor = RampSine(t, ramp_time_ms);
    }

    UpdateChannelsSmooth(peak_color, brightness_factor, 0.15);
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/
bool Xdrv100(uint32_t function) {
  bool result = false;
  uint8_t update_frequency = Settings->free_eb0.aquarium_light_settings.update_frequency;

  if (FUNC_INIT == function) {
    AquariumInit();
  }
  else if (init_success) {
    switch (function) {
        case FUNC_EVERY_50_MSECOND:
            if (update_frequency == 3) {
                UpdateAquariumBrightness();
            }
            break;
        case FUNC_EVERY_100_MSECOND:
            if (update_frequency == 2) {
                UpdateAquariumBrightness();
            }
            break;
        case FUNC_EVERY_250_MSECOND:
            if (update_frequency == 1) {
                UpdateAquariumBrightness();
            }
            break;
        case FUNC_EVERY_SECOND:
            if (update_frequency == 0) {
                UpdateAquariumBrightness();
            }
            break;
        case FUNC_COMMAND:
            AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Calling aquarium command"));
            result = DecodeCommand(CommandStrs, CommandFuncs);
            break;
    }
  }

  return result;
}
#endif  // USE_LIGHT
#endif  // USE_AQUARIUM_LIGHTING