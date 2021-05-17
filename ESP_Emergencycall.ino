/* ====================================================================
   Copyright (c) 2018 Juergen Liegner  All rights reserved.
   (https://www.mikrocontroller.net/topic/444994)
   
   Copyright (c) 2019 Florian Wernze
   (https://wernze-home.net/wordpress/hobbys/arduino-esp8266-tuerklingel/)
   
   Copyright (c) 2019 Thorsten Godau (dl9sec)
   (Did some optimizations, extensions and beautification of Florian's code
   and integrated the library)

   Copyright (c) 2021 Steivan Claglüna
   (Did some changes to work as a fire alarm)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. Neither the name of the author(s) nor the names of any contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.
   THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.
   ====================================================================*/

#include <ESP8266WiFi.h>
#include <ArduinoSIP.h> //can be found here: https://github.com/dl9sec/ArduinoSIP

#define LED_ESP12E    2
#define LED_NODEMCU   16

//------------------------------------------------
// Configuration with static IP
//------------------------------------------------

// WiFi parameters
const char* WiFiSSID    = "SSID";               // WiFi SSID
const char* WiFiPSK     = "WIFI_PASSWORD";      // WiFi WPA2 preshared key

const char *WiFiIP      = "192.168.178.69";     // WiFi IP of the ESP
const char *WiFiGW      = "192.168.178.1";      // WiFi GW
const char *WiFiNM      = "255.255.255.0";      // WiFi NM
const char *WiFiDNS     = "192.168.178.1";      // WiFi DNS

// Sip parameters
const char *SipIP      = "192.168.178.1";       // IP of the FRITZ!Box
const int   SipPORT    = 5060;                  // SIP port of the FRITZ!Box
const char *SipUSER    = "SIP_USERNAME";        // SIP-Call username at the FRITZ!Box
const char *SipPW      = "SIP_PASSWORD";        // SIP-Call password at the FRITZ!Box

// Dial parameters
const char *SipDIAL   = "SIP_DIAL";             // Dial number
const char *SipTEXT   = "SIP_TEXT";             // Dial text 

// Alarm Pin
const int alarm_pin = 2;                         //This pin will be used to determine the relay state
const int PinState = 0;                          //Depending on the mode: use 0 if alarm_pin is connected to GND during a fire alarm, otherwise 1

// Alarm Timeout
const long myTimeout = 10000;                    //Timeout in Milliseconds after an alarm

//------------------------------------------------

long AlarmTime = 0;
char acSipIn[2048];
char acSipOut[2048];

Sip aSip(acSipOut, sizeof(acSipOut));


bool AlarmIsNew(long AlarmTime) {
//This function checks uf the current alarm is old or new depending on the myTimeout variable
  if (millis() <  AlarmTime + myTimeout){
    return false;
  } else {
    return true;
  }
}

void setup()
{
  int i = 0;

  IPAddress myIP;
  IPAddress myGW;
  IPAddress myNM;
  IPAddress myDNS;

  ESP.wdtDisable();
  ESP.wdtEnable(WDTO_8S);

  Serial.begin(115200);
  Serial.setDebugOutput(false);
  delay(10);

  pinMode(LED_ESP12E, OUTPUT);
  pinMode(LED_NODEMCU, OUTPUT);

  pinMode(alarm_pin, INPUT_PULLUP);

  digitalWrite(LED_ESP12E, 1);  // LED off
  digitalWrite(LED_NODEMCU, 1); // LED off

  Serial.printf("\r\n\r\nConnecting to %s\r\n", WiFiSSID);

  WiFi.setAutoConnect (true);
  WiFi.setAutoReconnect (true);
  WiFi.softAPdisconnect (true);

  myIP.fromString(WiFiIP);
  myGW.fromString(WiFiGW);
  myNM.fromString(WiFiNM);
  myDNS.fromString(WiFiDNS);

  WiFi.config(myIP, myGW, myNM, myDNS);

  if ( String(WiFiSSID) != WiFi.SSID() )
  {
    Serial.print("Wifi initializing...\r\n");
    WiFi.begin(WiFiSSID, WiFiPSK);
  }
  
  while ( WiFi.status() != WL_CONNECTED )
  {
    delay(500);
    Serial.print(".");
  }

  WiFi.persistent(true);

  Serial.printf("\r\nWiFi connected to: %s\r\n", WiFi.localIP().toString().c_str());
  digitalWrite(LED_ESP12E, 0);

  aSip.Init(SipIP, SipPORT, WiFiIP, SipPORT, SipUSER, SipPW, 15);

}


void loop(void)
{
  // SIP processing
  aSip.Processing(acSipIn, sizeof(acSipIn));

  // Firealarm handling begin ===========================

  if ( digitalRead(alarm_pin) == PinState && AlarmIsNew(AlarmTime) )
  {
        aSip.Dial(SipDIAL, SipTEXT);
        Serial.println("Fire Alarm!!");
        AlarmTime=millis();
  }

  // Firealarm handling end =============================

  ESP.wdtFeed();  // Retrigger watchdog
  
}
