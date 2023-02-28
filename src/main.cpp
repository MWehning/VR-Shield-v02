
#include "bluetooth_handler.h"

unsigned long previousMillis;

const bool debugflag = true;

void setup() {
  if(SetupBluetooth()&&SetupSerial())
    if(debugflag) Publish("VR-Shield v2");
  previousMillis = millis();
  //Publish(IniPacket());
}

void loop() {
  switch (Receiver())
  {
  case 1:
    
    if(DecoderD(receivedPackage)){                // relocates package values into array
      if(debugflag) printPackageContents('D');    

      // TODO: RESPOND (EXAMPLE WITHOUT VALUES)
      Publish(PacketBuilder(contentsD[0],contentsD[2],1234,contentsD[3],360,contentsD[4],90));    
      if(debugflag) printPackageContents('R');

    }else{
      if(debugflag) Serial.println("D-Type\nDevice does not exist or frame was violated");
      Publish(ErrorPacket(1));                     // Deep error in message frame
    }

    break;
  
  case 2:

    if(DecoderM(receivedPackage)){
      if(debugflag) printPackageContents('M');

      // TODO: CAST EVENT AND RESPOND (EXAMPLE OF CONFIRMATION)
      Publish(PacketBuilder(contentsM[0],1,1));
      if(debugflag) printPackageContents('R');

    }else{
      if(debugflag) Serial.println("M-Type\nDevice does not exist or frame was violated");
      Publish(ErrorPacket(2));                   // Deep error in message frame
    }

    break;

  case 3:
    if(debugflag) {
      Serial.println("Unknown package");
      Serial.println(receivedPackage);
    }
    Publish(ErrorPacket(3));                     // faulty message arrived
    break;

  default:                                      // nothing arrived
    break;
  }
    

  if (millis() - previousMillis > 1000) {
    if(!debugflag){
      Publish(PacketBuilder(16,1,1234,2,360));  // TODO: WHY DOES LEADING ZERO LEAD TO DIFFERENT ID?
      printPackageContents('R');
    } 

    previousMillis  = millis();
  }
}