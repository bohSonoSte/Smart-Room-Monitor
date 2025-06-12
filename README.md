<div id="readme-erpc"></div>

<!--TITLE-->
<br />
<div align="center">

<h3 align="center">Smart Light System</h3>



<img src="" alt="Logo" width="300" height="250">
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
We've all been there at least once, watching a movie on our computer while sitting on the bed. In these moments, simple actions like adjusting the volume or pausing the movie can be annoying. That's why we came up with the ERPC project (Essential Remote PC Control), a device built with the Texas Instruments MSP432P401R board, the educational boosterpack MKII and the ESP-EYE. By using the joystick and some buttons, the remote control can emulate mouse functions, control the computer's volume, open a virtual keyboard and exit full-screen mode. 
<br>These are just some of the functions the remote control can perform. <br>By adding the appropriate sensors, many other features could be integrated, such as voice search through a microphone or the inclusion of a touch screen.
<br>


<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- BUILT WITH -->
## Built with
C <br>  
Python
 
<br>


<h3 style="margin-left: 20px;" id="software-requirements">Software requirements</h3>
<p style="margin-left: 40px;">Software required to program the boards: <br>
 &nbsp; &nbsp; → visual studio code: https://code.visualstudio.com/download <br>
 &nbsp; &nbsp; → code composer studio: https://www.ti.com/tool/download/CCSTUDIO/12.7.0 <br>
 &nbsp; &nbsp; → arduino ide (for ESP-EYE): https://www.arduino.cc/en/software <br>
 &nbsp; &nbsp; → only for linux install Libasound2 dev with the command: "apt-get install libasound2-dev"
</p>

<h3 style="margin-left: 20px;" id="hardware-requirements">Hardware requirements</h3>
<p style="margin-left: 40px;">Hardware components needed to built the device: 

* Texas Instruments MSP432P401R microcontroller 
* Educational BoosterPack MKII 
* ESP-EYE (ESP32) 
* Server with Python (PC)
* Items for supply (3 batteries AA, copper or tin, insulating tape and electrical wires)
* Additional buttons and potentiometer (welded to one perfboard)
* Items for the box (plywood, screws)

</p>

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- SETTING UP THE HARDWARE -->
## Setting up the hardware
The installed microcontroller requires a power supply between 3.5V and 5V to function properly. During testing, it was found that 3.5V is not sufficient to power all components. To solve this issue, three 1.5V batteries were connected in series, resulting in a total voltage of 4.5V. The assembly was carried out using electrical tape, copper wires to connect the batteries, and electrical cables to link them to the board’s power input pin.
<br>There is no need to worry about components that support a maximum of 3.5V, as they are equipped with voltage regulation circuits. In fact, the presence of resistors ensures that the input voltage is reduced when necessary.
<br>As for the ESP-EYE, it is powered directly by the board through the 5V output pins, using electrical wires for the connection.
<br>A button and a potentiometer still need to be connected. They are mounted on a breadboard and soldered with tin to ensure greater stability. Finally, the entire circuit has been placed inside a plywood box, making it more convenient to use and transport.

<br>

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- PROJECT STRUCTURE -->
## Project structure
<pre>

Docs
  ├── Presentation
          ├── ERPC.pdf                  
          ├── ERPC.pptx                 
          └── ERPC_photo.jpg       
  ├── Specifications
          ├── ESP32 configuration tasks.odt             
          ├── ESP32 key features.odt                    
          ├── esp32_datasheet_en.pdf                    
          ├── microcontroller_comparision.md            #Comparison between nRF52 DK and ESP-EYE
          └── nRF52_DK_User_Guide_v3.x.x.pdf            


ServerPC	              #server/client communication
  ├── LinuxVolume.c           #volume management
  ├── Server.py               #UDP connection between MSP432P401R and ESP32
  └── setup.py                #script compiles and installs a Python C-extension module


ESP32 project
  └── ESP32.ino       #firmware for sending data to PC


MSP432P401R project
  ├── targetConfigs				                                     #MSP432 configuration
  ├── main.c					                                     #main for MSP432
  └── msp432p401r.cmd, startup_msp432p401r_ccs.c, system_msp432p401r.c               #linker configuration, interrupt management, mcu startup 

</pre>
<br>

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- FUNCTIONALITIES -->
## Functionalities
<p>This project allows us to control a PC remote by using a socket connection between the ESP32 and the computer. The main functionalities of the remote control are:</p>

<ol>
  <li>Initial connection and network setup</li>
  <li>PC control
    <ol>
      <li>Left button </li>
      <li>Right button </li>
      <li>Esc button </li>
      <li>Joystick: cursor movement</li></ol></li>
      <li>Volume control (with a potentiometer)</li>
      <li>Opening the virtual keyboard </li>
</ol>
<br>On the MSP432, the buttons are configured and linked to the respective pins described above. <br>
When a button is pressed, so a specific action is activated, it transmits the command to the ESP-EYE, which then becomes a real action. <br>
Depending on the button, I can trigger actions such as mouse buttons, volume control, escape button, and virtual keyboard opening.


<br>

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- USER GUIDE -->
## User guide
### Configuring the Python Server (`Server.py`)

In the `Server.py` file, you need to set the IP address and port where the server will receive data from the ESP-EYE.

#### Modifying the Settings
Locate these lines in the file:
```python
# Server settings
UDP_IP = "0.0.0.0"  # Listen on all network interfaces
UDP_PORT = 5006      # Listening port
```

#### Changing the IP
If you want to specify a precise server IP (e.g., `192.168.1.100`), modify the line as follows:
```python
UDP_IP = "192.168.1.100"
```
Ensure that the PC is connected to the same network as the ESP-EYE.
<br>
<br>
### Configuring the ESP-EYE Client (`esp.ino`)

In the `esp.ino` file, you need to specify:
- SSID and password of the WiFi network
- The IP address of the Python server

Find these lines:
```cpp
const char* ssid = "";         // your wifi ssid
const char* password = "";     // your wifi password
const char* hostname = "ERPC"; // the host name of this device
const char serverAddr[] = "";  // the address of the server (local ip address)
```

#### Setting Up WiFi Connection
Enter your WiFi network details:
```cpp
const char* ssid = "YourSSID";
const char* password = "YourPassword";
```

#### Setting the Server IP Address
Modify `serverAddr` with the IP set in `Server.py`, for example:
```cpp
const char serverAddr[] = "192.168.1.100";
```
<br>

### Network Considerations
- **The PC and ESP-EYE must be on the same local network**. If the ESP-EYE is connected via WiFi, the PC must be on the same WiFi or LAN network.
- **If the Python server is on a different network**, you must configure **port forwarding** on the router to forward traffic on port `5006` to the server's IP.

Following these steps, your ESP-EYE will be able to communicate correctly with the Python server.
<br>
<br>


### Install the library on linux
You need to run two commands
```cpp
python3 setup.py build
python3 setup.py install
```

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- FUTURE UPGRADES -->
## Future upgrades
<p style="margin-left: 40px;"> Some of the possible future updates are listed below:  

* **Default IP address for connection**: ensure the device connects only to networks with specific IP addresses to limit connections to authorized devices.
* **Encrypted connection**: implement a secure connection to protect the data travelling over the network, by using a protocol like TLS/SSL.
* **Support for additional hardware functions**: add support for other hardware devices, such as motion sensors or pressure sensors; or even a microphone for voice control.
* **Visual feedback via screen**: add a screen to the system to provide real-time information, such as connection status.

</p>
<br>

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>

<!-- USEFUL LINKS -->
## Useful links
Video: [https://drive.google.com/file/d/1agivKv0jqHUVvvNMQd2S5w90nPasPmut/view?usp=sharing](https://drive.google.com/file/d/1vc2E4uRM63oCAUw0LnEkV0MCEFqxgfME/view?usp=sharing)
<br> Presentation: [ERPC.pptx](https://docs.google.com/presentation/d/1srpceYGneGkNDBblu5enkKkrtPL8FoVE/edit?usp=sharing&ouid=112294706705879927242&rtpof=true&sd=true)
<br> Project link: https://github.com/Fb1234566/ERPC.git
<br>

<p align="right">(<a href="#readme-erpc">back to top</a>)</p>


<!-- TEAM MEMBERS -->
## Team members
Filippo Benedetti <br>
&nbsp; &nbsp; - Contributed on the creation of the container for the MSP432P401R and ESP-EYE <br>
&nbsp; &nbsp; - Managed the configuration of the potentiometer <br>
&nbsp; &nbsp; - Contributed in the development of a protocol to transmit the values through a socket connection <br>
&nbsp; &nbsp; - Mail: filippo.benedetti3@gmail.com
<br><br>
Beatrice Faccioli <br>
&nbsp; &nbsp; - Contributed on looking for codes that could be useful for the project <br>
&nbsp; &nbsp; - Managed the button for the virtual keyboard <br>
&nbsp; &nbsp; - Managed the documentation, presentation and video <br>
&nbsp; &nbsp; - Mail: beatricefaccioli.v@gmail.com
<br> <br>
Matteo Marchiori <br>
&nbsp; &nbsp; - Contributed on searching libraries and code examples in python for socket communication <br>
&nbsp; &nbsp; - Worked on the configuration of the joystick movement and left button <br>
&nbsp; &nbsp; - Contributed in the development of a protocol to transmit the values through a socket connection <br>
&nbsp; &nbsp; - Mail: matteo.marchiori03@gmail.com
<br> <br>
Lucia Pecora <br>
&nbsp; &nbsp; - Contributed on power supply of the boards <br>
&nbsp; &nbsp; - Worked on the configuration of the pins for the right and esc button <br>
&nbsp; &nbsp; - Managed the documentation, the presentation and the video <br>
&nbsp; &nbsp; - Mail: luciapecora01@gmail.com
<br>



<p align="right">(<a href="#readme-erpc">back to top</a>)</p>
