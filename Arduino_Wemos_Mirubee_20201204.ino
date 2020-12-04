//4/12/2020
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
ESP8266WiFiMulti WiFiMulti;
#include <Adafruit_NeoPixel.h>
#define NUM_LEDS 114
#define HBRIGHTNESS 200
#define LBRIGHTNESS 30
#define LOOPTIME 7

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266_SSL.h>

BlynkTimer timer100ms;
BlynkTimer timer1s;
BlynkTimer timer30s;
BlynkTimer timer3h;
BlynkTimer timer87s; //1000 times per day (Mirubee max)

String mirubee_token="";
String watts="";
String hora="";
String temp="";
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, D5, NEO_GRB + NEO_KHZ800);
long contadorms=0;
int r=0;int g=100; int b=100;
int ra=0;int ga=255; int ba=0;
char ssid[] = "";
char pass[] = "";
long n=0;

uint8_t h=0;
uint8_t m=0;
uint8_t s=0;


void setup(){
  timer100ms.setInterval(100L, cada100ms);
  timer1s.setInterval(1000L, cada1s);
  timer30s.setInterval(300000,cada5m);
  timer3h.setInterval(10800000,cada3h);
  timer87s.setInterval(87000,cada87s);
  
  Serial.begin(115200);
  WiFiMulti.addAP(ssid, pass);
  pixels.setBrightness(HBRIGHTNESS);
  pixels.begin();
  while(WiFiMulti.run() != WL_CONNECTED) {delay(1000);Serial.println(".");}
  geonames();
  weather();
  //mirubee_watt();
}

void loop()
{  
  n++;
  timer1s.run();
  timer30s.run();
  timer87s.run();
  timer3h.run();
  timer100ms.run();
}

void cada3h(){}

void cada1s(){s++;if (s>=60){s=0;m++;if (m>=60){m=0;h++;if (h>=24){h=0;}}}}

void cada5m(){if (m%2==0){ geonames(); } else { weather(); }}

void cada87s(){//mirubee_watt();
  }

void cada100ms() {contadorms++;pintaDisplay();}

void geonames(){
    if((WiFiMulti.run() == WL_CONNECTED)) {
        HTTPClient http;
        Serial.print("[HTTP] begin...\n");
        http.begin("http://api.geonames.org/timezoneJSON?formatted=true&lat=39.53&lng=2.58&username=Menyiques&style=full");
        Serial.print("[HTTP] GET...\n");
        int httpCode = http.GET();
        if(httpCode > 0) {
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
            if(httpCode == HTTP_CODE_OK) {
                int len = http.getSize();
                 uint8_t buff[512] = { 0 };
                WiFiClient * stream = http.getStreamPtr();
                while(http.connected() && (len > 0 || len == -1)) {
                        int c = stream->readBytes(buff, sizeof(buff));
                        if(len > 0) {len -= c;}
                }
                 String sstr = (char*)buff;
                 int a=sstr.indexOf("time\"")+19;
                 hora=sstr.substring(a,a+5);
                 h=hora.substring(0,2).toInt();
                 m=hora.substring(3,5).toInt();

                Serial.println();
                Serial.print("[HTTP] connection closed or file end.\n");
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
    }
  }

void weather(){
    if((WiFiMulti.run() == WL_CONNECTED)) {
        HTTPClient http;
        http.begin("http://api.openweathermap.org/data/2.5/weather?q=Palma,es&appid=e808a9fc312bbe9506825f1d1bb66692&units=metric");
        Serial.print("[HTTP] GET...\n");
        int httpCode = http.GET();
        if(httpCode > 0) {
            if(httpCode == HTTP_CODE_OK) {
                int len = http.getSize();
                uint8_t buff[512] = { 0 };
                WiFiClient * stream = http.getStreamPtr();
                while(http.connected() && (len > 0 || len == -1)) {
                  
                        //int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        int c = stream->readBytes(buff, sizeof(buff));
                        if(len > 0) {len -= c;}
                  
                }
                 String sstr = (char*)buff;
                 Serial.println();
                 Serial.println("SSTR:"+sstr);
                 int a=sstr.indexOf("feels_like")+12;
                 Serial.println(sstr);
                 temp=sstr.substring(a,a+2);
                 temp.replace(".","");
                  if (temp.length()==1){temp="0"+temp;}
                  
                
                 
            }
        }
        http.end();
    }
  }

void forecast(){
    int level=0;
    if((WiFiMulti.run() == WL_CONNECTED)) {
        HTTPClient http;
        Serial.print("[HTTP] begin...\n");
        //http.begin("http://api.geonames.org/weatherIcaoJSON?ICAO=LEPA&username=Menyiques&style=full");
        http.begin("http://api.openweathermap.org/data/2.5/forecast?q=Palma&mode=xml&appid=e808a9fc312bbe9506825f1d1bb66692&units=metric&type=accurate");
        
        Serial.print("[HTTP] GET...\n");
        int httpCode = http.GET();
        if(httpCode > 0) {
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
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
                        //Serial.write(buff, c);
                        if(len > 0) {len -= c;}
                    }
                    delay(1);
                    
                 String sstr = (char*)buff;
                 while ((sstr.indexOf("number")>=0)&&(muestras<8)){
                      muestras++;
                      int b=sstr.indexOf("number");
                      String number=sstr.substring(b+8, b+11);
                      int ni=number.toInt();
                      
                      if ((level==0)&&(
                         (ni==500)||(ni==501)||
                         ((ni>=300)&&(ni<400))||
                         ((ni>=700)&&(ni<800))||
                         ((ni>=900)&&(ni<=906))||
                         (ni==957)
                         )){level=1;ra=255;ga=140;ba=0;}
                         
                      if ((level==1)&&(
                         ((ni>=200)&&(ni<=299))||
                         ((ni>=502)&&(ni<=550))||
                         ((ni>=600)&&(ni<=699))||
                         ((ni>=958)&&(ni<=956))
                         )){level=2;ra=255;ga=0;ba=0;}
                         
                          
                      sstr=sstr.substring(b+6);
                      }  
                }  
           Serial.print("[HTTP] connection closed or file end.\n");
         }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
        http.end();
        Serial.print("Level:");
        Serial.println(level);
    }
  }

void mirubee_watt()
{
  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    HTTPClient http;
    http.begin("https://app.mirubee.com/api/v2/buildings/12757/meters/40715/channels/1/last");
    
    http.addHeader("Authorization", mirubee_token);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK||httpCode== 302) {
      String payload = http.getString();
      int p1 = payload.indexOf("P\":[") + 4;
      int p2 = payload.indexOf(".", p1);
      watts = payload.substring(p1, p2).toInt();
      watts=f4(watts.toInt());
      Serial.println("Watts:" + String(watts));
      Serial.println(payload);
    }
    else if (httpCode == 401) {
      http.end();
      Serial.println("401");
      http.begin("http://app.mirubee.com/api/v2/login");
      http.addHeader("Content-Type", "application/json");
      String em = "smoralesg@gmail.com";
      String pa = "M0r05555";
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

  
void pintaDisplay(){

  if (h>=0 && h<9){  pixels.setBrightness(LBRIGHTNESS);}else{pixels.setBrightness(HBRIGHTNESS);}

  
  hora=f2(h)+":"+f2(m);
  
  if ( s%LOOPTIME<5 ){ //5 first seconds
    numero(3,hora.substring(0,1).toInt(),r,g,b);
    numero(2,hora.substring(1,2).toInt(),r,g,b);
    numero(1,hora.substring(3,4).toInt(),r,g,b);
    numero(0,hora.substring(4,5).toInt(),r,g,b);
    //blynking dots
    if ( s%2==0 ) {pixels.setPixelColor(0,r,g,b);pixels.setPixelColor(1,r,g,b);} else {pixels.setPixelColor(0,0,0,0);pixels.setPixelColor(1,0,0,0);} 
  } 

  if( s%LOOPTIME>=5 && s%LOOPTIME<=7 ){ // 3 next seconds Temp Celsius
    pixels.setPixelColor(0,0,0,0);pixels.setPixelColor(1,0,0,0);  
    numero(0,8,0,0,0);
    numero(1,10,ra,ga,ba);//degrees celsius symbol     
    numero(2,temp.substring(1,2).toInt(),ra,ga,ba); 
    uint8_t t=temp.substring(0,1).toInt();
    if (t==0){numero(3,8,0,0,0);}else{numero(3,t,ra,ga,ba);}
    }
/*
  if( s%LOOPTIME>=8 ){ // 2 last seconds Watts
    pixels.setPixelColor(0,0,0,0);pixels.setPixelColor(1,0,0,0);  

    int v=(watts.toInt()/4000.0)*255;
    if (v>255){v=255;}
    numero(3,watts.substring(0,1).toInt(),v,255-v,0);
    numero(2,watts.substring(1,2).toInt(),v,255-v,0);
    numero(1,watts.substring(2,3).toInt(),v,255-v,0);
    numero(0,watts.substring(3,4).toInt(),v,255-v,0);
    }
    
*/
pixels.show();

  }


String f2(int n){
  String f="00"+(String)n;
  return f.substring(f.length()-2,f.length());
  }

String f4(int n){
  String f="0000"+(String)n;
  return f.substring(f.length()-4,f.length());
  }
  
void numero(uint8_t pos,uint8_t n, uint8_t r,uint8_t g,uint8_t b){
segment(pos,0,0,0,0);
segment(pos,1,0,0,0);
segment(pos,2,0,0,0);
segment(pos,3,0,0,0);
segment(pos,4,0,0,0);
segment(pos,5,0,0,0);
segment(pos,6,0,0,0);

switch (n) {
  case 0: segment(pos,0,r,g,b);segment(pos,1,r,g,b);segment(pos,2,r,g,b);segment(pos,3,r,g,b);segment(pos,4,r,g,b);segment(pos,5,r,g,b);break;
  case 1: segment(pos,0,r,g,b);segment(pos,5,r,g,b);break;
  case 2: segment(pos,1,r,g,b);segment(pos,0,r,g,b);segment(pos,6,r,g,b);segment(pos,3,r,g,b);segment(pos,4,r,g,b);break;
  case 3: segment(pos,1,r,g,b);segment(pos,0,r,g,b);segment(pos,5,r,g,b);segment(pos,4,r,g,b);segment(pos,6,r,g,b);break;
  case 4: segment(pos,2,r,g,b);segment(pos,0,r,g,b);segment(pos,6,r,g,b);segment(pos,5,r,g,b);break;
  case 5: segment(pos,1,r,g,b);segment(pos,2,r,g,b);segment(pos,4,r,g,b);segment(pos,5,r,g,b);segment(pos,6,r,g,b);break;
  case 6: segment(pos,1,r,g,b);segment(pos,2,r,g,b);segment(pos,4,r,g,b);segment(pos,5,r,g,b);segment(pos,6,r,g,b);segment(pos,3,r,g,b);break;
  case 7: segment(pos,1,r,g,b);segment(pos,0,r,g,b);segment(pos,5,r,g,b);break;
  case 8: segment(pos,0,r,g,b);segment(pos,1,r,g,b);segment(pos,2,r,g,b);segment(pos,3,r,g,b);segment(pos,4,r,g,b);segment(pos,5,r,g,b);segment(pos,6,r,g,b);break;
  case 9: segment(pos,1,r,g,b);segment(pos,2,r,g,b);segment(pos,0,r,g,b);segment(pos,4,r,g,b);segment(pos,5,r,g,b);segment(pos,6,r,g,b);break;
  case 10: segment(pos,1,r,g,b);segment(pos,2,r,g,b);segment(pos,0,r,g,b);segment(pos,6,r,g,b);break;
  }
  pixels.show();
}
void segment(uint8_t pos, uint8_t n, uint8_t r,uint8_t g,uint8_t b){    
 
 pixels.setPixelColor((n*4)+(28*pos)+2, pixels.Color(r,g,b));
 pixels.setPixelColor((n*4)+(28*pos)+3, pixels.Color(r,g,b));
 pixels.setPixelColor((n*4)+(28*pos)+4, pixels.Color(r,g,b));
 pixels.setPixelColor((n*4)+(28*pos)+5, pixels.Color(r,g,b));

}