#include <EEPROM.h>
#include <avr/wdt.h>
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>


#define Sensor_panel A5     // solar panel sensor
#define Sensor_boiler A4    // boiler sensor
#define Power_boiler 9      // boiler power socket
#define Power_panel_pump 10 // solar panel pump power socket
#define tollerancePlusButton A2
#define tolleranceMinusButton A0

bool sensorPanelReady, sensorBoilerReady;
float Sensor_panel_tempC, Sensor_boiler_tempC; // memo the temepratures
float tempMinim_Sensor_panou = 30;
float tempPower_panel_pump_Tolerance = 0;
float tempPower_panel_pump_ON = 10;
int tollerancePlusButtonState = 0;
int tolleranceMinusButtonState = 0;
int eeToleranceAddress = 0; // Memory location where we want the data to be saved between sessions.

OneWire Sensor_panel_T(Sensor_panel);
OneWire Sensor_boiler_T(Sensor_boiler);
DallasTemperature Sensor_panel_dallas(&Sensor_panel_T);
DallasTemperature Sensor_boiler_dallas(&Sensor_boiler_T);
DeviceAddress Sensor_panel_Address;
DeviceAddress Sensor_boiler_Address;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup()
{
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print(" Controler pa-");
  lcd.setCursor(0, 1);
  lcd.print("nou solar/boiler");
  delay(1000);
  lcd.clear();
  pinMode(Power_boiler, OUTPUT);
  digitalWrite(Power_boiler, LOW);
  pinMode(Power_panel_pump, OUTPUT);
  digitalWrite(Power_panel_pump, LOW);

  pinMode(tollerancePlusButton, INPUT);
  pinMode(tolleranceMinusButton, INPUT);

  tempPower_panel_pump_Tolerance = EEPROM.read(eeToleranceAddress);

  Sensor_panel_dallas.begin();
  if (!Sensor_panel_dallas.getAddress(Sensor_panel_Address, 0))
    Sensor_panel_dallas.setResolution(Sensor_panel_Address, 9);
  Sensor_boiler_dallas.begin();
  if (!Sensor_boiler_dallas.getAddress(Sensor_boiler_Address, 0))
    Sensor_boiler_dallas.setResolution(Sensor_boiler_Address, 9);
}

void loop()
{
  adjustTollerance();

  if (!Sensor_panel_dallas.getAddress(Sensor_panel_Address, 0))
  {
    lcd.setCursor(0, 0);
    lcd.print("lipsa senzor panou");
    sensorPanelReady = false;
  }
  else
  {
    Sensor_panel_dallas.requestTemperatures(); // Send the command to get temperatures
    Sensor_panel_tempC = Sensor_panel_dallas.getTempC(Sensor_panel_Address);
    if (Sensor_panel_tempC == -127.00)
    {
      lcd.setCursor(0, 0);
      lcd.print("eroare senzor panou");
      sensorPanelReady = false;
    }
    else
    {
      lcd.setCursor(0, 0);
      lcd.print("panou");
      lcd.setCursor(6, 0);
      lcd.print(Sensor_panel_tempC);
      lcd.setCursor(12, 0);
      lcd.print(Sensor_panel_tempC - Sensor_boiler_tempC);
      sensorPanelReady = true;
    }
  }

  if (!Sensor_boiler_dallas.getAddress(Sensor_boiler_Address, 0))
  {
    lcd.setCursor(0, 1);
    lcd.print("lipsa senzor boiler");
    sensorPanelReady = false;
  }
  else
  {
    Sensor_boiler_dallas.requestTemperatures();
    Sensor_boiler_tempC = Sensor_boiler_dallas.getTempC(Sensor_boiler_Address);
    if (Sensor_boiler_tempC == -127.00)
    {
      lcd.setCursor(0, 1);
      lcd.print("eroare senzor boiler");
      sensorBoilerReady = false;
    }
    else
    {
      Serial.println(Sensor_boiler_tempC);
      lcd.setCursor(0, 1);
      lcd.print("boiler");
      lcd.setCursor(7, 1);
      lcd.print(Sensor_boiler_tempC);
      sensorBoilerReady = true;
    }
  }

  if (sensorBoilerReady && sensorBoilerReady)
  {
    if (Sensor_panel_tempC <= tempMinim_Sensor_panou)
    {
      digitalWrite(Power_panel_pump, LOW);
      digitalWrite(Power_boiler, HIGH);
    }
    else
    {
      digitalWrite(Power_boiler, LOW);

      if (Sensor_panel_tempC - Sensor_boiler_tempC >= tempPower_panel_pump_ON)
      {
        digitalWrite(Power_panel_pump, HIGH);
      }
      // if in between, it keeps the command from the previous cycle
      else if (Sensor_panel_tempC - Sensor_boiler_tempC <= tempPower_panel_pump_ON - tempPower_panel_pump_Tolerance)
      {
        digitalWrite(Power_panel_pump, LOW);
      }
    }
  }
  else
  {
    digitalWrite(Power_panel_pump, LOW);
    digitalWrite(Power_boiler, LOW);
  }
}

void adjustTollerance()
{
  tollerancePlusButtonState = digitalRead(tollerancePlusButton);
  tolleranceMinusButtonState = digitalRead(tolleranceMinusButton);

  if (tollerancePlusButtonState == HIGH && tempPower_panel_pump_Tolerance < 10)
  {
    delay(100);
    tempPower_panel_pump_Tolerance++;
  }

  if (tolleranceMinusButtonState == HIGH && tempPower_panel_pump_Tolerance > 0)
  {
    delay(100);

    if (tempPower_panel_pump_Tolerance == 10)
    {
      // clears when the number of digits is about to decrease
      lcd.clear();
    }

    tempPower_panel_pump_Tolerance--;
  }

  // writes data only if it is different from the previous content
  EEPROM.update(eeToleranceAddress, tempPower_panel_pump_Tolerance);

  lcd.setCursor(12, 1);
  lcd.print(" T");
  lcd.setCursor(14, 1);
  lcd.print((int)tempPower_panel_pump_Tolerance);
}
