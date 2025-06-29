<div id="readme_sls"></div>

<!--TITLE-->
<br />
<div align="center">

<!-- <h3 align="center">Smart Room Monitor</h3>-->
<img src=Docs/logo.png width=40%>

<br>


 <p align="center">
 
  <br />
  <br /><br />
  </p>
  </div> 

<!-- TABLE OF CONTENTS -->
- [Project introduction](#project-introduction)
- [Built with](#built-with)
- [Software requirements](#software-requirements)
- [Hardware requirements](#hardware-requirements)
- [Setting up the hardware](#setting-up-the-hardware)
- [Project structure](#project-structure)
- [Project Layout](#project-layout)
- [Getting Started](#getting-started)
- [User guide](#user-guide)
- [Future upgrades](#future-upgrades)
- [Useful links](#useful-links)
- [Team members](#team-members)


<!-- PROJECT INTRODUCTION -->
# Project introduction
Meet the MSP432-powered Smart Room Monitor: your all-in-one environmental sidekick. With a clear always-on time/date display, four easy-to-navigate modes put total control at your fingertips. It keeps track of ambient heat and the intensity of light in the room. It can recalibrate an erroneous clock and select the preferred illumination mode.
With built-in safety alerts and effortless customization, mastering your room has never been this simple, or this smart.


<br>


<p align="right">(<a href="#readme">back to top</a>)</p>

<!-- BUILT WITH -->
# Built with
C <br>  

<br>


# Software requirements
Software required to program the boards: <br>
Code Composer Studio (available [here](https://www.ti.com/tool/CCSTUDIO)) <br>
- MSP Driver Library (available [here](https://www.ti.com/tool/MSPDRIVERLIB)) <br>
- MSP Graphics Library (available [here](https://www.ti.com/tool/MSP-GRLIB)) <br>
</p>


# Hardware requirements
<p style="margin-left: 40px;">Here we show the hardware components needed to built the device. 

## Texas Instruments MSP432P401R microcontroller
<img src=Docs/MSP432-Board.jpg width="50%"> <br><br>
The Texas Instruments MSP432P401R is a 32-bit microcontroller designed for embedded applications. It is built around the ARM Cortex-M4F Core and it supports low-power applications that require increased CPU speed, memory, analog, and 32-bit performance. It features a system clock speed of up to 48 MHz and includes 256 KB of flash memory, 64 KB of SRAM and 32 KB of ROM preloaded with SimpleLink MSP432 SDK libraries. The microcontroller offers a rich set of peripherals: there are four 16-bit timers with capture, compare or PWM modes, two 32-bit timers and a real-time clock (RTC). In particular, we used in our project the RTC to show the exact time and date on the display. This topic will be discussed in the next sections. For communication, it supports up to eight serial channels, including I²C, SPI and UART.



## BOOSTXL-EDUMKII Educational BoosterPack <br>
<img src=Docs/boosterpack.jpg width="50%"> <br><br>
The BOOSTXL-EDUMKII Educational BoosterPack is a plug-in module that offers various analog and digital inputs and outputs. The module also includes an analog joystick, environmental and motion sensors, RGB LED, a microphone, buzzer, color LCD display and more. In our project we used: <br>
### TI OPT3001 light sensor
It's a digital ambient light sensor. It measures the intensity of light (visible by the human eye) present in the room. The measurement can go from 0.01 lux to 83k lux. We used the sensor to module the color of the LED.
<br>
 ### TI TMP006 temperature sensor 
It's a digital infrared thermopile contactless temperature sensor that measures the temperature of an object without being in direct contact. It is specified to operate from -40°C to +125°C. We used it to measure the temperature in the room.
<br>
### Kionix KXTC9-2050 accelerometer
It's a 3-axis analog accelerometer that measures g-forces. If the board moves along the axes, the analog signal generated will change. The full-scale output range is of +/-2g (19.6 m/s^2). The sensing is based on the principle of a differential capacitance arising from acceleration-induced motion of the sense element. We used this accelerometer to detect earthquakes and to generate an alarm message.
<br>
### Cree CLV1A-FKB RGB multicolor LED
It can output an high intensity light of any color by mixing red, green and blue. To achieve a specific color, each color channel can be individually modified by pulse width modulation (PWM). In our project is the main light source of the room.
<br>
### CUI CEM-1203(42) piezo buzzer
This piezo buzzer can play various frequencies based on the provided PWM signal. We used it as the audible alarm signal.
<br>
### Color 128x128 TFT LCD display
This small display packs 128x128 full-color pixels into one square inch of active display area. It updates up to 20 frames per second. It has a color depth of 262k colors and a contrast ratio of 350. It displays the main menu.
<br>
### ITEAD studio IM130330001 2-axis joystick with pushbutton
It's composed of two potentiometers, one for each axis. The select button is present and is actuated when the joystick is pressed down. We used the joystick mainly for the select button, to choose various options in the menu.
<br>
### User push buttons 
They are connected to pullup resistors that drive the pin low if the buttons are pressed. We are used to scroll the menu or to change the time of the clock. 
<br>


## HC-SR501 PIR Sensor <br>
<img src=Docs/pir.png width="30%"> <br><br>
The HC-SR501 PIR is a motion detector module. It senses infrared radiation changes in the environment. In our case, the radiation of human body heat is interpreted as motion. When motion is detected, it outputs a digital signal HIGH (3.3V), otherwise it stays LOW. The detection range can be adjusted between 3 to 7 meters. Also the delay time can be adjusted from 5 seconds to 5 minutes.The sensor operates on 5V but can handle a voltage range of 5V to 20V, and it consumes very little power—around 50 microamps when idle. We connected the PIR to pin P6.4 using a breadboard and some cables.



</p>

<p align="right">(<a href="#readme">back to top</a>)</p>

<!-- SETTING UP THE HARDWARE -->
# Setting up the hardware

<img src=Docs/EMBEDDED.png width="70%">

First, we insert the BoosterPack into the MSP432, ensuring every pin is correctly aligned. Then we connect the MSP432 to the PC using a USB-A to Micro USB cable.
To connect the PIR sensor to the board, we used male-to-female jumper wires and a breadboard. Following the motion sensor's datasheet, we wired the VCC to the board's 5V pin, GND to the board's ground and the DATA to pin P6.4. 

<br>

<p align="right">(<a href="#readme">back to top</a>)</p>

<!-- PROJECT STRUCTURE -->
# Project structure
The main menu, continuously displaying time and date, provides access to all system functions:
<br>
<img src=Docs/menuDisplay.jpg width=30%>
<br>

### Temperature
Displays the current ambient temperature. 
<br>
<img src=Docs/tempDisplay.jpg width=30%>
<br>

### Light
Reports the current illuminance in lux, enabling precise monitoring of ambient lighting conditions.
<br>
<img src=Docs/luxDisplay.jpg width=30%>
<br>

### Change Time
Allows manual adjustment of the real-time clock to compensate for drift, Daylight Saving Time shifts, or initial configuration. Dedicated buttons increment and decrement hours, minutes, day, month, and year; pressing SELECT confirms the updated time/date.
<br>
<img src=Docs/editDisplay.jpg width=30%>
<br>

### LED
Controls the onboard RGB LED. In "Auto" mode, the LED color dynamically maps to measured illuminance. For a low lux value, we have a warm yellow, for high lux, a cool white. The user can also select one of several fixed color outputs: blue, green, red, or cold white.
<br>
<img src=Docs/ledDisplay.jpg width=30%>
<br>

If the temperature exceeds a predefined high or low thresholds (+40°C and 0°C), or if the onboard accelerometer detects significant motion (for example an earthquake), the system activates a visual alert and an audible alarm (buzzer). In the event of an over‑temperature emergency, the LED will glow bright red. If the temperature falls below zero degrees celsius, the LED will be of a cool blue. If seismic activity is detected, the LED will switch to a yellow‑green color to indicate an earthquake alert. We need to press SELECT to acknowledge and silence the alarm.
<br>
<img src=Docs/errDisplay.jpg width=30%>
<br>
Each function is accessed via the tactile buttons on the BoosterPack MKII, ensuring intuitive navigation and reliable operation in a compact embedded system.


<p align="right">(<a href="#readme">back to top</a>)</p>


<!-- PROJECT LAYOUT -->
# Project Layout

```
├───docs
├───SmartRoomController_v1
    ├───LcdDriver
    ├───Sensors
    │   ├─── ... Files used in the program (temperature, rtc, ...)
    ├───images
    │   ├─── ... Files for creating icons (logo, lightbulb, ...)
    ├───targetConfig

```

<p align="right">(<a href="#readme">back to top</a>)</p>



<!-- come includere librerie e far andare progetto -->
# Getting Started

Place the TI driverlib `source` folder inside the `driverlib` folder like this:

IMMAGINE QUI

> You can download the driverlib folder
> from [here](https://drive.google.com/file/d/1_5TsECed3wNJpIpllxYYdD06aFbkk7Fc/view)

Open the folder `NOME CARTELLA DEL PROGETTO` in CCS.

<!-- > Note: do NOT open in CCS the whole `embedded_project` folder, since it contains additional files that are not needed for the CCS project. -->

The project includes are already set up to include the driverlib folder, you should be able to build the project without
any additional setup.

ARM compiler include options: `${PROJECT_ROOT}/../driverlib/source`

ARM linker file search path options: `${PROJECT_ROOT}/../driverlib/source/ti/devices/msp432p4xx/driverlib/ccs/msp432p4xx_driverlib.lib`

To burn and run the project, use the CCS GUI 
User's guide available [here](https://softwaredl.ti.com/ccs/esd/documents/users_guide_ccs_20.0.0/index.html)


<br>

<p align="right">(<a href="#readme">back to top</a>)</p>

<!-- USER GUIDE -->
# User guide

The user interface is designed to be highly intuitive and user-friendly. 
Navigate the menu using the two tactile buttons on the right side of the BoosterPack MKII: press the lower button to scroll down, the upper button to scroll up. Press the joystick's SELECT button to confirm the chosen menu function. Once in, if the button SELECT is pressed again you can return to the main menu. In the event that an alarm is triggered, due to temperature conditions, the red LED will light up and the buzzer will sound. Once you have addressed the underlying issue, press SELECT one more time to silence the buzzer and turn off the red LED, restoring the system to normal operation.



<p align="right">(<a href="#readme">back to top</a>)</p>



<!-- FUTURE UPGRADES -->
# Future upgrades

### 1. Onboard Battery Backup  
Adding a rechargeable Li‑ion or Li‑Po battery and a power‑path management circuit can lead to autonomous operation without a constant PC connection.  Also uninterrupted real‑time clock (RTC) and settings retention through power loss or optional low‑battery warning routine can be implemented.  

### 2. Configurable PIR Hold‑Time in Firmware  
If we move “HIGH” hold‑time configuration from hardware trimmers into a firmware parameter (for example 5–30 seconds) we can create precise, software‑driven control of motion-detection duration.

### 3. ESP‑Powered Remote Connectivity  
An ESP32/ESP8266 module can be integrated to host a web server and expose RESTful APIs. This way, we can have mobile or desktop access for real‑time control and alerts and enhance user comfort and security.
Some examples of features that can be added are: 
  - Web‑based dashboard (for monitor conditions and change settings)  
  - SSL/TLS encryption and user authentication  
  - Push notifications via MQTT or Firebase  


<br>

<p align="right">(<a href="#readme">back to top</a>)</p>

<!-- USEFUL LINKS -->
# Useful links
- [Video presentation]()
- [Presentation]()
- [Pdf presentation]()

<p align="right">(<a href="#readme">back to top</a>)</p>


<!-- TEAM MEMBERS -->
# Team members
Every team member is responsible and worked together for the whole project. We worked to the project meeting each other in person everytime.
- Elisa Raffaella Sterpu (elisa.sterpu@studenti.unitn.it)
- Mattia Bernabè (mattia.bernabe@studenti.unitn.it)
- Stefano Corelli (stefano.corelli@studenti.unitn.it)
- Nicolas Venturi (nicolas.venturi@studenti.unitn.it)

<p align="right">(<a href="#readme">back to top</a>)</p>
