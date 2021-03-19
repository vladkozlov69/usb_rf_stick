#include <Arduino.h>
#include <RCSwitch.h>
#include <PlainMemList.h>

RCSwitch mySwitch = RCSwitch();

const char* CMD_REGISTER = "REGISTER ";
const char* CMD_SEND = "SEND ";
const char* CMD_LIST = "LIST";
const char* CMD_LEARN = "LEARN";
const char* CMD_DEBUG = "DEBUG";
const char* CMD_DELETE = "DELETE ";

PlainMemList registered_devices;

bool debug_status = false;
bool learn_status = false;
unsigned long learn_start = 0;

void setup() 
{
    Serial.begin(9600);
    while (!Serial);

    mySwitch.enableReceive(EXTERNAL_INT_10); 
    mySwitch.enableTransmit(9);

    pinMode(8, INPUT_PULLUP);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.println("#RCSLink v0.0.1");
}

void loop() 
{
    if (mySwitch.available()) 
    {
        long value = mySwitch.getReceivedValue();

        if (debug_status)
        {
            Serial.print("#Received ");
            Serial.print(value);
            Serial.print(" / ");
            Serial.print( mySwitch.getReceivedBitlength() );
            Serial.print("bit ");
            Serial.print("Protocol: ");
            Serial.println( mySwitch.getReceivedProtocol() );
        }

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
                Serial.print("#:LEARNED ");
                Serial.println(value);
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
            }
        }

        if (command.startsWith(CMD_LEARN))
        {
            learn_status = true;
            learn_start = millis();
            digitalWrite(LED_BUILTIN, LOW);
            Serial.println("#Learn ON");
        }

        if (command.startsWith(CMD_DEBUG))
        {
            debug_status = true;
            Serial.println("#Debug ON");
        }
    }
    
    if (learn_status && (millis() - learn_start) > 30000)
    {
        learn_status = false;
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("#Learn OFF");
    }
}
