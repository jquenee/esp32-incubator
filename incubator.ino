/**********************************************************************
  Filename    : Drive LiquidCrystal I2C to display characters
  Description : I2C is used to control the display characters of LCD1602.
  Auther      : www.freenove.com
  Modification: 2020/07/11
**********************************************************************/
// Version 1.1.2
// https://github.com/johnrickman/LiquidCrystal_I2C
//#include <LiquidCrystal_I2C.h>

//  Version 2.0.2
// https://github.com/locple/LCDI2C_Multilingual
#include <LCDI2C_Multilingual.h>

#include <Wire.h>

#define SDA 21                    //Define SDA pins
#define SCL 22                    //Define SCL pins

// Version 1.4.6
// https://github.com/adafruit/DHT-sensor-library
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 32
#define DHTTYPE DHT11

// init sensor temperature and humidity
DHT_Unified dht(DHTPIN, DHTTYPE);

// DTH values
float temperature = 0;
float humidity = 0;

bool heaterOn = false;
bool blink = false;

// Relay Heater
#define RELAY 15
#define LED 2

// every 1s
#define REFRESH_RATE 1000

#define LOWEST_TEMP 37
#define HIGHEST_TEMP 38

#define DAY_MS (1000 * 60 * 60 * 24)

/*
 * note:If lcd1602 uses PCF8574T, IIC's address is 0x27,
 *      or lcd1602 uses PCF8574AT, IIC's address is 0x3F.
 * 16x2 screen size
*/
//LiquidCrystal_I2C lcd(0x3F, 16, 2); 
LCDI2C_Latin_Symbols lcd(0x3F, 16, 2);

void setup() {
  Serial.begin(115200); // opens serial port, sets data rate to 115200 bps
  pinMode(RELAY, OUTPUT);
  pinMode(LED, OUTPUT);
 
  // sensor t° and humidity startup
  dht.begin();

  Wire.begin(SDA, SCL);           // attach the IIC pin
  lcd.init();                     // LCD driver initialization
  lcd.backlight();                // Open the backlight
  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
  lcd.print("Couveuse 3.0");     // The print content is displayed on the LCD
}

void relayOff() {
  Serial.println("Stop heating...");
  lcd.setCursor(0,1);
  lcd.print("Stop heating...");
  digitalWrite(LED, LOW);
  digitalWrite(RELAY, LOW);
  heaterOn = false;
}

void relayOn() {
  Serial.println("Start heating...");
  lcd.setCursor(0,1);
  lcd.print("Start heating...");
  digitalWrite(LED, HIGH);
  digitalWrite(RELAY, HIGH);
  heaterOn = true;
}

// in case of error we stop heating
void sensorError() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("!!ERROR SENSOR!!");
  relayOff();
}

// Le DHT11 refresh data every 1s
void sensorDTH11Update() {
  char line[20];

  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Fail to recieve temperature DTH11 data");
    sensorError();
    return;
  } else {
    temperature = event.temperature;
  }
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Fail to recieve humidity DTH11 data");
    sensorError();
  } else {
    humidity = event.relative_humidity;
  }
  lcd.setCursor(0,1);
  sprintf(line, "%0.1f°C %0.1f%%H   ", temperature, humidity);
  Serial.println(line);
  lcd.print(line);
}

// Relay heater control
void heaterControl() {
  if ((temperature < 37) && (!heaterOn)) {
    relayOn();
  } 
  if ((temperature > 38) && (heaterOn)) {
    relayOff();
  }
}

void timeUpdate() {
  char line[5];
  lcd.setCursor(12,0);
  sprintf(line, " J%d", millis() / DAY_MS);
  lcd.print(line);
}

// 0 to 19 days humidity must be between 45 and 50%
// 19 to 21  days humidity must be between 70 and 75%
void humidityCheck() {
  int day = millis() / DAY_MS;
  lcd.setCursor(13,1);

  if (blink) {
    lcd.print("   ");
    blink = !blink;
    return;
  }

  if (day < 19) {
    if (humidity > 50) {
      lcd.print(" DN");
      blink = !blink;
      return;
    }
    if (humidity < 45) {
      lcd.print(" UP");
      blink = !blink;
      return;
    }
  } else {
    if (humidity > 75) {
      lcd.print(" DN");
      blink = !blink;
      return;
    }
    if (humidity < 70) {
      lcd.print(" UP");
      blink = !blink;
      return;
    }
  }
}

// Main function
void loop() {
  sensorDTH11Update();
  timeUpdate();
  heaterControl();
  humidityCheck();
  delay(REFRESH_RATE);
}
