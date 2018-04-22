#include <LiquidCrystal.h>
#include <NewTone.h>
#include <IRremote.h>

int tempPin = 0;
int receiverPin = 6;

int hours = 0;
int minutes = 0;
int seconds = 0;

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
IRrecv irrecv(receiverPin);
decode_results results;
String timerDuration = "";
int delayDuration = 200;
unsigned long timerRingAt = 0;

void setup()
{ 
  Serial.begin(9600);
  lcd.begin(16, 2);
  irrecv.enableIRIn(); // Start the receiver
}

void loop()
{
  // Temperature every minute
  int modulo = round(millis() / 1000) % 10;
  if(modulo == 0 && delayDuration == 200) {
    int tempReading = analogRead(tempPin);
  
    double tempK = log(10000.0 * ((1024.0 / tempReading - 1)));
    tempK = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * tempK * tempK )) * tempK );       //  Temp Kelvin
    double tempC = tempK - 273.15; // Convert Kelvin to Celcius
    
    // Display Temperature in C
    lcd.setCursor(0, 0);
    lcd.print("Temp       C  ");
    lcd.setCursor(6, 0);
    // Display Temperature in C
    lcd.print(tempC);
    // Remove last digit
    lcd.setCursor(10, 0);
    lcd.print(" ");

    delayDuration = 1000;
  }
  else if(modulo != 0 && delayDuration == 1000) {
    delayDuration = 200;
  }

  // Get IR
  if (irrecv.decode(&results)) {
    translateIR(); 
    irrecv.resume(); // receive the next value
  }

  // Display timer
  if(timerDuration != "") {
    displayDuration(hours, minutes, seconds);
  }
  else if(timerRingAt != 0) {
    updateTimer();
  }
  
  delay(200);
}

void translateIR() // takes action based on IR code received
{
  switch(results.value)
  {
    case 0xE0E08877: add(0);        break;
    case 0xE0E020DF: add(1);        break;
    case 0xE0E0A05F: add(2);        break;
    case 0xE0E0609F: add(3);        break;
    case 0xE0E010EF: add(4);        break;
    case 0xE0E0906F: add(5);        break;
    case 0xE0E050AF: add(6);        break;
    case 0xE0E0B04F: add(8);        break;
    case 0xE0E0708F: add(9);        break;
    case 0xE0E016E9: startTimer();  break;
    case 0xE0E040BF: killTimer();   break;
  
    default: 
      break;
  }

  if(timerDuration != "") {
    String tmpDuration = timerDuration;
    if(timerDuration.length() == 5) {
      hours = tmpDuration.substring(0, 1).toInt();
      tmpDuration = tmpDuration.substring(1);
    }

    if(tmpDuration.length() == 4) {
      minutes = tmpDuration.substring(0, 2).toInt();
      tmpDuration = tmpDuration.substring(2);      
    }

    if(tmpDuration.length() == 3) {
      minutes = tmpDuration.substring(0, 1).toInt();
      tmpDuration = tmpDuration.substring(1);      
    }
    
    seconds = tmpDuration.toInt();
  }
}

void displayDuration(int hoursToDisplay, int minutesToDisplay, int secondsToDisplay)
{
  lcd.setCursor(0, 1);
  lcd.print("Timer " + String(hoursToDisplay) + "h" + String(minutesToDisplay) + "min" + String(secondsToDisplay) + "s   ");
}

void updateTimer()
{
  long timeLeft = (timerRingAt - millis() / 1000);

  if(timeLeft < 0) {
    ring();

    return;
  }

  int hoursLeft = (int) timeLeft / 3600;
  int minutesLeft = (timeLeft / 60) % 60;
  int secondsLeft = timeLeft % 60;

  displayDuration(hoursLeft, minutesLeft, secondsLeft);
}

void add(int number)
{
  if(timerDuration.length() < 5) {
    timerDuration += number;
  }
}

void startTimer()
{
  if(timerDuration != "") {
    if(seconds >= 60 || minutes >= 60) {
      killTimer();
      lcd.setCursor(0, 1);
      lcd.print("Timer error     ");
  
      return;
    }
  
    timerDuration = "";
    timerRingAt = millis() / 1000 + (seconds + minutes * 60 + hours * 3600);
  }
}

void ring()
{
  killTimer();
  
  lcd.setCursor(0, 1);
  lcd.print("Timer over");

  for (int i = 1; i <= 6; ++i) {
    NewTone(2, 880);
    
    delay(500);
    noNewTone(2);
    delay(200);
    
    if(i % 3 == 0) {
      delay(500);
    }
  }
  
  NewTone(2, 1480);
  delay(700);
  noNewTone(2);
}

void killTimer()
{
  timerDuration = "";
  timerRingAt = 0;
  hours = 0;
  minutes = 0;
  seconds = 0;
  
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

