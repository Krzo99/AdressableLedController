#include "SoundDetector.h"
#include <WebSocketsServer.h>
#include <soc/sens_reg.h>
#include <WiFi.h>
#include <WebServer.h>

LedController Control = LedController();

static const char ssid[] = "DG57A";
static const char password[] = "omrezje57a";
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<script>
var websock;
function start() {
  websock = new WebSocket('ws://' + window.location.hostname + ':81/');
}
</script>
)rawliteral";

//TaskHandle_t DebugSerialTask;

/*uint64_t reg_a;
uint64_t reg_b;
uint64_t reg_c;*/

void setup() {

  Serial.begin(9600);

  //Save WIFI Off register status
  /*reg_a = READ_PERI_REG(SENS_SAR_START_FORCE_REG);
  reg_b = READ_PERI_REG(SENS_SAR_READ_CTRL2_REG);
  reg_c = READ_PERI_REG(SENS_SAR_MEAS_START2_REG);*/

  pinMode(ENV_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  

  FastLED.addLeds<WS2812, LED_PIN, GRB>(Control.leds, NUM_LEDS);
  
  /*while(!Control.bSetupComplete)
  {
  }
  WiFi.mode(WIFI_OFF);

  //Write WIFI Off in registers, so ADC2 starts working
  WRITE_PERI_REG(SENS_SAR_START_FORCE_REG, reg_a);
  WRITE_PERI_REG(SENS_SAR_READ_CTRL2_REG, reg_b);
  WRITE_PERI_REG(SENS_SAR_MEAS_START2_REG, reg_c);*/

  ConnectToWifi();

}

void loop() {
  Control.ControllerTask();

  if (WiFi.status() != WL_CONNECTED)
  {
     ConnectToWifi();
  }
  webSocket.loop();
  server.handleClient();
}

void ConnectToWifi()
{
  while(WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    WiFi.begin(ssid, password);
    delay(5000);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, %d, ...)\r\n", num, type, payload);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
                
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
        //Send the current status
        //char buffer[64];
        //sprintf(buffer,"R;%01d;%01d;%d;%d;%d", LedMode, bRandomColors, SelectedColor, Intensity, LedMode == 1 ? 0 : 1);
        //webSocket.sendTXT(num, buffer, strlen(buffer));
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\r\n", num, payload);


      //Requests for data for tabs
      if (payload[0] == 'R')
      {
        
        int val = GetIntFromPayload(payload);
        SendUpdate(val, num);
        
      }
      if (payload[0] == 'C')
      {
        char SendBuff[32];
        sprintf(SendBuff, "S;%s", Control.CurrentLedMode->ModeName);
        
        webSocket.sendTXT(num, SendBuff, strlen(SendBuff));
        delay(50);
        
        Control.bSetupComplete = true;
      }
      else if (payload[0] == '0')
      {
        Control.ChangeLedMode(0);
      }
      else if (payload[0] == '1')
      {

        int val = GetIntFromPayload(payload);
        Control.MaxVolLevel = val;
        Control.ChangeLedMode(1);

        SendUpdate(1, num);
      }
      else if (payload[0] == '2')
      {
        if (payload[1] == '0')
        {
          uint8_t *data = &payload[2];
          char Getbuffer[8];
          sprintf(Getbuffer,"%s", data);
          int val = String(Getbuffer).toInt();

          Control.BulletSpeedDelay = val;
        }
        else if (payload[1] == '1')
        {
          uint8_t *data = &payload[2];
          char Getbuffer[8];
          sprintf(Getbuffer,"%s", data);
          int val = String(Getbuffer).toInt();

          Control.BulletLength = val;
        }
        else if (payload[1] == '2')
        {
          uint8_t *data = &payload[2];
          char Getbuffer[8];
          sprintf(Getbuffer,"%s", data);
          int val = String(Getbuffer).toInt();

          Control.BulletLife = val;
        }
        /*else if (payload[1] == 3)   TODO TODO TODO TODO MaxBullets je const trenutno
        {
          uint8_t *data = &payload[2];
          char Getbuffer[8];
          sprintf(Getbuffer,"%s", data);
          int val = String(Getbuffer).toInt();

          Control.MaxBullets = val;
          SendUpdate(2, num);
        }*/
        
        SendUpdate(2, num);
        Control.ChangeLedMode(2);

      }
      else if (payload[0] == '3')
      {
        int val = GetIntFromPayload(payload);

        Control.RainbowDelay = val;
        
        SendUpdate(3, num);
        Control.ChangeLedMode(3);
      }
      else if (payload[0] == '4')
      {
        int val = GetIntFromPayload(payload);

        Control.BreathingDelay = val;
        
        SendUpdate(4, num);
        Control.ChangeLedMode(4);
      }
      else if (payload[0] == '5')
      {
        int val = GetIntFromPayload(payload);

        Control.FadeMoveDelay = val;
        
        SendUpdate(5, num);
        Control.ChangeLedMode(5);
      }
      else if (payload[0] == '6')
      {
        int val = GetIntFromPayload(payload);

        Control.MoveDelay = val;
        
        SendUpdate(6, num);
        Control.ChangeLedMode(6);
      }
      else if (payload[0] == '7')
      {
        if (payload[1] == '0')
        {
          uint8_t *data = &payload[2];
          char Getbuffer[8];
          sprintf(Getbuffer,"%s", data);
          int val = String(Getbuffer).toInt();

          Control.EventDelay = val;
        }
        else if (payload[1] == '1')
        {
          uint8_t *data = &payload[2];
          char Getbuffer[8];
          sprintf(Getbuffer,"%s", data);
          int val = String(Getbuffer).toInt();

          Control.Detect.BigJumpVal = val;
        }
        else if (payload[1] == '2')
        {
          uint8_t *data = &payload[2];
          char Getbuffer[8];
          sprintf(Getbuffer,"%s", data);
          int val = String(Getbuffer).toInt();

          Control.bJumpByAverage = (val != 0);
        }
        else if (payload[1] == '3')
        {
          uint8_t *data = &payload[2];
          char Getbuffer[8];
          sprintf(Getbuffer,"%s", data);
          int val = String(Getbuffer).toInt();

          Control.bAutoMusicMode = (val != 0);
        }
        else if (payload[1] == '4')
        {
          uint8_t *data = &payload[2];
          char Getbuffer[8];
          sprintf(Getbuffer,"%s", data);
          int val = String(Getbuffer).toInt();

          Control.bAutoNonMusicMode = (val != 0);
        }
        SendUpdate(7, num);
      }
      /*else if (payload[0] == 'P')
      {
        NewPulse();
      }

      Broadcast();*/
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      //hexdump(payload, length);

      // echo data back to browser
      //webSocket.sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

void handleRoot()
{
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void SendUpdate(int TabNum, int DeviceNum)
{
  char SendBuff[64];
  GetUpdateResponse(TabNum, SendBuff);

  webSocket.sendTXT(DeviceNum, SendBuff, strlen(SendBuff));
}


int GetIntFromPayload(uint8_t * payload)
{
  uint8_t *data = &payload[1];

  char Getbuffer[16];
  sprintf(Getbuffer,"%s", data);
  int val = String(Getbuffer).toInt();
  
  return val;
}

void GetUpdateResponse(int TabNum, char* Buff)
{
  switch(TabNum)
        {
          case 1:
            sprintf(Buff,"U;%d;%d;", TabNum, Control.MaxVolLevel);
            break;
          case 2:
            sprintf(Buff,"U;%d;%d;%d;%d;%d;", TabNum, Control.BulletSpeedDelay, Control.BulletLength, Control.BulletLife, Control.MaxBullets);
            break;
          case 3:
            sprintf(Buff,"U;%d;%d;", TabNum, Control.RainbowDelay);
            break;
          case 4:
            sprintf(Buff,"U;%d;%d;", TabNum, Control.BreathingDelay);
            break;
          case 5:
            sprintf(Buff,"U;%d;%d;", TabNum, Control.FadeMoveDelay);
            break;
          case 6:
            sprintf(Buff,"U;%d;%d;", TabNum, Control.MoveDelay);
            break;
          case 7:
            sprintf(Buff,"U;%d;%d;%d;%d;%d;%d;", TabNum, Control.EventDelay, Control.Detect.BigJumpVal, Control.bJumpByAverage, Control.bAutoMusicMode, Control.bAutoNonMusicMode);
            break;
        }
}
