#include "tones.h"

#include <DHT22_430.h>

#define DHT_PIN PD_0
#define ESP8266_CH_PD_PIN PB_5
#define BUZZER_PIN PB_4
#define RED_LED PF_1

const unsigned kMaxRcvBufSz = 1024U;
const unsigned kMaxSndBufSz = 256U;
const unsigned kMaxDHTBufSz = 256U;
const unsigned kMeasurementsDelayMs = 10000U;
const unsigned kAlarmDelayMs = 1000U;
const unsigned kMaxRetryCnt = 100U;

char in_buf[kMaxRcvBufSz];
char send_buf[kMaxSndBufSz];
char dht_buf[kMaxDHTBufSz];

DHT22 dht (DHT_PIN);

void setup()
{
  pinMode (ESP8266_CH_PD_PIN, OUTPUT);
  pinMode (BUZZER_PIN, OUTPUT);

  playInitTone ();

  Serial1.begin (115200);
  Serial.begin (115200);

  dht.begin ();

  esp8266reboot ();

  Serial.println ("Loop started...");

  esp8266cmd ("AT");    
  esp8266cmd ("AT+CIPMODE=0");
  esp8266cmd ("AT+CIPMUX=0");
  esp8266cmd ("AT+CIPSTART=\"TCP\",\"192.168.1.100\",9977");
}

void loop()
{
  esp8266rx (NULL);  

  boolean flag = dht.get ();
  int32_t h = dht.humidityX10 ();
  int32_t t = dht.temperatureX10 ();

  if (!flag) {
    failure ("Failed to read from DHT!");
  } 
  else {
    sprintf (dht_buf, "[H:%d.%d,T:%d.%d]", h / 10, h % 10, t / 10, t % 10);
    Serial.println (dht_buf);
    esp8266send (dht_buf); 
  }

  delay (kMeasurementsDelayMs);
}

void sTone (unsigned note, unsigned len)
{
  unsigned duration = 1000U / len;

  tone (BUZZER_PIN, note, duration);
  delay (duration * 1.3);
  noTone (BUZZER_PIN);
}

void playInitTone ()
{
  sTone (NOTE_E4, 8);
  sTone (NOTE_GS4, 4);
  sTone (NOTE_B4, 8);
  sTone (NOTE_E5, 2);
}

void playFailureTone ()
{
  sTone (NOTE_B3, 8);
  sTone (NOTE_A4, 4);
  sTone (NOTE_A4, 8);
  sTone (NOTE_A4, 8);
  sTone (NOTE_GS4, 4);
  sTone (NOTE_FS4, 8);
  sTone (NOTE_E4, 8);
}

void esp8266shutdown ()
{
  Serial.println ("Shutdown ESP8266...");
  digitalWrite (ESP8266_CH_PD_PIN, LOW);
  delay (5000);
}

void esp8266poweron ()
{
  Serial.println ("Power on ESP8266...");
  digitalWrite (ESP8266_CH_PD_PIN, HIGH);

  Serial.println ("Clean ESP8266 boot buffer...");
  delay (5000);

  while (Serial1.available () > 0) { 
    Serial1.read (); 
  }
}

void esp8266reboot () 
{
  Serial.println ();
  Serial.println ("***");
  Serial.println ();  

  esp8266shutdown ();
  esp8266poweron ();
}

void esp8266waitrx (const char * cmd) {
  unsigned retry_cnt = 0;

  while (!Serial1.available ()) { 
    ++retry_cnt;
    delay (100); 

    if (retry_cnt > kMaxRetryCnt) { 
      failure (cmd);
    }
  }
}

void esp8266rx (const char * cmd) {
  unsigned bytes_available = 0;
  unsigned offset = 0;

  while (bytes_available = Serial1.available ()) {
    offset += Serial1.readBytes (in_buf + offset, bytes_available);        
  }

  if (offset == 0) {
    return;
  }

  in_buf[offset+1] = '\0';  

  if (strstr (in_buf, "ERROR") != NULL) {
    failure (cmd);
  }

  Serial.println (in_buf);
  Serial.flush ();
}

void esp8266cmd (const char * cmd) 
{
  Serial1.println (cmd);
  Serial1.flush ();

  esp8266waitrx (cmd);
  esp8266rx (cmd);
}

void esp8266send (const char * packet)
{ 
  unsigned l = strlen (packet) + 2U;
  sprintf (send_buf, "AT+CIPSEND=%d", l);

  esp8266cmd (send_buf);
  esp8266cmd (packet);
}

void failure (const char * cmd)
{  
  pinMode (RED_LED, OUTPUT);

  unsigned cnt = 0;
  unsigned tone_interval = 1U;

  while (true) {
    Serial.print ("System halted: ");
    if (cmd) {
      Serial.println (cmd);
    } 
    else {
      Serial.println ("[NO_CMD]");
    }

    digitalWrite (RED_LED, (cnt % 2) ? HIGH : LOW);
    delay (kAlarmDelayMs);

    if ((cnt % tone_interval) == 0) {
      playFailureTone ();
      tone_interval *= 10U;
    }

    ++cnt;
  }
}

