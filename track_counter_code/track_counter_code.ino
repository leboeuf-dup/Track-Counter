/*
  Program to receive voltage signals from a Sharp IR sensor,
  increment a counter and display the count on an LCD display
  module.

  Use Bill Perry's HD44780 library.
  Install through the Arduino IDE library manager:
  (Tools > Manage libraries, then search for "Extensible hd44780")
  See https://github.com/duinoWitchery/hd44780 for documentation and other examples
*/

#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h> // Use the I2C interface

// *** Configuration for I/O pins ***
const int ioPinLed = 7;      // The D7 pin is connected to the LED
const int ioPinSensor = A0;  // Using A0 as the sensor input
const int ioPinDisplay = 2;  // Using D2 to activate the display
const int ioPinPulse = 3;    // Using D3 to output a pulse
const int ioPinReset = 4;    // Using D4 to receive a reset signal

// *** Global Constants relating to the LCD ***
hd44780_I2Cexp lcd;          // Autoconfigure the LCD
const int LCD_COLS = 16;
const int LCD_ROWS = 2;
const int PULSE_DELAY = 50;

// *** Other configuration constants ***
const int UPDATE_INTERVAL_MSEC = 100;   // Defines between sensor readings (100 ms)
const float CROSS_OVER_VOLT = 0.75;     // Sensor voltage == 40cm reading
const float VOLTAGE_CONV_RATIO = (1023 / 5);   // Ratio to convert sensor signal to volts
const uint32_t BAUD_RATE = 115200;      // High "baud rate" to minimize the time taken to send serial outputs

unsigned long lastUpdateTime = -1;

// *** Global variables ***
int count = 0;
int tickOver = 0; // Tick to prevent over counting.

void setup() {
  // Assign pins
  pinMode(ioPinLed, OUTPUT);
  pinMode(ioPinDisplay, INPUT);
  pinMode(ioPinPulse, OUTPUT);
  Serial.begin(BAUD_RATE);

  // Set up LCD
  int status = lcd.begin(LCD_COLS, LCD_ROWS);
  if (status != 0) {
    hd44780::fatalError(status);  // Hangs the program
  }
  lcd.clear();
  lcd.print("COUNT:");
  lcd.setCursor(0, 1);
  lcd.print(count);
}

void loop() {
  // Turn the LED on during each sample/output to give
  // a visual indication of how quickly the sampling is happening.
  digitalWrite(ioPinLed, HIGH);
  digitalWrite(ioPinLed, LOW);
  delay(UPDATE_INTERVAL_MSEC);

  // Read and return sensor values and convert to a voltage.
  float adcValue = analogRead(ioPinSensor);
  float sensorVoltage = adcValue / (VOLTAGE_CONV_RATIO);
 
  // Process voltage and increase count accordingly.
  countUp(sensorVoltage);
  printVals(sensorVoltage, count);

  // Print the current counter in the corresponding column
  lcd.setCursor(0, 1);
  lcd.print(count);
 
  // Switches display on and off via a switch
  int switchDisplay = digitalRead(ioPinDisplay);
  manageDisplay(switchDisplay);

  int switchReset = digitalRead(ioPinReset);
  resetDisplay(switchReset);
}

void countUp(float voltage) {
  if (tickOver == 0) {
      if (voltage <= CROSS_OVER_VOLT) {
        tickOver = 1;
      }
    }
    else {
      if (voltage > CROSS_OVER_VOLT) {
        count += 1;
        digitalWrite(ioPinPulse, HIGH);
        delay(PULSE_DELAY);
        digitalWrite(ioPinPulse, LOW);
        tickOver = 0;
      }
    }
}

void printVals(float voltage, int count) {
  Serial.print("Voltage:"); // Label the data in the serial plotter
  Serial.println(voltage);
  Serial.print("count:"); // Label the data in the serial plotter
  Serial.println(count);
  }

void manageDisplay(int digiVal) {   // Manages the display light using a digital value
  if (digiVal == 1) {
    lcd.on();
    }
    else {
      lcd.off();
    }
}

void resetDisplay(int digiVal) {   // Resets count
  if (digiVal == 1) {
    count = 0;
    lcd.clear();
    lcd.print("COUNT:");
  }
}