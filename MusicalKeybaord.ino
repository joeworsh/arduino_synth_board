//
// Arduino sketch for bluetooth
// powered musical keyboard.
//
// Joe Worsham 2017
//

#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
  #include <SoftwareSerial.h>
#endif

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"

#include "BluefruitConfig.h"

/*
 * Hardware Configuration
 */
#define  LATCH_PIN    12 // shift register pins
#define  CLOCK_PIN    8
#define  DATA_PIN     11
#define  BUZZER_PIN   10

#define  C_LED        1  // LEDs are represented as bit shifted int values for shift register
#define  D_LED        2
#define  E_LED        4
#define  F_LED        8
#define  G_LED        16
#define  A_LED        32
#define  B_LED        64
#define  C2_LED       128

/*
 * Frequencies in Hz of notes played on the arduino board. The frequencies are
 * in the lowest octave. The octaves can range from 0 to 8. The correct value of
 * frequency in octave is: freq * 2^octave
 */
#define  C_FREQ  16.35
#define  D_FREQ  18.35
#define  E_FREQ  20.60
#define  F_FREQ  21.83
#define  G_FREQ  24.50
#define  A_FREQ  27.50
#define  B_FREQ  30.87

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);

// the packet buffer
extern uint8_t packetbuffer[];

// track the current octave of the notes
double octave = 4.0; // on start up use the middle octave

// Create the bluefruit object, either software serial...uncomment these lines
SoftwareSerial bluefruitSS = SoftwareSerial(BLUEFRUIT_SWUART_TXD_PIN, BLUEFRUIT_SWUART_RXD_PIN);
Adafruit_BluefruitLE_UART ble(bluefruitSS, BLUEFRUIT_UART_MODE_PIN,
                      BLUEFRUIT_UART_CTS_PIN, BLUEFRUIT_UART_RTS_PIN);

///
/// Called once on startup.
///
void setup()
{
  Serial.begin(115200);
  
  // define the pins used for this application
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // initialize the bluetooth connection
  if (!ble.begin(VERBOSE_MODE)) {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  
  // Disable command echo from Bluefruit
  ble.echo(false);
  
  // Print Bluefruit information
  Serial.println("Requesting Bluefruit info:");
  ble.info();
  ble.verbose(false);
  
   /* Change the device name to make it easier to find */
  Serial.println(F("Setting device name to 'ArduinoSynth': "));
  if (! ble.sendCommandCheckOK(F( "AT+GAPDEVNAME=ArduinoSynth" )) ) {
    error(F("Could not set device name?"));
  }
  
  // Wait for connection
  while (! ble.isConnected()) {
      delay(500);
  }
  
  // set data mode on the BLE chip
  ble.setMode(BLUEFRUIT_MODE_DATA);
}

///
/// Called continuously through lifetime.
///
void loop()
{
  // Wait for new data to arrive
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;

  // If a note packet was received, process it now
  if (packetbuffer[1] == 'N') {
    uint8_t note = packetbuffer[2];
    playNote(note);
  }
  
  // if an octave packet is sent, set the octave for the system
  if (packetbuffer[1] == 'O') {
    octave = (double)packetbuffer[2]; 
  }
}

///
/// Play the provided note index where 1 = C4 up to 8 = C5.
void playNote(uint8_t noteIndex)
{
  int led = 0, freq = 0;
  switch(noteIndex)
  {
    case 1:
      led = C_LED;
      freq = (int) (C_FREQ * pow(2, octave));
      break;
    case 2:
      led = D_LED;
      freq = (int) (D_FREQ * pow(2.0, octave));
      break;
    case 3:
      led = E_LED;
      freq = (int) (E_FREQ * pow(2.0, octave));
      break;
    case 4:
      led = F_LED;
      freq = (int) (F_FREQ * pow(2.0, octave));
      break;
    case 5:
      led = G_LED;
      freq = (int) (G_FREQ * pow(2.0, octave));
      break;
    case 6:
      led = A_LED;
      freq = (int) (A_FREQ * pow(2.0, octave));
      break;
    case 7:
      led = B_LED;
      freq = (int) (B_FREQ * pow(2.0, octave));
      break;
    case 8:
      led = C2_LED;
      // the next higher octave
      freq = (int) (C_FREQ * pow(2.0, octave + 1));
      break;
  }
  
  playNote(led, freq);
  delay(500);
  stopPlaying();
}

///
/// Play the provided frequency on the buzzer and turn on the
/// corresponding LED.
///
void playNote(int led, int freq)
{
  // play the tone of the note and turn on the LED
  tone(BUZZER_PIN, freq);
  
  // shift the value of the LED to turn on
  digitalWrite(LATCH_PIN, LOW); //ground latchPin and hold low for as long as you are transmitting
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, led);
  digitalWrite(LATCH_PIN,HIGH); //pull the latchPin to save the data
}

///
/// Stop playing any tone and turn off all LEDs
///
void stopPlaying()
{
  // stop playing the tone
  noTone(BUZZER_PIN);
  
  // shift the value to turn all LEDs off
  digitalWrite(LATCH_PIN, LOW); //ground latchPin and hold low for as long as you are transmitting
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);
  digitalWrite(LATCH_PIN,HIGH); //pull the latchPin to save the data
}

///
/// Print an error to the serial console.
///
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}
