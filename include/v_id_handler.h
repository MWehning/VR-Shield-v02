#include <Arduino.h>
#include <Wire.h>

// Setup
bool getDeviceData();
bool setupDevices();

// Debug Tools
void printOutMask();

// Data
void getData(float buffer[],byte device_id, byte data_id);