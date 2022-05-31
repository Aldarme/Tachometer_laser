/*
 * Tachymetre laser avec fonctionnalité suivantes: 
 *  - affichage valeur écran OLED
 *  - Transmission suivante 2 vecteurs:
 *    - USB
 *    - mqtt
 *  - Transmission des données: 2 modes:
 *    - manuel: on envoie un event au device pour que celui-ci envoi une valeur
 *    - automatique: envoie des données à une fréquence de 100 Hertz par défaut
 * 
 */
#include <Arduino.h>

#define sensor_laser 7
#define led 4

/***************************
 * Variables declarations
 ***************************/

//var state detect
uint8_t curState = LOW;
uint8_t prvState = LOW;

//var rpm calculate
unsigned long curTime = 0;
unsigned long prvTime = 0;
unsigned long rpm = 0;      //final rpm value

//working mode
boolean workMod = true;     //true: automatic, false: manual
uint8_t sendFrequency = 10; //frequency to send data in ms
unsigned long currTime = 0; //current time to calculate send frequency
unsigned long oldTime = 0;  //old time to calculate send frequency

//serial Communication
String rcvBuff = "";        //receiving buffer

//mqtt conf
boolean mqttCom = false;    //false: mqtt communication unactivated


//Methods declarations

void configPrint();
void blink_led();
void rpmCalc();
void coreTachy();
void serialConf();
void usbSend();
void mqqtConf(); 
void mqttSend();
void autoSend();
void manualSend();


/***************************
 * Core program
 ***************************/
 
void setup() {
  
  pinMode(sensor_laser, INPUT);
  pinMode(led, OUTPUT);

  Serial.begin(115200);

  configPrint();
  
}

void loop() {

  serialConf();   //configure tachymeter working mode

  mqqtConf();     //enable or not to communication through mqtt

  coreTachy();    //tachometer algo

  autoSendData(); //send data if the device is in automatic mode

  manualSend();   //send data if the device is in manual mode, so if sendEvent is received

  
  
  
}


/*************************************************
 *                  Functions
 *************************************************/

/*
 * Print device config on serial communication 
 */
void configPrint()
{
  Serial.println("Device workMod: " + workMod);
  Serial.println("Send Frequency: " + sendFrequency);
  Serial.println("MQTT communication: " + mqttCom);
}

/*
 * Serial communication configuration
 */
void serialConf(){
  if (Serial.available() > 0)
  {
    rcvBuff = Serial.readString();

    if(rcvBuff == "m"){
      workMod = false;
      Serial.println(rcvBuff);
      Serial.println("Device set in manual mode");
    }

    if(rcvBuff == "auto"){
      workMod = true;
      Serial.println(rcvBuff);
      Serial.println("Device set in automatic mode");
    }
  }
}


/*
 * Mqtt communication configuration
 */
void mqqtConf()
{
  if (Serial.available() > 0)
  {
    rcvBuff = Serial.readString();

    if(rcvBuff == "mqtt:On"){
      mqttCom = true;
      Serial.println(rcvBuff);
      Serial.println("mqtt communication actived");
    }

    if(rcvBuff == "mqtt:Off"){
      mqttCom = false;
      Serial.println(rcvBuff);
      Serial.println("mqtt communication unactived");
    }
  }
}


/*
 * Calculate RPM value
 */
void coreTachy()
{
  digitalWrite(led, LOW); //Led low at begenning to avoid delay in blink

  curState = digitalRead(sensor_laser);

  if(curState != prvState
      && curState == HIGH)
  {
    curTime = micros();   //store the current time to calculate RPM
    
    blink_led();          //blink led when object is detected
    rpmCalc();            //calculate RPM value

    prvState = curState;  //store the current state as old one, for the next front detection
    prvTime = curTime;    //store the current time as old one, for the next RPM calculation
  }
}


/*
 * Blink led according to RPM value
 */
void blink_led()
{
  digitalWrite(led, HIGH);
}


/*
 * RPM equation
 */
void rpmCalc()
{
  rpm = 60000000 / (curTime - prvTime);
}


/*
 * Send data using Serial Communication
 */
void usbSend()
{
  Serial.println(rpm);
}


/*
 * Send data to mqtt broker
 */
void mqttSend()
{
  if( mqttCom = true)
  {
    //send data
  }
}


/*
 * Send data if the device is in automatic mode
 */
void autoSendData()
{
  if (workMod == true) // workmod in automatic
  {
    currTime = millis();
    if(currTime - oldTime >= sendFrequency)
    {
      usbSend();      //send rpm throug serialCom to pc every new rpm value is calculated
  
      mqttSend();     //send rpm through wifi using mqtt every new rpm value is calculated
  
      oldTime = millis();
    }
  }
}


/*
 * Send data if the device is in manual mode
 * If a sendEvent is received
 */
void manualSend()
{
  if (workMod == false) // workmod in automatic
  {
    if (Serial.available() > 0)
    {
      rcvBuff = Serial.readString();
  
      if(rcvBuff == "m:ask")
      {
        usbSend();      //send rpm throug serialCom to pc every new rpm value is calculated
  
        mqttSend();     //send rpm through wifi using mqtt every new rpm value is calculated
      }
    }
  }
}





 
