#define USE_SERIAL Serial
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
ESP8266WiFiMulti WiFiMulti;
#include <Adafruit_NeoPixel.h>
#define NUM_LEDS 30
#define BRIGHTNESS 255
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266_SSL.h>

BlynkTimer timer;
BlynkTimer timer2;
BlynkTimer timer3;

String current_time="00:00";
String temp="00";
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, D1, NEO_GRB + NEO_KHZ800);
int counter=0;
int r=0;int g=0; int b=255;
int ra=0;int ga=0; int ba=255;
char auth[] = "Your Auth Sync";

char ssid[] = "Your SSID";
char pass[] = "Your WIFI Key";
long n=0;

void setup(){
  
  timer.setInterval(1000L, everySecond);
  timer2.setInterval(30000L,everyMinute);
  timer3.setInterval(10800000,every3Hours);
  USE_SERIAL.begin(115200);
  Blynk.begin(auth, ssid, pass);
  WiFiMulti.addAP(ssid, pass);
  pixels.setBrightness(BRIGHTNESS);
  pixels.begin();
  forecast();
  geonames();
  weather();

}

void loop(){  
  n++;
  Blynk.run();
  timer.run();
  timer2.run();
  timer3.run();
}

void every3Hours(){
  forecast();
  }

void everySecond(){
  displayRefresh();
}

void everyMinute(){
  geonames();
  weather();
}

BLYNK_WRITE(V1) {
  r = param[0].asInt();
  g = param[1].asInt();
  b = param[2].asInt();
  pixels.show();
}

BLYNK_WRITE(V0) //Button Widget is writing to pin V0
{
  int pinData = param.asInt(); 
  pixels.setBrightness(pinData);
  pixels.show();
}

void geonames(){
    //Call geonames to get the time
    if((WiFiMulti.run() == WL_CONNECTED)) {
        HTTPClient http;
        USE_SERIAL.print("[HTTP] begin...\n");
        http.begin("http://api.geonames.org/timezoneJSON?formatted=true&lat=39.53&lng=2.58&username=YourUSerName&style=full");
        USE_SERIAL.print("[HTTP] GET...\n");
        int httpCode = http.GET();
        if(httpCode > 0) {
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
            if(httpCode == HTTP_CODE_OK) {
                int len = http.getSize();
                uint8_t buff[512] = { 0 };

                // get tcp stream
                WiFiClient * stream = http.getStreamPtr();
                while(http.connected() && (len > 0 || len == -1)) {
                    size_t size = stream->available();
                    if(size) {
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        USE_SERIAL.write(buff, c);
                        if(len > 0) {len -= c;}
                    }
                    delay(1);
                }
                 String sstr = (char*)buff;
                 int a=sstr.indexOf("time\"")+19;
                 current_time=sstr.substring(a,a+5);
                 if (current_time=="00:00"){  pixels.setBrightness(20);}
                 if (current_time=="08:00"){  pixels.setBrightness(255);}
                USE_SERIAL.println();
                USE_SERIAL.print("[HTTP] connection closed or file end.\n");
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
  }

void weather(){
    if((WiFiMulti.run() == WL_CONNECTED)) {
        HTTPClient http;
        USE_SERIAL.print("[HTTP] begin...\n");
        http.begin("http://api.openweathermap.org/data/2.5/weather?q=Palma,es&appid=YourKey&units=metric");
        
        USE_SERIAL.print("[HTTP] GET...\n");
        int httpCode = http.GET();
        if(httpCode > 0) {
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
            if(httpCode == HTTP_CODE_OK) {
                int len = http.getSize();
                uint8_t buff[512] = { 0 };
                // get tcp stream
                WiFiClient * stream = http.getStreamPtr();
                while(http.connected() && (len > 0 || len == -1)) {
                    size_t size = stream->available();
                    if(size) {
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        USE_SERIAL.write(buff, c);
                        if(len > 0) {len -= c;}
                    }
                    delay(1);
                }
                 String sstr = (char*)buff;
                 int a=sstr.indexOf("temp\":");
                 temp=sstr.substring(a+6,a+8);    
                USE_SERIAL.print("[HTTP] connection closed or file end.\n");
            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
  }

void forecast(){
    int level=0;
    if((WiFiMulti.run() == WL_CONNECTED)) {
        HTTPClient http;
        USE_SERIAL.print("[HTTP] begin...\n");
        http.begin("http://api.openweathermap.org/data/2.5/forecast?q=Palma&mode=xml&appid=YourKey&units=metric&type=accurate");
        
        USE_SERIAL.print("[HTTP] GET...\n");
        int httpCode = http.GET();
        if(httpCode > 0) {
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
            if(httpCode == HTTP_CODE_OK) {
                int len = http.getSize();
                uint8_t buff[512] = { 0 };

                // get tcp stream
                WiFiClient * stream = http.getStreamPtr();
                int muestras=0;
                 while(http.connected() && (len > 0 || len == -1)) {
                    size_t size = stream->available();
                    if(size) {
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        //USE_SERIAL.write(buff, c);
                        if(len > 0) {len -= c;}
                    }
                    delay(1);
                 String sstr = (char*)buff;
                 while ((sstr.indexOf("number")>=0)&&(muestras<8)){
                      muestras++;
                      int b=sstr.indexOf("number");
                      String number=sstr.substring(b+8, b+11);
                      int ni=number.toInt();
                      

                      //Orange Weather (Warning)
                      if ((level==0)&&(
                         (ni==500)||(ni==501)||
                         ((ni>=300)&&(ni<400))||
                         ((ni>=700)&&(ni<800))||
                         ((ni>=900)&&(ni<=906))||
                         (ni==957)
                         )){level=1;ra=255;ga=140;ba=0;}
                      
                      //Red weather (Danger)   
                      if ((level==1)&&(
                         ((ni>=200)&&(ni<=299))||
                         ((ni>=502)&&(ni<=550))||
                         ((ni>=600)&&(ni<=699))||
                         ((ni>=958)&&(ni<=956))
                         )){level=2;ra=255;ga=0;ba=0;}
                         
                          
                      sstr=sstr.substring(b+6);
                      }  
                }  
           USE_SERIAL.print("[HTTP] connection closed or file end.\n");
         }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
        USE_SERIAL.print("Level:");
        USE_SERIAL.println(level);
    }
  }

void displayRefresh(){
  counter=counter+1;
  if (counter%10<5){
    //Show Time
    displayNumber(3,current_time.substring(0,1).toInt(),r,g,b);
    displayNumber(2,current_time.substring(1,2).toInt(),r,g,b);
    displayNumber(1,current_time.substring(3,4).toInt(),r,g,b);
    displayNumber(0,current_time.substring(4,5).toInt(),r,g,b);
    if (counter%2==0){
      //Two Dots On
      pixels.setPixelColor(0,r,g,b);pixels.setPixelColor(1,r,g,b);} 
    else {
      //Two Dots Off
      pixels.setPixelColor(0,0,0,0);pixels.setPixelColor(1,0,0,0);} 
  }
  else {
    //Show Temp
    pixels.setPixelColor(0,0,0,0);pixels.setPixelColor(1,0,0,0);  
    displayNumber(0,8,0,0,0);
    displayNumber(1,10,ra,ga,ba);     
    displayNumber(3,temp.substring(0,1).toInt(),ra,ga,ba);
    displayNumber(2,temp.substring(1,2).toInt(),ra,ga,ba); 
  }
  pixels.show();
  }
  
void displayNumber(int pos, int n, int r, int g, int b){
  pos=3-pos;
  int num[11][8]={{6,0,1,2,3,4,5},{2,1,2},{5,0,1,6,4,3},{5,0,1,2,3,6},{4,5,1,6,2},{5,0,5,6,2,3},{6,0,5,6,2,3,4},{3,0,1,2},{7,0,1,2,3,4,5,6},{6,0,1,6,5,2,3},{4,0,1,5,6}};
  int seg[4][7]={
    {25,24,28,29,30,26,27},
    {18,17,21,22,23,19,20},
    {8,9,5,4,3,7,6},
    {11,10,14,15,16,12,13}
  };


  for (int i=0;i<7;i++){pixels.setPixelColor(seg[pos][i]-1,0,0,0);}
  for (int i=0; i<num[n][0];i++){
    pixels.setPixelColor(seg[pos][num[n][i+1]]-1,r,g,b);
   
  }  
   pixels.show();
  }