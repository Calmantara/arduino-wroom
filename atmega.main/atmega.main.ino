#include <SoftwareSerial.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <ArduinoJson.h>

#include "config/config.h"
#include "BTS7960.h"

void interruptHandler();
void serialMonitorHandler(void *pvParameters);
void motorCommand(void *pvParameters);

int adcValue = 0;
short pressedCount = 0;

// Declaring semaphore handler
SemaphoreHandle_t interruptSemaphore;
StaticJsonDocument<128> jsonBuffer;
SoftwareSerial sfSerial(SOFTWARESERIAL_RX, SOFTWARESERIAL_TX);
//Motor initiate
BTS7960 Motor(MOTOR_L_EN, MOTOR_R_EN, MOTOR_PWM);

void setup()
{

    //initiate threads procedures
    xTaskCreate(serialMonitorHandler,     // Task function
                "Serial Monitor Handler", // Task name
                128,                      // Stack size
                NULL,
                2, // Priority
                NULL);
    xTaskCreate(motorCommand,    // Task function
                "Motor command", // Task name
                128,             // Stack size
                NULL,
                1, // Priority
                NULL);

    //Serial setup for debug purpose only and communication
    sfSerial.begin(9600);
    Serial.begin(115200);

    //BUTTON INTERRUPT setup
    pinMode(BUTTON_INTERRUPT, INPUT);
    //POTENTIO setup
    pinMode(POTENTIO, INPUT);

    interruptSemaphore = xSemaphoreCreateBinary();
    if (interruptSemaphore != NULL)
    {
        //attach interrupt for Arduino digital pin
        attachInterrupt(digitalPinToInterrupt(BUTTON_INTERRUPT), interruptHandler, FALLING);
    }
}

void loop()
{
    //for idle thread
}

/*Declaration Procedure*/
void interruptHandler()
{
    if (pressedCount % 10 == 0)
        pressedCount = 0;
    else
        pressedCount += 1;

    jsonBuffer["pressedCount"] = pressedCount;
    jsonBuffer["adcValue"] = adcValue;

    if (sfSerial.available())
    {
        serializeJson(jsonBuffer, sfSerial);
    }

    xSemaphoreGiveFromISR(interruptSemaphore, NULL);
}

void serialMonitorHandler(void *pvParameters)
{
    (void)pvParameters;
    while (1)
    {
        if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS)
        {
            Serial.print("ADC value : ");
            Serial.println(adcValue);
            Serial.print("Button pressed : ");
            Serial.println(pressedCount);
            //only print value every 1 second
            vTaskDelay(1000);
        }
    }
}

void motorCommand(void *pvParameters)
{
    (void)pvParameters;
    while (1)
    {
        if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS)
        {
            //read adv value
            adcValue = analogRead(POTENTIO);
            //set motor speed
            Motor.SetSpeed(map(adcValue, 0, 1024, 0, 255));
            //sampling time of procedure 0.1 second
            vTaskDelay(100);
        }
    }
}