#include <Arduino.h>
#include <RCSwitch.h>
#include <FlashAsEEPROM_SAMD.h>
#include <PlainMemList.h>

RCSwitch mySwitch = RCSwitch();

const char* CMD_REGISTER = "REGISTER ";
const char* CMD_SEND = "SEND ";
const char* CMD_LIST = "LIST";
const char* CMD_LEARN = "LEARN";
const char* CMD_DELETE = "DELETE ";

const int NVS_SIZE = PLAINMEMLIST_SIZE;
const int WRITTEN_SIGNATURE = 0xBEEFDEED;

PlainMemList registered_devices;

bool learn_status = false;
unsigned long learn_start = 0;

void eraseEEProm(boolean commit);
void checkEEPromFormatted();
void saveDevices(boolean commit);
void commitEEProm(boolean commit);

void setup() 
{
    Serial.begin(9600);
    while (!Serial);

    mySwitch.enableReceive(EXTERNAL_INT_10); 
    mySwitch.enableTransmit(9);

    pinMode(8, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    checkEEPromFormatted();

    for (int i = 0; i < NVS_SIZE; i++)
    {
        long data = 0;
        int eeprom_addr = sizeof(SAMD_EEPROM_EMULATION_SIGNATURE) + i * sizeof(long);
        EEPROM.get(eeprom_addr, data);
        if (data != 0 && data != -1L)
        {
            registered_devices.append(data);
        }
    }

    Serial.println("#RCSLink v0.0.1");
}

void loop() 
{
    if (digitalRead(8) == LOW)
    {
        eraseEEProm(true);
    }

    if (mySwitch.available()) 
    {
        long value = mySwitch.getReceivedValue();
        Serial.print("#Received ");
        Serial.print(value);
        Serial.print(" / ");
        Serial.print( mySwitch.getReceivedBitlength() );
        Serial.print("bit ");
        Serial.print("Protocol: ");
        Serial.println( mySwitch.getReceivedProtocol() );

        mySwitch.resetAvailable();

        if (registered_devices.indexOf(value) >=0)
        {
            Serial.println(value);
        }
        else
        {
            if (learn_status)
            {
                registered_devices.append(value);
                saveDevices(true);
            }
        }
    }

    if (Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        
        if (command.startsWith(CMD_SEND))
        {
            command.replace(CMD_SEND, "");
            long code = command.toInt();
            Serial.print("#Sending:"); Serial.println(code);
            mySwitch.send(code, 24);
        }

        if (command.startsWith(CMD_LIST))
        {
            for (int i = 0; i < registered_devices.length; i++)
            {
                Serial.print("#:");
                Serial.println(registered_devices.data[i]);
            }
        }

        if (command.startsWith(CMD_REGISTER))
        {
            command.replace(CMD_REGISTER, "");
            long code = command.toInt();
            Serial.print("#Registering:"); Serial.println(code);
            if (registered_devices.indexOf(code) < 0)
            {
                registered_devices.append(code);
                saveDevices(true);
            }
        }

        if (command.startsWith(CMD_DELETE))
        {
            command.replace(CMD_DELETE, "");
            long code = command.toInt();
            Serial.print("#Deleting:"); Serial.println(code);
            int index = registered_devices.indexOf(code);
            if (index >= 0)
            {
                registered_devices.remove(index);
                eraseEEProm(false);
                saveDevices(true);
            }
        }

        if (command.startsWith(CMD_LEARN))
        {
            learn_status = true;
            learn_start = millis();
            digitalWrite(LED_BUILTIN, LOW);
            Serial.println("#Learn ON");
        }
    }
    
    if (learn_status && (millis() - learn_start) > 30000)
    {
        learn_status = false;
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("#Learn OFF");
    }
}

void eraseEEProm(boolean commit)
{
    for (int i = sizeof(SAMD_EEPROM_EMULATION_SIGNATURE); i < EEPROM_EMULATION_SIZE; i++)
    {
        EEPROM.write(i, 0);
    }

    commitEEProm(commit);
}

void checkEEPromFormatted()
{
    unsigned int signature;
    EEPROM.get(0, signature);

    if (signature != SAMD_EEPROM_EMULATION_SIGNATURE)
    {
        Serial.println("EEPROM is empty, writing WRITTEN_SIGNATURE and some example data:");

        EEPROM.put(0, SAMD_EEPROM_EMULATION_SIGNATURE);

        eraseEEProm(true);
    }
}

void saveDevices(boolean commit)
{
    for (int i = 0; i < registered_devices.length; i++)
    {
        int eeprom_addr = sizeof(SAMD_EEPROM_EMULATION_SIGNATURE) + i * sizeof(long);
        EEPROM.put(eeprom_addr, registered_devices.data[i]);
    }

    commitEEProm(commit);
}

void commitEEProm(boolean commit)
{
    if (commit && !EEPROM.getCommitASAP())
    {
        Serial.println("CommitASAP not set. Need commit()");
        EEPROM.commit();
    }
}
