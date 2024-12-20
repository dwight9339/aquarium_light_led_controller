# This firmware is built on
![Tasmota logo](/tools/logo/TASMOTA_FullLogo_Vector.svg#gh-light-mode-only)![Tasmota logo](/tools/logo/TASMOTA_FullLogo_Vector_White.svg#gh-dark-mode-only)

# Aquarium LED Controller
The project implements a custom driver for Tasmota that turns an ESP8266-based device into an aquarium light controller for RGBW LED strips. The custom functionality is fairly simple, implementing only two main features:
- **Onboard Brightness Ramping**: Brightness smoothly ramps from 0 to a specified full brightness value for each RGBW channel and back down to 0 over each day. Implementing brightness ramping directly on the controller makes it more robust against wireless connectivity issues associated with controlling the light via a central home automation server and relieves such a server from having to execute updates at the rates required to produce a smooth lighting curve.
- **Custom Ramp Configuration**: Users can configure the ramp shape, ramp time, peak hold time, and steepness for precise control over their lighting schedule.
- **Override of Default Functionality**: The ramping functionality described previously is the default for the device but can be overridden, if desired, to make the light essentially just a normal LED smart light.

## Custom Commands
The following commands can be executed either in the Tasmota web console or via MQTT to update lighting parameters:
- `UpdateSunriseTime {hh}:{mm}`: Sets the `sunrise_hour` and `sunrise_minute` parameters to hour (`hh`) and minute (`mm`) respectively.
- `UpdatePeakBrightness {r},{g},{b},{w}`: Sets the maximum brightness (0-255) reached by each of the light channels (R, G, B, and W(cold white or warm white depending on your LED strip)).
- `ToggleOverride`: Toggles the parameter which determines whether or not the default functionality is overridden.
- `UpdateRampShape {0-2}`: Sets the ramp shape. 0 = sine, 1 = S-curve, 2 = linear.
- `UpdateSteepness {0.0-65.0}`: Adjusts the steepness parameter used by the S-curve ramp.
- `UpdateRampTime {minutes}`: Sets the length for each ramp cycle (up and down) in minutes. Accepts values between 0 and 1440 (24 hours).
- `UpdatePeakHoldTime {minutes}`: Sets the length of time (in minutes) that the light is held at peak brightness between ramp cycles. Accepts values between 0 and 1440 (24 hours).
- `UpdateOverrideColor {r},{g},{b},{w}`: Sets the brightness (0-255) for each channel when the default functionality is overridden.
- `UpdateOverride`: Mainly used when implementing MQTT-based controls in smart home systems like Home Assistant. Updates the override parameters (on-off and channel brightnesses) but expects a JSON object of the form: <br></br> `{"status": "ON|OFF", "color": {"r":{0-255}, "g": {0-255}, "b": {0-255}, "w": {0-255}}}` <br></br>
- `UpdateUpdateFrequency {0-3}`: Adjusts the light update frequency setting. Changing this is not advised. The default setting (every 250 seconds) currently provides the best results in transition smoothing tests. 0 = every second, 1 = every 250 milliseconds, 2 = every 100 milliseconds, 3 = every 50 milliseconds.

## Ramp Shape Playground
I've created interactive graphs for each of the available ramp shapes to help dial in the best lighting schedule.
![ramp_shape_playground_screenshot](https://github.com/user-attachments/assets/42abe918-a608-4242-a14e-d5ed3fd3439f)

## Flashing Firmware and Initial Configuration
To install the firmware on your device and connect it to your home network, download the latest build [here](https://github.com/dwight9339/aquarium_light_led_controller/releases) and follow the instructions provided [here](https://tasmota.github.io/docs/Getting-Started/).

## Integrating with your Smart Home System
To integrate the light with your smart home system, you'll need to have an MQTT broker running and add the light as a client. The exact methods for doing so will depend on your specific system. For example, here is my Mosquitto broker configuration for Home Assistant.
![Screenshot of Home Assistant add-ons page with an arrow pointing to the Mosquitto Broker tile.](https://github.com/user-attachments/assets/d68be58b-328d-40b1-980d-a2a4f4c0e39b)
![Screenshot of the Mostquitto MQTT broker configuration page with an arrow pointing to client credentials for the Tasmota device.](https://github.com/user-attachments/assets/d8e6db05-7a5b-4356-97e2-2a2acfdb1aee)

You can then follow the instructions provided [here](https://tasmota.github.io/docs/MQTT/#configure-mqtt-using-webui) to configure MQTT on your device.
![Screenshot of Tasmota MQTT config form with example data.](https://github.com/user-attachments/assets/8d867807-e168-4539-9161-3d3d9392623e)

## Home Assistant Override Entity Controller
To implement an entity that can override the aquarium light in Home Assistant, make sure that you have set the light up as an MQTT client as described in the previous sections then add the following code to your `configuration.yaml` file:
```
mqtt:
  - light:
      schema: json
      name: aquarium_light_override
      command_topic: "cmnd/your_light_topic/UpdateOverride"
      state_topic: "stat/your_light_topic/RESULT"
      brightness: true
      supported_color_modes: ["rgbw"]
```
This creates a light entity which can be used to toggle override mode on and off and update the override color of the light. Unfortunately, the entity does not support setting the brightness of all four (R, G, B, and W) channels. Adjusting the brightness of the white channel will turn off the RGB channels and adjusting the brightness for the color channels will turn off the white channel. However, it is still very useful if you want to experiment with different colors or incorporate the light into any scenes.
![override_entity](https://github.com/user-attachments/assets/aba7593c-be62-4211-a7ec-e97cc807fe12)
Make sure to replace `your_light_topic` with the actual topic used by your light.
![Screenshot of Tasmota web console output with device's topic underlined.](https://github.com/user-attachments/assets/984ee050-8e99-4e7f-9f2d-d0e5271aa2d6)

## Contributing
If you'd like to contribute to the project, go ahead and fork the repo and create a pull request! The repo uses the entire sprawling Tasmota codebase but the only files relevant to this project in particular are:
- `tasmota/include/tasmota_types.h`: Where all Tasmota settings are declared. The `AquariumLightSettings` struct with the settings used by the custom driver is declared on line 487.
- `tasmota/tasmota_xdrv_driver/xdrv_100_aquarium.ino`: The custom driver file. All of the custom logic which controls the light and implements the commands to update the relevant settings is housed here.

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
