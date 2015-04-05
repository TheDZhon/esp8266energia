#include <DHT22_430.h>

char in_buf[1024];
char send_buf[256];
char dht_buf[256];

DHT22 dht (PD_0);

const unsigned max_retry_cnt = 100U;

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

void setup()
{
  pinMode (PB_5, OUTPUT);
  pinMode (PB_4, OUTPUT);

  play_init_tone ();

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

  delay (10000);
}

void sTone (unsigned note, unsigned len)
{
  unsigned duration = 1000U / len;

  tone (PB_4, note, duration);
  delay (duration * 1.3);
  noTone (PB_4);
}

void play_init_tone ()
{
  sTone (NOTE_E4, 8);
  sTone (NOTE_GS4, 4);
  sTone (NOTE_B4, 8);
  sTone (NOTE_E5, 2);
}

void play_failure_tone ()
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
  digitalWrite (PB_5, LOW);
  delay (5000);
}

void esp8266poweron ()
{
  Serial.println ("Power on ESP8266...");
  digitalWrite (PB_5, HIGH);

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

    if (retry_cnt > max_retry_cnt) { 
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
  pinMode (PF_1, OUTPUT);

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

    digitalWrite (PF_1, (cnt % 2) ? HIGH : LOW);
    delay (1000);

    if ((cnt % tone_interval) == 0) {
      play_failure_tone ();
      tone_interval *= 10U;
    }

    ++cnt;
  }
}

