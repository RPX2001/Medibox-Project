//Libararies for functionalities
#include <Wire.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ESP32Servo.h>


//define screen details for the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

//define the Pins
#define BUZZER 5 
#define LED_1 15 

//define the push buttons
#define PB_CANCEL 26 
#define PB_OK 32 
#define PB_UP 33 
#define PB_DOWN 35 

//define DHT 22
#define DHTPIN 12

// LDR pins
#define LDRleft 34
#define LDRright 39

//for time update
#define NTP_SERVER "pool.ntp.org"
int UTC_OFFSET = 0;
int UTC_OFFSET_DST = 0;

//Declare objects - OLED display and DHT Sensor
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;


WiFiClient espClient;
PubSubClient mqttClient(espClient);


//Global Variables
int days = 0;
int hours = 0;
int minutes = 0;
int seconds = 0;

//for buzzer
int C = 262; //define note C
int D = 294; //define note D
int E = 330; //define note E
int F = 349; //define note F
int G = 392; //define note G
int A = 440; //define note A
int B = 494; //define note B
int C_H = 523; //define note C_H
int notes[] = {C,D,E,F,G,A,B,C_H};
int n = 8;

bool alarm_enabled = true; //turn all alarm on
int n_alarms = 3; // keep track of the alarm
int alarm_hours[] = {0,1,2}; // set hours for two alarms
int alarm_minutes[] = {1,10,20}; // set minutes for two alarms
bool alarm_triggered[] = {false,false,false};//keep the track whether alarm triggered

int current_mode = 0; //current mode in the menu
int max_modes = 5; //number of modes in the menu
String modes[] = {"1 - Set Time Zone", "2 - Set Alarm 1", "3 - Set Alarm 2", 
"4 - Set Alarm 3", "5 - Disable Alarm"};

char tempArr[6];
char LDRl[4];
char LDRr[4];

Servo servo;  //servo motor
// default servo controlling values
int minAngle = 30;
float cf = 0.75;

/********** Setting Up ************/                           

void setup() {
  pinMode(BUZZER, OUTPUT); //define the buzzer pin mode as a output
  pinMode(LED_1, OUTPUT); //define the LED indicator pin mode as a output
  //define the Push button pin mode as input
  pinMode(PB_CANCEL,INPUT); 
  pinMode(PB_OK,INPUT);
  pinMode(PB_UP,INPUT);
  pinMode(PB_DOWN,INPUT);
  pinMode(LDRleft, INPUT);
  pinMode(LDRright, INPUT);

  dhtSensor.setup(DHTPIN, DHTesp::DHT22); //configure the sensor setup

  //Intialize serial monitor and OLED display
  Serial.begin(115200);
  if (! display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  //turn on OLED display
  display.display();
  delay(2000);

  //connect to the WIFI to automatically update the time
  WiFi.begin("Wokwi-GUEST", "", 6 );
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    display.clearDisplay();
    printLine("Connecting to WiFi", 0, 0, 2); //until wifi is connected display the message
  }

  display.clearDisplay();
  printLine("Connected to WiFi", 0, 0, 2); // Display after wifi is connected

  setupMqtt();
  //timeClient.begin();
  servo.attach(23);


  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER); //configuer the time from internet

  //clear oled display
  display.clearDisplay();

  printLine("Welcome to Medibox", 10, 20, 2); //Display Welcome message
  display.clearDisplay();

  for(int i=0;i<5;i++){
    tone(BUZZER,notes[i]); // Beep
    delay(100);
  }
  noTone(BUZZER);

  delay(1000);
}


/********** Looping the Code *********/

void loop() {
  /////new lines
  if (!mqttClient.connected()) {
    Serial.println("Reconnecting to MQTT Broker");
    connectTOBroker();
  }

  mqttClient.loop();

  // updateTemp();
  // Serial.println(tempArr);
  // mqttClient.publish("ENTC-TEMP_NI", tempArr);
  // checkSchedule();
  // delay(50);

  updateLight();

  // Serial.print(ldrLArr);
  // Serial.println(ldrRArr);

  mqttClient.publish("LDR_left", LDRl);
  delay(50);
  mqttClient.publish("LDR_right", LDRr);
  delay(100);

  ////////////////////////////
  update_time_with_check_alarm();

  if(digitalRead(PB_OK)==LOW){
    delay(200);
    go_to_menu();
  }

  check_temp_and_humidity();
}


/*********** Blink LED **********/

void LED_Blink(void){
  digitalWrite(LED_1, HIGH);
  delay(500);
  digitalWrite(LED_1, LOW);
  delay(500);
}


/******** Function to printLine in OLED *********/

void printLine(String text, int column, int row, int text_size){
  display.setTextSize(text_size);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(column,row);
  display.println(text);

  display.display();
}


/****** Function to Print Current Time in OLED ********/

void printTimeNow(void){
  display.clearDisplay();
  printLine(String(days),0,0,2);
  printLine(":",20,0,2);
  printLine(String(hours),30,0,2);
  printLine(":",50,0,2);
  printLine(String(minutes),60,0,2);
  printLine(":",80,0,2);
  printLine(String(seconds),90,0,2);
}


/********* Function to Update Time **********/

void updateTime(void){
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  
  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  hours = atoi(timeHour);

  char timeMinute[3];
  strftime(timeMinute, 3, "%M", &timeinfo);
  minutes = atoi(timeMinute);

  char timeSecond[3];
  strftime(timeSecond, 3, "%S", &timeinfo);
  seconds = atoi(timeSecond);

  char timeDay[3];
  strftime(timeDay, 3, "%d", &timeinfo);
  days = atoi(timeDay);
  
}


/********* Function to Ring Alarms *********/

void ring_alarm(void){
  display.clearDisplay();
  printLine("MEDICINE TIME!",0,0,2); //Display when alarm triggered

  LED_Blink(); //Blink the LED when alarm triggered

  bool break_happened = false; //alarm status

  while(break_happened == false && digitalRead(PB_CANCEL)==HIGH){
    for (int i = 0; i < n; i++ ){
      if (digitalRead(PB_CANCEL)==LOW){
        delay(200);
        break_happened = true;//stop the alarm when PB_cancle is pressed
        break;
      }
      tone(BUZZER, notes[i]); //ring the buzzer
      delay(500);
      noTone(BUZZER);
      delay(2);
    }
  }
  
  digitalWrite(LED_1,LOW);
  display.clearDisplay();
}


/***** Function to Updating the time with Checking Alarm *****/

void update_time_with_check_alarm(void){
  updateTime();
  printTimeNow();

  if (alarm_enabled == true){
    for (int i=0; i<n_alarms; i++){
      if (alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes){
        ring_alarm();
        alarm_triggered[i] = true;
      }
    }
  }
}


/******* Function to Waiting for the button press ******/

int wait_for_button_press(){
  //function for buttons
  while(true){ //infinite loop until any button is pressed
    if (digitalRead(PB_UP) == LOW){
      delay(200);
      return PB_UP;
    }
    else if (digitalRead(PB_DOWN) == LOW){
      delay(200);
      return PB_DOWN;
    }
    else if (digitalRead(PB_OK) == LOW){
      delay(200);
      return PB_OK;
    }
    else if (digitalRead(PB_CANCEL) == LOW){
      delay(200);
      return PB_CANCEL;
    }
  }
}


/********* Function to Access Menu *********/

void go_to_menu(){
  //function for the menu access

  current_mode = 0; 
  //When we stay on another mode and then pressed cancel, 
  //if menu is pressed again, then menu is started from that lasted mode.
  // that is fixed here.

  display.clearDisplay();
  printLine("Menu",0,2,2);
  delay(1000);

  while (digitalRead(PB_CANCEL) == HIGH){
    display.clearDisplay();
    printLine(modes[current_mode], 0, 0, 2);

    int pressed = wait_for_button_press();

    if (pressed == PB_UP){
      delay(200);
      current_mode += 1;
      current_mode = current_mode % max_modes;
    }
    else if (pressed == PB_DOWN){
      delay(200);
      current_mode -= 1;
      if (current_mode < 0){
        current_mode = max_modes - 1;
      }
    }
    else if (pressed == PB_OK){
      delay(200);
      Serial.println(current_mode);
      run_mode(current_mode);
    }
    else if (pressed == PB_CANCEL){
      delay(200);
      break;
    }
  }

}


/******* Function to Set the Alarms **********/

void set_alarm(int alarm){
  //function to set the alarm from menu
  int temp_hour = alarm_hours[alarm];
  while(true){
    display.clearDisplay();
    printLine("Enter alarm hour: "+String(temp_hour),0,0,2);

    int pressed = wait_for_button_press();

    if (pressed == PB_UP){
      delay(200);
      temp_hour += 1;
      temp_hour = temp_hour % 24;
    }
    else if (pressed == PB_DOWN){
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0){
        temp_hour = 23;
      }
    }
    else if (pressed == PB_OK){
      delay(200);
      alarm_hours[alarm] = temp_hour;
      break;
    }

    else if (pressed == PB_CANCEL){
      delay(200);
      break;
    }
  }

  int temp_minute = alarm_minutes[alarm];
  while(true){
    display.clearDisplay();
    printLine("Enter alarm minutes: "+String(temp_minute),0,0,2);

    int pressed = wait_for_button_press();

    if (pressed == PB_UP){
      delay(200);
      temp_minute += 1;
      temp_minute = temp_minute % 60;
    }
    else if (pressed == PB_DOWN){
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0){
        temp_minute = 59;
      }
    }
    else if (pressed == PB_OK){
      delay(200);
      alarm_minutes[alarm] = temp_minute;
      break;
    }

    else if (pressed == PB_CANCEL){
      delay(200);
      break;
    }
  }

  display.clearDisplay();
  printLine("Alarm is set",0,0,2);
  delay(1000);
}


/******* Function to define the modes in Menu *******/

void run_mode(int mode){
  //function to identify the mode
  if (mode == 0){
    set_time_zone();
  }
  if (mode == 1 || mode == 2 || mode == 3){
    set_alarm(mode-1);
  }
  if (mode == 4){
    alarm_enabled =false;
  }
}


/****** Function to Check Temperature and Humidity *******/

void check_temp_and_humidity(){
  // function to check the temprature and humidity
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  if (data.temperature > 32){
    display.clearDisplay();
    printLine("TEMP_HIGH",0,40,1);
    LED_Blink();
  } 
  else if (data.temperature < 26){
    display.clearDisplay();
    printLine("TEMP_LOW",0,40,1);
    LED_Blink();
  }
  if (data.humidity > 80){
    display.clearDisplay();
    printLine("HUMIDITY_HIGH",0,50,1);
    LED_Blink();
  } 
  else if (data.humidity < 60){
    display.clearDisplay();
    printLine("HUMIDITY_LOW",0,50,1);
    LED_Blink();
  }  
}


/******* Function to Set the Time Zone **********/

void set_time_zone(){
  int temp_hour = 0;

  while(true){
    display.clearDisplay();
    printLine("Enter hour: " + String(temp_hour),0,0,2);

    int pressed = wait_for_button_press();

      if(pressed == PB_UP){
        delay(200);
        temp_hour += 1;
        temp_hour = temp_hour % 24;
      }
      else if(pressed == PB_DOWN){
        delay(200);
        temp_hour -= 1;
      }
      else if(pressed == PB_OK){
        delay(200);
        UTC_OFFSET = (3600 * temp_hour);
        break;
      }
      else if(pressed == PB_CANCEL){
        delay(200);
        break;
      }
  }

  int temp_minute = 0;

  while(true){
    display.clearDisplay();
    printLine("Enter Minute: " + String(temp_minute),0,0,2);

    int pressed = wait_for_button_press();

      if(pressed == PB_UP){
        delay(200);
        temp_minute += 30;
        temp_minute = temp_minute % 60;
      }
      else if(pressed == PB_DOWN){
        delay(200);
        temp_minute -= 30;
        if(temp_minute <= -1){
          temp_minute = 59;
        }
      }
      else if(pressed == PB_OK){
        delay(200);
        if(temp_hour >= 0){
          UTC_OFFSET = UTC_OFFSET+(60 * temp_minute);
        }
        else{
          UTC_OFFSET = UTC_OFFSET-(60 * temp_minute);
        }
        break;
      }
      else if(pressed == PB_CANCEL){
        delay(200);
        break;
      }
  }

  configTime(UTC_OFFSET,UTC_OFFSET_DST,NTP_SERVER);


  display.clearDisplay();
  printLine("Time Zone is set",0,0,2); 
  delay(1000);

}

void setupMqtt() {
  mqttClient.setServer("test.mosquitto.org", 1883);
  mqttClient.setCallback(recieveCallback);
}

void connectTOBroker() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32Client-sdfsjdfsdfsdf")) {
      Serial.println("MQTT Connected");
      mqttClient.subscribe("control_factor");
      mqttClient.subscribe("angle");
      mqttClient.subscribe("LDR_left");
      mqttClient.subscribe("LDR_right");
    } else {
      Serial.print("Failed To connect to MQTT Broker");
      Serial.print(mqttClient.state());
      delay(5000);
    }
  }
}

void recieveCallback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char payloadCharAr[length];
  Serial.print("Message Recieved: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i];
  }
  Serial.println();
  if (strcmp(topic, "angle") == 0) {
    minAngle = String(payloadCharAr).toInt();

  } else if (strcmp(topic, "control_factor") == 0) {
    cf = String(payloadCharAr).toFloat();
  }
}

void updateLight() {

  float lsv = analogRead(LDRleft) * 1.00;
  float rsv = analogRead(LDRright) * 1.00;

  float lsv_cha = (float)(lsv - 4063.00) / (32.00 - 4063.00);
  float rsv_cha = (float)(rsv - 4063.00) / (32.00 - 4063.00);
  //  Serial.println(String(lsv_cha)+" "+String(rsv_cha));

  updateAngle(lsv_cha, rsv_cha);

  String(lsv_cha).toCharArray(LDRl, 4);
  String(rsv_cha).toCharArray(LDRr, 4);
}

void updateAngle(float lsv, float rsv) {
  float max_I = lsv;
  float D = 1.5;

  if (rsv > max_I) {
    max_I = rsv;
    D = 0.5;
  }

  int theta = minAngle * D + (180 - minAngle) * max_I * cf;
  theta = min(theta, 180);

  servo.write(theta);
}