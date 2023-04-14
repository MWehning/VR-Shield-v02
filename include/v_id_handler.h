#include <Arduino.h>
#include <Wire.h>

// Setup

bool getDeviceData();
void updateDeviceData();
bool setupDevices(bool debugflag);

// Debug Tools

void printOutMask();

// Data

void getData(float buffer[],byte type, byte count);