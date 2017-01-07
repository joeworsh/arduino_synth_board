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

#define  C4_LED        1  // LEDs are represented as bit shifted int values for shift register
#define  D4_LED        2
#define  E4_LED        4
#define  F4_LED        8
#define  G4_LED        16
#define  A4_LED        32
#define  B4_LED        64
#define  C5_LED        128

/*
 * Frequencies in Hz of notes played on the arduino board.
 */
#define  C4_FREQ  262  // middle C
#define  D4_FREQ  294
#define  E4_FREQ  330
#define  F4_FREQ  349
#define  G4_FREQ  392
#define  A4_FREQ  440
#define  B4_FREQ  494
#define  C5_FREQ  523

// function prototypes over in packetparser.cpp
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);

// the packet buffer
extern uint8_t packetbuffer[];

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
}

///
/// Play the provided note index where 1 = C4 up to 8 = C5.
void playNote(uint8_t noteIndex)
{
  int led = 0, freq = 0;
  switch(noteIndex)
  {
    case 1:
      led = C4_LED;
      freq = C4_FREQ;
      break;
    case 2:
      led = D4_LED;
      freq = D4_FREQ;
      break;
    case 3:
      led = E4_LED;
      freq = E4_FREQ;
      break;
    case 4:
      led = F4_LED;
      freq = F4_FREQ;
      break;
    case 5:
      led = G4_LED;
      freq = G4_FREQ;
      break;
    case 6:
      led = A4_LED;
      freq = A4_FREQ;
      break;
    case 7:
      led = B4_LED;
      freq = B4_FREQ;
      break;
    case 8:
      led = C5_LED;
      freq = C5_FREQ;
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
