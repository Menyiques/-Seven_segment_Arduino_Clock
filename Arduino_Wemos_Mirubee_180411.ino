//Versión de 8/4/2018
#include <Arduino.h>
#include "arduino_secrets.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <SimpleTimer.h>

#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;
#include <Adafruit_NeoPixel.h>
#define NUM_LEDS 30
#define BRIGHTNESS 255
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, D1, NEO_GRB + NEO_KHZ800);

int r = 0;
int g = 0;
int b = 255;

int tc_r = 0;
int tc_g = 0;
int tc_b = 255;

String w = "";
String hora = "00:00";
String temp = "00";
int contador = 0;
long n = 0;
int watts = 0;
String mirubee_token = MIRUBEE_TOKEN;
int level = 0; 
  
SimpleTimer timer;
SimpleTimer timer2;
SimpleTimer timer3;
SimpleTimer timer4;

void setup()
{
  //I'm alive
  pixels.setBrightness(50);
  pixels.begin();
  pixels.setPixelColor(0, random(256), random(256),random(256));
  pixels.show();
  
  USE_SERIAL.begin(115200);

  for (uint8_t t = 4; t > 0; t--)
  {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(SECRET_SSID, SECRET_PASS);

  while (WiFiMulti.run() != WL_CONNECTED) {
    USE_SERIAL.print(".");
    delay(500);
  }
  if (WiFiMulti.run() == WL_CONNECTED) {
    USE_SERIAL.println("");
    USE_SERIAL.println("WiFi connected");
    USE_SERIAL.println("IP address: ");
    USE_SERIAL.println(WiFi.localIP());
  } else {
    USE_SERIAL.println("NotConnected");
  }

  timer.setInterval(1000L, cadaSegundo);
  timer2.setInterval(30000L, cada30Seg);
  timer3.setInterval(90000, cada2Min);//1:30 really
  timer4.setInterval(10800000, cada3Horas);

  forecast();
  geonames_time();
  weather_temp();
  mirubee_watt();
  forecast();

}

void loop()
{
  n++;
  timer.run();
  timer2.run();
  timer3.run();
  timer4.run();
}

void cada3Horas()
{
  forecast();
}

void cadaSegundo()
{
  pintaDisplay();
}

void cada30Seg()
{
  geonames_time();
}

void cada2Min()
{
  mirubee_watt();
  weather_temp();
}

void forecast()
{
  int muestras = 0;


  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    HTTPClient http;
    http.begin("http://api.openweathermap.org/data/2.5/"
               "forecast?id=2512989&mode=xml&appid=" + String(OPENWEATHER_TOKEN) + "&units=metric&"
               "type=accurate");
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        String sstr = http.getString();
        while ((sstr.indexOf("number") >= 0) && (muestras < 8))
        {
          muestras++;
          int b = sstr.indexOf("number");
          String number = sstr.substring(b + 8, b + 11);
          int ni = number.toInt();

          //Red. Level 2. Default.
          int thislevel = 2;

          // Orange. Level 1;
          if (
            (ni == 200) || (ni == 210) || (ni == 230) || (ni == 300) || (ni == 310) ||
            (ni == 500) || (ni == 501) || (ni == 511) || (ni == 520) || (ni == 521) ||
            (ni == 600) || (ni == 611) || (ni == 612) || (ni == 615) || (ni == 616) ||
            (ni == 620) || (ni == 621)
          ) {
            thislevel = 1;
          }

          //Blue. Level 0.
          if (
            (ni == 800) || (ni == 801) || (ni == 802) || (ni == 803) || (ni == 804) ||
            (ni == 900) || (ni == 903) || (ni == 904) || (ni == 905) || (ni == 951) ||
            (ni == 952) || (ni == 953) || (ni == 954) || (ni == 955) || (ni == 956)
          ) {
            thislevel = 0;
          }

          if (thislevel > level) {
            level = thislevel;
          }
          sstr = sstr.substring(b + 6);
        }
        switch (level){
          case 0:
            tc_r=0;
            tc_g=255;
            tc_b=0;
            break;
          case 1:
            tc_r=255;
            tc_g=165;
            tc_b=0;
            break;
          case 2:
            tc_r=255;
            tc_g=0;
            tc_b=0;
            break;
          }
      }
    }
    else
    {
      USE_SERIAL.printf(
        "[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}
void geonames_time()
{
  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    HTTPClient http;
    http.begin(
      "http://api.geonames.org/timezoneJSON?formatted=true&lat=39.53&lng=2.58&username=" + String(GEONAMES_USER) + "&style=full");
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        String sstr = http.getString();
        int a = sstr.indexOf("time\"") + 19;
        hora = sstr.substring(a, a + 5);
        if (hora.substring(0, 2).toInt() < 8)
        {
          pixels.setBrightness(10);//10
        }
        else
        {
          pixels.setBrightness(100);
        } // Dimm between 00:00 and 08:00
      }
    }
    else
    {
      USE_SERIAL.printf(
        "[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}
void weather_temp()
{
  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    HTTPClient http;
    http.begin("http://api.openweathermap.org/data/2.5/weather?id=2512989&appid="
               + String(OPENWEATHER_TOKEN) + "&units=metric");
    int httpCode = http.GET();
    if (httpCode > 0)
    {
      if (httpCode == HTTP_CODE_OK)
      {
        String sstr = http.getString();
        int a = sstr.indexOf("temp\":");
        temp = sstr.substring(a + 6, a + 8);
        if (temp.substring(1, 2) == ",")
        {
          temp = "0" + temp.substring(0, 1);
        }
      }
    }
    else
    {
      USE_SERIAL.printf(
        "[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}
void mirubee_watt()
{
  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    HTTPClient http;
    http.begin("http://app.mirubee.com/api/v2/buildings/12757/meters/40715/channels/1/last");
    http.addHeader("Authorization", mirubee_token);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      int p1 = payload.indexOf("P\":[") + 4;
      int p2 = payload.indexOf(".", p1);
      watts = payload.substring(p1, p2).toInt();
      Serial.println("Watts:" + String(watts));
    }
    else if (httpCode == 401) {
      http.end();
      Serial.println("401");
      http.begin("http://app.mirubee.com/api/v2/login");
      http.addHeader("Content-Type", "application/json");
      String em = EMAIL;
      String pa = PASSWORD;
      String client_id = "0000000000" + String(random(0, 9999999999));
      int l = client_id.length();
      client_id = client_id.substring(l - 10);

      String post = "{\"email\":\"" + em + "\",\"password\":\"" + pa + "\",\"client_id\":\"" + client_id + "\",\"client_type_id\":\"android\"}";
      Serial.println(post);
      auto httpCode = http.POST(post);
      Serial.println(httpCode);
      String payload = http.getString();

      mirubee_token = payload.substring(payload.indexOf(":\"") + 2 , payload.indexOf("\"}"));
      Serial.println("Payload:" + payload);
      Serial.println("token:" + mirubee_token);
    }
    http.end();
  }
}

void pintaDisplay()
{
  contador = contador + 1;
  if (contador % 10 < 6) //sec 0 to 6 display time
  { // Hora
    numero(3, hora.substring(0, 1).toInt(), r, g, b);
    numero(2, hora.substring(1, 2).toInt(), r, g, b);
    numero(1, hora.substring(3, 4).toInt(), r, g, b);
    numero(0, hora.substring(4, 5).toInt(), r, g, b);
    if (contador % 2 == 0)
    {
      pixels.setPixelColor(0, r, g, b);
      pixels.setPixelColor(1, r, g, b);
    }
    else
    {
      pixels.setPixelColor(0, 0, 0, 0);
      pixels.setPixelColor(1, 0, 0, 0);
    }
  }
  else if (contador % 10 < 8)//next 2 secs display temp
  { // Temp
    pixels.setPixelColor(0, 0, 0, 0);
    pixels.setPixelColor(1, 0, 0, 0);
    numero(0, 8, 0, 0, 0);
    numero(1, 10, tc_r,tc_g,tc_b);
    numero(3, temp.substring(0, 1).toInt(), tc_r, tc_g, tc_b);
    numero(2, temp.substring(1, 2).toInt(), tc_r, tc_g, tc_b);
  }
  else //next 2 secs display watts
  { // Watios
    w = "0000" + String(watts);
    byte l = w.length();
    byte rw = 0;
    byte gw = 0;
    byte bw = 0;
    w = w.substring(l - 4, l);
    byte c = w.substring(0, 2).toInt();
    if (c < 13)
    {
      rw = 0;gw = 255;bw = 0;
    }
    else if (c < 26)
    {
      rw = 51;gw = 204;bw = 0;
    }
    else if (c < 39)
    {
      rw = 102;gw = 153;bw = 0;
    }
    else if (c < 53)
    {
      rw = 153;gw = 102;bw = 0;
    }
     else if (c < 66)
    {
      rw = 204;gw = 51;bw = 0;
    }
    else
    {
      rw = 255;gw = 0;bw = 0;
    }

    numero(3, w.substring(0, 1).toInt(), rw, gw, bw);
    numero(2, w.substring(1, 2).toInt(), rw, gw, bw);
    numero(1, w.substring(2, 3).toInt(), rw, gw, bw);
    numero(0, w.substring(3, 4).toInt(), rw, gw, bw);
    pixels.setPixelColor(0, rw, gw, bw);
  }
  pixels.show();
}

void numero(int pos, int n, int r, int g, int b)
{
  pos = 3 - pos;
  int num[11][8] = { { 6, 0, 1, 2, 3, 4, 5 }, { 2, 1, 2 }, { 5, 0, 1, 6, 4, 3 },
    { 5, 0, 1, 2, 3, 6 }, { 4, 5, 1, 6, 2 }, { 5, 0, 5, 6, 2, 3 }, { 6, 0, 5, 6, 2, 3, 4 },
    { 3, 0, 1, 2 }, { 7, 0, 1, 2, 3, 4, 5, 6 }, { 6, 0, 1, 6, 5, 2, 3 }, { 4, 0, 1, 5, 6 }
  };
  int seg[4][7] = { { 25, 24, 28, 29, 30, 26, 27 }, { 18, 17, 21, 22, 23, 19, 20 },
    { 8, 9, 5, 4, 3, 7, 6 }, { 11, 10, 14, 15, 16, 12, 13 }
  };


  for (int i = 0; i < 7; i++)
  {
    pixels.setPixelColor(seg[pos][i] - 1, 0, 0, 0);
  }
  for (int i = 0; i < num[n][0]; i++)
  {
    pixels.setPixelColor(seg[pos][num[n][i + 1]] - 1, r, g, b);
  }
  pixels.show();
}

