<div id="readme_sls"></div>

<!--TITLE-->
<br />
<div align="center">

<h3 align="center">Smart Light System</h3>


<br>


 <p align="center">
 
  <br />
  <br /><br />
  </p>
  </div> 

<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol style="counter-reset: section;">
    <li><a href="#project-introduction">Project introduction</a></li>
    <li><a href="#built-with">Built with</a>
    <ol>
    <li><a href="#software-requirements">Software requirements</a></li>
    <li><a href="#hardware-requirements">Hardware requirements</a></li>
    </ol></li>
    <li><a href="#setting-up-the-hardware">Setting up the hardware</a></li>
    <li><a href="#project-structure">Project structure</a></li>
    <li><a href="#functionalities">Functionalities</a></li>
    <li><a href="#user-guide">User guide</a></li>
    <li><a href="#future-upgrades">Future upgrades</a></li>
    <li><a href="#useful-links">Useful links</a></li>
    <li><a href="#team-members">Team members</a></li>
  </ol>
</details>
<br>
<!--
<style>
    ol{
        list-style-type: none;
    }
    ol > li {
        counter-increment: section;
    }
    ol > li::before {
        content: counter(section) ". "; 
    }
    ol ol > li {
        counter-increment: subsection;
    }
    ol ol > li::before {
        content: counter(section) "." counter(subsection) " ";
    }
    </style> -->



<!-- PROJECT INTRODUCTION -->
## Project introduction
Meet the MSP432-powered Smart Room Monitor: your all-in-one environmental sidekick. With a clear always-on time/date display, four easy-to-navigate modes put total control at your fingertips. It keeps track of ambient heat and the intensity of light in the room. It can recalibrate an erroneous clock and select the preferred illumination mode.
With built-in safety alerts and effortless customization, mastering your room has never been this simple, or this smart.


<br>


<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- BUILT WITH -->
## Built with
C <br>  

<br>


<h3 style="margin-left: 20px;" id="software-requirements">Software requirements</h3>
<p style="margin-left: 40px;">Software required to program the boards: <br>
Code Composer Studio (https://www.ti.com/tool/CCSTUDIO) <br>
- MSP Driver Library (https://www.ti.com/tool/MSPDRIVERLIB) <br>
- MSP Graphics Library (https://www.ti.com/tool/MSP-GRLIB) <br>
</p>


<h3 style="margin-left: 20px;" id="hardware-requirements">Hardware requirements</h3>
<p style="margin-left: 40px;">Here we show the hardware components needed to built the device. <br> <br>
→ Texas Instruments MSP432P401R microcontroller <br>
<img src=Docs/MSP432-Board.jpg width="50%"> <br><br>
The Texas Instruments MSP432P401R is a 32-bit microcontroller designed for embedded applications. It is built around the ARM Cortex-M4F Core and it supports low-power applications that require increased CPU speed, memory, analog, and 32-bit performance. It features a system clock speed of up to 48 MHz and includes 256 KB of flash memory, 64 KB of SRAM and 32 KB of ROM preloaded with SimpleLink MSP432 SDK libraries. The microcontroller offers a rich set of peripherals: there are four 16-bit timers with capture, compare or PWM modes, two 32-bit timers and a real-time clock (RTC). In particular, we used in our project the RTC to show the exact time and date on the display. This topic will be discussed in the next sections. For communication, it supports up to eight serial channels, including I²C, SPI and UART.



→ BOOSTXL-EDUMKII Educational BoosterPack <br>
<img src=Docs/boosterpack.jpg width="50%"> <br><br>
The BOOSTXL-EDUMKII Educational BoosterPack is a plug-in module that offers various analog and digital inputs and outputs. The module also includes an analog joystick, environmental and motion sensors, RGB LED, a microphone, buzzer, color LCD display and more. In our project we used: <br>
&nbsp; &nbsp; - **TI OPT3001 light sensor** = it's a digital ambient light sensor. It measures the intensity of light (visible by the human eye) present in the room. The measurement can go from 0.01 lux to 83k lux. We used the sensor to module the color of the LED.
<br>
&nbsp; &nbsp; - **TI TMP006 temperature sensor** = It's a digital infrared thermopile contactless temperature sensor that measures the temperature of an object without being in direct contact. It is specified to operate from -40°C to +125°C. We used it to measure the temperature in the room.
<br>
&nbsp; &nbsp; - **Kionix KXTC9-2050** = It's a 3-axis analog accelerometer that measures g-forces. If the board moves along the axes, the analog signal generated will change. The full-scale output range is of +/-2g (19.6 m/s^2). The sensing is based on the principle of a differential capacitance arising from acceleration-induced motion of the sense element. We used this accelerometer to detect earthquakes and to generate an alarm message.
<br>
&nbsp; &nbsp; - **Cree CLV1A-FKB RGB multicolor LED** = It can output an high intensity light of any color by mixing red, green and blue. To achieve a specific color, each color channel can be individually modified by pulse width modulation (PWM). In our project is the main light source of the room.
<br>
&nbsp; &nbsp; - **CUI CEM-1203(42) piezo buzzer** = This piezo buzzer can play various frequencies based on the provided PWM signal. We used it as the audible alarm signal.
<br>
&nbsp; &nbsp; - **Color 128x128 TFT LCD display** = This small display packs 128x128 full-color pixels into one square inch of active display area. It updates up to 20 frames per second. It has a color depth of 262k colors and a contrast ratio of 350. It displays the main menu.
<br>
&nbsp; &nbsp; - **ITEAD studio IM130330001 2-axis joystick with pushbutton** = It's composed of two potentiometers, one for each axis. The select button is present and is actuated when the joystick is pressed down. We used the joystick mainly for the select button, to choose various options in the menu.
<br>
&nbsp; &nbsp; - **User push buttons** = They are connected to pullup resistors that drive the pin low if the buttons are pressed. We are used to scroll the menu or to change the time of the clock. 
<br>


→ HC-SR501 PIR Sensor <br>
<img src=Docs/pir.png width="30%"> <br><br>
The HC-SR501 PIR is a motion detector module. It senses infrared radiation changes in the environment. In our case, the radiation of human body heat is interpreted as motion. When motion is detected, it outputs a digital signal HIGH (3.3V), otherwise it stays LOW. The detection range can be adjusted between 3 to 7 meters. Also the delay time can be adjusted from 5 seconds to 5 minutes.The sensor operates on 5V but can handle a voltage range of 5V to 20V, and it consumes very little power—around 50 microamps when idle. We connected the PIR to pin P6.4 using a breadboard and some cables.



</p>

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- SETTING UP THE HARDWARE -->
## Setting up the hardware


<img src=Docs/EMBEDDED.png width="70%"> <br><br>

First, we insert the BoosterPack into the MSP432, ensuring every pin is correctly aligned. Then we connect the MSP432 to the PC using a USB-A to Micro USB cable.
To connect the PIR sensor to the board, we used male-to-female jumper wires and a breadboard. Following the motion sensor's datasheet, we wired the VCC to the board's 5V pin, GND to the board's ground and the DATA to pin P6.4. 

<br>

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- PROJECT STRUCTURE -->
## Project structure
The main menu, continuously displaying time and date, provides access to all system functions:

"Temperature"
Displays the current ambient temperature. 
*foto temperatura*

"Light"
Reports the current illuminance in lux, enabling precise monitoring of ambient lighting conditions.
*foto lux*

"Change Time"
Allows manual adjustment of the real-time clock to compensate for drift, Daylight Saving Time shifts, or initial configuration. Dedicated buttons increment and decrement hours, minutes, day, month, and year; pressing SELECT confirms the updated time/date.

"LED"
Controls the onboard RGB LED. In "Auto" mode, the LED color dynamically maps to measured illuminance. For a low lux value, we have a warm yellow, for high lux, a cool white. The user can also select one of several fixed color outputs: blue, green, red, or cold white.

If the temperature exceeds a predefined high or low thresholds (+40°C and 0°C), or if the onboard accelerometer detects significant motion (for example an earthquake), the system activates a visual alert (red LED) and an audible alarm (buzzer). We need to press SELECT to acknowledge and silence the alarm.

Each function is accessed via the tactile buttons on the BoosterPack MKII, ensuring intuitive navigation and reliable operation in a compact embedded system.






<p align="right">(<a href="#readme-erpc">back to top</a>)</p>


<!-- PROJECT LAYOUT -->
## Project Layout

ORGANIZZAZIONE PROGRAMMA/CODICE

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>



<!-- come includere librerie e far andare progetto -->
## Getting Started

COME FAR AVVIARE IL PROGRAMMA

<br>

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- USER GUIDE -->
## User guide

In our MSP432-powered Smart Room Monitor, you're the master of micro-kingdom. From the main menu, complete with always-on time and date, you can dive into many functions. The "Temperature" mode shows the room temperature. If the room it's too hot or too cold, the board flashes a red LED and emits an insistent buzz. The alarm is also triggered if the accelerometer senses an earthquake. If the emergency is finished, press SELECT to stop the alarm. Next,for an instant lux reading select the "Light" mode. Need to correct for Daylight Saving Time or simply recalibrate an errant clock? No problem, with the mode "Change Time" it's possible to get the clock back in sync. We only need the buttons to increase, decrease and select the time. The real party is under the LED mode: choose dynamic ambient coloring that shifts from cozy yellow at dusk to dazzling white at noon. You can also pick other different colors: blue, gree, red or arctic cold white.  Who ever thought room control could be so easy?

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- PROBLEMS    DA METTERE????????-->
<!-- ## Problems and how we solved them
Pir senson pin -->


<!-- FUTURE UPGRADES -->
## Future upgrades
ESP
batteria
tempo attivazione pir non analogico

<br>

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- USEFUL LINKS -->
## Useful links


<p align="right">(<a href="#readme-erpc">back to top</a>)</p>


<!-- TEAM MEMBERS -->
## Team members

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>
