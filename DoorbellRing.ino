#include "RTClib.h"

#define uS_TO_S_FACTOR 1000000
#define BELL_POWER_PIN 13

RTC_DS1307 DS1307_RTC;
BluetoothSerial BT;

RTC_DATA_ATTR std::list int todaySchedule;
RTC_DATA_ATTR int alarmIndex = 0;
RTC_DATA_ATTR int todaySchedule[];

struct setAlarm {
  int secondsFromMidnight;
  string ringtone;
  setAlarm(secondsFromMidnight, ringtone) {
    this -> secondsFromMidnight = secondsFromMidnight;
    this -> ringtone = ringtone;
  }
};


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

  // Take the information given by the user's phone and convert it to an object that we're gonna call schedule
  const char schedule_stringify = BT.read()
  RTC_DATA_ATTR StaticJsonDocument<200> schedule;
  deserializeJson(schedule, schedule_stringify);
}

void wakeUpByTimer() {
  
}

void wakeUpProcesses() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason){
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : wakeUpByTimer(); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }

}

void loop () {
  // ask for the actual time to the RTC module
  DateTime now = DS1307_RTC.now();

  // Here, i check the "one use"d dates in the schedule
  for(int i = 0; i< schedule.specificDate.length; i++) {
    // Check if the date is today
    if (now.year() == schedule.specificDate[i].year && now.day() == schedule.specificDate[i].day && now.mouth() == schedule.specificDate[i].mouth) {
      // If it is, save how many time there is to wait till it's time to ring it (starting from 00:00) and what ringtone it has, this is save as a structure called set alarm
      todaySchedule.push_front(setAlarm(schedule.specificDate[i].secondsFromMidnight, schedule.specificDate[i].ringtone));

      //and then delete that element 
      auto specificDate = schedule.specificDate.begin() + i;
      schedule.specificDate.erase(specificDate);
    }
  };

  // I've not made this function up, but it's suppost to  put the first array onto the second one
  concatenate(schedule.weeklyDates[now.dayOfTheWeek()], todaySchedule);

  // Here, i sort today's "alarms" from sooner to less sonner
  std::sort(todaySchedule.begin(), todaySchedule.end(), [](const setAlarm& previusalarm, const setAlarm& nextalarm) { return previusalarm.secondsFromMidnight < nextalarm.secondsFromMidnight; });

  //seconds that have passed throught the day
  todaysSeconds = (now.hour() * 3600) + (now.minute() * 60) + now.seconds();

  //calculate the time that is to wait till the next alarm from now
  timeToSleep = todaySchedule[alarmIndex].secondsFromMidnight - todaysSeconds;

  // makes the esp32 wait for that long
  esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);

  wakeUpProcesses();
  
}

//recordatorio
//Gregorio XIII tuvo que mandar perfeccionar ese calendario juliano que celebraba religiosamente el año bisiesto cada cuatro. Si el año también es divisible por 100, no es un año bisiesto a menos que también sea divisible por 400. Por lo tanto, 2004, 2008, 2012, 2016 fueron todos años bisiestos. Si bien 2000 fue un año bisiesto, 1900 no lo fue y 2100 tampoco lo será.


