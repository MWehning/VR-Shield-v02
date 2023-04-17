#include <Arduino.h>
#include <Wire.h>

// device matrice sizing 
#define SAME_DEVICE_COUNT 5
#define DEVICE_TYPES      4
#define TOTAL_DEVICES     SAME_DEVICE_COUNT*DEVICE_TYPES

// pin defines
#define LED_PIN           32
#define I2C_POWER         19

// device types
#define MPU               0
#define QMC               1
#define BHL               2
#define REL               3

// Device type conversion: 
// _00_|_01_|_02_|_03_|_04_|________
// 0x16| ...|    |    |    | 0=>MPU
// ... |    |    |    |    | 1=>QMC
//     |    |    |    |    | 2=>BHL
//     |    |    |    |    | 3=>REL
byte vid_mask[DEVICE_TYPES][SAME_DEVICE_COUNT]={ 0 };
byte adresses[TOTAL_DEVICES]={ 0 };


// writes a single message byte to an I²C device
// @param addr is the address of said device
// @param message is the message byte
void wireWrite(byte addr, byte message){
  Wire.beginTransmission(addr);
  Wire.write(message);
  Wire.endTransmission(true);
}

float wireGet(){
  return(Wire.read() << 8 | Wire.read());
}

void setupMPU(){
  for(int i = 0;i<SAME_DEVICE_COUNT;i++){
    byte addr = vid_mask[0][i];
    if(addr!=0){
      Wire.beginTransmission(addr);       // Start communication with MPU6050 // MPU=0x68
      Wire.write(0x6b);                  // Talk to the register 6B
      Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
      Wire.endTransmission(true); 
      
      // wireWrite(addr,0x1C);
      // wireWrite(addr,0x10); // +/- 8g full scale range
      // wireWrite(addr,0x1B); 
      // wireWrite(addr,0x10);// 1000deg/s full scale
    }
  } 
}

void setupQMC(){
  for(int i = 0;i<SAME_DEVICE_COUNT;i++){
    byte addr = vid_mask[1][i];
    if(addr!=0){
      Wire.beginTransmission(addr);       // Start communication with QMC
      Wire.write(0x09);                  // Talk to the register 09
      Wire.write(0x1d);                  
      Wire.endTransmission(true);
    }
  } 
}

void  setupBHL(){
  for(int i = 0;i<SAME_DEVICE_COUNT;i++){
    byte addr = vid_mask[2][i];
    if(addr!=0){
      wireWrite(addr,0x10);
      //wireWrite(addr,0x0f);
      // TODO:Implement Measurement time long
      //wireWrite(addr,0x0f);   // HIGHBYTE
      //wireWrite(addr,0x0f);   // LOWBYTE
    }
  } 
}

void setupREL(){
  for(int i = 0;i<SAME_DEVICE_COUNT;i++){
    byte addr = vid_mask[0][i];
    if(addr!=0){
      wireWrite(addr,0x00);
    }
  } 
}

// Gets Data from an I²C device specified in other arguments  // TODO: Doesnt return anything if device is only added in rescan
// @param values buffer array to store values in
// @param type device type based on type table(see top of code)
// @param count device count 
// @return if it was able to get a value
bool getData(float values[],byte type, byte count){  

  if(type<0 || type>DEVICE_TYPES || count<0 || count>SAME_DEVICE_COUNT){  // check if V_ID is in range
    return false;
  }

  byte addr = vid_mask[type][count];
  
  if(addr == 0){                                                          // check if ID empty
    return false;                                                         
  }

  bool available = false;
  for(int i = 0;i<TOTAL_DEVICES;i++){
    if(addr==adresses[i]){                // search until end of adress list or until device is found in there
      available = true;
      break;
    }
  }

  if(!available){                         // no device found
    return false;
  }

  switch (type)
  {
  case MPU:
    wireWrite(addr,0x3b);
    if(Wire.requestFrom((int)addr, 6) != 6)return false; 
    for(int i = 0;i<3;i++){
      values[i]=wireGet();
    }
    wireWrite(addr,0x43);
    if(Wire.requestFrom((int)addr, 6) != 6)return false;  
    for(int i = 3;i<6;i++){
      values[i]=wireGet();
      }
    return true;

  case QMC:
    wireWrite(addr,0x00);
    if(Wire.requestFrom((int)addr, 6) != 6)return false; 
    for(int i = 0;i<3;i++){
      values[i]=wireGet();
    }
    return true;

  case BHL:
    if(Wire.requestFrom((int)addr, 2) != 2)return false;
    values[0]=wireGet();
    return true;

  case REL:
    // TODO: get current Relais state
    break;
  
  default:
    break;
  }
  return false;
}

void addToList(byte mux,byte addr){
  for(byte p=0;p<TOTAL_DEVICES;p++){
    if(adresses[p]==0){
        adresses[p] = addr;     // *puts device into occupied address list
        Serial.printf("Added device 0x%02x at %d\n",addr,p);
        break;
    }
  }
}

// Inserts addr into the V_ID Mask, type is decided by 
// value segregation, first empty slot is used for count 
// @param mux Multiplexer to be used
// @param addr Address of device to be used
// @return false if device somehow cant be added
bool addToMask(byte mux ,byte addr){ 

  byte type = 0;    
  switch (addr)
  {
  case 0x60 ... 0x70:         // *These define which address could be which device
    type = MPU;
    setupMPU();
    break;
  case 0x00 ... 0x0f:
    type = QMC;
    setupQMC();
    break;
  case 0x20 ... 0x30:
    type = BHL;
    setupBHL();
    break;
  case 0x40 ... 0x50:
    type = REL;
    setupREL();
    break;
  
  default:
    return false;           // Not an address the system can work with
    break;
  }   

  for(byte count = 0;count<SAME_DEVICE_COUNT;count++){
    if(vid_mask[type][count] == 0){
      vid_mask[type][count] = addr; // *Add working device to VID Mask 
      addToList(mux,addr);
      return true;
    }
  }
  return false;
}


// removes a device from the mask and the list
// @param type device type based on type table(see top of code)
// @param count device count 
// return true once device removed
bool removeFromMask(byte type, byte count){
  byte addr = vid_mask[type][count];
  vid_mask[type][count] = 0;            // remove from mask
  for(byte i = 0; i<TOTAL_DEVICES;i++){ // remove from list
    if(adresses[i]==addr){
      adresses[i] = 0;
      break;
    }
  }
  return true;

}

// Attempt to reach device at addr
// @param mux Multiplexer to be used
// @param addr Address of device to be usedr
// @return state of device
bool reachOut(byte mux,byte addr){       
  Wire.beginTransmission(addr); 
  if(Wire.endTransmission(true)){
    return false;
  }
  //Serial.printf("Found device at 0x%02x\n",addr);
  return true;
}

// Loop through range of adresses and put ones with
// device attached into V_ID mask (range = 0-127)
bool getDeviceData(){
  bool connections = false;
  for(int addr_tryout = 0x00;addr_tryout<0x80;addr_tryout++){  
    if(reachOut(0x01,addr_tryout)){
      addToMask(0x01,addr_tryout);
      connections = true;
    }
  }


  /* for(byte i=0;adresses[i]!=0;i++){
    Serial.printf("0x%02x  ",adresses[i]);
  }
  Serial.println(""); */


  return connections;
}


// debug tool for checking device function
void healthCheck(bool debugflag){
  for(int i=0;i<SAME_DEVICE_COUNT;i++){   // get values from all connected MPUs
    if(vid_mask[MPU][i]!=0){
      float values[6] = { 0 };
      if(reachOut(0,vid_mask[MPU][i])){   // *first check connection, then request data to catch faulty connections
        if(getData(values,MPU,i)){
          if(debugflag)Serial.printf("MPU %01d functional\n", i);
        }
      }else{
        if(debugflag)Serial.printf("MPU %01d doesn't return values, removing from table\n", i);
        removeFromMask(MPU,i);              
      }
    }
  }

  for(int i=0;i<SAME_DEVICE_COUNT;i++){   // get values from all connected QMCs
    if(vid_mask[QMC][i]!=0){
      float values[3] = { 0 };
      if(reachOut(0,vid_mask[QMC][i])){
        if(getData(values,QMC,i)){
          if(debugflag)Serial.printf("QMC %01d functional\n", i);
        }
      }else{
        if(debugflag)Serial.printf("QMC %01d doesn't return values, removing from table\n", i);
        removeFromMask(QMC,i);
      }
    }
  }

  for(int i=0;i<SAME_DEVICE_COUNT;i++){   // get values from all connected BHLs
    if(vid_mask[2][i]!=0){
      float values[1] = { 0 };
      if(reachOut(0,vid_mask[BHL][i])){
        if(getData(values,BHL,i)){
          if(debugflag)Serial.printf("BHL %01d functional\n", i);
        }
      }else{
        if(debugflag)Serial.printf("BHL %01d doesn't return values, removing from table\n", i);
        removeFromMask(BHL,i);
      }
    }
  }
}

// manually triggered by user. Scans for all new devices and then removes ones not responding
void updateDeviceData(bool debugflag){
  bool occupance;                       // flag that tells us if an address is already in use
  for(byte addr_tryout=0x00;addr_tryout<0x80;addr_tryout++){
    for(int i = 0;i<TOTAL_DEVICES;i++){
      occupance = false;
      if(addr_tryout==adresses[i]){
        occupance = true;
        break;
      }
    }
    if(!occupance){
      if(reachOut(0x01,addr_tryout)){   // Only add device again if it isnt in use already
        addToMask(0x01,addr_tryout);
      }   
    }	         
  }
  healthCheck(debugflag);  
}


// Debug tool for displaying all devices contained
// in V_ID Mask (filled out on startup)
void printOutMask(bool debugflag){
  if(debugflag){
    Serial.println("   | MPU | QMC | BHL | REL |");
  for(int i = 0;i<SAME_DEVICE_COUNT;i++){
    Serial.printf("%03d|",i);
    for(int j = 0;j<DEVICE_TYPES;j++){
      if(vid_mask[j][i]==0){
        Serial.printf(" --- |");
      }else{
        Serial.printf(" 0x%02x|",vid_mask[j][i]);
      }
    }
    Serial.println("");
  }
  }
}


// I²C setup and first scan
// @param debugflag If true some information will be printed
void setupDevices(bool debugflag) {
  pinMode(I2C_POWER,OUTPUT);
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(I2C_POWER,LOW);
  digitalWrite(LED_PIN,LOW);
  
  
  delay(1000);
  digitalWrite(I2C_POWER,HIGH);
  Wire.begin();
  Serial.begin(115200);
  delay(1000);

  if(debugflag)Serial.println("Scanning for connected devices...");
  getDeviceData();
  delay(1000);
  if(debugflag)Serial.println("Setting up devices...");
  setupMPU();
  setupQMC();
  setupBHL();
  setupREL();
  delay(1000);
  if(debugflag)Serial.println("Testing function...");
  healthCheck(debugflag);
  delay(1000);
}

