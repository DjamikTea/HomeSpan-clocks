#include "HomeSpan.h" 
#include "Clocks.h"
#include "time.h"

const char* ntpServer = "ntp.msk-ix.ru"; // ntp server
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 10800;

unsigned long timer;

void setup() {
  Serial.begin(115200);
  pixels.begin();
  pixels.clear();

  homeSpan.enableOTA(); 
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  homeSpan.begin(Category::Lighting,"Clocks");
  homeSpan.enableAutoStartAP();

  homeSpan.setSketchVersion("1.0.0");
  // homeSpan.setPairingCode("42156847");
  // homeSpan.setWifiCredentials("SSID", "PSK"); // wifi setup
  
  new SpanAccessory();                                                          
    new Service::AccessoryInformation();    
      new Characteristic::Identify();                    
      new Characteristic::Name("Clocks");
      new Characteristic::FirmwareRevision("1.0.0");
      new Characteristic::Manufacturer("DjamikTea");  

    new RGB_Clocks();         
  timer = millis();

}

int delayTimer = 20000; // after about 20 seconds all homespan services start and connect to wifi

void loop(){
  if (millis() - timer > delayTimer)         // a timer that calls the function to change the time
    {
      time_change();
      timer = millis();
    }
  homeSpan.poll();
}


void time_change(){
  struct tm timeinfo;
  if(getLocalTime(&timeinfo)){ // if the time is received
    char hour [3] = "";
    strftime (hour, 3, "%H", &timeinfo);
    int hours = hour[0] - '0'; //     1 segment
    int hours2 = hour[1] - '0'; //    2 segment hours

    char mins [3] = "";
    strftime (mins, 3, "%M", &timeinfo);
    int min = mins[0] - '0'; //   3 segment
    int min2 = mins[1] - '0'; //  4 segment minutes

    char Secss [3] = "";
    strftime (Secss, 3, "%S", &timeinfo);
    String Secsz = String(Secss[0] - '0') + String(Secss[1]- '0');

    LOG1(String(Secsz) + '\n');


    write_num(0, hours); // write numbers on clocks
    write_num(1, hours2);
    write_num(2, min);
    write_num(3, min2);

    int delayToMin = Secsz.toInt() * 1000; // calculation until the next minute
    LOG1(String(60000 - delayToMin) + '\n');
    LOG1("time: " + String(hours) + String(hours2) + String(min) + String(min2) + '\n');
    delayTimer = (60000 - delayToMin); // set timer
  } else {
    delayTimer = 5000;
  }
}