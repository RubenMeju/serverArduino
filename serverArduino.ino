//-----------------------------------------
//Arduino DHT11 Web Server using AJAX
//HTML code of webpage is stored on SD card
//-----------------------------------------
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <DHT.h>
#include <DS3231.h>
#include <LiquidCrystal_I2C.h>
//------------------------------------------------
DHT dht(9, DHT11);
File HMTL_file;

LiquidCrystal_I2C lcd(0x27, 16, 2);
DS3231 clock;
RTCDateTime dt;

//Led
#define led 7

//tiempos
int periodo = 1000;
unsigned long TiempoAhora1;
//unsigned long TiempoAhora2;

boolean estadoLed = false;
//------------------------------------------------
// dirección MAC.
byte mac[] = {
    0x90, 0xA2, 0xDA, 0x0D, 0xA0, 0x88};
// dirección ip
IPAddress ip(192, 168, 0, 49); // IP address, may need to change depending on network
EthernetServer server(80);
//==================================================================================
void setup()
{
  //LED
  pinMode(led, OUTPUT);

  digitalWrite(led, LOW);

  clock.begin();

  Serial.begin(9600);
  dht.begin();
  pinMode(53, OUTPUT);

  //inicializar el lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("--MEJURDINO--");
  delay(1000);
  lcd.clear();
  //-----------------------------------------------------
  Serial.println(F("Initializing SD card..."));
  if (!SD.begin(4))
  {
    Serial.println(F("Initialization failed!"));
    return;
  }
  Serial.println(F("Initialization OK"));
  //-----------------------------------------------------
  if (!SD.exists("dhtajax.htm"))
  {
    Serial.println(F("dhtajax.htm not found!"));
    return;
  }
  Serial.println(F("dhtajax.htm found"));
  //-----------------------------------------------------
  Ethernet.begin(mac, ip);
  // Verificar que el Ethernet Shield está correctamente conectado.
  if (Ethernet.hardwareStatus() == EthernetNoHardware)
  {
    Serial.println("Ethernet shield no presente :(");
    while (true)
    {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  if (Ethernet.linkStatus() == LinkOFF)
  {
    Serial.println("El cable Ethernet no está conectado o está defectuoso.");
  }

  server.begin();
  Serial.print(F("Server Started...\nLocal IP: "));
  Serial.println(Ethernet.localIP());
}
//==================================================================================

void loop()
{
  fn_fecha_lcd();

  listenForClients();
}

void listenForClients()
{
  String HTTP_req;
  EthernetClient client = server.available();
  //---------------------------------------------------------------------------
  if (client)
  {
    boolean currentLineIsBlank = true;
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        HTTP_req += c;
        if (c == '\n' && currentLineIsBlank)
        {
          client.println("HTTP/1.1 200 OK\n\rContent-Type: text/html\n\r\n\r");
          //-------------------------------------------------------------------
          if (HTTP_req.indexOf("readDHT11") > -1)
          {
            client.println("Temp&nbsp;&nbsp;&nbsp;&nbsp;: ");
            client.println(dht.readTemperature());
            client.println("°C<br>Humidity:&nbsp;&nbsp;&nbsp;");
            client.println(int(dht.readHumidity()));
            client.println("&nbsp;%");
          }

          else if (HTTP_req.indexOf("readFecha") > -1)
          {
            dt = clock.getDateTime();

            client.println("Fecha: ");
            client.println(dt.day);
            client.println("/");
            client.println(dt.month);
            client.println("/");
            client.println(dt.year);
            client.println("<br>");
            client.println("Hora:");
            client.println(dt.hour);
            client.println(":");
            client.println(dt.minute);
            client.println(":");
            client.println(dt.second);
          }
          //-------------------------------------------------------------------
          else
          {
            HMTL_file = SD.open("dhtajax.htm");
            if (HMTL_file)
            {
              while (HMTL_file.available())
                client.write(HMTL_file.read());
              HMTL_file.close();
            }
          }
          //-------------------------------------------------------------------
          //HTTP_req = "";
          break;
        }
        //---------------------------------------------------------------------
        if (c == '\n')
          currentLineIsBlank = true;
        else if (c != '\r')
          currentLineIsBlank = false;
      }
    }
    delay(10);
    client.stop();
    if (HTTP_req.indexOf("?button7on") > 0)
    {
      digitalWrite(led, HIGH);
      estadoLed = true;
      Serial.println("ENCIENDO EL LEDDD!!!!!!!!!!!!!!!!!!!!!");
    }

    if (HTTP_req.indexOf("?button7off") > 0)
    {
      digitalWrite(led, LOW);
      estadoLed = false;
      Serial.println("APAGO EL LED!!!!!!!!!!!!!!!");
    }
    HTTP_req = "";
  }
}

//fecha y hora en lcd
void fn_fecha_lcd()
{
  if (millis() > TiempoAhora1 + periodo)
  {
    TiempoAhora1 = millis();

    dt = clock.getDateTime();

    lcd.setCursor(0, 0);
    if (dt.hour < 10)
    {
      lcd.print("0");
    }
    lcd.print(dt.hour);
    lcd.print(":");
    if (dt.minute < 10)
    {
      lcd.print("0");
    }
    lcd.print(dt.minute);
    lcd.print(":");
    if (dt.second < 10)
    {
      lcd.print("0");
    }
    lcd.print(dt.second);
    lcd.setCursor(0, 1);

    lcd.print(dt.day);
    lcd.print("/");
    lcd.print(dt.month);
    lcd.print("/");
    lcd.print(dt.year);
  }
}
