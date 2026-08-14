# MiniPod 

MiniPod is a simple MP3 player which functions using the Arduino Nano, and even has an OLED Display. It has an Audio Jack for the output and plays songs from a MicroSD card. 

It is made so that I can listen to songs offline and without the distractions of a Smartphone in environments like the GYM. It is inspired by the Apple Ipod.

## Features:
- 3D printed body with 6 buttons for controls. 
- 128x32 OLED Display to display current song and artists.
- 6 controls (Pause/Play, Previous, Next, Vol up, Vol down).
- Usb C charging port


## CAD Model:
The pcb is attached to the base at a height of 5mm using 4 M3 screws to have enough space for the battery. The top plate is attached to the body using smaller M1 screws and heat inserts due to narrow walls. 

It has 8 separate printed pieces. The base where the PCB sits, the top cover, the 6 hand made buttons, and the side power button. The project and my name is embossed on the top plate as well.

<img src=assets/Model.png alt="Model" />

Made in Fusion360. 


## PCB
Here's my PCB! It was made in KiCad. The components are placed on both sides of the PCB to save space.

Schematic
<img src=assets/schematics2.PNG alt="Schematic" width = 300/>

PCB
<img src=assets/pcb.PNG alt="Schematic" width =300 />

## Firmware Overview
MiniPod uses two programs for firmware. 

-A python script which renames all songs into simple indexes (000-999), so that they can be easily accesed by the DFPlayer Mini. It also creates a csv file linking all indexes to their song names.

-The main Ardunio script written in C which runs the MiniPod and all its functions. (I used C, because I learned it a few months ago while doing Harvards CS50x).

- The buttons will be hand labelled. 
- There is Vol up and Vol down option as well. 
- The OLED is the song name, artist name, index, and playing status.

## AI Usage:
While I tried to keep AI usage minimum, it was used for these things: 


-All the research and finalization of the components.

-Fusion360 controls help

-Major help in the Ardunio part of the firmware. I tried to not copy paste any code at all, at least not without understanding it first. 

## MicroSD Card Format

Your MicroSD card must be formatted to **FAT16/FAT32** and contain an `index.csv` file in the root folder using the following format:

csv

Index,Title,Artist

000,Bohemian Rhapsody,Queen

001,Hotel California,Eagles

002,Billie Jean,Michael Jackson

## BOM:
These are the components used in the GamePad 9:

-Arduino Nano

-DFPlayer Mini MP3 Audio Player Module

-TRRS Module PJ-320A 3.5mm Female Audio Jack Connector

-0.96 inch I2C SSD1306 128x64 OLED Display Module

-3.7V LiPo Battery 

-TP4056 + Boost Charger Module

-SPDT Power Switch

-5 Tactile Switch 6x6

-1k Ohm Resistor

-2 2.54mm Single Row Female Pin Header Strip

-2 2.54mm Single Row Male Pin Header Strip

-M1.6 Heat-Set Brass Inserts

-M1.6 Machine Screws

-M3 Screws

-3D printed body

## Info
This is my first solo hardware project. Please let me know if you spot any kind a flaw!
