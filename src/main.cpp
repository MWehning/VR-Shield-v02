#include "bluetooth_handler.h"
//#include "v_id_handler.h"

unsigned long previousMillis;

bool debugflag = false;

void setup() {
  if(SetupBluetooth()&&SetupSerial())
    if(debugflag) 
      Serial.println("VR-Shield v2");
  previousMillis = millis();
  //Publish(IniPacket());
}

void loop() {
  switch (Receiver())                             // 0= No Packet 1=D-Type 2=M-Type 3=Unknown, puts contents into Sring receivedPackage
  {
  case 1:
      if(debugflag){
        printPackageContents('D');
      } 
      // TODO: RESPOND (EXAMPLE WITHOUT VALUES)
      if(PacketBuilder(0x1,0x2,303,0x4,505,0x6,707)){
        Publish(byteR);
      }
      
      //Publish(PacketBuilder(contentsD[0],contentsD[2],4141,contentsD[3],4242,contentsD[4],4343));    
      if(debugflag) 
        printPackageContents('R');
      break;
  
  case 2:
      if(debugflag) 
        printPackageContents('M');

      // TODO: CAST EVENT AND RESPOND (EXAMPLE OF CONFIRMATION)
      if(PacketBuilder(0x1,0x1,1,0x1,1,0x1,1)){
        Publish(byteR);
      }
      if(debugflag) 
        printPackageContents('R');
      break;

  case 3:
    if(debugflag) {
      Serial.println("Unknown package");
    }
    Publish(error);                     // faulty message arrived
    break;

  default:                                      // nothing arrived
    break;
  }
    

  if (millis() - previousMillis > 1000) {
    if(!debugflag){
      //Publish(PacketBuilder(16,1,1234,2,360));  // TODO: WHY DOES LEADING ZERO LEAD TO DIFFERENT ID?
      //printPackageContents('R');
    } 

    previousMillis  = millis();
  }
}