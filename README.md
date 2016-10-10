# arduboysmartdisplay
A library for a better performance on updating the ArduBoy display.

## Description

An small benchmark program for comparing the default screen update mechanism versus only updating the parts of the screen that really changed. The new mechanism is located in SmartDisplay.h/.cpp and can be easy integrated in other projects.

## Controls:
* A - default mechanism / smart display
* B - pause the benchmark
* Up / Down - control the number of items to draw (with a maximum of 25)

## Installation
1. Download Zip or Git Clone from: https://github.com/sersanchus/arduboysmartdisplay
2. Open benchmark.ino with Arduino IDE
3. Compile and upload to your Arduboy