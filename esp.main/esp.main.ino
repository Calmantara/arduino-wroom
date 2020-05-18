#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <HttpClient.h>

#include "config/config.h"

void generateNewLog();
void sendToServer();
void getFromClient(WiFiClient client);
void logDataIntoDataLogger(String adcValue, String pressedCount);

int adcValue = 0;
short pressedCount = 0;
short iteration;
String currentFileName;
boolean sendLogic = false;

// Declaring semaphore handler
StaticJsonDocument<128> jsonBuffer;
SoftwareSerial sfSerial(SOFTWARESERIAL_RX, SOFTWARESERIAL_TX);
File myFile;

//Wifi init
WiFiServer server(80);
WiFiClient clientToServer;

void setup()
{
    //Serial setup for debug purpose only and communication
    sfSerial.begin(9600);
    Serial.begin(115200);
    //find latest file in sd card
    while (myFile)
    {
        String fileName = DEFAULT_FILE_NAME + String(iteration) + ".txt";
        currentFileName = fileName;
        myFile = SD.open(fileName, FILE_WRITE);
        iteration += 1;
    }
    //WiFi setup
    WiFi.begin(WIFI_SSID, WIFI_PWD);
    delay(10000);
}

void loop()
{
    deserializeJson(jsonBuffer, sfSerial);
    adcValue = jsonBuffer["adcValue"];
    pressedCount = jsonBuffer["pressCount"];

    if (int(pressedCount) >= 10)
    {
        generateNewLog();
    }
    logDataIntoDataLogger(String(adcValue), String(pressedCount));

    WiFiClient client = server.available();
    if (client)
    {
        getFromClient(client);
    }
    client.stop();

    if (sendLogic == true)
    {
        sendToServer();
    }
}

/*Declaration Procedure*/
void logDataIntoDataLogger(String adcValue, String pressedCount)
{
    myFile.println(pressedCount + ". " + adcValue);
}

void generateNewLog()
{
    myFile.close();
    String fileName = DEFAULT_FILE_NAME + String(iteration) + ".txt";
    currentFileName = fileName;
    myFile = SD.open(fileName, FILE_WRITE);
    iteration += 1;
}

void sendToServer()
{
    myFile.close();
    myFile = SD.open(currentFileName);
    while (myFile.available())
    {
        if (clientToServer.connect(SERVER_END_POIN, SERVER_END_POIN_PORT))
        {
            clientToServer.println("POST /param=" + String(myFile.read()) + " HTTP/1.1");
        }
    }
    myFile.close();
    //reopen to write
    myFile = SD.open(currentFileName, FILE_WRITE);
    sendLogic = false;
}

void getFromClient(WiFiClient client)
{
    Serial.println("new client"); // print a message out the serial port
    String currentLine = "";      // make a String to hold incoming data from the client
    while (client.connected())
    { // loop while the client's connected
        if (client.available())
        {                           // if there's bytes to read from the client,
            char c = client.read(); // read a byte, then
            Serial.write(c);        // print it out the serial monitor
            if (c == '\n')
            {
                if (currentLine.length() == 0)
                {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-type:text/html");
                    client.println();
                    // the content of the HTTP response follows the header:
                    client.print("Click <a href=\"/H\">here</a> Send Data to server");
                    // The HTTP response ends with another blank line:
                    client.println();
                    // break out of the while loop:
                    break;
                }
                else
                { // if you got a newline, then clear currentLine:
                    currentLine = "";
                }
            }
            else if (c != '\r')
            {                     // if you got anything else but a carriage return character,
                currentLine += c; // add it to the end of the currentLine
            }
            // Check to see if the client request was "GET /H" or "GET /L":
            if (currentLine.endsWith("GET /H"))
            {
                sendLogic = true;
            }
        }
    }
}