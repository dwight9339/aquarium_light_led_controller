# This firmware is built on
![Tasmota logo](/tools/logo/TASMOTA_FullLogo_Vector.svg#gh-light-mode-only)![Tasmota logo](/tools/logo/TASMOTA_FullLogo_Vector_White.svg#gh-dark-mode-only)

# Aquarium LED Controller
The project implements a custom driver for Tasmota and turns an ESP8266-based device into a controller for RGBW LED strips. The custom functionality is fairly simple, implementing only two main features:
- **Onboard Brightness Ramping**: Brightness smoothly ramps from 0 to a specified full brightness value for each RGBW channel and back down to 0 over the course of a day between specified sunrise and sunset times following a sine-squared function. Implementing brightness ramping directly on the controller makes it more robust against wireless connectivity issues associated with controlling the light via a central home automation server and relieves such a server from having to execute updates at the rates required to produce a smooth lighting curve.
- **Override of Default Functionality**: The ramping functionality described previously is the default for the device but can be overridden if desired.

## Custom Commands
The following commands can be executed either in the Tasmota web console or via MQTT to update lighting parameters:
- `UpdateSunriseTime {hh}:{mm}`: Sets the `sunrise_hour` and `sunrise_minute` parameters to hour (`hh`) and minute (`mm`) respectively.
- `UpdateSunsetTime {hh}:{mm}`: Sets the `sunset_hour` and `ssunset_minute` parameters to hour (`hh`) and minute (`mm`) respectively.
- `UpdatePeakBrightness {r},{g},{b},{w}`: Sets the maximum brightness (0-255) reached by each of the light channels (R, G, B, and W(cold white or warm white depending on your LED strip)).
- `ToggleOverride`: Toggles the parameter which determines whether or not the default functionality is overridden.
- `UpdateOverrideColor {r},{g},{b},{w}`: Sets the brightness (0-255) for each channel when the default functionality is overridden.
- `UpdateOverride`: Updates the override parameters (on-off and channel brightnesses) but expects a JSON object of the form: <br></br> `{"status": "ON|OFF", "color": {"r":{0-255}, "g": {0-255}, "b": {0-255}, "w": {0-255}}}` <br></br> Mainly used when implementing MQTT-based controls in smart home systems like Home Assistant.

<hr></hr>
<hr></hr>

The following are select excerpts from the Tasmota ReadMe document. The full thing can be found [here](https://github.com/arendst/Tasmota/blob/development/README.md).

<hr></hr>

## Easy install

Easy initial installation of Tasmota can be performed using the [Tasmota WebInstaller](https://tasmota.github.io/install/).

If you like **Tasmota**, give it a star, or fork it and contribute!

[![GitHub stars](https://img.shields.io/github/stars/arendst/Tasmota.svg?style=social&label=Star)](https://github.com/arendst/Tasmota/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/arendst/Tasmota.svg?style=social&label=Fork)](https://github.com/arendst/Tasmota/network)
[![donate](https://img.shields.io/badge/donate-PayPal-blue.svg)](https://paypal.me/tasmota)

See [RELEASENOTES.md](https://github.com/arendst/Tasmota/blob/master/RELEASENOTES.md) for release information.

Firmware binaries can be downloaded from http://ota.tasmota.com/tasmota/release/ or http://ota.tasmota.com/tasmota32/release/ for ESP32 binaries.

## Disclaimer

:warning: **DANGER OF ELECTROCUTION** :warning:

If your device connects to mains electricity (AC power) there is danger of electrocution if not installed properly. If you don't know how to install it, please call an electrician (***Beware:*** certain countries prohibit installation without a licensed electrician present). Remember: _**SAFETY FIRST**_. It is not worth the risk to yourself, your family and your home if you don't know exactly what you are doing. Never tinker or try to flash a device using the serial programming interface while it is connected to MAINS ELECTRICITY (AC power).

We don't take any responsibility nor liability for using this software nor for the installation or any tips, advice, videos, etc. given by any member of this site or any related site.

## Note

Please do not ask to add new devices unless it requires additional code for new features. If the device is not listed as a module, try using [Templates](https://tasmota.github.io/docs/Templates) first. If it is not listed in the [Tasmota Device Templates Repository](http://templates.blakadder.com) create your own [Template](https://tasmota.github.io/docs/Templates#creating-your-template).

## Important User Compilation Information
If you want to compile Tasmota yourself keep in mind the following:

- For ESP8285 based devices only Flash Mode **DOUT** is supported. Do not use Flash Mode DIO / QIO / QOUT as it might seem to brick your device.
- For ESP8285 based devices Tasmota uses a 1M linker script WITHOUT spiffs **1M (no SPIFFS)** for optimal code space.
- To make compile time changes to Tasmota use the `user_config_override.h` file. It assures keeping your custom settings when you download and compile a new version. You have to make a copy from the provided `user_config_override_sample.h` file and add your setting overrides.

## Configuration Information

Please refer to the installation and configuration articles in our [documentation](https://tasmota.github.io/docs).

### Documentation

* [Documentation Site](https://tasmota.github.io/docs): For information on how to flash Tasmota, configure, use and expand it
* [FAQ and Troubleshooting](https://tasmota.github.io/docs/FAQ/): For information on common problems and solutions.
* [Commands Information](https://tasmota.github.io/docs/Commands): For information on all the commands supported by Tasmota.

### Support's Community

* [Tasmota Discussions](https://github.com/arendst/Tasmota/discussions): For Tasmota usage questions, Feature Requests and Projects.
* [Tasmota Users Chat](https://discord.gg/Ks2Kzd4): For support, troubleshooting and general questions. You have better chances to get fast answers from members of the Tasmota Community.
* [Search in Issues](https://github.com/arendst/Tasmota/issues): You might find an answer to your question by searching current or closed issues.
* [Software Problem Report](https://github.com/arendst/Tasmota/issues/new?template=Bug_report.md): For reporting problems of Tasmota Software.

## License

This program is licensed under GPL-3.0-only
