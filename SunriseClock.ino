/*
   FILE          :  SunriseClock
   PROJECT       :  PROG8125
   PROGRAMMER    :  Rohit Bhardwaj
   FIRST VERSION :  2016-08-09
   DESCRIPTION   :  This code asks the user to first set the time and date and then enter the alarm time,
                    30 minutes before the user want to wake, the RGB Led turns ON. At first dimly Red,then
                    increasing its brightness and colour until it is fully ON and white at the wake time.
                    It uses an RGB led to simulate sunrise. The format to enter Time and Date is:
                    $20:20:20/05/08/2016 and for Alarm Time : 20:50
*/

//Header Files
#include<TimeLib.h>
#include <LiquidCrystal.h>

//create an lcd object
LiquidCrystal lcd(19, 18, 17, 16, 15, 14);
//                RS, En, D4, D5, D6, D7


//PWM pins are used to control the brightness of the RGB lED
const int8_t redLedPin = 3;                                             //RedPin is connected at pin3 of Teensy
const int8_t greenLedPin = 5;                                           //GreenPin is connected at pin5 of Teensy
const int8_t blueLedPin = 6;                                            //BluePin is connected at pin6 of Teensy
const int8_t speakerPin = 10;                                           //speaker is connected to pin 10 of Teensy

uint8_t flag = 0;                                                       //It is used to run a loop only once
uint32_t ledTurnOnTime = 0;                                             //It stores the Turn On Time for RGB Led
uint32_t glow = 0;                                                      //It stores the value given to Red Led of RGB
uint8_t count = 0;                                                      //It is used to counts the 30 minute interval between dimly Red and fully White

//protoype declaration
char *checkForRecvdChar();
void strtokTimeString(char recTimeBuffer[]);
void strtokAlarmTimeString(char recAlarmBuffer[]);
uint32_t  displayClock();
void printDigits(int digits);
void checkAlarm(uint32_t currentTimeInMinutes);
void alarmTone();

void setup()
{
  Serial.begin(9600);                                                   // opens serial port, sets data rate to 9600 bps
  lcd.begin (16, 2);                                                    // set up the LCD's number of rows and columns
  //Configures the specified pins to behave as an output.
  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  lcd.print("Enter Time");
  lcd.setCursor(0, 1);                                                  //Lcd Curosr is Set to first Row and zeroth Column
  lcd.print("and Date:");
}

void loop()
{
  if (Serial.available())                                               //check if any serial data is available on the serial port (for Time String)
  {
    if (flag == 0)                                                      //check if the value of flag is equal to zero
    {
      lcd.clear();
      char *stringTime = checkForRecvdChar();                           //Here the address of the string received from the serial port is stored in a char pointer
      strtokTimeString (stringTime);                                    //calling the strtokTimeString function to parse the Time String
    }
    while (flag == 1)                                                   //AS long as value of Flag is 1 this will run continously
    {
      displayClock ();                                                 //calling //calling the strtokTimeString function to parse the Time String
      if (Serial.available())                                           //Again check if any serial data is available on the serial port( for Alarm Time String)
      {
        char *stringAlarm = checkForRecvdChar();                        //Here the address of the string received from the serial port is stored in a char pointer
        strtokAlarmTimeString(stringAlarm);                             //calling the strtokAlarmTimeString function to parse the Alarm Time String
        while (1)
        {
          uint32_t currentTime = displayClock();                        //Here the current time displayed by the dispalyClock function is stored in a varaible
          checkAlarm(currentTime);                                      //Calling the checkAlarm function to pass the CurrentTime
        }
      }

    }
  }
}


// FUNCTION      : checkForRecvdChar()
// DESCRIPTION   : Checks the serial port to see if any chars are available.If
//                 so, reads the chars using  the serial.read method is read.
// PARAMETERS    : none
// RETURNS       : char* - recbuffer
char *checkForRecvdChar()
{
  static int recByteCtr = 0;                                               //declare and initialize the rec'd byte counter variable
  static char recBuffer [255];
  char byteRead;

  while (Serial.available())                                               //then let's read the byte
  {
    if (recByteCtr == 0)                                                   //then check to see if it's the start of a message
    {
      byteRead = Serial.read();                                            //read the byte
      if (byteRead == '$')
      {
        recBuffer[recByteCtr] = byteRead;                                  //copy the byte read into the rec buffer
        recByteCtr++;
      }
    }
    if (recByteCtr != 0)                                                   //then we're reading bytes after receiving an STX in the message stream
    {
      byteRead = Serial.read();                                            //read the byte
      if (byteRead != 13) //13 = CR
      {
        recBuffer[recByteCtr] = byteRead;                                  //copy the byte read into the rec buffer
        recByteCtr++;
      }
      else //recbyte == 13
      {
        recBuffer[recByteCtr] = '\0';                                      //null terminate the rec'd string in rec buffer
        recByteCtr = 0;
        flag = 1;                                                          //Changing the state of the flag
        return recBuffer;
      }
    }
  }
}

// FUNCTION      : strtokTimeString()
// DESCRIPTION   : It parse the recevied string with the help of
//                 delimiter(:) for time, delimiter(/) for date and
//                 set the time and date using SetFunction
// PARAMETERS    : Char Array - recBuffer
// RETURNS       : nothing
void strtokTimeString(char recTimeBuffer[])
{
  const char *delim  = ":";                                           //a colon is the delimiter for time
  const char *delimi = "/";                                           //a backslah is the delimiter for date

  //Now initialize all the pointer variables as NULL pointer
  // As subsquent calls to strtok are done using the Null pointer
  char *hh = NULL;                                                    //pointer variable for hours
  char *mm = NULL;                                                    //pointer variable for minutes
  char *ss = NULL;                                                    //pointer variable for seconds
  char *dd = NULL;                                                    //pointer variable for date
  char *mo = NULL;                                                    //pointer variable for month
  char *yy = NULL;                                                    //pointer variable for year

  hh = strtok(recTimeBuffer, delim);                                  //it will parse the string and returns the beginning address of the string
  mm = strtok(NULL, delim);
  ss = strtok(NULL, delimi);
  dd = strtok(NULL, delimi);
  mo = strtok(NULL, delimi);
  yy = strtok(NULL, delimi);

  uint32_t hours = atoi(hh + 1);                                      //It will return the integer value for hours
  uint32_t minutes = atoi(mm);                                        //It will return the integer value for minutes
  uint32_t seconds = atoi(ss);                                        //It will return the integer value for seconds
  uint32_t days = atoi(dd);                                           //It will return the integer value for date
  uint32_t months = atoi(mo);                                         //It will return the integer value for month
  uint32_t years = atoi(yy);                                          //It will return the integer value for year
  setTime(hours, minutes, seconds, days, months, years);              //It set the current time, based on its parameters
}


// FUNCTION      : strtokAlarmTimeString()
// DESCRIPTION   : It parse the recevied string with the help of
//                 delimiter(:) for Alarm time
// PARAMETERS    : Char Array - recAlarmBuffer
// RETURNS       : nothing
void strtokAlarmTimeString(char recAlarmBuffer[])
{
  const char *delim  = ":";                                            //a colon is the delimiter
  char *ah = NULL;                                                     //pointer variable for Alarm hours
  char *am = NULL;                                                     //pointer variable for Alarm Minutes

  ah = strtok(recAlarmBuffer, delim);                                  //It will parse the string
  am = strtok(NULL, delim);

  uint32_t alarmHours = atoi(ah + 1);                                  //It will return the integer value for Alarm hours
  uint32_t alarmMinutes = atoi(am);                                    //and minutes
  uint32_t hoursToMinutes = alarmHours * 60;                           //Converting Alarm Hours value into minutes
  ledTurnOnTime = (hoursToMinutes + alarmMinutes) - 30;                //To get the time at which RGB led should turn on
  //(total minutes - 30 minutes)
}


// FUNCTION      : checkAlarm()
// DESCRIPTION   : It checks the 30 minute before you want to wake Alarm
//                 Time condition and turns the RGB led On,first dimly red,
//                 then increasing in brightness and colour until it is
//                 fully on and white at your wake time
// PARAMETERS    : None
// RETURNS       : nothing
void checkAlarm(uint32_t currentTimeInMinutes)
{
  if (currentTimeInMinutes == ledTurnOnTime)                             //check condition
  {
    glow = (count * 8) + 15;                                             //(minutes*8)+ base that is 15
    analogWrite(redLedPin, glow );                                       //Turn on the redLed with analog value equal to glow
    analogWrite(greenLedPin, 0 );
    analogWrite(blueLedPin, 0 );
    delay(1000);
    ledTurnOnTime++;                                                     // It will get incremented till it reaches the Wake up time
    count++;                                                             // Increment the count too along with ledturnontime so that it can count 30 minutes
    if (count == 31)                                                     // It checks for that 30 minute time interval
    {
      // RGB turn white after 30 minute
      analogWrite(redLedPin, 255);
      analogWrite(greenLedPin, 255);
      analogWrite(blueLedPin, 255 );
      lcd.clear();                                                       // Clears the Lcd Screen
      lcd.setCursor(1, 0);                                               // Cursor is Set to Zeroth Row and first Column of a Lcd
      lcd.print("Wake up!");                                             // Prints a wake up message on the LCD screen
      alarmTone ();                                                      // Calling the alarmTone function to make an Alarm Tone
      while (1);                                                         // Loop will Stop its execution
    }
  }
}

// FUNCTION    : alarmTone
// DESCRIPTION : This function is used to produce the sound at Alarm Time
// PARAMETERS  : None
// RETURNS     : Nothing
void alarmTone()  // Function Declaration
{
  for (int16_t i = 0; i < 5; i++)
  { // Loop will be executed for specific number of iterations
    tone(speakerPin, 200, 1000);                                          // Produces a tone
    delay(100);
    noTone(speakerPin);                                                   // Stops the tone
    delay(100);
  }
}

// FUNCTION    : dispalyClock
// DESCRIPTION : This function will display the Current Time in Hrs:Min:Sec
//               and also the Current date in DD:MM:YY
// PARAMETERS  : None
// RETURNS     : uint32_t - currentTimeInMinutes
uint32_t  displayClock()
{
  uint32_t hours = hour();                                                // Stores the current time, in hours (0 to 23)
  uint32_t currentHoursToMinutes = hours * 60;                            // Converting Current hours to minutes
  uint32_t minutes = minute();                                            // Stores the current minutes (0 to 59)
  lcd.setCursor(0, 0);
  lcd.print("Time");                                                      // Prints a message that is Time on LCD
  lcd.setCursor(6, 0);                                                    // Lcd Curosr is Set to Zeroth Row and Sixth Column
  lcd.print(hour());                                                      // Prints the current hours (0 to 23)
  printDigits (minute());                                                 // Calling the printDigits Function and passing the minutes as parameter
  printDigits (second());                                                 // Calling the printDigits Function and passing the seconds as parameter
  lcd.setCursor(0, 1);                                                    // Cursor is Set to first Row and zeroth Column of a Lcd
  lcd.print("Date");                                                      // Prints a message that is Date on LCD
  lcd.setCursor(6, 1);                                                    // Cursor is Set to first  Row and sixth Column of a Lcd
  lcd.print(day());                                                       // Prints the current date(1 to 31)
  lcd.print(" ");
  lcd.print(month());                                                     // Prints the current month(1 to 12)
  lcd.print(" ");
  lcd.print(year());                                                      // Prints the current year(2016)
  uint32_t currentTimeInMinutes = currentHoursToMinutes + minutes;        // Converting the total current time into minutes
  return currentTimeInMinutes;
}


// FUNCTION    : printDigits
// DESCRIPTION : This function will print current minutes and seconds on the Lcd
// PARAMETERS  : It takes minutes and seconds as parameters
// RETURNS     : Nothing
void printDigits(int digits)
{
  // utility function for digital clock display: prints preceding colon and leading 0
  lcd.print(":");
  if (digits < 10)
  {
    lcd.print('0');
  }
  lcd.print(digits);                                                       // Prints the current seconds and minutes
}


