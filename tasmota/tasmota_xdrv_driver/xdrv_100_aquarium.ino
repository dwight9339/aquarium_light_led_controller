#ifdef USE_AQUARIUM_LIGHTING
#define XDRV_100 100
#include "include/tasmota_types.h"  // Access to TSettings and AquariumLightSettings
#include <math.h>                   // For sine functions

// This variable will be set to true after initialization
bool initSuccess = false;

/* 
  Optional: if you need to pass any command for your device 
  Commands are issued in Console or Web Console
  Commands:
    Say_Hello  - Only prints some text. Can be made something more useful...
    SendMQTT   - Send a MQTT example message
    Help       - Prints a list of available commands  
*/

const char CommandStrs[] PROGMEM = "|"  // No Prefix
  "UpdateSunriseTime|" 
  "UpdateSunsetTime|"
  "UpdatePeakBrightness|"
  "OverrideLight";

void (* const CommandFuncs[])(void) PROGMEM = {
  &CmdUpdateSunriseTime,
  &CmdUpdateSunsetTime,
  &CmdUpdatePeakBrightness,
  &CmdOverrideLight
};

void CmdUpdateSunriseTime(void) {
    uint8_t hour, minute;
    if (sscanf(XdrvMailbox.data, "%hhu,%hhu", &hour, &minute) == 2) {
        if (hour < 24 && minute < 60) {
            Settings->free_eb0.aquarium_light_settings.sunrise_hour = hour;
            Settings->free_eb0.aquarium_light_settings.sunrise_minute = minute;
            SettingsSave(0);
            Response_P(PSTR("{\"%s\":\"Sunrise time updated to %02d:%02d\"}"), XdrvMailbox.command, hour, minute);
        } else {
            ResponseCmndChar(PSTR("Invalid sunrise time. Use: 0-23,0-59"));
        }
    } else {
        ResponseCmndChar(PSTR("Syntax: UpdateSunriseTime <hour>,<minute>"));
    }
}


void CmdUpdateSunsetTime(void) {
    uint8_t hour, minute;
    if (sscanf(XdrvMailbox.data, "%hhu,%hhu", &hour, &minute) == 2) {
        if (hour < 24 && minute < 60) {
            Settings->free_eb0.aquarium_light_settings.sunset_hour = hour;
            Settings->free_eb0.aquarium_light_settings.sunset_minute = minute;
            SettingsSave(0);
            Response_P(PSTR("{\"%s\":\"Sunset time updated to %02d:%02d\"}"), XdrvMailbox.command, hour, minute);
        } else {
            ResponseCmndChar(PSTR("Invalid sunset time. Use: 0-23,0-59"));
        }
    } else {
        ResponseCmndChar(PSTR("Syntax: UpdateSunsetTime <hour>,<minute>"));
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
            Response_P(PSTR("{\"%s\":\"Peak brightnes color updated to RGBW(%d,%d,%d,%d)\"}"), XdrvMailbox.command, r, g, b, w);
        } else {
            ResponseCmndChar(PSTR("Invalid color values. Use: 0-255 for each channel."));
        }
    } else {
        ResponseCmndChar(PSTR("Syntax: UpdatePeakBrightness <R>,<G>,<B>,<W>"));
    }
}

void CmdOverrideLight(void) {
    uint8_t enable, r, g, b, w;
    if (sscanf(XdrvMailbox.data, "%hhu,%hhu,%hhu,%hhu,%hhu", &enable, &r, &g, &b, &w) == 5) {
        if (enable <= 1 && r <= 255 && g <= 255 && b <= 255 && w <= 255) {
            Settings->free_eb0.aquarium_light_settings.override_enabled = enable;
            Settings->free_eb0.aquarium_light_settings.override_color[0] = r;
            Settings->free_eb0.aquarium_light_settings.override_color[1] = g;
            Settings->free_eb0.aquarium_light_settings.override_color[2] = b;
            Settings->free_eb0.aquarium_light_settings.override_color[3] = w;
            SettingsSave(0);
            Response_P(PSTR("{\"%s\":\"Override status updated. Override enabled: %d, Override color: RGBW(%d,%d,%d,%d)\"}"), XdrvMailbox.command, enable, r, g, b, w);
        } else {
            ResponseCmndChar(PSTR("Invalid input. Enable: 0 or 1; RGBW: 0-255 each."));
        }
    } else {
        ResponseCmndChar(PSTR("Syntax: OverrideLight <Enable>,<R>,<G>,<B>,<W>"));
    }
}

/*********************************************************************************************\
 * Tasmota Functions
\*********************************************************************************************/
void AquariumInit()
{
  AddLog(LOG_LEVEL_INFO, PSTR("Initializing aquarium light driver"));

  Serial.begin(115200);

  // Set initSuccess at the very end of the init process
  // Init is successful
  initSuccess = true;
}

uint32_t GetMillisecondsSinceMidnight() {
    uint32_t current_millis = millis() % 1000;
    uint32_t milliseconds = ((RtcTime.hour * 3600UL) + (RtcTime.minute * 60) + RtcTime.second) * 1000;
    milliseconds += current_millis; 
    return milliseconds;
}

static uint8_t prev_channels[5] = {0, 0, 0, 0, 0}; // Store previous RGBW values

void SmoothTransition(uint8_t* target_channels, uint8_t* smooth_channels, float alpha) {
    for (int i = 0; i < 5; i++) {
        smooth_channels[i] = (uint8_t)(prev_channels[i] * (1.0 - alpha) + target_channels[i] * alpha);
    }
    memcpy(prev_channels, smooth_channels, sizeof(prev_channels)); // Update previous values
}

// Function to update aquarium brightness based on settings and time
void UpdateAquariumBrightness() {
    uint32_t ms_since_midnight = GetMillisecondsSinceMidnight();

    // Convert sunrise and sunset times to milliseconds since midnight
    uint32_t sunrise_time_ms = (Settings->free_eb0.aquarium_light_settings.sunrise_hour * 3600000UL) +
                               (Settings->free_eb0.aquarium_light_settings.sunrise_minute * 60000UL);
    uint32_t sunset_time_ms = (Settings->free_eb0.aquarium_light_settings.sunset_hour * 3600000UL) +
                              (Settings->free_eb0.aquarium_light_settings.sunset_minute * 60000UL);

    // Handle override
    if (Settings->free_eb0.aquarium_light_settings.override_enabled) {
        uint8_t* override_color = Settings->free_eb0.aquarium_light_settings.override_color;
        uint8_t channels[5] = {override_color[0], override_color[1], override_color[2], 0, override_color[3]};
        light_controller.changeChannels(channels, true);
        AddLog(LOG_LEVEL_DEBUG, PSTR("Aquarium Override Enabled: Setting color RGBW(%d,%d,%d,%d)"),
               override_color[0], override_color[1], override_color[2], override_color[3]);
        return;
    }

    // Check if current time is outside sunrise-sunset window
    if (ms_since_midnight < sunrise_time_ms || ms_since_midnight > sunset_time_ms) {
        // Turn lights off
        uint8_t channels[5] = { 0, 0, 0, 0, 0 };
        light_controller.changeChannels(channels, true);
        AddLog(LOG_LEVEL_DEBUG, PSTR("Aquarium Lights Off: Outside sunrise-sunset window"));
        return;
    }

    // Calculate progress through the day (0.0 to 1.0)
    uint32_t day_length_ms = sunset_time_ms - sunrise_time_ms;
    float progress = (float)(ms_since_midnight - sunrise_time_ms) / (float)day_length_ms;

    // Brightness factor using squared sine wave
    float sine_val = sinf(M_PI * progress);
    float brightness_factor = sine_val * sine_val;

    // Adjust RGBW values
    uint8_t* peak_color = Settings->free_eb0.aquarium_light_settings.peak_brightness_color;
    uint8_t R = (uint8_t)(peak_color[0] * brightness_factor);
    uint8_t G = (uint8_t)(peak_color[1] * brightness_factor);
    uint8_t B = (uint8_t)(peak_color[2] * brightness_factor);
    uint8_t W = (uint8_t)(peak_color[3] * brightness_factor);

    uint8_t target_channels[5] = {R, G, B, 0, W};
    uint8_t smooth_channels[5];
    SmoothTransition(target_channels, smooth_channels, 0.5);
    light_controller.changeChannels(smooth_channels, false);

    AddLog(LOG_LEVEL_DEBUG, PSTR("Aquarium Lights Set to RGBW(%d,%d,%d,%d)"), R, G, B, W);
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/
bool Xdrv100(uint32_t function)
{
  bool result = false;

  if (FUNC_INIT == function) {
    AquariumInit();
  }
  else if (initSuccess) {

    switch (function) {
      // Select suitable interval for polling your function
        case FUNC_EVERY_250_MSECOND:
            UpdateAquariumBrightness();
        break;

      // Command support
      case FUNC_COMMAND:
        AddLog(LOG_LEVEL_DEBUG_MORE, PSTR("Calling custom command"));
        result = DecodeCommand(CommandStrs, CommandFuncs);
        break;

    }

  }

  return result;
}

#endif  // USE_AQUARIUM_LIGHTING