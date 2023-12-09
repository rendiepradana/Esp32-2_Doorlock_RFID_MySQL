//Program ini dibuat oleh kelompok 4 
//jika ada pertanyaan lebih lanjut mengenai program
//hubungi sosial media yang tertera di profil github
//by
//1. Rendie Pradana         (21507334032)
//2. Ilham Widyo Nugroho    (21507334002)
//3. Dio Faziko Pratama     (21507334017)
//4. Riski Saputra          (21507334018)

#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>

const char* ssid = "XXXXXXXXXXX";
const char* password = "XXXXXXXX";
const char* host = "XXX.XXX.X.XXX";

#define LED_PIN 2
#define BTN_PIN 4
#define relayPin 14

//variabel RFID
#define SDA_PIN 5
#define RST_PIN 15

LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SDA_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  lcd.begin();

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.hostname("Esp32");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(LED_PIN, OUTPUT);
  pinMode(BTN_PIN, OUTPUT);
  pinMode(relayPin, OUTPUT);

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Dekatkan Kartu RFID Anda");
  Serial.println();

  lcd.setCursor(0, 0);
  lcd.print("Door Lock RFID");
  lcd.setCursor(0, 1);
  lcd.print("Kelompok 4");
  delay(5000);
}
void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tempelkan Kartu Anda");
  lcd.setCursor(0, 1);
  lcd.print("kelompok 4");
  delay(1000);

  if (digitalRead(BTN_PIN)==1) {
    //nyalakan lampu LED
    //ditekan
    digitalWrite(LED_PIN, HIGH);
    while (digitalRead(BTN_PIN) == 1) //menahan proses sampai tombol dilepas
    ;
    //ubah mode absensi di aplikasi web
    String getData, Link;
    HTTPClient http ;
    //Get Data
    Link = "http://"+String(host)+"/doorlock/ubahmode.php";
    http.begin(Link);
    
    int httpCode = http.GET();
    String payload = http.getString();

    Serial.println(payload);
    http.end();
  }

  //matikan led
  digitalWrite(LED_PIN, LOW);

  if(! mfrc522.PICC_IsNewCardPresent())
    return;
  if(! mfrc522.PICC_ReadCardSerial())
  return;

  String IDTAG = "";
  for(byte i=0; i<mfrc522.uid.size; i++)
  {
    IDTAG += mfrc522.uid.uidByte[i];
  }

  //nyalakan led
  digitalWrite(LED_PIN, HIGH);

  //kirim no kartu RFID ke tabel tmprfid
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort))
  {
    Serial.println("Connection Failed");
    return;
  }

  String Link;
  HTTPClient http;
  Link = "http://"+String(host)+"/doorlock/kirimkartu.php?nokartu=" +IDTAG;
  http.begin(Link);

  int httpCode = http.GET();
  String payload = http.getString();
  Serial.println(payload);
  http.end();


  String relayLink = "http://"+String(host)+"/doorlock/relayset.php";
  HTTPClient relayHttp;
  relayHttp.begin(relayLink);

  int relayHttpCode = relayHttp.GET();
  String relayPayload = relayHttp.getString();
  Serial.println(relayPayload);  

  if (relayPayload.toInt() == 1) {
    digitalWrite(relayPin, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kartu Terdeteksi,");
    lcd.setCursor(0, 1);
    lcd.print("Silahkan Masuk");
    delay(6000);

    digitalWrite(relayPin, LOW);
  } 

  else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Kartu Tidak Ada");
    lcd.setCursor(0, 1);
    lcd.print("Dalam Database");
    delay(3000);
  }

  relayHttp.end();  
}