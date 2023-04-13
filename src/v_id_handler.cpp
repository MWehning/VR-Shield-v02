#include <Arduino.h>
#include <Wire.h>

// device matrice sizing 
#define SAME_DEVICE_COUNT 5
#define DEVICE_TYPES      4

// pin defines
#define LED_PIN           32
#define I2C_POWER         19


// Device type conversion: 
// _00_|_01_|_02_|_03_|_04_|________
// 0x16| ...|    |    |    | 0=>MPU
// ... |    |    |    |    | 1=>QMC
//     |    |    |    |    | 2=>BHL
//     |    |    |    |    | 3=>REL
byte vid_mask[DEVICE_TYPES][SAME_DEVICE_COUNT]={ 0 };

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

bool getMPU(float values[],byte pos){
  byte addr = vid_mask[0][pos];   // access device of type MPU(0) at position pos
  wireWrite(addr,0x3b);
  if(Wire.requestFrom(addr, 6, true)!=6)return false; 
  for(int i = 0;i<3;i++){
    values[i]=wireGet();
    Serial.println(values[i]);
  }
  wireWrite(addr,0x43);
  if(Wire.requestFrom(addr, 6, true)!=6)return false;  
  for(int i = 3;i<6;i++){
    values[i]=wireGet();
    Serial.println(values[i]);
  }
  return true;
}

bool getQMC(float values[3],byte pos){
  byte addr = vid_mask[1][pos];   // access device of type QMC(0) at position pos
  wireWrite(addr,0x00);
  if(Wire.requestFrom(addr, 6, true)<1)return false; 
  for(int i = 0;i<3;i++){
    values[i]=wireGet();
    Serial.println(values[i]);
  }
  return true;
}

bool getBHL(float *value, byte pos){
  byte addr = vid_mask[1][pos];   // access device of type BHL(0) at position pos
  if(Wire.requestFrom(addr, 2, true)<1)return false;
  *value=wireGet();
  Serial.println(*value);
  return true;
}


// Inserts addr into the V_ID Mask, type is decided by 
// value segregation, first empty slot is used for count
bool addToMask(byte mux ,byte addr){ 

  byte type = 0;    
  switch (addr)
  {
  case 0x60 ... 0x70:
    type = 0;
    break;
  case 0x00 ... 0x0f:
    type = 1;
    break;
  case 0x20 ... 0x30:
    type = 2;
    break;
  case 0x40 ... 0x50:
    type = 1;
    break;
  
  default:
    return false;
    break;
  }   

  for(byte count = 0;count<SAME_DEVICE_COUNT;count++){
    if(vid_mask[type][count] == 0){
      vid_mask[type][count] = addr; // Add working device to VID Mask 
      //Serial.printf("Wrote 0x%02x to %01d\n",addr,count);
      return true;
    }
  }
  return false;
}

// Attempt to reach device at addr
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
  return connections;
}


// Debug tool for displaying all devices contained
// in V_ID Mask (filled out on startup)
void printOutMask(){
  String typeStr;
  for(int type = 0;type<DEVICE_TYPES;type++){
    for(int count = 0;count<SAME_DEVICE_COUNT;count++){
      if(vid_mask[type][count]==0){
        break;
      }
      typeStr = "";
      switch (type)
      {
        case 0:
          typeStr += "MPU";
          break;
        
        case 1:
          typeStr += "QMC";
          break;
        
        case 2:
          typeStr += "BHL";
          break;

        case 3:
          typeStr += "REL";
          break;
        
        default:
          break;
      }
      Serial.printf("%s%02x  ||  0x%02x\n",typeStr,count,vid_mask[type][count]);
    }
  }
}

// debug tool for checking device function
void healthCheck(){
  for(int i=0;i<SAME_DEVICE_COUNT;i++){   // get values from all connected MPUs
    if(vid_mask[0][i]!=0){
      float values[6] = { 0 };
      if(getMPU(values,i)){
        Serial.printf("MPU %01d functional\n", i);
      }else{
        Serial.printf("MPU %01d doesn't return values\n", i);
      }
    }
  }

  for(int i=0;i<SAME_DEVICE_COUNT;i++){   // get values from all connected QMCs
    if(vid_mask[1][i]!=0){
      float values[3] = { 0 };
      if(getQMC(values,i)){
        Serial.printf("QMC %01d functional\n", i);
      }else{
        Serial.printf("QMC %01d doesn't return values\n", i);
      }
    }
  }

  for(int i=0;i<SAME_DEVICE_COUNT;i++){   // get values from all connected BHLs
    if(vid_mask[2][i]!=0){
      float value = 0;
      if(getBHL(&value,i)){
        Serial.printf("BHL %01d functional\n", i);
      }else{
        Serial.printf("BHL %01d doesn't return values\n", i);
      }
    }
  }
}

void setupDevices() {
  pinMode(I2C_POWER,OUTPUT);
  pinMode(LED_PIN,OUTPUT);
  digitalWrite(I2C_POWER,LOW);
  digitalWrite(LED_PIN,LOW);
  
  
  delay(1000);
  digitalWrite(I2C_POWER,HIGH);
  Wire.begin();
  Serial.begin(115200);
  delay(1000);

  Serial.println("Scanning for connected devices...");
  getDeviceData();
  delay(1000);
  Serial.println("Scanning done, found:");
  delay(1000);
  Serial.println("Setting up devices...");
  setupMPU();
  setupQMC();
  setupBHL();
  setupREL();
  delay(1000);
  Serial.println("Testing function...");
  healthCheck();
  delay(1000);
}

void getData(float buffer[],byte device_id, byte data_id){
byte type = device_id>>4;     //  Left 4 bits
byte count= device_id & 0xf;  //  Right 4 bits
switch (type)
{
case 0:
    getMPU(buffer, count);
    break;
case 1:
    getQMC(buffer, count);
    break;
case 2:
    getBHL(buffer, count);
    break;

default:
    break;
}
}
