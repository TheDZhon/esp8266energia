#include "tones.h"

#include <DHT22_430.h>

#define SRV_ADDR "192.168.1.100"
#define SRV_PORT "9977"

#define DHT_PIN P2_1
#define ESP8266_CH_PD_PIN P2_2
#define BUZZER_PIN P1_4
#define RED_LED P1_0

#define MAX_IN_BUF_SZ 64U
#define MAX_OUT_BUF_SZ 64U
#define MAX_AUX_BUF_SZ 32U

#define MEASUREMENTS_DELAY_S 60U
#define ALARM_DELAY_MS 1000U
#define MAX_RETRY_CNT 100U

char in_buf[MAX_IN_BUF_SZ];
char out_buf[MAX_OUT_BUF_SZ];
char aux_buf[MAX_AUX_BUF_SZ];

DHT22 dht (DHT_PIN);

void setup()
{
  pinMode (ESP8266_CH_PD_PIN, OUTPUT);
  pinMode (BUZZER_PIN, OUTPUT);

  playInitTone ();

  Serial.begin (115200);

  dht.begin ();

  esp8266reboot ();
}

void loop()
{
  esp8266rx (NULL);  

  boolean flag = dht.get ();
  int h = dht.humidityX10 ();
  int t = dht.temperatureX10 ();

  if (!flag) {
    failure (); // failed to read from DHT sensor
  } 
  else {
    sprintf (aux_buf, "[H:%d.%d,T:%d.%d]", h / 10, h % 10, t / 10, t % 10);
    esp8266send (aux_buf); 
  }

  goLowPower ();
}

void goLowPower ()
{
  sprintf (aux_buf, "AT+GSLP=", MEASUREMENTS_DELAY_S * 1000 - 250);
  esp8266cmd (aux_buf);
  sleepSeconds (MEASUREMENTS_DELAY_S);   
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
  digitalWrite (ESP8266_CH_PD_PIN, LOW);
  delay (100);
}

void esp8266poweron ()
{
  digitalWrite (ESP8266_CH_PD_PIN, HIGH);
  delay (5000);

  while (Serial.available () > 0) { 
    Serial.read (); 
  }
}

void esp8266reboot () 
{
  esp8266shutdown ();
  esp8266poweron ();

  esp8266cmd ("AT");    
  esp8266cmd ("AT+CIPMODE=0");
  esp8266cmd ("AT+CIPMUX=0");
  esp8266cmd ("AT+CIPSTART=\"TCP\",\"" SRV_ADDR "\"," SRV_PORT);
}

void esp8266waitrx (const char * cmd) {
  unsigned retry_cnt = 0;

  while (!Serial.available ()) { 
    ++retry_cnt;
    delay (100); 

    if (retry_cnt > MAX_RETRY_CNT) { 
      failure (); // Failed to read from ESP8266
    }
  }
}

void esp8266rx (const char * cmd) {
  unsigned bytes_available = 0;
  unsigned offset = 0;

  while ((bytes_available = Serial.available ())) {
    offset += Serial.readBytes (in_buf + offset, bytes_available);        
  }

  if (offset == 0) {
    return;
  }

  in_buf[offset+1] = '\0';  

  if (strstr (in_buf, "ERROR") != NULL) {
    failure (); // ESP8266 returned error
  }
}

void esp8266cmd (const char * cmd) 
{
  Serial.println (cmd);
  Serial.flush ();

  esp8266waitrx (cmd);
  esp8266rx (cmd);
}

void esp8266send (const char * packet)
{ 
  unsigned l = strlen (packet) + 2U;
  sprintf (out_buf, "AT+CIPSEND=%d", l);

  esp8266cmd (out_buf);
  esp8266cmd (packet);
}

void failure ()
{  
  pinMode (RED_LED, OUTPUT);

  unsigned cnt = 0;
  unsigned tone_interval = 1U;

  while (true) {
    digitalWrite (RED_LED, (cnt % 2) ? HIGH : LOW);
    sleep (ALARM_DELAY_MS);

    if ((cnt % tone_interval) == 0) {
      playFailureTone ();
      tone_interval *= 10U;
    }

    ++cnt;
  }
}


