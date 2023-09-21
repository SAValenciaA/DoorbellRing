#include "RTClib.h"

RTC_DS1307 DS1307_RTC;
BluetoothSerial BT;


void setup () {
  Serial.begin(115200);

  // Initiate the bluetooth conection with the user's phone
  BT.begin("Doorbell-Ring");

  // Check for the RTC module to be aviable and stops everything if it isn't
  if (!DS1307_RTC.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  // Set time to the time i'm uploading the sketch to the ESP32, but once, 
  //i uploaded it to the RTC module, then i delete this line so the RTC module doesn't get trap in a span of time
  DS1307_RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // Wait till the ESP32 recibe a message from the user's phone
  while(!BT.available()) {1};
}

void loop () {
  DateTime now = DS1307_RTC.now();
  
  const char schedule_stringify = BT.read()
  StaticJsonDocument<200> schedule;
  deserializeJson(schedule, schedule_stringify);
  
}
