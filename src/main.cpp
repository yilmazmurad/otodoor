#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

#define SS_PIN 10
#define RST_PIN 9
#define relayPin 3

MFRC522 mfrc522(SS_PIN, RST_PIN);   // MFRC522 örneği

const int maxCards = 50;            // Yetkili kartlar için maksimum dizi boyutu
const int UIDLength = 12;           // UID uzunluğu (kart UID'si 12 karakter olarak saklanacak)
int cardCount = 0;                  // Yetkili kart sayısı
String masterUID = "D3 EC 2B 28";   // Anahtar kartın UID'si
String eepromResetUID = "B2 C8 2C 1B";
bool addMode = false;               // Kart ekleme modu

unsigned long lastReadTime = 0; // Son kart okuma zamanı
const unsigned long debounceDelay = 3000; // 3 saniye

// Önceden tanımlı kart UID'leri (statik olarak EEPROM'a yazılacak)
String predefinedUIDs[] = {
  "4A 9F 4D 99",
  "3A E0 D7 99",
  "5A 92 09 99",
  "63 8B 27 2D",
  "E3 82 57 18",
  "43 35 5D 2D",
  "5A 68 8A 99",
  "5A 4F 21 99",
  "33 5C 3F 2D",
  "F3 5A 17 2D",
  "3A E2 B1 99",
  "73 36 87 13",
  "4A 1D 1F 99",
  "DA 8E 7A 99",
  "13 9D 45 2D",
  "4A 23 0B 99",
  "DA 1A F8 99",
  "3A F2 9E 99",
  "5A 36 8E 99",
  "4A D6 B3 99",
  "DA 11 39 99",
  "DA 82 B1 99",
  "DA EA 6D 99",
  "D3 E3 25 2D",
  "4A 27 F5 99",
  "DA 09 28 99",
  "83 D8 33 2D",
  "5A 1C 0B 99",
  "5A 90 E8 99",
  "DA 99 51 99",
  "F3 CC 2A 2D",
  "4A F4 24 99"
};
const int numPredefinedUIDs = sizeof(predefinedUIDs) / sizeof(predefinedUIDs[0]);


// Fonksiyon prototipleri
void addUID(String newUID);
bool checkUID(String UID);
void printAuthorizedUIDs();
void loadUIDsFromEEPROM();
void resetEEPROM();
void saveUIDToEEPROM(int index, String UID);
String readUIDFromEEPROM(int index);
void addPredefinedUIDs();


void setup() 
{
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  //Serial.begin(9600);               // Seri haberleşmeyi başlat
  SPI.begin();                      // SPI başlat
  mfrc522.PCD_Init();               // MFRC522 başlat
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);

  //Serial.println("Kartınızı okuyucuya yaklaştırın...");

  // Önceden tanımlı kartları EEPROM'a yaz
  addPredefinedUIDs();

  // EEPROM'dan kartları yükle
  cardCount = EEPROM.read(0);  // EEPROM'daki ilk byte kart sayısını tutar
  //Serial.print("EEPROM'dan kart sayısı yüklendi: ");
  //Serial.println(cardCount);
}

void loop() 
{
  // Eğer belirli bir süre (3 saniye) geçmemişse kart okuma işlemi yapma
  if (millis() - lastReadTime < debounceDelay) {
    return;
  }

  // Yeni kart aranıyor
  if (!mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  
  // Kart bilgilerini oku
  if (!mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  // Kart okunduğunda son okuma zamanını güncelle
  lastReadTime = millis();

  // Kartın UID'sini al
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  
  content.toUpperCase();
  content.trim();  // Başındaki ve sonundaki boşlukları temizle
  //Serial.print("Okunan Kart UID: ");
  //Serial.println(content);

  // Anahtar kartı okuduğumuzu kontrol et
  if (content == masterUID) 
  {
    //Serial.println("Anahtar kart okundu. Kart ekleme moduna geçildi.");
    addMode = true;  // Kart ekleme moduna geç
    digitalWrite(relayPin, LOW);
    delay(1000);
    digitalWrite(relayPin, HIGH);
    delay(1000);
    digitalWrite(relayPin, LOW);
    delay(1000);
    digitalWrite(relayPin, HIGH);
    return;
  }else if(content == eepromResetUID){
      // EEPROM'u sıfırlama
      resetEEPROM();
  }

  // Kart ekleme modundaysak ve anahtar karttan farklı bir kart okunduysa, yeni kart ekle
  if (addMode) 
  {
    //Serial.println("Yeni kart eklendi: " + content);
    addUID(content);  // Yeni kartı ekle
 
    addMode = false;  // Kart ekleme modundan çık
    delay(2000);
    return;
  }

  // Yetkili kartlar listesinde olup olmadığını kontrol et
  if (checkUID(content)) 
  {
    mfrc522.PICC_HaltA();  // Kart okutma işlemi sonrasında kartı durdurur
    //Serial.println("Yetkili erişim.");
    digitalWrite(relayPin, LOW);
    delay(2000);
    digitalWrite(relayPin, HIGH);
  } 
  else 
  {
    //Serial.println("Erişim reddedildi.");
  }
delay(1000);
}

void resetEEPROM() {
  EEPROM.write(0, 0);  // EEPROM'daki ilk bayta kart sayısı olarak 0 yaz
  //Serial.println("EEPROM sıfırlandı. Kart sayısı 0 yapıldı.");
  // Yazılım reseti
  asm volatile ("  jmp 0"); // Programın başlangıcına atla
}

// Kart UID'sini yetkili kartlar listesine ekler
void addUID(String newUID) 
{
  if (cardCount < maxCards) 
  {
    // Eğer yeni UID zaten mevcut değilse ekleyelim
    if (!checkUID(newUID)) {
      saveUIDToEEPROM(cardCount++, newUID);  // Yeni UID'yi EEPROM'a ekle
      EEPROM.write(0, cardCount);  // Yeni kart sayısını EEPROM'a kaydet
      //Serial.println("Kart başarıyla eklendi.");
    } else {
      //Serial.println("Bu kart zaten eklenmiş.");
    }
  } 
  else 
  {
    //Serial.println("Kart ekleme limiti aşıldı.");
  }
}

// Kartın yetkili olup olmadığını kontrol eder
bool checkUID(String UID) 
{
  for (int i = 0; i < cardCount; i++) 
  {
    if (readUIDFromEEPROM(i) == UID) 
    {
      return true;
    }
  }
  return false;
}

// Yetkili kartlar listesini yazdırır
void printAuthorizedUIDs() 
{
  //Serial.println("Yetkili Kartlar:");
  for (int i = 0; i < cardCount; i++) 
  {
    //Serial.println(readUIDFromEEPROM(i));  // EEPROM'daki her kartı yazdır
  }
  //Serial.println("-------------------------");
}

// EEPROM'a kart UID'si kaydet
void saveUIDToEEPROM(int index, String UID) 
{
  int start = 1 + (index * UIDLength);  // 1. bayttan başla, 0. bayt kart sayısını tutuyor
  for (int i = 0; i < UIDLength; i++) 
  {
    EEPROM.write(start + i, UID[i]);
  }
}

// EEPROM'dan kart UID'sini oku
String readUIDFromEEPROM(int index) 
{
  int start = 1 + (index * UIDLength);  // 1. bayttan başla
  char UID[UIDLength + 1];
  for (int i = 0; i < UIDLength; i++) 
  {
    UID[i] = char(EEPROM.read(start + i));
  }
  UID[UIDLength] = '\0';  // Null terminator ekle
  return String(UID);
}

// Önceden tanımlı kart UID'lerini EEPROM'a kaydeden fonksiyon
void addPredefinedUIDs() {
  for (int i = 0; i < numPredefinedUIDs; i++) {
    if (!checkUID(predefinedUIDs[i])) {
      addUID(predefinedUIDs[i]);  // Eğer EEPROM'da yoksa ekle
      //Serial.println("Önceden tanımlı kart eklendi: " + predefinedUIDs[i]);
    }
  }
}
