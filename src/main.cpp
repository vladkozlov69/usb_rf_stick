#include <Arduino.h>
#include <RCSwitch.h>
#include <FlashAsEEPROM_SAMD.h>

RCSwitch mySwitch = RCSwitch();

unsigned long NVS_SIZE = 64L;
const int WRITTEN_SIGNATURE = 0xBEEFDEED;

void eraseEEProm();
void checkEEPromFormatted();

void setup() 
{
    Serial.begin(9600);
    while (!Serial);

    mySwitch.enableReceive(EXTERNAL_INT_10); 
    mySwitch.enableTransmit(9);

    pinMode(8, INPUT_PULLUP);

    if (digitalRead(8) == LOW)
    {
        eraseEEProm();
    }
    else
    {
        checkEEPromFormatted();
    }
    
}

void loop() 
{
    if (mySwitch.available()) 
    {
        Serial.print("Received ");
        Serial.print( mySwitch.getReceivedValue() );
        Serial.print(" / ");
        Serial.print( mySwitch.getReceivedBitlength() );
        Serial.print("bit ");
        Serial.print("Protocol: ");
        Serial.println( mySwitch.getReceivedProtocol() );

        mySwitch.resetAvailable();
    }

    delay(1000);
    Serial.println(EEPROM.length());

    unsigned long data;

    for (unsigned long i = 0; i < 4; i++)
    {
        Serial.println(EEPROM.get(i*4, data), HEX);
    }
    

    //delay(5000);
    //mySwitch.send(7946242, 24);
}

void eraseEEProm()
{
    for (int i = sizeof(SAMD_EEPROM_EMULATION_SIGNATURE); i < EEPROM_EMULATION_SIZE; i++)
    {
      EEPROM.write(i, 0);
    }

    if (!EEPROM.getCommitASAP())
    {
        Serial.println("CommitASAP not set. Need commit()");
        EEPROM.commit();
    }
}

void checkEEPromFormatted()
{
    unsigned int signature;
    EEPROM.get(0, signature);

    if (signature != SAMD_EEPROM_EMULATION_SIGNATURE)
    {
        Serial.println("EEPROM is empty, writing WRITTEN_SIGNATURE and some example data:");

        EEPROM.put(0, SAMD_EEPROM_EMULATION_SIGNATURE);

        eraseEEProm();
    }
}
