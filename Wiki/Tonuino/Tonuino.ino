/* ToDo:
   #########################Open#########################
  Major: satischer analoger Input blockiert restlich Inputs, neu Bibliothek könnte ventuell helfen?

   Test: New Rotray Encoder with AiO
   Add: Startansage Puzzle/Quiz, ähnlich Rechnen lernen
   Minor Hörbuch nach ende des letzten Tracks, wenn Titel nicht auf Karte gespeichert, keine Ansage
   Minor Trackspeicher, Fehlerhafte abhandlung wenn Track nicht gespeichert und karte wieder erwartet wird.

   #########################ToDo#########################
   ---Umzug auf PlatformIO---
   Alterssperre
   Sleepmodifier, Lautstärke ausfaden lassen
   RFID Empfindlichkeit über Adminmenü
   Neopixel LED als Status einfügen
   Steuerung über RFID
   Steuerung über Serial<
   Modifierjingle + Ansagen
   Always Queue
   Jukebox Modifier: Rndom Track bis RFID aufgelegt, dann einen Track aus RFID und weiter mit Random
   Memory: Modifier karte, die mit einem Satz memory RFID Tags mehrere Motive zulässt

   #########################Done#########################
      -OK-:IR: Anlernen falsche Sprachausgaben
      -OK-:IR: Anlernen fehlerhaft, alle Buttons = pause
      -OK-:voiceMenu: NextTrack wird ausgeführt und Titel angespielt.
      -OK-:Modifier: Caluclate Spiel fehlerhaft, Eingabe nur in 10er Schritten
      -OK-:Album mit Speicher: Wenn letzter Titel erreicht, wird bei langem Druck der ShortCut, statt next Track ausgeführt
      -OK-:Trackspeicher: Bei Neuauflegen einer Karte mit Speicher und ungespeichertem Inhalt -> wenn pause: Ansage, das speicher nicht geschrieben wurde -> wenn play, kein Neustart!
      -OK-:Modifier: Konfiguration falsch, nur 10 Optionen
      -OK-:Trackspeicher: Album lesen oder schreiben der Track nummer falsch -> Gelichzeitiges Auslösen von Next/previous mit dem aktuellen shortcut, DANACH ausführen des neuen ShortCuts
      -OK-:Trackspeicher: Short Cut Fortschritt wird nicht in EEPROM geschrieben
      -OK-:Trackspeicher: Short Cut Hörbuch, bei wiederaufnehemen des Tracks, wird der folge Track gespielt
      -OK-:voiceMenu: Abbruchausgabe wird nicht gespielt -> muss getestet werden
      -OK-:voiceMenu: 1 vor/zurück nur durch langen Druck
      -OK-:voiceMenu: besseres Handling mit waitforTrackToFinish
      -OK-:AdminMenu abbruch Menu
      -OK-:voiceMenu: next Track wird ausgeführt
      -OK-:ShortCut: wird ausgeführt, trotz laufendem Track
      -OK-:ShortCut: "Oh ein neuer ShortCut" Ansage kommt nicht -> waitForTrackToFinish Abbruch durch PAuse; alle Tasten haben stat wasReleaased SHORT_PRESS erhalten
      -OK-:Short Cut: Short Cut 0 lässt keine Steuerung zu, kein next Track
      -OK-:Short Cut: Modifier entfernen falls einer aktiv ->Bug mit delete activeModifer?
      -OK-:loop: InvertVolume Button führt zu komischen verhalten, bei langem DRuck von Tasten im 3 Button modus
      -OK-:Buttons: Longpress führt auch Shortpress aus
      -OK-:Karte anlernen Album von bis, keine Speicher abfrage.
      -OK-:Major Lock & Toddler Modifier verhindert nicht das starten von ShortCuts
      -OK-:Major ButtonSmash Aktuell laufender Track wird nicht beendet und kann dann auch nicht mehr beendet werden
      -OK-:Major Repeat  Mofifer kein next Track auf befehl
      -OK-:Mainor: Kindergarten Modifier startet keine Wiedergabe, wenn im Leerlauf aktiviert.
      -OK-: Minor: Puzzel/Quiz Feedbacksound fehlerhaft
      -OK- Minor: Shortcut enable in Puzzle/Quiz
      -OK-: Minor: Queue Probleme in Quiz, Fragen werden widerholt
      -OK-: Minor: Erstellung Puzzle Karte, Teil speicher verdreht?
      -OK-: Major: Pause wenn Kart weg ohne Funktion.
      -OK-: Major: Wenn Pause wenn Karte weg aktiv, next Track und Play ohne Funktion, wenn Kart weg -> knownCard = false pause Trigger triggert pause nicht
      -OK-: Update: Rotary Encoder without Interrupts, usable with nearly all Pins
      -OK-: Update: Standard IR Remote werte ergänzt
      -OK-:Major: Quiz, bei aUflegen einer Antwort wird der Track der Antwort übernommen und die nächste Frage ist Antwort +1
      -OK-:Critical: Lautstärke auf langem Druck, wird nur um eins erhöht pro Druck
      -OK-: Major: unprogrammierter Shortcut löscht Karte
      -OK-:Minor: Default AnalogIn Werte passen nicht zur Buttonplatine
      -OK-:Minor: Puzzle & Quiz, vorheriger Abspielmodus wird nicht unterbrochen
      -OK-: Major: Satischer Analoger input, Anlernprozedur falsch. EIngang wird direkt übernommen, anstatt auf wechsel zu warten, bzw sollte stellung bestätigt werden.
      -OK-: Major: statischer analoger input, falsche Erkennung der Zustände, Bsp. Admin MEnü erst bei wechsel erkannt.


*/
#include "Configuration.h"
#include <DFMiniMp3.h>
#include <EEPROM.h>
//#include <JC_Button.h> //https://github.com/JChristensen/JC_Button
#include <AceButton.h>
#include <MFRC522.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#if not defined AiO
#include <avr/sleep.h>
#endif
#if defined ROTARY_ENCODER
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h> //https://github.com/PaulStoffregen/Encoder
#endif
#if defined IRREMOTE
#include <IRremote.h> //https://github.com/Arduino-IRremote/Arduino-IRremote
#endif
#include "Emulated_EEPROM.h"

using namespace ace_button;

/*
   _____         _____ _____ _____ _____
  |_   _|___ ___|  |  |     |   | |     |
    | | | . |   |  |  |-   -| | | |  |  |
    |_| |___|_|_|_____|_____|_|___|_____|
    TonUINO Version 2.2
    created by Thorsten Voß and licensed under GNU/GPL.
    Information and contribution at https://tonuino.de.
    Fork by Marco Schulz DEVELOPER
*/
///////// General globals ////////////////////////////////////////////////
unsigned long sleepAtMillis = 0;

///////// MFRC522 ////////////////////////////////////////////////////////
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522
MFRC522::MIFARE_Key key;
bool successRead;
uint8_t sector = 1;
uint8_t blockAddr = 4;
uint8_t trailerBlock = 7;
MFRC522::StatusCode status;
//////////////////////////////////////////////////////////////////////////

///////// card cookie ////////////////////////////////////////////////////
static const uint32_t cardCookie = 322417479;
//////////////////////////////////////////////////////////////////////////

static const uint16_t EEPROM_size = 512;
static const uint16_t EEPROM_settingsStartAdress = 1;

static const uint8_t availableShortCuts = 12; //Die Anzahl Inputs einer 3x4 Matrix oder eines 0-9,#,* Tastenfeldes
///////// setup buttons //////////////////////////////////////////////////
//Button pauseButton(buttonPause);
//Button upButton(buttonUp);
//Button downButton(buttonDown);
//#if defined FIVEBUTTONS
//Button buttonFour(buttonFourPin);
//Button buttonFive(buttonFivePin);
//#endif
AceButton pauseButton(buttonPause);
AceButton upButton(buttonUp);
AceButton downButton(buttonDown);
#if defined FIVEBUTTONS
AceButton buttonFour(buttonFourPin);
AceButton buttonFive(buttonFivePin);
#endif
//////////////////////////////////////////////////////////////////////////

typedef enum Enum_Trigger {
  NoTrigger = 0,
  PauseTrackTrigger,
  NextTrigger,
  NextPlusTenTrigger,
  PreviousTrigger,
  PreviousPlusTenTrigger,
  VolumeUpTrigger,
  VolumeDownTrigger,
  AbortTrigger,
  AdminMenuTrigger,
  ResetTrackTrigger,
  ShortcutTrigger //muss an letzter Stelle stehen bleiben!
};

struct inputTrigger {
  bool noTrigger;
  bool pauseTrack;
  bool next;
  bool nextPlusTen;
  bool previous;
  bool previousPlusTen;
  bool volumeUp;
  bool volumeDown;
  bool cancel;
  bool adminMenu;
  bool resetTrack;
  bool shortCutNo [availableShortCuts]; //muss an letzter Stelle stehen bleiben!
};

static inputTrigger myTrigger;
static inputTrigger myTriggerEnable;

static const uint8_t sizeOfInputTrigger = sizeof(inputTrigger);

#if defined AiO
const uint8_t folderMemoryCount = 3;
#else
const uint8_t folderMemoryCount = 8;
#endif

//////// IR Remote ////////////////////////////////////////////////////
#if defined IRREMOTE
const uint8_t irRemoteCodeCount = sizeOfInputTrigger;
//IRrecv IrReceiver(IRREMOTE_PIN); // create IRrecv instance
//decode_results irReading;
#endif
//////////////////////////////////////////////////////////////////////////

//////// Rotary Encoder ///////////////////////////////////////////////
#if defined ROTARY_ENCODER
Encoder myEnc(ROTARY_ENCODER_PIN_A, ROTARY_ENCODER_PIN_B);
#endif
//////////////////////////////////////////////////////////////////////////

///////// DFPlayer Mini //////////////////////////////////////////////////
SoftwareSerial mySoftwareSerial(2, 3); // RX, TX
uint8_t numTracksInFolder;
uint8_t currentTrack;
uint8_t firstTrack;
uint8_t queue[255];
uint8_t volume;
static uint8_t _lastTrackFinished;
//////////////////////////////////////////////////////////////////////////

//////// analog input /////////////////////////////////////////////////
#if defined ANALOG_INPUT
AceButton analogButtons[sizeOfInputTrigger];
AceButton* analogButton[sizeOfInputTrigger];

#endif
//#if defined ANALOG_INPUT
//int8_t analogInLastPosition = -1;
//unsigned long analogInMillis = 0;
//#endif
typedef enum analogTriggerType
{
  NoType = 0,
  Impuls,
  Permament
};
//////////////////////////////////////////////////////////////////////////

///////// Enums //////////////////////////////////////////////////////////
typedef enum Enum_PlayMode
{
  AudioDrama = 1,
  Album = 2,
  Party = 3,
  Single = 4,
  AudioBook = 5,
  AdminMenu = 6,
  AudioDrama_Section = 7,
  Album_Section = 8,
  Party_Section = 9,
  AudioBook_Section = 10,
  PuzzlePart = 11
};
typedef enum Enum_Modifier
{
  ModifierMode = 0,
  SleepTimerMod = 1,
  FreezeDanceMod = 2,
  LockedMod = 3,
  ToddlerModeMod = 4,
  KindergardenModeMod = 5,
  RepeatSingleMod = 6,
  PuzzleGameMod = 7,
  QuizGameMod = 8,
  ButtonSmashMod = 9,
  //TheQuestMod,
  CalculateMod = 10,
  AdminMenuMod = 255
};

typedef enum Enum_AdminMenuOptions
{
  Exit = 0,
  ResetCard,
  MaxVolume,
  MinVolume,
  InitVolume,
  EQ,
  SetupShortCuts,
  SetupStandbyTimer,
  CreateFolderCards,
  InvertButtons,
  StopWhenCardAway,
  SetupAnalogInputLeraning,
  SetupAnalogInputType,
  SetupIRRemote,
  ResetEEPROM,
  LockAdminMenu
};
typedef enum Enum_PCS
{
  PCS_NO_CHANGE     = 0, // no change detected since last pollCard() call
  PCS_NEW_CARD      = 1, // card with new UID detected (had no card or other card before)
  PCS_CARD_GONE     = 2, // card is not reachable anymore
  PCS_CARD_IS_BACK  = 3 // card was gone, and is now back again
};

typedef enum Enum_NfcGain
{
  minimum = 0,
  average = 1,
  maximum = 2
};
//////////////////////////////////////////////////////////////////////////

///////// this object stores nfc tag data ///////////////////////////////
struct folderSettings {
  uint8_t folder;
  uint8_t mode;
  uint8_t special; //von
  uint8_t special2; //bis
  uint8_t special3; //Trackmemory
  uint8_t special4;
};
folderSettings *myFolder;
folderSettings shortCuts[availableShortCuts];

uint8_t sizeOfFolderSettings = sizeof(folderSettings);

struct nfcTagObject {
  uint32_t cookie;
  uint8_t version;
  folderSettings nfcFolderSettings;
};
nfcTagObject myCard;
//////////////////////////////////////////////////////////////////////////

///////// admin settings stored in eeprom ///////////////////////////////
struct adminSettings {
  uint32_t cookie;
  byte version;
  uint8_t maxVolume;
  uint8_t minVolume;
  uint8_t initVolume;
  uint8_t eq;
  bool locked;
  unsigned long standbyTimer;
  bool invertVolumeButtons;
  uint8_t adminMenuLocked;
  folderSettings savedModifier;
  bool stopWhenCardAway;
  Enum_NfcGain nfcGain;
  uint8_t userAge;
  uint16_t irRemoteUserCodes[sizeOfInputTrigger];//Ein Slot pro möglichem Trigger
  uint16_t analogInUserValues[sizeOfInputTrigger]; //Ein Slot pro möglichem Trigger
  uint8_t analogInputTolerance;
  uint16_t analogInputTriggerTime;
  uint8_t analogInputTriggerType;
};
adminSettings mySettings;
uint16_t sizeOfAdminSettings = sizeof(adminSettings);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
static bool hasCard = false;
static byte lastCardUid[4];
static byte retries;
static bool lastCardWasUL;
static bool skipNext = false;
static bool knownCard = false;
static int8_t activeShortCut = -1;
static uint8_t trackToStoreOnCard = 0;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void shuffleQueue(uint8_t startTrack = firstTrack, uint8_t endTrack = numTracksInFolder);
void writeSettings();
void getSettings();
void resetShortCuts();
void writeShortCuts();
void getShortCuts();
void updateShortCutTrackMemory();
void resetSettings();
void playFolder();
void activateShortCut (uint8_t shortCutNo);
static void nextTrack(uint8_t track, bool force = false) ;
static void previousTrack() ;
bool isPlaying();
void mp3Pause(uint16_t delayTime = 100);
void waitForTrackToFinish (bool interruptByTrigger = true, uint16_t timeOut = 5000);
void waitForTrackToStart(uint16_t timeOut = 5000);
void setup();
void readButtons(AceButton *button, uint8_t eventType, uint8_t /* buttonState */);
#if defined IRREMOTE
void readIr ();
#endif
#if defined ANALOG_INPUT
void readAnalogIn(AceButton *button, uint8_t eventType, uint8_t /* buttonState */);
#endif
#if defined ROTARY_ENCODER
void RotEncSetVolume ();
#endif
void checkNoTrigger ();
void readTrigger(bool invertVolumeButtons = false);
void resetTriggerEnable();
void volumeUpAction(bool rapidFire = false);
void volumeDownAction(bool rapidFire = false);
void nextAction() ;
void previousAction();
void pauseAction();
void loop();
void adminMenu(bool fromCard = false);
uint8_t voiceMenu(int16_t numberOfOptions, uint16_t startMessage, int messageOffset,
                  bool preview = false, uint8_t previewFromFolder = 0, int defaultValue = 0,
                  bool enableSkipTen = false);
bool setupFolder(folderSettings * theFolder) ;
void setupShortCut(int8_t shortCutNumber = -1);
void setupCard();
bool readCard(nfcTagObject * nfcTag);
void resetCard() ;
void writeCard(nfcTagObject nfcTag, int8_t writeBlock = -1, bool feedback = true) ;
void dump_byte_array(byte * buffer, byte bufferSize) ;
bool setupModifier(folderSettings * tmpFolderSettings);
bool RemoveModifier();
void writeCardMemory (uint8_t track, bool announcement = false);
void checkForUnwrittenTrack  ();
byte pollCard();
Enum_PCS handleCardReader();
void onNewCard();
void setstandbyTimer();
void disablestandbyTimer();
void checkStandbyAtMillis();
void shutDown();
#if defined FADING_LED
void fadeStatusLed(bool isPlaying);
#endif
//////////////////////////////////////////////////////////////////////////

// implement a notification class,
// its member methods will get called
//
class Mp3Notify {
  public:

    static void OnError(uint16_t errorCode) {
#if defined DFPLAYER_PRINT
      switch (errorCode) {
        case DfMp3_Error_Busy: {
            Serial.print(F("busy"));
            break;
          }
        case DfMp3_Error_Sleeping: {
            Serial.print(F("sleep"));
            break;
          }
        case DfMp3_Error_SerialWrongStack: {
            Serial.print(F("serial stack"));
            break;
          }
        case DfMp3_Error_CheckSumNotMatch: {
            Serial.print(F("checksum"));
            break;
          }
        case DfMp3_Error_FileIndexOut: {
            Serial.print(F("file index"));
            break;
          }
        case DfMp3_Error_FileMismatch: {
            Serial.print(F("file mismatch"));
            break;
          }
        case DfMp3_Error_Advertise: {
            Serial.print(F("advertise"));
            break;
          }
        case DfMp3_Error_RxTimeout: {
            Serial.print(F("rx timeout"));
            break;
          }
        case DfMp3_Error_PacketSize: {
            Serial.print(F("packet size"));
            break;
          }
        case DfMp3_Error_PacketHeader: {
            Serial.print(F("packet header"));
            break;
          }
        case DfMp3_Error_PacketChecksum: {
            Serial.print(F("packet checksum"));
            break;
          }
        case DfMp3_Error_General: {
            Serial.print(F("general"));
            break;
          }
        default: {
            Serial.print(F("unknown"));
            break;
          }
      }
#endif
    }

    static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action) {
#if defined DFPLAYER_PRINT
      if (source & DfMp3_PlaySources_Sd) Serial.print("SD ");
      if (source & DfMp3_PlaySources_Usb) Serial.print("USB ");
      if (source & DfMp3_PlaySources_Flash) Serial.print("Flash ");
      Serial.println(action);
#endif

    }

    static void OnPlaySourceOnline(DfMp3_PlaySources source) {
#if defined DFPLAYER_PRINT
      PrintlnSourceAction(source, "online");
#endif

    }
    static void OnPlaySourceInserted(DfMp3_PlaySources source) {
#if defined DFPLAYER_PRINT
      PrintlnSourceAction(source, "ready");
#endif
    }
    static void OnPlaySourceRemoved(DfMp3_PlaySources source) {
#if defined DFPLAYER_PRINT
      PrintlnSourceAction(source, "removed");
#endif
    }
    static void OnPlayFinished(DfMp3_PlaySources source, uint8_t track) {
      nextTrack(track);
    }
};

static DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(mySoftwareSerial);
//////////////////////////////////////////////////////////////////////////
void shuffleQueue(uint8_t startTrack = firstTrack, uint8_t endTrack = numTracksInFolder) {

#if defined DEBUG || defined QUEUE_PRINT
  Serial.println(F("shuffleQueue"));
#endif
  // Queue für die Zufallswiedergabe erstellen
  for (uint8_t x = 0; x < endTrack - startTrack + 1; x++)
    queue[x] = x + startTrack;
  // Rest mit 0 auffüllen
  for (uint8_t x = endTrack - startTrack + 1; x < 255; x++)
    queue[x] = 0;
  // Queue mischen
  for (uint8_t i = 0; i < endTrack - startTrack + 1; i++)
  {
    uint8_t j = random (0, endTrack - startTrack + 1);
    uint8_t t = queue[i];
    queue[i] = queue[j];
    queue[j] = t;
  }
#ifdef QUEUE_PRINT
  Serial.println(F("Queue :"));
  for (uint8_t x = 0; x < endTrack - startTrack + 1 ; x++) {
    Serial.print(x + 1);
    Serial.print(". : ");
    Serial.println(queue[x]);
  }
#endif
}
//////////////////////////////////////////////////////////////////////////
void writeSettings() {

#if defined DEBUG
  Serial.print(F("writeSettings at "));
  Serial.println(EEPROM_settingsStartAdress);
#endif
#if defined AiO
  EEPROM_put(EEPROM_settingsStartAdress, mySettings);
#else
  EEPROM.put(EEPROM_settingsStartAdress, mySettings);
#endif

}

void getSettings() {
#if defined AiO
  EEPROM_get(EEPROM_settingsStartAdress, mySettings);
#else
  EEPROM.get(EEPROM_settingsStartAdress, mySettings);
#endif

  if (mySettings.version != 3 || mySettings.cookie != cardCookie) {
    resetSettings();
  }

  if (mySettings.irRemoteUserCodes[0] != 0) {
    mySettings.irRemoteUserCodes[0] = 0;
  }

#if defined DEBUG

  Serial.print(F("Version "));
  Serial.println(mySettings.version);

  Serial.print(F("Max Vol "));
  Serial.println(mySettings.maxVolume);

  Serial.print(F("Min Vol "));
  Serial.println(mySettings.minVolume);

  Serial.print(F("Init Vol "));
  Serial.println(mySettings.initVolume);

  Serial.print(F("EQ "));
  Serial.println(mySettings.eq);

  Serial.print(F("Locked "));
  Serial.println(mySettings.locked);

  Serial.print(F("Sleep Timer "));
  Serial.println(mySettings.standbyTimer);

  Serial.print(F("Inverted Buttons "));
  Serial.println(mySettings.invertVolumeButtons);

  Serial.print(F("Stop when card away "));
  Serial.println(mySettings.stopWhenCardAway);

  Serial.print(F("Admin Menu locked "));
  Serial.println(mySettings.adminMenuLocked);

  Serial.print(F("User Age "));
  Serial.println(mySettings.userAge);

  Serial.print(F("NFC Gain "));
  switch (mySettings.nfcGain) {
    case 0:
      Serial.println(F("minimum"));
      break;
    case 1:
      Serial.println(F("average"));
      break;
    case 2:
      Serial.println(F("maximum"));
      break;
  }
  Serial.print(F("Saved Modifier "));
  Serial.println(mySettings.savedModifier.mode);
#endif
#if defined ANALOG_INPUT_PRINT
  Serial.print(F("Analog input tolerance "));
  Serial.println(mySettings.analogInputTolerance);

  Serial.print(F("Analog input trigger time "));
  Serial.println(mySettings.analogInputTriggerTime);

  for (uint8_t i = 0; i < sizeOfInputTrigger; i++) {
    Serial.print(F("Analog input value "));
    Serial.print(i);
    Serial.print(F(": "));
    Serial.println(mySettings.analogInUserValues[i]);
  }
  Serial.print(F("Analog input trigger type "));
  Serial.println(mySettings.analogInputTriggerType);
#endif
#if defined IRREMOTE_PRINT
  for (uint8_t i = 0; i < sizeOfInputTrigger; i++) {
    Serial.print(F("IRRemote code "));
    Serial.print(i);
    Serial.print(F(": "));
    Serial.print(F("0x"));
    Serial.print(mySettings.irRemoteUserCodes[i] <= 0x0010 ? "0" : "");
    Serial.print(mySettings.irRemoteUserCodes[i] <= 0x0100 ? "0" : "");
    Serial.print(mySettings.irRemoteUserCodes[i] <= 0x1000 ? "0" : "");
    Serial.println(mySettings.irRemoteUserCodes[i], HEX);
  }
#endif
}

void resetSettings() {
#if defined DEBUG
  Serial.println(F("resetSettings"));
#endif
  mySettings.cookie = cardCookie;
  mySettings.version = 3;
  mySettings.maxVolume = 25;
  mySettings.minVolume = 1;
  mySettings.initVolume = 18;
  mySettings.eq = 1;
  mySettings.locked = false;
  mySettings.standbyTimer = 0;
  mySettings.invertVolumeButtons = false;
  mySettings.adminMenuLocked = 0;
  mySettings.savedModifier.folder = 0;
  mySettings.savedModifier.special = 0;
  mySettings.savedModifier.special2 = 0;
  mySettings.savedModifier.mode = 0;
  mySettings.stopWhenCardAway = false;
  mySettings.userAge = 0;
  mySettings.nfcGain = 1;

  mySettings.analogInputTolerance = 5;
  mySettings.analogInputTriggerTime = SHORT_PRESS;
  mySettings.analogInputTriggerType = NoType;

  mySettings.analogInUserValues[0] = 1023;

  for (uint8_t i = 1; i < ShortcutTrigger; i++) {
    mySettings.analogInUserValues[i] = 6000;
  }
#if defined AiO
  mySettings.analogInUserValues[ShortcutTrigger] = 10;
  mySettings.analogInUserValues[ShortcutTrigger + 1] = 500;
  mySettings.analogInUserValues[ShortcutTrigger + 2] = 950;
  mySettings.analogInUserValues[ShortcutTrigger + 3] = 1500;
  mySettings.analogInUserValues[ShortcutTrigger + 4] = 2500;
  mySettings.analogInUserValues[ShortcutTrigger + 5] = 2975;
  mySettings.analogInUserValues[ShortcutTrigger + 6] = 3375;
  mySettings.analogInUserValues[ShortcutTrigger + 7] = 3775;
  mySettings.analogInUserValues[ShortcutTrigger + 8] = 4064;
  mySettings.analogInUserValues[ShortcutTrigger + 9] = 6000;
  mySettings.analogInUserValues[ShortcutTrigger + 10] = 6000;
  mySettings.analogInUserValues[ShortcutTrigger + 11] = 6000;
#else
  mySettings.analogInUserValues[ShortcutTrigger] = 0;
  mySettings.analogInUserValues[ShortcutTrigger + 1] = 102;
  mySettings.analogInUserValues[ShortcutTrigger + 2] = 194;
  mySettings.analogInUserValues[ShortcutTrigger + 3] = 307;
  mySettings.analogInUserValues[ShortcutTrigger + 4] = 405;
  mySettings.analogInUserValues[ShortcutTrigger + 5] = 512;
  mySettings.analogInUserValues[ShortcutTrigger + 6] = 608;
  mySettings.analogInUserValues[ShortcutTrigger + 7] = 698;
  mySettings.analogInUserValues[ShortcutTrigger + 8] = 773;
  mySettings.analogInUserValues[ShortcutTrigger + 9] = 6000;
  mySettings.analogInUserValues[ShortcutTrigger + 10] = 6000;
  mySettings.analogInUserValues[ShortcutTrigger + 11] = 6000;
#endif

  mySettings.irRemoteUserCodes[NoTrigger] = 0;
  mySettings.irRemoteUserCodes[PauseTrackTrigger] = 0x1C;
  mySettings.irRemoteUserCodes[NextTrigger] = 0x5a;
  mySettings.irRemoteUserCodes[NextPlusTenTrigger] = 0;
  mySettings.irRemoteUserCodes[PreviousTrigger] = 0x8;
  mySettings.irRemoteUserCodes[PreviousPlusTenTrigger] = 0x5a;
  mySettings.irRemoteUserCodes[VolumeUpTrigger] = 0x18;
  mySettings.irRemoteUserCodes[VolumeDownTrigger] = 0;
  mySettings.irRemoteUserCodes[AbortTrigger] = 0xd;
  mySettings.irRemoteUserCodes[AdminMenuTrigger] = 0x16;
  mySettings.irRemoteUserCodes[ResetTrackTrigger] = 0;
  mySettings.irRemoteUserCodes[ShortcutTrigger] = 0x45;
  mySettings.irRemoteUserCodes[ShortcutTrigger + 1] = 0x46;
  mySettings.irRemoteUserCodes[ShortcutTrigger + 2] = 0x47;
  mySettings.irRemoteUserCodes[ShortcutTrigger + 3] = 0x44;
  mySettings.irRemoteUserCodes[ShortcutTrigger + 4] = 0x40;
  mySettings.irRemoteUserCodes[ShortcutTrigger + 5] = 0x43;
  mySettings.irRemoteUserCodes[ShortcutTrigger + 6] = 0x7;
  mySettings.irRemoteUserCodes[ShortcutTrigger + 7] = 0x15;
  mySettings.irRemoteUserCodes[ShortcutTrigger + 8] = 0x9;
  mySettings.irRemoteUserCodes[ShortcutTrigger + 9] = 0x19;
  mySettings.irRemoteUserCodes[ShortcutTrigger + 10] = 0;
  mySettings.irRemoteUserCodes[ShortcutTrigger + 11] = 0;

  writeSettings();
}
//////////////////////////////////////////////////////////////////////////
void resetShortCuts() {
#if defined DEBUG
  Serial.println(F("reset short cuts"));
#endif
  for (uint8_t i = 0; i < availableShortCuts; i++) {
    shortCuts[i].folder = 0;
    shortCuts[i].mode = 0;
    shortCuts[i].special = 0;
    shortCuts[i].special2 = 0;
    shortCuts[i].special3 = 0;
  }
  writeShortCuts();
}
void writeShortCuts() {
#if defined SHORTCUTS_PRINT
  Serial.print(F("write short cuts at "));
  Serial.println(EEPROM_settingsStartAdress + sizeOfAdminSettings);
#endif

#if defined AiO
  EEPROM_put(EEPROM_settingsStartAdress + sizeOfAdminSettings, shortCuts);
#else
  EEPROM.put(EEPROM_settingsStartAdress + sizeOfAdminSettings, shortCuts);
#endif

}
void getShortCuts() {
#if defined SHORTCUTS_PRINT
  Serial.print(F("get short cuts from "));
  Serial.println(EEPROM_settingsStartAdress + sizeOfAdminSettings);
#endif
  EEPROM_get(EEPROM_settingsStartAdress + sizeOfAdminSettings, shortCuts);

#if defined SHORTCUTS_PRINT
  for (uint8_t i = 0; i < availableShortCuts; i++) {
    Serial.print(F("short cut no "));
    Serial.println(i);

    Serial.print(F("folder: "));
    Serial.println(shortCuts[i].folder);

    Serial.print(F("mode "));
    Serial.println(shortCuts[i].mode);

    Serial.print(F("special1 "));
    Serial.println(shortCuts[i].special);

    Serial.print(F("special2 "));
    Serial.println(shortCuts[i].special2);

    Serial.print(F("special3 "));
    Serial.println(shortCuts[i].special3);
  }
#endif
}
void updateShortCutTrackMemory(uint8_t track) {
#if defined SHORTCUTS_PRINT
  Serial.print(F("update short cuts at "));
  Serial.print(EEPROM_settingsStartAdress + sizeOfAdminSettings + (sizeOfFolderSettings * activeShortCut) - 1);
  Serial.print(F(" with track "));
  Serial.println(track);
#endif
  shortCuts[activeShortCut].special3 = track;
#if defined AiO
  EEPROM_put(EEPROM_settingsStartAdress + sizeOfAdminSettings + (sizeOfFolderSettings * activeShortCut), shortCuts[activeShortCut]);
#else
  EEPROM.put(EEPROM_settingsStartAdress + sizeOfAdminSettings + (sizeOfFolderSettings * activeShortCut), shortCuts[activeShortCut]);
#endif

}
//////////////////////////////////////////////////////////////////////////
class Modifier {
  public:
    virtual void loop() {}
    virtual bool handlePause() {
      return false;
    }
    virtual bool handleNext() {
      return false;
    }
    virtual bool handlePrevious() {
      return false;
    }
    virtual bool handleNextAction() {
      return false;
    }
    virtual bool handlePreviousAction() {
      return false;
    }
    virtual bool handleVolumeUpAction() {
      return false;
    }
    virtual bool handleVolumeDownAction() {
      return false;
    }
    virtual bool handleShortCut(uint8_t shortCutNo) {
      return false;
    }
    virtual bool handleShutDown() {
      return false;
    }
    virtual bool handleRFID(nfcTagObject *newCard) {
      return false;
    }
    virtual bool handleStopWhenCardAway() {
      return false;
    }
    virtual bool handleSaveModifier() {
      return false;
    }
    virtual bool handleAdminMenu() {
      return false;
    }
    virtual uint8_t getActive() {
      return 0;
    }
    virtual void deleteModifier() {
      delete this;
    }
    Modifier() {

    }
};

Modifier *activeModifier = NULL;
/*///////////////////////////////////////////////////////////////////////////
  //// An modifier can also do somethings in addition to the modified action
  //// by returning false (not handled) at the end
  //// This simple FeedbackModifier will tell the volume before changing it and
  //// give some feedback once a RFID card is detected.
  //class FeedbackModifier: public Modifier {
  //  public:
  //    virtual bool handleRFID(nfcTagObject *newCard) {
  //#if defined DEBUG
  //      Serial.println(F("Feedback > RFID"));
  //#endif
  //      return false;
  //    }
  //    uint8_t getActive() {
  //      return FeedbackMod;
  //    }
  //};
  /////////////////////////////////////////////////////////////////////////*/
//////////////////////////////////////////////////////////////////////////
class SleepTimer: public Modifier {
  private:
    unsigned long sleepAtMillis = 0;

  public:
    void loop() {
      if (this->sleepAtMillis != 0 && millis() > this->sleepAtMillis) {
#if defined DEBUG
        Serial.println(F("SleepTimer > sleep"));
#endif
        mp3Pause();
        setstandbyTimer();
        activeModifier = NULL;
        delete this;
      }
    }

    SleepTimer(uint8_t minutes) {
#if defined DEBUG
      Serial.print(F("SleepTimer minutes: "));
      Serial.println(minutes);
#endif
      this->sleepAtMillis = millis() + minutes * 60000;
    }
    uint8_t getActive() {
      return SleepTimerMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class FreezeDance: public Modifier {
  private:
    unsigned long nextStopAtMillis = 0;
    const uint8_t minSecondsBetweenStops = 5;
    const uint8_t maxSecondsBetweenStops = 30;

    void setNextStopAtMillis() {
      uint8_t seconds = random(this->minSecondsBetweenStops, this->maxSecondsBetweenStops + 1);
#if defined DEBUG
      Serial.print(F("FreezeDance set next stop at "));
      Serial.print(seconds);
      Serial.println(F(" s."));
#endif
      this->nextStopAtMillis = millis() + seconds * 1000;
    }

  public:
    void loop() {
      if (this->nextStopAtMillis != 0 && millis() > this->nextStopAtMillis) {
#if defined DEBUG
        Serial.println(F("FreezeDance > freeze"));
#endif
        if (isPlaying()) {
          mp3.playAdvertisement(301);
          delay(500);
        }
        setNextStopAtMillis();
      }
    }
    FreezeDance(void) {
#if defined DEBUG
      Serial.println(F("FreezeDance"));
#endif
      if (isPlaying()) {
        delay(1000);
        mp3.playAdvertisement(300);
        delay(500);
      }
      setNextStopAtMillis();
    }
    uint8_t getActive() {
      return FreezeDanceMod ;
    }
};
//////////////////////////////////////////////////////////////////////////
class PuzzleGame: public Modifier {
  private:
    uint8_t PartOneSpecial = 0;
    uint8_t PartOneFolder = 0;
    uint8_t PartOneSpecial2 = 0;

    uint8_t PartTwoSpecial = 0;
    uint8_t PartTwoFolder = 0;
    uint8_t PartTwoSpecial2 = 0;

    uint8_t mode = 0;

    bool PartOneSaved = false;
    bool PartTwoSaved = false;

    void Success ()
    {
      PartOneSaved = false;
      PartTwoSaved = false;
#if defined DEBUG
      Serial.println(F("PuzzleGame > success"));
#endif

      mp3.playMp3FolderTrack(256); //Toll gemacht! Das ist richtig.
      waitForTrackToFinish();
    }

    void Failure ()
    {
      if (mode == 0) {
        PartOneSaved = false;
      }
      PartTwoSaved = false;

#if defined DEBUG
      Serial.println(F("PuzzleGame > Failure"));
#endif

      mp3.playMp3FolderTrack(257);
      waitForTrackToFinish();
    }

    void CompareParts()
    {
      if ((PartOneSpecial == PartTwoSpecial) && (PartOneFolder == PartTwoFolder) && (PartOneSpecial2 == PartTwoSpecial2)) {
        PartTwoSaved = false;
        return;
      }
      else if (((PartOneSpecial !=  PartTwoSpecial) || (PartOneFolder != PartTwoFolder)) && (PartOneSpecial2 == PartTwoSpecial2)) {
        Success();
      }
      else {
        Failure();
      }
    }

  public:
    void loop()
    {
      if (PartOneSaved && PartTwoSaved) {
        if (!isPlaying()) {
          CompareParts();
        }
      }
      if (myTrigger.resetTrack) {
#if defined DEBUG
        Serial.println(F("PuzzleGame > delete part"));
#endif
        mp3Pause();
        mp3.playMp3FolderTrack(298); //Okay, lass uns was anderes probieren.
        PartOneSaved = false;
        PartTwoSaved = false;
      }
    }

    PuzzleGame(uint8_t special)
    {
      mp3.loop();
#if defined DEBUG
      Serial.println(F("PuzzleGame"));
#endif
      mp3Pause();
      knownCard = false;
      activeShortCut = -1;

      mp3.playMp3FolderTrack(407); // intro
      waitForTrackToFinish();
      mode = special;
    }

    virtual bool handlePause() {
      return false;
    }

    virtual bool handleNext() {
      return true;
    }

    virtual bool handlePrevious() {
      return true;
    }

    virtual bool handleNextAction()       {
      return true;
    }

    virtual bool handlePreviousAction() {
      return true;
    }

    virtual bool handleShutDown() {
      return true;
    }

    virtual bool handleShortCut(uint8_t shortCutNo) {
      return true;
    }

    virtual bool handleRFID(nfcTagObject *newCard) {

      //this->tmpCard = *newCard;
      if (newCard->nfcFolderSettings.mode != PuzzlePart) {
#if defined DEBUG
        Serial.println(F("PuzzleGame > no valid part"));
#endif
        mp3Pause();
        mp3.playMp3FolderTrack(258); //Diese Karte ist nicht Teil des Spiels. Probiere eine Andere.
        return true;
      }
      else {
#if defined DEBUG
        Serial.println(F("PuzzleGame > valid part"));
#endif
        if (!PartOneSaved)
        {
          PartOneSpecial = newCard->nfcFolderSettings.special;
          PartOneFolder = newCard->nfcFolderSettings.folder;
          PartOneSpecial2 = newCard->nfcFolderSettings.special2;
          PartOneSaved = true;
        }
        else if (PartOneSaved && !PartTwoSaved) {
          PartTwoSpecial = newCard->nfcFolderSettings.special;
          PartTwoFolder = newCard->nfcFolderSettings.folder;
          PartTwoSpecial2 = newCard->nfcFolderSettings.special2;
          PartTwoSaved = true;
        }
        return false;
      }
    }
    virtual bool handleStopWhenCardAway() {
      return true;
    }
    uint8_t getActive() {
      return PuzzleGameMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class QuizGame: public Modifier {
  private:
    uint8_t PartOneSpecial = 0;
    uint8_t PartOneFolder = 0;
    uint8_t PartOneSpecial2 = 0;

    uint8_t PartTwoSpecial = 0;
    uint8_t PartTwoFolder = 0;
    uint8_t PartTwoSpecial2 = 0;

    bool PartOneSaved = false;
    bool PartTwoSaved = false;

    void Success () {
#if defined DEBUG
      Serial.println(F("QuizGame > success"));
#endif
      mp3.playMp3FolderTrack(256);
      waitForTrackToFinish();
      this->PartOneSaved = false;
      this->PartTwoSaved = false;
      next();
    }

    void Failure () {
      PartTwoSaved = false;
#if defined DEBUG
      Serial.println(F("QuizGame > failure"));
#endif
      mp3.playMp3FolderTrack(257);
      waitForTrackToFinish();
    }

    void CompareParts() {
      if ((this->PartOneSpecial == this-> PartTwoSpecial) && (this->PartOneFolder == this->PartTwoFolder) && (this->PartOneSpecial2 == this->PartTwoSpecial2)) {
        PartTwoSaved = false;
        return;
      }
      else if (((this->PartOneSpecial !=  this->PartTwoSpecial) || (this->PartOneFolder != this->PartTwoFolder)) && (this->PartOneSpecial2 == this->PartTwoSpecial2)) {
        Success();
      }
      else {
        Failure();
      }
    }

  public:
    void loop() {
      if (this->PartOneSaved && this->PartTwoSaved) {
        if (!isPlaying()) {
          CompareParts();
        }
      }

      if (myTrigger.resetTrack) {
#if defined DEBUG
        Serial.println(F("QuizGame > delete part"));
#endif
        mp3Pause();
        mp3.playMp3FolderTrack(298); //Okay, lass uns was anderes probieren.
        waitForTrackToFinish();
        this->PartOneSaved = false;
        this->PartTwoSaved = false;
        next();
      }
    }

    QuizGame(uint8_t special) {
      mp3.loop();
#if defined DEBUG
      Serial.println(F("QuizGame"));
#endif
      mp3Pause();
      knownCard = false;
      activeShortCut = -1;

      mp3.playMp3FolderTrack(408); // intro
      waitForTrackToFinish();

      this->PartOneFolder = special;
      numTracksInFolder = mp3.getFolderTrackCount(this->PartOneFolder);
      firstTrack = 1;
      shuffleQueue();
      currentTrack = 0;

#if defined DEBUG
      Serial.println(F("QuizGame > queue set"));
#endif
      next();
    }

    void  next() {
      //numTracksInFolder = mp3.getFolderTrackCount(this->PartOneFolder);

      if (currentTrack < numTracksInFolder) {
#if defined DEBUG
        Serial.println(F("QuizGame > queue next"));
#endif
        currentTrack++;
      } else {
#if defined DEBUG
        Serial.println(F("QuizGame > end of queue"));
#endif
        mp3Pause();
        mp3.playMp3FolderTrack(409);
        waitForTrackToFinish();
        RemoveModifier();
        return;
      }
      this->PartOneSaved = true;
      this->PartOneSpecial = queue[currentTrack - 1];
      this->PartOneSpecial2 = this->PartOneSpecial;
#if defined DEBUG
      Serial.print(F("Track: "));
      Serial.println(currentTrack);
      Serial.print(F("Part: "));
      Serial.println(this->PartOneSpecial);
#endif
      mp3.playFolderTrack(this->PartOneFolder, this->PartOneSpecial);
    }

    virtual bool handleNext() {
      return true;
    }

    virtual bool handlePrevious() {
      return true;
    }
    virtual bool handlePause() {
#if defined DEBUG
      Serial.println(F("QuizGame > pause and repeat "));
#endif
      if (isPlaying()) {
        mp3Pause();
      } else {
        mp3.playFolderTrack(this->PartOneFolder, this->PartOneSpecial);
      }
      return true;
    }

    virtual bool handleNextAction()       {
      return true;
    }

    virtual bool handlePreviousAction() {
      return true;
    }

    virtual bool handleShutDown() {
      return true;
    }

    virtual bool handleShortCut(uint8_t shortCutNo) {
      return true;
    }

    virtual bool handleRFID(nfcTagObject * newCard) {

      if (newCard->nfcFolderSettings.mode != PuzzlePart) {
#if defined DEBUG
        Serial.println(F("QuizGame > no valid part"));
#endif
        mp3Pause();
        mp3.playMp3FolderTrack(258); //Diese Karte ist nicht Teil des Spiels. Probiere eine Andere.
        return true;
      }
      else {
#if defined DEBUG
        Serial.println(F("QuizGame > valid part"));
#endif
        if (this->PartOneSaved && !this->PartTwoSaved) {
          this->PartTwoSpecial = newCard->nfcFolderSettings.special;
          this->PartTwoFolder = newCard->nfcFolderSettings.folder;
          this->PartTwoSpecial2 = newCard->nfcFolderSettings.special2;
          this->PartTwoSaved = true;
        }

        mp3.playFolderTrack(newCard->nfcFolderSettings.folder, newCard->nfcFolderSettings.special);
        waitForTrackToFinish();
        return true;
      }
    }
    virtual bool handleStopWhenCardAway() {
      return true;
    }
    uint8_t getActive() {
      return QuizGameMod;
    }
};
//////////////////////////////////////////////////////////////////////////
///////////WIP!!//////////////////////////////////////////////////////////
/*class TheQuest: public Modifier {
  private:
    uint8_t PartOneSpecial = 0;
    uint8_t PartOneFolder = 0;
    uint8_t PartOneSpecial2 = 0;

    bool PartOneSaved = false;

    unsigned long NextAnnouncementAtMillis = 0;
    uint8_t TimeOut = 1;
    uint8_t AnnouncementTrack = 0;
    uint8_t AnnouncementFolder = 0;

    void setNextAnnouncementAtMillis() {
  #if defined DEBUG
      Serial.print(F("TheQuest > next announcement in "));
      Serial.print(this->TimeOut);
      Serial.println(F("min"));
  #endif
      this->NextAnnouncementAtMillis = millis() + (this->TimeOut * 60000);
  #if defined DEBUG
      Serial.print(this->NextAnnouncementAtMillis);
      Serial.println(F("ms"));
  #endif
    }

    void Success () {
      this->PartOneSaved = false;
  #if defined DEBUG
      Serial.println(F("TheQuest > success"));
  #endif
      mp3.playMp3FolderTrack(297);
      waitForTrackToFinish();
      next();
    }
  public:
    void loop() {
      if (this->NextAnnouncementAtMillis != 0 && millis() > this->NextAnnouncementAtMillis) {
  #if defined DEBUG
        Serial.print(millis());
        Serial.println(F("ms"));
        Serial.print(this->NextAnnouncementAtMillis);
        Serial.println(F("ms"));
  #endif
        this->AnnouncementTrack++;
        if (this->AnnouncementTrack <= 3) {
  #if defined DEBUG
          Serial.println(F("TheQuest > Time out, play announcement"));
  #endif
          setNextAnnouncementAtMillis();
  mp3Pause();
          mp3.playFolderTrack(this->AnnouncementFolder, this->AnnouncementTrack);
          waitForTrackToFinish();
        } else {
          this->AnnouncementTrack = 0;
          next();
        }
      }

      // Abbruch der Aktuellen Questaufgabe
      if (upButton.pressedFor(LONG_PRESS) && downButton.pressedFor(LONG_PRESS)) {
        do {
          readTrigger();
        } while (myTrigger.resetTrack);
  #if defined DEBUG
        Serial.println(F("TheQuest > delete part"));
  #endif
  mp3Pause();
        mp3.playMp3FolderTrack(298); //Okay, lass uns was anderes probieren.
        waitForTrackToFinish();
        this->PartOneSaved = false;
        next();
      }
      //Steuerung über Pause Button
      //2s Druck: Wiederholen der Questaufgabe
      if (pauseButton.pressedFor(LONGER_PRESS)) {
        if (pauseButton.wasReleased()) {
  #if defined DEBUG
          Serial.println(F("QuizGame > pause and repeat "));
  #endif
  mp3Pause();
          mp3.playFolderTrack(PartOneFolder, PartOneSpecial);
        }
        //Druck <1s: Questaufgabe erfüllt
      } else if (pauseButton.wasReleased()) {
        Success();
      }
    }

    TheQuest(uint8_t special, uint8_t special2) {
      mp3.loop();
  #if defined DEBUG
      Serial.println(F("TheQuest"));
  #endif
  mp3Pause();

      this->AnnouncementFolder = special2;
      this->PartOneFolder = special;
      numTracksInFolder = mp3.getFolderTrackCount(this->PartOneFolder);
      firstTrack = 1;
      shuffleQueue();
      currentTrack = 0;

  #if defined DEBUG
      Serial.println(F("TheQuest > queue set"));
  #endif
      next();
    }

    void  next() {
      numTracksInFolder = mp3.getFolderTrackCount(this->PartOneFolder);

      if (currentTrack != numTracksInFolder - firstTrack + 1) {
  #if defined DEBUG
        Serial.println(F("TheQuest > queue next"));
  #endif
        currentTrack++;
      } else {
  #if defined DEBUG
        Serial.println(F("TheQuest > queue repeat"));
  #endif
        currentTrack = 1;
      }
      this->PartOneSaved = true;
      this->PartOneSpecial = queue[currentTrack - 1];
      this->PartOneSpecial2 = PartOneSpecial;
  #if defined DEBUG
      Serial.println(currentTrack);
      Serial.println(this->PartOneSpecial);
  #endif
      mp3.playFolderTrack(this->PartOneFolder, this->PartOneSpecial);

      setNextAnnouncementAtMillis();
    }

    virtual bool handleNext() {
      return true;
    }

    virtual bool handlePrevious() {
      return true;
    }

    virtual bool handlePause() {
      return true;
    }

    virtual bool handleNextAction() {
      if (this->TimeOut < 10) {
        this->TimeOut++;
        setNextAnnouncementAtMillis();
      }
      return true;
    }

    virtual bool handlePreviousAction() {
      if (this->TimeOut > 1) {
        this->TimeOut--;
        setNextAnnouncementAtMillis();
      }
      return true;
    }

    virtual bool handleShutDown() {
      return true;
    }

    virtual bool handleShortCut() {
      return true;
    }

    virtual bool handleRFID(nfcTagObject * newCard) {
      return true;
    }
    virtual bool handleStopWhenCardAway() {
      return true;
    }
    uint8_t getActive() {
      return TheQuestMod;
    }
  };*/
//////////////////////////////////////////////////////////////////////////
class ButtonSmash: public Modifier {
  private:
    uint8_t Folder;

    nfcTagObject tmpCard;

  public:
    void loop() {
    }

    ButtonSmash(uint8_t special, uint8_t special2) {
      mp3.loop();
#if defined DEBUG
      Serial.println(F("ButtonSmash"));
#endif

      mp3Pause();

      knownCard = false;
      activeShortCut = -1;

      mp3.setVolume(special2);
      delay(100);

      this->Folder = special;
      numTracksInFolder = mp3.getFolderTrackCount(Folder);

      firstTrack = 1;
      shuffleQueue();
      currentTrack = 0;
    }

    void  next() {
      mp3Pause();
      numTracksInFolder = mp3.getFolderTrackCount(this->Folder);

      if (currentTrack != numTracksInFolder - firstTrack + 1) {
#if defined DEBUG
        Serial.println(F("ButtonSmash > queue next"));
#endif
        currentTrack++;
      } else {
#if defined DEBUG
        Serial.println(F("ButtonSmash > repeat queue"));
#endif
        firstTrack = 1;
        shuffleQueue();
        currentTrack = 1;
      }
      mp3.playFolderTrack(this->Folder, queue[currentTrack - 1]);
      waitForTrackToFinish();
    }

    virtual bool handleNext() {
      return true;
    }

    virtual bool handlePrevious() {
      return true;
    }

    virtual bool handlePause() {
      next();
      return true;
    }
    virtual bool handleNextAction() {
      next();
      return true;
    }
    virtual bool handlePreviousAction() {
      next();
      return true;
    }
    virtual bool handleVolumeUpAction() {
      next();
      return true;
    }
    virtual bool handleVolumeDownAction() {
      next();
      return true;
    }
    virtual bool handleShortCut(uint8_t shortCutNo) {
      next();
      return true;
    }
    virtual bool handleShutDown() {
      next();
      return true;
    }
    virtual bool handleAdminMenu() {
      next();
      return true;
    }
    virtual bool handleRFID(nfcTagObject * newCard) {
      next();
      return true;
    }
    virtual bool handleStopWhenCardAway() {
      return true;
    }
    uint8_t getActive() {
      return ButtonSmashMod ;
    }
};
//////////////////////////////////////////////////////////////////////////
class Calculate: public Modifier {
  private:
    uint8_t mode = 1; // learn mode (1 = addition, 2 = subtraction, 3 = multiplication, 4 = division, 5 = mixed)
    uint8_t upperBound = 0; // max number (can not exceed 254)
    uint8_t opA = 0;
    uint8_t opB = 0;
    int16_t answer = 0;
    uint8_t answerOld = 0;
    uint8_t result = 0;
    uint8_t opr = 0; // operator, see mode (0..3)
    bool setAnswer = false;
    bool invertVolumeButtonsMemory = false;

    void nextQuestion(bool repeat = false) {
#if defined DEBUG
      Serial.println(F("Calculate > next Question"));
#endif
      this->answer = 0;
      this->answerOld = 0;

      if (!repeat) {
        if (this->mode == 5)
        {
          this->opr = random(1, 5);
        }
        else {
          this->opr = this->mode;
        }
        this->result = 0;

        switch (this->opr) {
          case 2: // subtraction
            {
              this->opA = random(2, this->upperBound);
              this->opB = random(1, this->opA);
            }
            break;
          case 3: // multiplication
            {
              this->opA = random(1, this->upperBound);
              this->opB = random(1, floor(this->upperBound / this->opA));

            }
            break;
          case 4: // division
            {
              this->opA = random(2, this->upperBound);
              do {
                this->opB = random(1, this->upperBound);
              } while (this->opA % this->opB  != 0);
            }
            break;
          default: // addition
            {
              this->opA = random(1, this->upperBound - 1);
              this->opB = random(1, this->upperBound - this->opA);
            }
            break;
        }
      }

      switch (this->opr) {
        case 1:
          {
            this->result = this->opA + this->opB;
          }
          break;
        case 2:
          {
            this->result = this->opA - this->opB;
          }
          break;
        case 3:
          {
            this->result = this->opA * this->opB;
          }
          break;
        case 4:
          {
            this->result = this->opA / this->opB;
          }
          break;
      }
#if defined DEBUG
      Serial.print(F("Calculate > correct result: "));
      Serial.println(this->result);
#endif
      mp3.playMp3FolderTrack(411); // question "how much is"
      waitForTrackToFinish();
      mp3.playMp3FolderTrack(this->opA); // 1..254
      waitForTrackToFinish();
      mp3.playMp3FolderTrack(411 + this->opr); // 402, 403, 404, 405
      waitForTrackToFinish();
      mp3.playMp3FolderTrack(this->opB); // 1..254
      waitForTrackToFinish();
    }

    void readAnswer() {
      if (this->answer != this->answerOld) {
        this->answerOld = this->answer;
        mp3.playMp3FolderTrack(this->answer);
        waitForTrackToStart();
      }
    }

  public:
    void loop() {
      this->readAnswer();
      if (this->answer == -1) {
        this->answer = 0;
        this->answerOld = 0;
        mp3.playMp3FolderTrack(298); // next Question
        waitForTrackToFinish();
        this->nextQuestion();
        return;
      } else if (this->answer >= 1 && this->setAnswer) {
        setAnswer = false;
        if (this->result == this->answer) {
#if defined DEBUG
          Serial.println(F("Calculate > right"));
#endif

          mp3.playMp3FolderTrack(420); // richtig
          waitForTrackToFinish();
          this->nextQuestion();
        }
        else {
#if defined DEBUG
          Serial.println(F("Calculate > wrong"));
#endif

          mp3.playMp3FolderTrack(421); // falsch
          waitForTrackToFinish();
          this->nextQuestion(true); // repeat question
        }
      }
      return;
    }
    virtual bool handlePause() {
      if (this->answer != 0) {
        this->setAnswer = true;
#if defined DEBUG
        Serial.print(F("= "));
        Serial.println(this->answer);
#endif
        mp3Pause();
      }
      return true;
    }
    virtual bool handleNext() {
      return true;
    }
    virtual bool handlePrevious() {
      return true;
    }
    virtual bool handleNextAction() {
      this->answer ++;
      this->answer = min(this->answer, this->upperBound) ;
      return true;
    }
    virtual bool handlePreviousAction() {
      this->answer --;
      this->answer = max(this->answer, 1);
      return true;
    }
    virtual bool handleShortCut(uint8_t shortCutNo) {
      return true;
    }
    virtual bool handleVolumeUpAction() {
      this->answer += 10;
      this->answer = min(this->answer, this->upperBound);
      return true;
    }
    virtual bool handleVolumeDownAction() {
      this->answer -= 10;
      this->answer = max(this->answer, 1);
      return true;
    }
    virtual bool handleShutDown() {
      this->answer = -1;
      return true;
    }
    virtual bool handleRFID(nfcTagObject *newCard) {
      return true;
    }
    virtual bool handleAdminMenu() {
      return true;
    }
    Calculate(uint8_t special, uint8_t special2) {
#if defined DEBUG
      Serial.println(F("Calculate"));
#endif
      //delay(150); //Laufzeit Advert Modifier Jingle, nötig, damit laufender Track mit der folgenden Pause auch pausiert wird
      mp3.loop();
      knownCard = false;
      mp3Pause();
      this->invertVolumeButtonsMemory = mySettings.invertVolumeButtons;
      mySettings.invertVolumeButtons = false;
      this->mode = special;
      this->upperBound = special2;

      if (this->mode == 5) {
        this->opr = random(1, 5);
      }
      else {
        this->opr = this->mode;
      }
      //waitForTrackToFinish();
      mp3.playMp3FolderTrack(410); // intro
      waitForTrackToFinish();
      this->nextQuestion();
    }

    uint8_t getActive() {
      return CalculateMod;
    }
    virtual void deleteModifier() {
      mySettings.invertVolumeButtons = this->invertVolumeButtonsMemory;
      delete this;
    }
};
//////////////////////////////////////////////////////////////////////////
class Locked: public Modifier {
  public:
    virtual bool handlePause()     {
      return true;
    }
    virtual bool handleNextAction()       {
      return true;
    }
    virtual bool handlePreviousAction() {
      return true;
    }
    virtual bool handleVolumeUpAction() {
      return true;
    }
    virtual bool handleVolumeDownAction() {
      return true;
    }
    virtual bool handleShutDown() {
      return true;
    }
    virtual bool handleShortCut(uint8_t shortCutNo) {
      return true;
    }
    virtual bool handleAdminMenu() {
      return true;
    }

    virtual bool handleRFID(nfcTagObject *newCard) {
      return true;
    }
    Locked(void) {
#if defined DEBUG
      Serial.println(F("Locked Moifier"));
#endif
    }
    uint8_t getActive() {
      return LockedMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class ToddlerMode: public Modifier {
  public:
    virtual bool handlePause()     {
      return true;
    }
    virtual bool handleNextAction()       {
      return true;
    }
    virtual bool handlePreviousAction() {
      return true;
    }
    virtual bool handleVolumeUpAction() {
      return true;
    }
    virtual bool handleVolumeDownAction() {
      return true;
    }
    virtual bool handleShutDown() {
      return true;
    }
    //    virtual bool handleShortCut(uint8_t shortCutNo) {
    //      return true;
    //    }
    virtual bool handleAdminMenu() {
      return true;
    }

    ToddlerMode() {
#if defined DEBUG
      Serial.println(F("ToddlerMode"));
#endif
    }

    uint8_t getActive() {
      return ToddlerModeMod ;
    }
};
//////////////////////////////////////////////////////////////////////////
class KindergardenMode: public Modifier {
  private:
    nfcTagObject nextCard;
    bool cardQueued = false;

  public:
    virtual bool handleNext() {
#if defined DEBUG
      Serial.println(F("KindergardenMode > next"));
#endif
      if (this->cardQueued == true) {
        this->cardQueued = false;

        myCard = nextCard;
        *myFolder = myCard.nfcFolderSettings;
#if defined DEBUG
        Serial.println(myFolder->folder);
        Serial.println(myFolder->mode);
#endif
        playFolder();
        return true;
      }
      return false;
    }
    virtual bool handleNextAction()       {
      return true;
    }
    virtual bool handlePreviousAction() {
      return true;
    }
    virtual bool handleShutDown() {
      return true;
    }
    virtual bool handleAdminMenu() {
      return true;
    }

    virtual bool handleShortCut(uint8_t shortCutNo) {
      if (isPlaying())
        return true;
      else
        return false;
    }

    virtual bool handleRFID(nfcTagObject * newCard) { // lot of work to do!
#if defined DEBUG
      Serial.println(F("KindergardenMode > RFID queued"));
#endif
      if (knownCard) {
        this->nextCard = *newCard;
        this->cardQueued = true;
        if (!isPlaying()) {
          handleNext();
        }
      } else {
        myFolder = &newCard->nfcFolderSettings;
#if defined DEBUG
        Serial.println(myFolder->folder);
        Serial.println(myFolder->mode);
#endif
        knownCard = true;
        playFolder();
      }
      return true;
    }
    KindergardenMode() {
#if defined DEBUG
      Serial.println(F("KindergardenMode"));
#endif
    }
    uint8_t getActive() {
      return KindergardenModeMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class RepeatSingleModifier: public Modifier {
  public:
    virtual bool handleNext() {
#if defined DEBUG
      Serial.println(F("RepeatSingle > repeat track"));
#endif
      delay(50);
      if (isPlaying())
        return true;
      if (myFolder->mode == Party || myFolder->mode == Party_Section) {
        mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
      }
      else {
        mp3.playFolderTrack(myFolder->folder, currentTrack);
      }

      _lastTrackFinished = 0;
      return true;
    }
    RepeatSingleModifier() {
#if defined DEBUG
      Serial.println(F("RepeatSingle"));
#endif
    }
    uint8_t getActive() {
      return RepeatSingleMod;
    }
    virtual bool handleNextAction() {
      nextTrack(random(65536), true);
      return false;
    }
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void playFolder() {
  uint8_t counter = 0;
  bool queueTrack = false;

  mp3.loop();
  disablestandbyTimer();
  _lastTrackFinished = 0;
  numTracksInFolder = mp3.getFolderTrackCount(myFolder->folder);
  firstTrack = 1;


  switch (myFolder->mode) {
    case AudioDrama:
#if defined DEBUG
      Serial.println(F("Audio Drama"));
#endif
      currentTrack = random(1, numTracksInFolder + 1);
      break;
    case AudioDrama_Section:
#if defined DEBUG
      Serial.println(F("Audio Drama section"));
      Serial.print(myFolder->special);
      Serial.print(F(" to "));
      Serial.println(myFolder->special2);
#endif
      firstTrack = myFolder->special;
      currentTrack = random(myFolder->special, myFolder->special2 + 1);

      break;
    case Album:
#if defined DEBUG
      Serial.println(F("Album"));
#endif
      if (myFolder->special3 > 0) {
        if (myFolder->special3 < numTracksInFolder) {
          currentTrack = myFolder->special3 + 1;
        } else {
          currentTrack = 1;
        }
        writeCardMemory(currentTrack);
      } else {
        currentTrack = 1;
      }

      break;
    case Album_Section:
#if defined DEBUG
      Serial.println(F("Album section"));
      Serial.print(myFolder->special);
      Serial.print(F(" to "));
      Serial.println(myFolder->special2);
#endif
      firstTrack = myFolder->special;
      if (myFolder->special3 > 0) {
        if (myFolder->special3 < myFolder->special2) {
          currentTrack = myFolder->special3 + 1;
        } else {
          currentTrack = myFolder->special;
        }
        writeCardMemory(currentTrack);
      } else {
        currentTrack = myFolder->special;
      }

      break;
    case Party:
#if defined DEBUG
      Serial.println(F("Party"));
#endif
      currentTrack = 1;
      shuffleQueue();
      queueTrack = true;
      break;
    case Party_Section:
#if defined DEBUG
      Serial.println(F("Party section"));
#endif
      currentTrack = 1;
      firstTrack = myFolder->special;
      shuffleQueue(firstTrack, myFolder->special2);
      queueTrack = true;

      break;
    case AudioBook:
#if defined DEBUG
      Serial.println(F("Audio Book"));
#endif
      currentTrack = myFolder->special3;
      if (currentTrack == 0 || currentTrack > numTracksInFolder) {
        currentTrack = 1;
      }
      break;
    case AudioBook_Section:
#if defined DEBUG
      Serial.println(F("Audio Book section"));
#endif
      firstTrack = myFolder->special;
      currentTrack = myFolder->special3;
      if (currentTrack < firstTrack || currentTrack >= firstTrack + numTracksInFolder) {
        currentTrack = firstTrack;
#if defined DEBUG
        Serial.println(F("not correct tag, start from beginn"));
#endif
      }

      break;
    case Single:
#if defined DEBUG
      Serial.println(F("Single"));
#endif
      currentTrack = myFolder->special;

      break;
    case PuzzlePart:
#ifdef DEBUG
      Serial.println(F("Puzzle Part"));
#endif
      currentTrack = myFolder->special;
      break;
  }

#ifdef DEBUG
  Serial.print(numTracksInFolder);
  Serial.print(F(" tracks in folder "));
  Serial.println(myFolder->folder);
  Serial.print(F("play track: "));
#endif

  if (queueTrack) {
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
#ifdef DEBUG
    Serial.println(queue[currentTrack - 1]);
#endif
  }
  else {
    mp3.playFolderTrack(myFolder->folder, currentTrack);
#ifdef DEBUG
    Serial.println(currentTrack);
#endif
  }

  waitForTrackToStart();

}

void activateShortCut (uint8_t shortCutNo) {
  trackToStoreOnCard = 0;
#if defined DEBUG || defined SHORTCUTS_PRINT
  Serial.print("play short cut no ");
  Serial.println(shortCutNo);
#endif
  if (shortCuts[shortCutNo].mode != 0) {
    knownCard = false;
    switch (shortCuts[shortCutNo].folder) {
      case ModifierMode:
        SetModifier (&shortCuts[shortCutNo]);
        break;
      default:
        //        if (activeModifier != NULL) {
        //          if (activeModifier->handleShortCut(shortCutNo) == true) {
        //#if defined DEBUG ^ defined SHORTCUTS_PRINT
        //            Serial.println(F("locked"));
        //#endif
        //            return;
        //          }
        //        }
        myFolder = &shortCuts[shortCutNo];
        activeShortCut = shortCutNo;
        playFolder();
        break;
    }
  } /*else {
    mp3Pause();
    mp3.playMp3FolderTrack(297);
    waitForTrackToFinish();
    setupShortCut(shortCutNo);
  }*/
}
//////////////////////////////////////////////////////////////////////////
// Leider kann das Modul selbst keine Queue abspielen, daher müssen wir selbst die Queue verwalten
//////////////////////////////////////////////////////////////////////////
static void nextTrack(uint8_t track, bool force = false) {
  bool queueTrack = false;

#ifdef DEBUG
  Serial.println(F("next track"));
#endif
  if (!force && (track == _lastTrackFinished || (knownCard == false && activeShortCut < 0))) {
#ifdef DEBUG
    Serial.println(F("abort"));
#endif
    return;
  }
  _lastTrackFinished = track;

  if (activeModifier != NULL && !force) {
    if (activeModifier->handleNext() == true) {
#ifdef DEBUG
      Serial.println(F("locked"));
#endif
      return;
    }
  }

  disablestandbyTimer();

  switch (myFolder->mode) {
    //case AudioDrama:
    //case AudioDrama_Section:
    //if (myTrigger.next) { //nextTrack bei Hörspielen nur bei externem Trigger
    //currentTrack = random(1, numTracksInFolder + 1);
    //}
    //break;
    case Album:
      if (currentTrack < numTracksInFolder) {
        currentTrack = currentTrack + 1;
        if (myFolder->special3 > 0 && currentTrack != 0) {
          writeCardMemory (currentTrack);
        }
      } else {
        return;
      }
      break;
    case Album_Section:
      if (currentTrack < myFolder->special2) {
        currentTrack = currentTrack + 1;
        if (myFolder->special3 > 0 && currentTrack != 0) {
          writeCardMemory (currentTrack);
        }
      } else {
        return;
      }
      break;

    case Party:
      if (currentTrack < numTracksInFolder) {
        currentTrack = currentTrack + 1;
      }
      else {
        currentTrack = 1;
      }
      queueTrack = true;
      break;
    case Party_Section:
      if (currentTrack < (myFolder->special2 - firstTrack + 1)) {
        currentTrack = currentTrack + 1;
      }
      else {
        currentTrack = 1;
      }
      queueTrack = true;
      break;

    case AudioBook:
      if (currentTrack < numTracksInFolder) {
        currentTrack = currentTrack + 1;
        writeCardMemory (currentTrack);
      }
      else {
        writeCardMemory (firstTrack, !isPlaying());
        return;
      }

      break;
    case AudioBook_Section:
      if (currentTrack < myFolder->special2) {
        currentTrack = currentTrack + 1;
        writeCardMemory (currentTrack);
      }
      else {
        writeCardMemory (firstTrack, !isPlaying());
        return;
      }
      break;

    default:
#ifdef DEBUG
      Serial.println(F("no next Track"));
#endif
      setstandbyTimer();
      //delay(500);
      return;
      break;
  }
#if defined DEBUG
  Serial.print("track: ");
#endif
  if (queueTrack) {
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
#if defined DEBUG
    Serial.println(queue[currentTrack - 1]);
#endif
  }
  else {
    mp3.playFolderTrack(myFolder->folder, currentTrack);
#if defined DEBUG
    Serial.println(currentTrack);
#endif
  }
}

static void previousTrack() {
  bool queueTrack = false;

#ifdef DEBUG
  Serial.println(F("previous track"));
#endif

  if (knownCard == false && activeShortCut < 0) {
#ifdef DEBUG
    Serial.println(F("abort"));
#endif
    return;
  }

  if (activeModifier != NULL) {
    if (activeModifier->handlePrevious() == true) {
#ifdef DEBUG
      Serial.println(F("locked"));
#endif
      return;
    }
  }

  disablestandbyTimer();
  _lastTrackFinished = 0;

  switch (myFolder->mode) {
    case Album:
      if (currentTrack > 1) {
        currentTrack = currentTrack - 1;
        if (myFolder->special3 > 0 && currentTrack != 0) {
          writeCardMemory (currentTrack);
        }
      } else {
        return;
      }

      break;
    case Album_Section:
      if (currentTrack > firstTrack) {
        currentTrack = currentTrack - 1;
        if (myFolder->special3 > 0 && currentTrack != 0) {
          writeCardMemory (currentTrack);
        }
      } else {
        return;
      }

      break;

    case Party:
      if (currentTrack > 1) {
        currentTrack = currentTrack - 1;
      }
      else {
        currentTrack = numTracksInFolder;
      }
      queueTrack = true;
      break;
    case Party_Section:
      if (currentTrack > 1) {
        currentTrack = currentTrack - 1;
      }
      else {
        currentTrack = myFolder->special2 - firstTrack + 1;
      }
      queueTrack = true;
      break;

    case AudioBook:
      if (currentTrack > 1) {
        currentTrack = currentTrack - 1;
      } else {
        currentTrack = 1;
      }
      // Fortschritt im EEPROM abspeichern
      writeCardMemory (currentTrack);
      break;

    case AudioBook_Section:
      if (currentTrack > firstTrack) {
        currentTrack = currentTrack - 1;

      } else {
        currentTrack = firstTrack;
      }
      // Fortschritt im EEPROM abspeichern
      writeCardMemory (currentTrack);
      break;

    default:
#if defined DEBUG
      Serial.println(F("no previous Track"));
#endif
      setstandbyTimer();
      return;
      break;
  }

#if defined DEBUG
  Serial.print("track: ");
#endif

  if (queueTrack) {
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
#if defined DEBUG
    Serial.println(queue[currentTrack - 1]);
#endif
  }
  else {
    mp3.playFolderTrack(myFolder->folder, currentTrack);
#if defined DEBUG
    Serial.println(currentTrack);
#endif
  }
}
//////////////////////////////////////////////////////////////////////////
bool isPlaying() {
  return !digitalRead(busyPin);
}

void mp3Pause(uint16_t delayTime = 100) {
  mp3.pause();
  delay(delayTime);
}

void waitForTrackToFinish (bool interruptByTrigger = true, uint16_t timeOut = 5000) {
#if defined DFPLAYER_PRINT
  Serial.println(F("waitForTrackToFinish"));
#endif
  waitForTrackToStart(timeOut);
  while (isPlaying()) {
    mp3.loop();
    readTrigger();
    if (myTrigger.pauseTrack && myTriggerEnable.pauseTrack) {
      myTriggerEnable.pauseTrack = false;
#if defined DFPLAYER_PRINT
      Serial.println(F("abort wait"));
#endif
      mp3Pause();
      break;
    }

  }

#if defined DFPLAYER_PRINT
  Serial.println(F("track finish"));
#endif
}

void waitForTrackToStart(uint16_t timeOut = 5000) {
#if defined DFPLAYER_PRINT
  Serial.println(F("waitForTrackToStart"));
#endif
  unsigned long waitForTrackToStartMillis = millis();
  delay(100);
  while (!isPlaying()) {
    mp3.loop();
    if (millis() - waitForTrackToStartMillis >= timeOut) {
#ifdef DFPLAYER_PRINT
      Serial.println(F("track not starting")) ;
#endif
      break;
    }
  }
#if defined DFPLAYER_PRINT
  Serial.println(F("track startet"));
#endif
}
//////////////////////////////////////////////////////////////////////////
void setup() {
#if defined AiO
  // spannung einschalten
  pinMode(shutdownPin, OUTPUT);
  digitalWrite(shutdownPin, HIGH);

  // sd karten zugang aus
  pinMode(A5, OUTPUT);
  digitalWrite(A5, LOW);
#else
  pinMode(shutdownPin, OUTPUT);
  digitalWrite(shutdownPin, LOW);
#endif

#if defined AiO
  // verstärker aus
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH);
  delay(100);
#elif defined SPEAKER_SWITCH
  pinMode(SpeakerOnPin, OUTPUT);
  digitalWrite(SpeakerOnPin, LOW);
#endif

#if defined DEBUG || defined ANALOG_INPUT_PRINT || defined ROTARY_ENCODER_PRINT || defined SHORTCUTS_PRINT || defined QUEUE_PRINT || defined DFPLAYER_PRINT || defined IRREMOTE_PRINT
  Serial.begin(115200); // Es gibt ein paar Debug Ausgaben über die serielle Schnittstelle
  // Dieser Hinweis darf nicht entfernt werden
  //  Serial.println(F("\n _____         _____ _____ _____ _____"));
  //  Serial.println(F("|_   _|___ ___|  |  |     |   | |     |"));
  //  Serial.println(F(" | | | . |   |  |  |-   -| | | |  |  |"));
  //  Serial.println(F(" |_| |___|_|_|_____|_____|_|___|_____|\n"));

  Serial.print(F("TonUINO Version "));
  Serial.println(VERSION_TONUINO);
#if defined AiO
  Serial.println(F("AiO"));
#endif
  Serial.println(F("created by Thorsten Voß and licensed under GNU/GPL."));
  Serial.println(F("Information and contribution at https://tonuino.de.\n"));
  Serial.println(F("Fork by Marco Schulz DEVELOP"));
#endif

#if defined AiO
  analogReference(INTERNAL4V096);
  analogReadResolution(12);
#else
  analogReference(DEFAULT);
#endif

  // Busy Pin
  pinMode(busyPin, INPUT);
  mp3.begin();


  // Wert für randomSeed() erzeugen durch das mehrfache Sammeln von rauschenden LSBs eines offenen Analogeingangs
  uint32_t ADC_LSB;
  uint32_t ADCSeed;
  for (uint8_t i = 0; i < 128; i++) {
    ADC_LSB = analogRead(openAnalogPin) & 0x1;
    ADCSeed ^= ADC_LSB << (i % 32);
  }
  randomSeed(ADCSeed); // Zufallsgenerator initialisieren

#if defined ROTARY_ENCODER_PIN_SUPPLY && defined ROTARY_ENCODER
  pinMode(ROTARY_ENCODER_PIN_SUPPLY, OUTPUT);
  digitalWrite(ROTARY_ENCODER_PIN_SUPPLY, HIGH);
#endif

#if defined IRREMOTE
  IrReceiver.begin(IRREMOTE_PIN, DISABLE_LED_FEEDBACK);
#endif

#if defined POWER_ON_LED
  pinMode(PowerOnLEDPin, OUTPUT);
  digitalWrite(PowerOnLEDPin, LOW);
#endif

  // activate standby timer
  setstandbyTimer();


  // NFC Leser initialisieren
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
#if defined DEBUG
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader
#endif
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

#if defined NFCgain_min
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_min);
#endif
#if defined NFCgain_avg
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_avg);
#endif
#if defined NFCgain_max
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
#endif

  pinMode(buttonPause, INPUT_PULLUP);
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
#if defined FIVEBUTTONS
  pinMode(buttonFourPin, INPUT_PULLUP);
  pinMode(buttonFivePin, INPUT_PULLUP);
#endif

  for (uint8_t i = 0; i <= sizeOfInputTrigger; i++) {
    analogButtons[i].init(i);
    analogButton[i] = &analogButtons[i];
  }

  LadderButtonConfig analogButtons(ANALOG_INPUT_PIN, sizeOfInputTrigger, mySettings.analogInUserValues, sizeOfInputTrigger, analogButton);

  ButtonConfig* buttonConfig = ButtonConfig::getSystemButtonConfig();
  buttonConfig->setEventHandler(readButtons);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setClickDelay(LONGER_PRESS);
  buttonConfig->setFeature(ButtonConfig::kFeatureLongPress);
  buttonConfig->setLongPressDelay(LONG_PRESS);

#if defined ANALOG_INPUT
  pinMode(ANALOG_INPUT_PIN, INPUT);
  analogButtons.setEventHandler(readAnalogIn);
  analogButtons.setFeature(LadderButtonConfig::kFeatureClick);
  analogButtons.setClickDelay(1000);
  analogButtons.setFeature(ButtonConfig::kFeatureLongPress);
  analogButtons.setLongPressDelay(LONGEST_PRESS);
#endif

  //  pinMode(buttonPause, INPUT_PULLUP);
  //  pinMode(buttonUp, INPUT_PULLUP);
  //  pinMode(buttonDown, INPUT_PULLUP);
  //#if defined FIVEBUTTONS
  //  pinMode(buttonFourPin, INPUT_PULLUP);
  //  pinMode(buttonFivePin, INPUT_PULLUP);
  //#endif

  resetTriggerEnable();

  // RESET --- ALLE DREI KNÖPFE BEIM STARTEN GEDRÜCKT HALTEN > alle EINSTELLUNGEN werden gelöscht
  if (digitalRead(buttonPause) == LOW && digitalRead(buttonUp) == LOW &&
      digitalRead(buttonDown) == LOW) {
    Serial.println(F("delete EEPROM"));
#if defined AiO
    for (uint16_t i = 0; i < EEPROM_size; i++) {
      EEPROM_update(i, 0);
    }
#else
    for (uint16_t i = 0; i < EEPROM.length(); i++) {
      EEPROM.update(i, 0);
    }
#endif
    resetSettings();
    resetShortCuts();
  }
  // load settings & sort cuts from EEPROM
  getSettings();
  getShortCuts();

#if defined AiO
  // verstärker an
  digitalWrite(8, LOW);
  delay(500);
#elif defined SPEAKER_SWITCH
  digitalWrite(SpeakerOnPin, HIGH);
  delay(500);
#endif

#if defined POWER_ON_LED
  digitalWrite(PowerOnLEDPin, HIGH);
#endif

  delay(2000);
  mp3.loop();
  volume = mySettings.initVolume;
  mp3.setVolume(volume);
  mp3.setEq(mySettings.eq - 1);

  mp3.playMp3FolderTrack(264);
  waitForTrackToFinish();

  // Set saved Modifier
  if (mySettings.savedModifier.mode != 0)
    SetModifier(&mySettings.savedModifier);
}
//////////////////////////////////////////////////////////////////////////
void readButtons(AceButton *button, uint8_t eventType, uint8_t /* buttonState */) { /*(bool invertVolumeButtons = false)*/

  switch (eventType) {
    case AceButton::kEventClicked: {
#if defined FIVEBUTTONS
        myTrigger.pauseTrack  |= (button->getPin() == pauseButton) ;
        myTrigger.volumeUp    |= (button->getPin() == upButton) ;
        myTrigger.volumeDown  |= (button->getPin() == downButton) ;
        myTrigger.next        |= (button->getPin() == buttonFour) ;
        myTrigger.previous    |= (button->getPin() == buttonFive) ;
#else

        break;
      }
    case AceButton::kEventLongPressed: {
#if defined FIVEBUTTONS
        myTrigger.shortCutNo[0] |= (button->getPin() == buttonFour) ;
        myTrigger.shortCutNo[1] |= (button->getPin() == buttonFive) ;
#else
        break;
      }
  }


  /* pauseButton.read();
    upButton.read();
    downButton.read();
  #if defined FIVEBUTTONS
    buttonFour.read();
    buttonFive.read();
  #endif
    myTrigger.cancel |= pauseButton.pressedFor(LONGER_PRESS) &&
                       !upButton.isPressed() && !downButton.isPressed();
    myTrigger.pauseTrack |= pauseButton.wasReleased() && myTriggerEnable.cancel;
  #if defined FIVEBUTTONS
    myTrigger.volumeUp |= upButton.wasReleased();
    myTrigger.volumeDown |= downButton.wasReleased();
    myTrigger.shortCutNo[0] |= buttonFour.pressedFor(LONG_PRESS);
    myTrigger.shortCutNo[1] |= buttonFive.pressedFor(LONG_PRESS);
    myTrigger.next |= buttonFour.wasReleased() && myTriggerEnable.shortCutNo[0];
    myTrigger.previous |= buttonFive.wasReleased() && myTriggerEnable.shortCutNo[1];
    myTrigger.nextPlusTen |= myTrigger.shortCutNo[0];
    myTrigger.previousPlusTen |= myTrigger.shortCutNo[1];
    //  }
  #else
    if (invertVolumeButtons) {
     //3 Buttons invertiert
  #if not defined ROTARY_ENCODER
     myTrigger.volumeUp |= upButton.wasReleased() && myTriggerEnable.next && myTriggerEnable.shortCutNo[0];
     myTrigger.volumeDown |= downButton.wasReleased() && myTriggerEnable.previous && myTriggerEnable.shortCutNo[1];
  #endif
     myTrigger.next |= upButton.pressedFor(LONG_PRESS) && myTriggerEnable.shortCutNo[0];
     myTrigger.previous |= downButton.pressedFor(LONG_PRESS) && myTriggerEnable.shortCutNo[1];
     myTrigger.shortCutNo[0] |= upButton.pressedFor(LONG_PRESS);
     myTrigger.shortCutNo[1] |= downButton.pressedFor(LONG_PRESS);
     myTrigger.nextPlusTen |= upButton.pressedFor(LONG_PRESS);
     myTrigger.previousPlusTen |= downButton.pressedFor(LONG_PRESS);
    } else {
     //3 Buttons nicht invertiert
  #if not defined ROTARY_ENCODER
     myTrigger.volumeUp |= upButton.pressedFor(LONG_PRESS); //&& myTriggerEnable.shortCutNo[0];
     myTrigger.volumeDown |= downButton.pressedFor(LONG_PRESS); //&& myTriggerEnable.shortCutNo[1];
  #endif
     myTrigger.next |= upButton.wasReleased() && myTriggerEnable.volumeUp && myTriggerEnable.shortCutNo[0];
     myTrigger.previous |= downButton.wasReleased() && myTriggerEnable.volumeDown && myTriggerEnable.shortCutNo[1];
     myTrigger.shortCutNo[0] |= upButton.pressedFor(LONG_PRESS);
     myTrigger.shortCutNo[1] |= downButton.pressedFor(LONG_PRESS);
     myTrigger.nextPlusTen |= upButton.pressedFor(LONG_PRESS);
     myTrigger.previousPlusTen |= downButton.pressedFor(LONG_PRESS);
    }
  #endif

    myTrigger.adminMenu |=  (pauseButton.pressedFor(LONG_PRESS) || upButton.pressedFor(LONG_PRESS) || downButton.pressedFor(LONG_PRESS)) &&
                           pauseButton.isPressed() && upButton.isPressed() && downButton.isPressed();
    myTrigger.resetTrack |= (upButton.pressedFor(LONGER_PRESS) || downButton.pressedFor(LONGER_PRESS)) && !pauseButton.isPressed() && upButton.isPressed() && downButton.isPressed();
  */
}

#if defined IRREMOTE
void readIr () {
  Enum_Trigger irRemoteEvent = NoTrigger;
  uint16_t irRemoteCode = 0;
  static unsigned long irRemoteOldMillis;

  if (IrReceiver.decode()) {
#if defined IRREMOTE_PRINT
    IrReceiver.printIRResultShort(&Serial);
#endif
    for (uint8_t i = 0; i < irRemoteCodeCount; i++) {
      //if we have a match, temporally populate irRemoteEvent and break
      if (IrReceiver.decodedIRData.command == mySettings.irRemoteUserCodes[i]) {
        irRemoteEvent = i;
        break;
      }
    }
    // if the inner loop had a match, populate inputEvent and break
    // ir remote key presses are debounced by 250ms
    if (irRemoteEvent != 0 && millis() - irRemoteOldMillis >= 250) {
#if defined IRREMOTE_PRINT
      Serial.print(F("IR debunced remote event: "));
      Serial.println(irRemoteEvent);
#endif
      irRemoteOldMillis = millis();
      switch (irRemoteEvent) {
        case PauseTrackTrigger:
          myTrigger.pauseTrack |= true;
          break;
        case NextTrigger:
          myTrigger.next |= true;
          break;
        case NextPlusTenTrigger:
          myTrigger.nextPlusTen |= true;
          break;
        case PreviousTrigger:
          myTrigger.previous |= true;
          break;
        case PreviousPlusTenTrigger:
          myTrigger.previousPlusTen |= true;
          break;
        case VolumeUpTrigger:
          myTrigger.volumeUp |= true;
          break;
        case VolumeDownTrigger:
          myTrigger.volumeDown |= true;
          break;
        case AbortTrigger:
          myTrigger.cancel |= true;
          break;
        case AdminMenuTrigger:
          myTrigger.adminMenu |= true;
          break;
        case ResetTrackTrigger:
          myTrigger.resetTrack |= true;
          break;
        default:
          if (irRemoteEvent >= ShortcutTrigger) {
            myTrigger.shortCutNo[irRemoteEvent - ShortcutTrigger] |= true;
          }
          break;
      }
    }
    IrReceiver.resume();
  }
}
#endif

#if defined ANALOG_INPUT
void readAnalogIn(AceButton *button, uint8_t eventType, uint8_t /* buttonState */) {





  /*uint16_t currentAnalogValue = analogRead(ANALOG_INPUT_PIN) ;
    int8_t analogInCurrentPosition = -1;
    uint8_t toleranceValue;
    int16_t valueMax;
    int16_t valueMin;

    //Abfrage der Position zu diesem Wert
    for (uint8_t i = 0; i < sizeOfInputTrigger; i++) {
    toleranceValue = (mySettings.analogInUserValues[i] * mySettings.analogInputTolerance) / 100;
    toleranceValue = max(toleranceValue, 1);
    valueMax = mySettings.analogInUserValues[i] + toleranceValue;
    valueMax = max(valueMax, mySettings.analogInputTolerance);
    valueMin = mySettings.analogInUserValues[i] - toleranceValue;
    if (valueMin <= mySettings.analogInputTolerance) {
      valueMin = 0;
    }
    if (currentAnalogValue >=  valueMin && currentAnalogValue <= valueMax) {
      //analoger Wert in Tabelle hinterlegt
      analogInCurrentPosition = i;
      break;
    }
    }

    if ((millis() - analogInMillis >= mySettings.analogInputTriggerTime) && (analogInLastPosition == analogInCurrentPosition)) {
    analogInLastPosition = analogInCurrentPosition; //aktuelle aktivierte Position speichern. Falls ein Drehschalter verwendet wird soll die Position nicht nach jedem Abaluf der Triggertime aktiviert werden.
    switch (analogInCurrentPosition) {
      case PauseTrackTrigger:
        if (mySettings.analogInputTriggerType == Impuls) {
          myTrigger.pauseTrack |= true;
        }
        break;
      case NextTrigger:
        if (mySettings.analogInputTriggerType == Impuls) {
          myTrigger.next |= true;
        }
        break;
      case NextPlusTenTrigger:
        if (mySettings.analogInputTriggerType == Impuls) {
          myTrigger.nextPlusTen |= true;
        }
        break;
      case PreviousTrigger:
        if (mySettings.analogInputTriggerType == Impuls) {
          myTrigger.previous |= true;
        }
        break;
      case PreviousPlusTenTrigger:
        if (mySettings.analogInputTriggerType == Impuls) {
          myTrigger.previousPlusTen |= true;
        }
        break;
      case VolumeUpTrigger:
        if (mySettings.analogInputTriggerType == Impuls) {
          myTrigger.volumeUp |= true;
        }
        break;
      case VolumeDownTrigger:
        if (mySettings.analogInputTriggerType == Impuls) {
          myTrigger.volumeDown |= true;
        }
        break;
      case AbortTrigger:
        if (mySettings.analogInputTriggerType == Impuls) {
          myTrigger.cancel |= true;
        }
        break;
      case AdminMenuTrigger:
        if (mySettings.analogInputTriggerType == Impuls) {
          myTrigger.adminMenu |= true;
        }
        break;
      case ResetTrackTrigger:
        if (mySettings.analogInputTriggerType == Impuls) {
          myTrigger.resetTrack |= true;
        }
        break;
      default:
        if (analogInCurrentPosition >= ShortcutTrigger) {
          myTrigger.shortCutNo[analogInCurrentPosition - ShortcutTrigger] |= true;
        }
        break;
    }
    } else if ( analogInCurrentPosition != analogInLastPosition) {
    analogInLastPosition = analogInCurrentPosition;
  #if defined ANALOG_INPUT_PRINT
    Serial.print(F("new analog position: "));
    Serial.println(analogInCurrentPosition);
    Serial.print(F("analog value: "));
    Serial.println(currentAnalogValue);
  #endif
    //Wenn aktueller analoger Wert +/- des Toleranzwertes vom letzten erkannten Wert abweicht
    if (mySettings.analogInputTriggerTime > 0) {
      analogInMillis = millis(); //Rücksetzen der Triggerzeit
    }
    }
  */
}
#endif

#if defined ROTARY_ENCODER
int8_t currentPosition  = 0;

void RotEncSetVolume () {
  int8_t newPosition = myEnc.read() / ROTARY_ENCODER_STEPS;

  if (newPosition != currentPosition) {
    if (newPosition > currentPosition) {
#if defined ROTARY_ENCODER
      Serial.println(F("encoder direction clockwise"));
#endif
      myTrigger.volumeUp |= true;
    } else if (newPosition < currentPosition) {
#if defined ROTARY_ENCODER
      Serial.println(F("encoder direction conterclockwise"));
#endif
      myTrigger.volumeDown |= true;
    }
    currentPosition = 0;
    myEnc.write(currentPosition);
  }
}
#endif

void readTrigger(bool invertVolumeButtons = false) {
  resetTrigger();

  readButtons(invertVolumeButtons);

#if defined IRREMOTE
  readIr();
#endif

#if defined ANALOG_INPUT
  readAnalogIn();
#endif

#if defined ROTARY_ENCODER
  RotEncSetVolume();
#endif

  checkNoTrigger ();

  if (myTrigger.noTrigger && myTriggerEnable.noTrigger) {
    resetTriggerEnable();
  } else {
    myTriggerEnable.noTrigger = true;
  }
}

void checkNoTrigger () {
  myTrigger.noTrigger = !(myTrigger.pauseTrack || myTrigger.next || myTrigger.nextPlusTen || myTrigger.previous || myTrigger.previousPlusTen
                          || myTrigger.volumeUp || myTrigger.volumeDown || myTrigger.pauseTrack || myTrigger.cancel || myTrigger.adminMenu || myTrigger.resetTrack);

  for (uint8_t i = 0; i < availableShortCuts; i++) {
    myTrigger.noTrigger &= !myTrigger.shortCutNo[i];
  }
}

void resetTrigger() {
  myTrigger.noTrigger = true;
  myTrigger.pauseTrack = false;
  myTrigger.next = false;
  myTrigger.nextPlusTen = false;
  myTrigger.previous = false;
  myTrigger.previousPlusTen = false;
  myTrigger.volumeUp = false;
  myTrigger.volumeDown = false;
  for (uint8_t i = 0; i < availableShortCuts; i++) {
    myTrigger.shortCutNo[i] = false;
  }
  myTrigger.cancel = false;
  myTrigger.adminMenu = false;
  myTrigger.resetTrack = false;
}

void resetTriggerEnable() {
  myTriggerEnable.noTrigger = false;
  myTriggerEnable.pauseTrack = true;
  myTriggerEnable.next = true;
  myTriggerEnable.nextPlusTen = true;
  myTriggerEnable.previous = true;
  myTriggerEnable.previousPlusTen = true;
  myTriggerEnable.volumeUp = true;
  myTriggerEnable.volumeDown = true;
  for (uint8_t i = 0; i < availableShortCuts; i++) {
    myTriggerEnable.shortCutNo[i] = true;
  }
  myTriggerEnable.cancel = true;
  myTriggerEnable.adminMenu = true;
  myTriggerEnable.resetTrack = true;
}
//////////////////////////////////////////////////////////////////////////
void volumeUpAction(bool rapidFire = false) {
  //Wenn Volume auf kurzem Tastendruck bei 3 Tasten liegt, kann sonst kein next/previous ausgefürt werden
  if (/*mySettings.invertVolumeButtons == 1 && */myTriggerEnable.volumeUp == true || rapidFire) {
    myTriggerEnable.volumeUp = false  || rapidFire;
    if (activeModifier != NULL) {
      if (activeModifier->handleVolumeUpAction() == true) {
#if defined DEBUG
        Serial.println(F("volume action locked"));
#endif
        return;
      }
    }
    if (isPlaying()) {
      if (volume < mySettings.maxVolume) {
        mp3.increaseVolume();
        volume++;
        if (rapidFire) {
          delay(75);
        }
      }
#if defined DEBUG
      Serial.print(F("volume up: "));
      Serial.println(volume);
#endif
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void volumeDownAction(bool rapidFire = false) {
  //Wenn Volume auf kurzem Tastendruck bei 3 Tasten liegt, kann sonst kein next/previous ausgefürt werden
  if (/*mySettings.invertVolumeButtons == 1 && */myTriggerEnable.volumeDown == true || rapidFire) {
    myTriggerEnable.volumeDown = false  || rapidFire;
    if (activeModifier != NULL) {
      if (activeModifier->handleVolumeDownAction() == true) {
#if defined DEBUG
        Serial.println(F("volume action locked"));
#endif
        return;
      }
    }
    if (isPlaying()) {
      if (volume > mySettings.minVolume && volume > 1) {
        mp3.decreaseVolume();
        volume--;
        if (rapidFire) {
          delay(75);
        }
      }
#if defined DEBUG
      Serial.print(F("volume down; "));
      Serial.println(volume);
#endif
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void nextAction() {
  if (myTriggerEnable.next == true) {
    myTriggerEnable.next = false;
    if (activeModifier != NULL) {
      if (activeModifier->handleNextAction() == true) {
#if defined DEBUG
        Serial.println(F("next action locked"));
#endif
        return;
      }
    }
    if (isPlaying()) {
      nextTrack(random(65536));
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void previousAction() {
  if (myTriggerEnable.previous == true) {
    myTriggerEnable.previous = false;
    if (activeModifier != NULL) {
      if (activeModifier->handlePreviousAction() == true) {
#if defined DEBUG
        Serial.println(F("previous action locked"));
#endif
        return;
      }
    }
    if (isPlaying()) {
      previousTrack();
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void pauseAction() {
  if (myTriggerEnable.pauseTrack == true) {
    myTriggerEnable.pauseTrack = false;
    if (activeModifier != NULL) {
      if (activeModifier->handlePause() == true) {
#if defined DEBUG
        Serial.println(F("pause action locked"));
#endif
        return;
      }
    }
    if (isPlaying()) {
#if defined DEBUG
      Serial.println(F("pause"));
#endif
      checkForUnwrittenTrack ();
      mp3Pause();
      setstandbyTimer();
    } else if (knownCard || activeShortCut >= 0) {
#if defined DEBUG
      Serial.println(F("play"));
#endif
      mp3.start();
      disablestandbyTimer();
    }
  }
}

//////////////////////////////////////////////////////////////////////////
void cancelAction() {
  if (myTriggerEnable.cancel == true) {
    myTriggerEnable.cancel = false;
    if (activeModifier != NULL) {
      if (activeModifier->handleShutDown() == true) {
#if defined DEBUG
        Serial.println(F("shut down action locked"));
#endif
        return;
      }
    }
    shutDown();
  }
}
//////////////////////////////////////////////////////////////////////////
void shortCutAction(uint8_t shortCutNo) {
  if (myTriggerEnable.shortCutNo[shortCutNo] == true) {
    myTriggerEnable.shortCutNo[shortCutNo] = false;
    if (activeModifier != NULL) {
      if (activeModifier->handleShortCut(shortCutNo) == true) {
#if defined DEBUG ^ defined SHORTCUTS_PRINT
        Serial.println(F("short cut action locked"));
#endif
        return;
      }
    }
    if (!isPlaying()) {
      activateShortCut(shortCutNo);
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void adminMenuAction() {
  if (myTriggerEnable.adminMenu == true) {
    myTriggerEnable.adminMenu = false;
    if (activeModifier != NULL) {
      if (activeModifier->handleAdminMenu() == true) {
#if defined DEBUG
        Serial.println(F("admin menu action locked"));
#endif
        return;
      }
    }
    adminMenu();
  }
}
//////////////////////////////////////////////////////////////////////////
void loop () {
  checkStandbyAtMillis();
  mp3.loop();

#if defined FADING_LED
  fadeStatusLed(isPlaying());
#endif

  readTrigger(mySettings.invertVolumeButtons);

  if (activeModifier != NULL) {
    activeModifier->loop();
  }

  // admin menu
  if (myTrigger.adminMenu) {
    myTriggerEnable.adminMenu = false;
    mp3Pause();
    do {
      readTrigger();
      if (myTrigger.noTrigger && myTriggerEnable.noTrigger) {
        myTriggerEnable.noTrigger = false;
        break;
      }

    } while (true);
    adminMenuAction();
  }

  //Springe zum ersten Titel zurück
  if (myTrigger.resetTrack && myTriggerEnable.resetTrack) {
    myTriggerEnable.resetTrack = false;
    mp3Pause();
    do {
      readTrigger();
      if (myTrigger.noTrigger && myTriggerEnable.noTrigger) {
        myTriggerEnable.noTrigger = false;
        break;
      }

    } while (true);
#if defined DEBUG
    Serial.println(F("back to first track"));
#endif
    if (currentTrack != firstTrack) {
      currentTrack = firstTrack;
      if (myFolder->special3 != 0) {
#if defined DEBUG
        Serial.println(F("reset memory"));
#endif
        writeCardMemory (currentTrack);
      }
    }
    playFolder();
  }

  if (myTrigger.pauseTrack) {
    pauseAction();
  }

  else if (myTrigger.cancel) {
    cancelAction();
  }

  if (myTrigger.volumeUp) {
#if defined FIVEBUTTONS || defined ROTARY_ENCODER
    volumeUpAction(false);
#else
    volumeUpAction(!mySettings.invertVolumeButtons);
#endif
  }

  if (myTrigger.volumeDown) {
#if defined FIVEBUTTONS || defined ROTARY_ENCODER
    volumeDownAction(false);
#else
    volumeDownAction(!mySettings.invertVolumeButtons);
#endif
  }

  if (myTrigger.next) {
    nextAction();
  }

  if (myTrigger.previous) {
    previousAction();
  }

  for (uint8_t i = 0; i < availableShortCuts; i++) {
    if (myTrigger.shortCutNo[i]) {
      shortCutAction(i);
    }
  }

  handleCardReader();

}
//////////////////////////////////////////////////////////////////////////
void adminMenu(bool fromCard = false) {
  uint8_t subMenu = 0;
#if defined DEBUG
  Serial.println(F("adminMenu"));
#endif
  knownCard = false;
  memset(lastCardUid, 0, sizeof(lastCardUid));
  activeShortCut = -1;

  mp3Pause();
  disablestandbyTimer();

  if (fromCard == false) {
    // Admin menu has been locked - it still can be trigged via admin card
    if (mySettings.adminMenuLocked == 1) {
      return;
    }
  }

  do {
    subMenu = voiceMenu(15, 900, 900, false, false, 0);
    if (subMenu == Exit) {
      writeSettings();
      setstandbyTimer();
      return;
    }
    else if (subMenu == ResetCard) {
      resetCard();
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
    else if (subMenu == MaxVolume) {
      // Maximum Volume
      mySettings.maxVolume = voiceMenu(30 - mySettings.minVolume, 930, mySettings.minVolume, false, false, mySettings.maxVolume - mySettings.minVolume) + mySettings.minVolume;
    }
    else if (subMenu == MinVolume) {
      // Minimum Volume
      mySettings.minVolume = voiceMenu(mySettings.maxVolume - 1, 931, 0, false, false, mySettings.minVolume);
    }
    else if (subMenu == InitVolume) {
      // Initial Volume
      mySettings.initVolume = voiceMenu(mySettings.maxVolume - mySettings.minVolume + 1, 932, mySettings.minVolume - 1, false, false, mySettings.initVolume - mySettings.minVolume + 1) + mySettings.minVolume - 1;
    }
    else if (subMenu == EQ) {
      // EQ
      mySettings.eq = voiceMenu(6, 920, 920);
      mp3.setEq(mySettings.eq - 1);
    }
    else if (subMenu == SetupShortCuts) {
      setupShortCut();
    }
    else if (subMenu == SetupStandbyTimer) {
      switch (voiceMenu(5, 960, 960)) {
        case 1: mySettings.standbyTimer = 5; break;
        case 2: mySettings.standbyTimer = 15; break;
        case 3: mySettings.standbyTimer = 30; break;
        case 4: mySettings.standbyTimer = 60; break;
        case 5: mySettings.standbyTimer = 0; break;
      }
    }
    else if (subMenu == CreateFolderCards) {
      // Create Cards for Folder
      // Ordner abfragen
      nfcTagObject tempCard;
      tempCard.cookie = cardCookie;
      tempCard.version = 1;
      tempCard.nfcFolderSettings.mode = Single;
      tempCard.nfcFolderSettings.folder = voiceMenu(99, 301, 0, true);
      uint8_t special = voiceMenu(mp3.getFolderTrackCount(tempCard.nfcFolderSettings.folder), 321, 0,
                                  true, tempCard.nfcFolderSettings.folder, true);
      uint8_t special2 = voiceMenu(mp3.getFolderTrackCount(tempCard.nfcFolderSettings.folder), 322, 0,
                                   true, tempCard.nfcFolderSettings.folder, special, true);

      mp3.playMp3FolderTrack(936);
      waitForTrackToFinish();
      for (uint8_t x = special; x <= special2; x++) {
        mp3.playMp3FolderTrack(x);
        tempCard.nfcFolderSettings.special = x;
#if defined DEBUG
        Serial.print(x);
        Serial.println(F(" place Tag"));
#endif
        do {
          readTrigger();
          if (myTrigger.cancel) {
#if defined DEBUG
            Serial.println(F("abort!"));
#endif
            myTriggerEnable.cancel = false;
            mp3.playMp3FolderTrack(802);
            return;
          }
        } while (!mfrc522.PICC_IsNewCardPresent());

        // RFID Karte wurde aufgelegt
        if (mfrc522.PICC_ReadCardSerial()) {
#if defined DEBUG
          Serial.println(F("write Tag"));
#endif
          writeCard(tempCard);
          delay(50);
          mfrc522.PICC_HaltA();
          mfrc522.PCD_StopCrypto1();
          waitForTrackToFinish();
        }
      }
    }
    else if (subMenu == InvertButtons) {
      // Invert Functions for Up/Down Buttons
#if defined FIVEBUTTONS || defined ROTARY_ENCODER
      mp3.playMp3FolderTrack(992);
      waitForTrackToFinish();
#else
      switch (voiceMenu(2, 989, 989, false)) {
        case 1:
          mySettings.invertVolumeButtons = false;
          break;
        case 2:
          mySettings.invertVolumeButtons = true;
          break;
      }
#endif
    }
    else if (subMenu == StopWhenCardAway) {
      switch (voiceMenu(2, 937, 933, false)) {
        case 1:
          mySettings.stopWhenCardAway = false;
          break;
        case 2:
          mySettings.stopWhenCardAway = true;
          break;
      }
    }
    else if (subMenu == SetupAnalogInputLeraning) {
#if defined ANALOG_INPUT
#if defined ANALOG_INPUT_PRINT
      Serial.println(F("analog input learning"));
#endif
      if (voiceMenu(2, 826, 933, false, false, 0) == 2) {
#if defined ANALOG_INPUT_PRINT
        Serial.println(F("analog input reset settings"));
#endif
        for (uint8_t i = 0; i < sizeOfInputTrigger; i++) {
          mySettings.analogInUserValues[i] = 6000;
        }
        mp3.playMp3FolderTrack(999);
        waitForTrackToFinish();
      }
      mp3Pause();
      mp3.playMp3FolderTrack(828);
      waitForTrackToFinish();

      bool exitWhile = true;
      bool neutralSet = false;
      uint8_t triggerNo = 0;
      uint32_t currentAnalogValue = 0;
      uint16_t newAnalogValue = 0;
      uint16_t upperTolerance = 0;
      uint16_t lowerTolerance = 0;
      static const uint8_t minValue = mySettings.analogInputTolerance;
      static const uint8_t analogValuesCount = 15;
      unsigned long millisTriggerTime = 0;

      do {
        mp3.loop();
        if (mySettings.analogInputTriggerType == Impuls) {
          triggerNo = voiceMenu((sizeOfInputTrigger - availableShortCuts) + 1, 834, 834, false, false, 0);
        } else {
          triggerNo = (sizeOfInputTrigger - availableShortCuts) + 1;
        }
        if (triggerNo == 0) {
          exitWhile = false;
          break;
        } else if (triggerNo >= (sizeOfInputTrigger - availableShortCuts) + 1) {
          triggerNo += voiceMenu(availableShortCuts, 940, 0) - 1;
          if (triggerNo < availableShortCuts) {
            exitWhile = false;
            break;
          }
        }
#if defined ANALOG_INPUT_PRINT
        Serial.print(F("analog input trigger no: "));
        Serial.println(triggerNo);
#endif
        mySettings.analogInUserValues[triggerNo] = 6000;

        if (!neutralSet) {
          currentAnalogValue = analogRead(ANALOG_INPUT_PIN);
          //Mittelwert des aktuellen Leerlaufs bilden
          for (uint8_t j = 0; j < analogValuesCount ; j++) {
            currentAnalogValue += analogRead(ANALOG_INPUT_PIN);
          }
          currentAnalogValue = currentAnalogValue / analogValuesCount + 1;
#if defined AiO
          currentAnalogValue = min (currentAnalogValue, 4064);
#else
          currentAnalogValue = min (currentAnalogValue, 1023);
#endif

        }
        if (currentAnalogValue != 0) {
          upperTolerance = ( currentAnalogValue + (( currentAnalogValue * mySettings.analogInputTolerance) / 100));
          upperTolerance = max(upperTolerance, mySettings.analogInputTolerance);
          lowerTolerance = ( currentAnalogValue - (( currentAnalogValue * mySettings.analogInputTolerance) / 100));
          if (lowerTolerance <= minValue) {
            lowerTolerance = 0;
          }
        } else {
          upperTolerance = minValue;
          lowerTolerance = 0;
        }

        if (!neutralSet && mySettings.analogInputTriggerType == Impuls) {
          mySettings.analogInUserValues[0] = currentAnalogValue;
          neutralSet = true;
        }

#if defined ANALOG_INPUT_PRINT
        Serial.print(F("analog input compare value: "));
        Serial.println(currentAnalogValue);
        Serial.print(F("lower tolerance: "));
        Serial.println(lowerTolerance);
        Serial.print(F("upper tolerance:"));
        Serial.println(upperTolerance);
#endif
        mp3Pause();
        mp3.playMp3FolderTrack(829);
        waitForTrackToStart();

#if defined ANALOG_INPUT_PRINT
        Serial.println(F("analog input wait for user..."));
#endif
        millisTriggerTime = millis();
        while (millis() - millisTriggerTime <= mySettings.analogInputTriggerTime/*currentAnalogValue >= lowerTolerance && currentAnalogValue <= upperTolerance*/) {
          currentAnalogValue = analogRead(ANALOG_INPUT_PIN);
          currentAnalogValue = max(currentAnalogValue, minValue);
          if (currentAnalogValue >= lowerTolerance && currentAnalogValue <= upperTolerance) {
            millisTriggerTime = millis();
          }
          readTrigger();
          if (myTrigger.cancel) {
            myTriggerEnable.cancel = false;
            break;
          }
        };

        if (!myTrigger.cancel) {
#if defined ANALOG_INPUT_PRINT
          Serial.print(F("analog input: "));
          Serial.print(currentAnalogValue);
          Serial.println(F(" detected, measure average value"));
          Serial.println(F("!!hold the analog input!!"));
#endif
          currentAnalogValue = 0;
          //Mittelwert des neuen Werts bilden
          for (uint8_t j = 0; j < analogValuesCount ; j++) {
            currentAnalogValue += analogRead(ANALOG_INPUT_PIN);
          }
          newAnalogValue = currentAnalogValue / analogValuesCount;


          mp3Pause();

          if (mySettings.analogInputTriggerType != Permament) {
            mp3.playMp3FolderTrack(830);
            waitForTrackToStart();

#if defined ANALOG_INPUT_PRINT
            Serial.println(F("analog input wait for release..."));
#endif
            while (currentAnalogValue < lowerTolerance || currentAnalogValue > upperTolerance) {
              readTrigger();
              currentAnalogValue = analogRead(ANALOG_INPUT_PIN);
              currentAnalogValue = max(currentAnalogValue, minValue);
              if (myTrigger.cancel) {
                myTriggerEnable.cancel = false;
                break;
              }

            };
#if defined ANALOG_INPUT_PRINT
            Serial.println(F("analog input released"));
#endif
          }
          if (!myTrigger.cancel) {
            mySettings.analogInUserValues[triggerNo] = newAnalogValue;

#if defined ANALOG_INPUT_PRINT
            Serial.print(F("analog input new value: "));
            Serial.println(mySettings.analogInUserValues[triggerNo]);
#endif
            mp3Pause();
            mp3.playMp3FolderTrack(400);
            waitForTrackToFinish();
          }
        }

      } while (exitWhile);

#if defined ANALOG_INPUT_PRINT
      Serial.println(F("analog input end learning"));
#endif

#else
      mp3.playMp3FolderTrack(827);
      waitForTrackToFinish();
#endif
    }
    else if (subMenu == SetupAnalogInputType) {
#if defined ANALOG_INPUT
      mp3Pause();
      mp3.playMp3FolderTrack(847);
      waitForTrackToFinish();

      mySettings.analogInputTriggerType = voiceMenu(2, 848, 848);
      switch (mySettings.analogInputTriggerType) {
        case NoType:
          break;
        case Impuls:
          mySettings.analogInputTriggerTime = SHORT_PRESS;
          break;
        case Permament:
          mySettings.analogInputTriggerTime = LONGER_PRESS;
          break;
      }
#else
      mp3.playMp3FolderTrack(827);
      waitForTrackToFinish();
#endif
    }
    else if (subMenu == SetupIRRemote) {
#if defined IRREMOTE
#if defined IRREMOTE_PRINT
      Serial.println(F("IRRemote start learning"));
#endif
      if (voiceMenu(2, 826, 933, false, false, 0) == 2) {
#if defined IRREMOTE_PRINT
        Serial.println(F("IRRemote reset settings"));
#endif
        for (uint8_t i = 0; i < sizeOfInputTrigger; i++) {
          mySettings.irRemoteUserCodes[i] = 0;
        }
        mp3.playMp3FolderTrack(999);
        waitForTrackToFinish();
      }
      mp3Pause();
      mp3.playMp3FolderTrack(832);
      waitForTrackToFinish();
      bool exitWhile = true;
      uint8_t triggerNo = 0;
      do {
        mp3.loop();
        triggerNo = voiceMenu((sizeOfInputTrigger - availableShortCuts) + 1, 834, 834, false, false, 0);
        if (triggerNo == 0) {
          exitWhile = false;
          break;
        } else if (triggerNo >= (sizeOfInputTrigger - availableShortCuts) + 1) {
          triggerNo += voiceMenu(availableShortCuts, 940, 0) - 1;
          if (triggerNo < availableShortCuts) {
            exitWhile = false;
            break;
          }
        }
#if defined IRREMOTE_PRINT
        Serial.print(F("IRRemote set trigger no.: "));
        Serial.println(triggerNo);
#endif
        mp3Pause();
        mp3.playMp3FolderTrack(833);
        // clear ir receive buffer
        IrReceiver.resume();
        // wait for ir signal
        while (!IrReceiver.decode()) {
        }
        mp3Pause();

#if defined IRREMOTE_PRINT
        IrReceiver.printIRResultShort(&Serial);
#endif
        mySettings.irRemoteUserCodes[triggerNo] = IrReceiver.decodedIRData.command;
#if defined IRREMOTE_PRINT
        Serial.print(F("IRRemote saved code: "));
        Serial.print(mySettings.irRemoteUserCodes[triggerNo] <= 0x0010 ? "0" : "");
        Serial.print(mySettings.irRemoteUserCodes[triggerNo] <= 0x0100 ? "0" : "");
        Serial.print(mySettings.irRemoteUserCodes[triggerNo] <= 0x1000 ? "0" : "");
        Serial.println(mySettings.irRemoteUserCodes[triggerNo], HEX);
#endif
        IrReceiver.resume();
      } while (exitWhile);
#if defined IRREMOTE_PRINT
      Serial.println(F("IRRemote end learning"));
#endif
#else
      mp3.playMp3FolderTrack(831);
      waitForTrackToFinish();
#endif
    }
    else if (subMenu == ResetEEPROM) {
      if (voiceMenu(2, 825, 933, false, false, 0) == 1) {
#if defined AiO
        for (int i = 0; i < EEPROM_size; i++) {
          EEPROM_update(i, 0);
        }
#else
        for (int i = 0; i < EEPROM.length(); i++) {
          EEPROM.update(i, 0);
        }
#endif
        resetSettings();
        mp3.playMp3FolderTrack(999);
      }
    }
    else if (subMenu == LockAdminMenu) {
      switch (voiceMenu(2, 980, 980)) {
        case 1:
          mySettings.adminMenuLocked = 0;
          break;
        case 2:
          mySettings.adminMenuLocked = 1;
          break;
      }
    }
  } while (true);

  writeSettings();
  setstandbyTimer();
}
//////////////////////////////////////////////////////////////////////////
uint8_t voiceMenu(int16_t numberOfOptions, uint16_t startMessage, int messageOffset,
                  bool preview = false, uint8_t previewFromFolder = 0, int defaultValue = 0,
                  bool enableSkipTen = false) {
  int16_t returnValue = -1;
  int16_t currentValue = defaultValue;
  int16_t currentValueOld = currentValue;


  mp3Pause();



  if (startMessage != 0) {
    mp3.playMp3FolderTrack(startMessage);
    waitForTrackToStart();
  }

#if defined DEBUG
  Serial.print(F("voiceMenu "));
  Serial.print(numberOfOptions);
  Serial.println(F(" Options"));
#endif

  do {
    readTrigger();
    mp3.loop();

    if (myTrigger.nextPlusTen && numberOfOptions > 10 && enableSkipTen && myTriggerEnable.nextPlusTen) {
      myTriggerEnable.nextPlusTen = false;
      currentValue += 10;
      if (currentValue == 0) {
        currentValue = 1;
      }
    }
    if (myTrigger.next && myTriggerEnable.next) {
      myTriggerEnable.next = false;
      currentValue++;
      if (currentValue == 0) {
        currentValue = 1;
      }
    }
    if (currentValue > numberOfOptions) {
      currentValue = currentValue - numberOfOptions;
    }

    if (myTrigger.previousPlusTen && numberOfOptions > 10 && enableSkipTen && myTriggerEnable.previousPlusTen) {
      myTriggerEnable.previousPlusTen = false;
      currentValue -= 10;
      if (currentValue == 0) {
        currentValue = -1;
      }
    }
    if (myTrigger.previous && myTriggerEnable.previous) {
      myTriggerEnable.previous = false;
      currentValue--;
      if (currentValue == 0) {
        currentValue = -1;
      }
    }
    if (currentValue == -1) {
      currentValue = numberOfOptions;
    } else if (currentValue < -1) {
      currentValue = numberOfOptions + currentValue;
    }

    if (currentValue != currentValueOld) {
      currentValueOld = currentValue;
#if defined DEBUG
      Serial.println(currentValue);
#endif
      mp3.playMp3FolderTrack(messageOffset + currentValue);
      if (preview) {
        waitForTrackToFinish(); // mit preview Track (in der Regel Nummer) komplett Spielen, da es sonst zu abgehakten Anssagen kommt.
        if (previewFromFolder == 0) {
          mp3.playFolderTrack(currentValue, 1);
        } else {
          mp3.playFolderTrack(previewFromFolder, currentValue);
        }
      } else {
        waitForTrackToStart(); // ohne preview Track nur anspielen um Menü flüssiger zu gestallten
      }
    }

    if (myTrigger.cancel && myTriggerEnable.cancel) {
      myTriggerEnable.cancel = false;

      mp3.playMp3FolderTrack(802);
      waitForTrackToFinish();

      checkStandbyAtMillis();
      returnValue = defaultValue;
    }
    if (myTrigger.pauseTrack && myTriggerEnable.pauseTrack) {
      myTriggerEnable.pauseTrack = false;
      if (currentValue != 0) {
        returnValue = currentValue;
      }
    }

#if defined DEBUG
    if (Serial.available() > 0) {
      int optionSerial = Serial.parseInt();
      if (optionSerial != 0 && optionSerial <= numberOfOptions)
        returnValue = optionSerial;
    }
#endif


  } while (returnValue == -1);

#if defined DEBUG
  Serial.print(F("= "));
  Serial.println(returnValue);
#endif
  mp3Pause();
  return returnValue;

}
//////////////////////////////////////////////////////////////////////////
bool setupFolder(folderSettings * theFolder) {
  uint8_t enableMemory = 0;

  theFolder->folder = 0;
  theFolder->mode = 0;
  theFolder->special = 0;
  theFolder->special2 = 0;
  theFolder->special3 = 0;

  // Ordner abfragen
  theFolder->folder = voiceMenu(99, 301, 0, true, 0, 0, true);
  if (theFolder->folder == 0) return false;

  // Wiedergabemodus abfragen
  theFolder->mode = voiceMenu(11, 305, 305);
  if (theFolder->mode == 0) return false;
#if defined DEBUG
  Serial.println(F("Setup folder"));
  Serial.print(F("Folder: "));
  Serial.println(theFolder->folder);
  Serial.print(F("Mode: "));
  Serial.println(theFolder->mode);
#endif
  //  // Hörbuchmodus > Fortschritt im EEPROM auf 1 setzen
  //  writeCardMemory (myFolder->folder,myFolder->special3, 1);

  switch (theFolder->mode) {
    case Single:
#if defined DEBUG
      Serial.println(F("Single"));
#endif
      //Einzeltrack in special speichern
      theFolder->special = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 320, 0,
                                     true, theFolder->folder, 0, true);
      break;
    case AudioDrama_Section:
    case Album_Section:
    case Party_Section:
    case AudioBook_Section:
#if defined DEBUG
      Serial.println(F("Section"));
#endif
      //Von (special), Bis (special2) speichern
      theFolder->special = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 321, 0,
                                     true, theFolder->folder);
      theFolder->special2 = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 322, 0,
                                      true, theFolder->folder, theFolder->special);

    case AudioBook:
    case Album:

      //Speicherplatz für Alben? Ja/Nein
      if (theFolder->mode == Album || theFolder->mode == Album_Section) {
        enableMemory = voiceMenu(2, 983, 933, false, false, 0) - 1;
        if (enableMemory == 1) {
          theFolder->special3 = 255;// Muss mit letztem möglichen Track initialisert werden, da bei start der Karte, der Track um eins erhöht wird. So kann nie Track 1 gespielt werden.
        }
      } else if (theFolder->mode == AudioBook || theFolder->mode == AudioBook_Section) {
        theFolder->special3 = 1;
      }

      break;
    case AdminMenu:
#if defined DEBUG
      Serial.println(F("AdminMenu"));
#endif
      theFolder->folder = 0;
      theFolder->mode = 255;

      break;
    case PuzzlePart:
#if defined DEBUG
      Serial.println(F("PuzzlePart"));
#endif
      theFolder->special = voiceMenu(mp3.getFolderTrackCount(theFolder->folder), 323, 0,
                                     true, theFolder->folder, 0, true);
      theFolder->special2 = voiceMenu(255, 324, 0,
                                      false, theFolder->folder, 0, true);
      break;
    default:
      break;
  }
#if defined DEBUG
  Serial.println(F("Setup folder ok"));
#endif
  return true;
}
//////////////////////////////////////////////////////////////////////////
void setupShortCut(int8_t shortCutNo = -1) {
  bool returnValue = true;
  mp3Pause();
#if defined DEBUG
  Serial.println(F("setupShortCut"));
#endif
  if (shortCutNo == -1) {
    shortCutNo = voiceMenu(availableShortCuts, 940, 0);
  }
  switch (voiceMenu(2, 941, 941)) {
    case 1:
      returnValue &= setupFolder(&shortCuts[shortCutNo]);
      break;
    case 2:
      returnValue &= setupModifier(&shortCuts[shortCutNo]);
      break;
    default:
      returnValue &= false;
      break;
  }
  if (returnValue) {
    mp3.playMp3FolderTrack(400);
    waitForTrackToStart();
    writeShortCuts();
  }
}

void setupCard() {
  bool returnValue = true;
  mp3Pause();
#if defined DEBUG
  Serial.println(F("setupCard"));
#endif
  nfcTagObject newCard;

  switch (voiceMenu(2, 941, 941)) {
    case 1:
      returnValue &= setupFolder(&newCard.nfcFolderSettings);
      break;
    case 2:
      returnValue &= setupModifier(&newCard.nfcFolderSettings);
      break;
    default:
      returnValue &= false;
      break;
  }
  if (returnValue) {
    // Karte ist konfiguriert > speichern
    mp3Pause();
    memset(lastCardUid, 0, sizeof(lastCardUid));
    writeCard(newCard);
  }
}

bool readCard(nfcTagObject * nfcTag) {
  nfcTagObject tempCard;
  // Show some details of the PICC (that is: the tag/card)

  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);

#if defined DEBUG
  Serial.println(F("Card UID "));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.print(F("PICC type "));
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
#endif

  byte buffer[18];
  byte size = sizeof(buffer);

  // Authenticate using key A
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
#if defined DEBUG
    Serial.println(F("Authenticating Classic using key A..."));
    dump_byte_array(key.keyByte, 6);
#endif
    status = mfrc522.PCD_Authenticate(
               MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
#if defined DEBUG
    Serial.println(status);
#endif
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the tempCard

    // Authenticate using key A
#if defined DEBUG
    Serial.println(F("Authenticating MIFARE UL"));
#endif
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
    Serial.print(F("PCD_Authenticate failed "));
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    return false;
  }

  // Show the whole sector as it currently is
  // Serial.println(F("Current data in sector:"));
  // mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  // Serial.println();

  // Read data from the block
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
#if defined DEBUG
    Serial.print(F("Reading data block"));
    Serial.println(blockAddr);
#endif
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
      Serial.print(F("MIFARE_Read failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte buffer2[18];
    byte size2 = sizeof(buffer2);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(8, buffer2, &size2);
    if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
      Serial.print(F("MIFARE_Read_1 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(9, buffer2, &size2);
    if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
      Serial.print(F("MIFARE_Read_2 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 4, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(10, buffer2, &size2);
    if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
      Serial.print(F("MIFARE_Read_3 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 8, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(11, buffer2, &size2);
    if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
      Serial.print(F("MIFARE_Read_4 failed "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 12, buffer2, 4);
  }

#if defined DEBUG
  Serial.println(F("Data on Card: "));
  //dump_byte_array(buffer, 16);
#endif
  uint32_t tempCookie;
  tempCookie = (uint32_t)buffer[0] << 24;
  tempCookie += (uint32_t)buffer[1] << 16;
  tempCookie += (uint32_t)buffer[2] << 8;
  tempCookie += (uint32_t)buffer[3];

  tempCard.cookie = tempCookie;
  tempCard.version = buffer[4];
  tempCard.nfcFolderSettings.folder = buffer[5];
  tempCard.nfcFolderSettings.mode = buffer[6];
  tempCard.nfcFolderSettings.special = buffer[7];
  tempCard.nfcFolderSettings.special2 = buffer[8];
  tempCard.nfcFolderSettings.special3 = buffer[9];
  tempCard.nfcFolderSettings.special4 = buffer[10];

#if defined DEBUG
  Serial.print(F("folder "));
  Serial.println(tempCard.nfcFolderSettings.folder);
  Serial.print(F("mode "));
  Serial.println(tempCard.nfcFolderSettings.mode);
  Serial.print(F("special "));
  Serial.println(tempCard.nfcFolderSettings.special);
  Serial.print(F("special2 "));
  Serial.println(tempCard.nfcFolderSettings.special2);
  Serial.print(F("special3 "));
  Serial.println(tempCard.nfcFolderSettings.special3);
  Serial.print(F("special4 "));
  Serial.println(tempCard.nfcFolderSettings.special4);
#endif
  if (tempCard.cookie == cardCookie) {

    if (activeModifier != NULL && tempCard.nfcFolderSettings.folder != 0) {
      if (activeModifier->handleRFID(&tempCard) == true) {
#if defined DEBUG
        Serial.println(F("RFID locked"));
#endif
        return false;
      }
    }
    if (tempCard.nfcFolderSettings.folder == 0)
      return SetModifier (&tempCard.nfcFolderSettings);
    else {
      memcpy(nfcTag, &tempCard, sizeof(nfcTagObject));
#if defined DEBUG
      Serial.println( nfcTag->nfcFolderSettings.folder);
#endif
      myFolder = &nfcTag->nfcFolderSettings;
#if defined DEBUG
      Serial.println( myFolder->folder);
#endif
    }
    return true;
  }
  else {
    memcpy(nfcTag, &tempCard, sizeof(nfcTagObject));
    return true;
  }
}

void resetCard() {
  mp3.playMp3FolderTrack(800);
  do {
    readTrigger();
    if (myTrigger.cancel) {
#if defined DEBUG
      Serial.print(F("Abgebrochen!"));
#endif
      myTriggerEnable.cancel = false;
      mp3.playMp3FolderTrack(802);
      return;
    }
  } while (!mfrc522.PICC_IsNewCardPresent());

  if (!mfrc522.PICC_ReadCardSerial())
    return;

#if defined DEBUG
  Serial.print(F("reset Tag"));
#endif
  setupCard();
}
//////////////////////////////////////////////////////////////////////////
void writeCard(nfcTagObject nfcTag, int8_t writeBlock = -1, bool feedback = true) {
  MFRC522::PICC_Type mifareType;
  byte buffer[16] = {0x13, 0x37, 0xb3, 0x47, // 0x1337 0xb347 magic cookie to
                     // identify our nfc tags
                     0x02,                   // version 1
                     nfcTag.nfcFolderSettings.folder,          // the folder picked by the user
                     nfcTag.nfcFolderSettings.mode,    // the playback mode picked by the user
                     nfcTag.nfcFolderSettings.special, // track or function for admin cards
                     nfcTag.nfcFolderSettings.special2,
                     nfcTag.nfcFolderSettings.special3,
                     nfcTag.nfcFolderSettings.special4,
                     0x00, 0x00, 0x00, 0x00, 0x00
                    };

  byte size = sizeof(buffer);

#if defined DEBUG
  Serial.println(F("write Card"));
#endif
  mifareType = mfrc522.PICC_GetType(mfrc522.uid.sak);

  // Authenticate using key B
  //authentificate with the card and set card specific parameters
  if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
#if defined DEBUG
    Serial.println(F("Authenticating again using key A..."));
#endif
    status = mfrc522.PCD_Authenticate(
               MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  }
  else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the NFCtag

    // Authenticate using key A
#if defined DEBUG
    Serial.println(F("Authenticating UL..."));
#endif
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
    Serial.print(F("PCD_Authenticate failed "));
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    if (feedback) {
      mp3.playMp3FolderTrack(401);
      waitForTrackToFinish();
    }
    return;
  }

  // Write data to the block
#if defined DEBUG
  Serial.print(F("Writing data "));
  Serial.print(blockAddr);
  Serial.println(F("..."));
  dump_byte_array(buffer, 16);
  Serial.println();
#endif

  if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_1K ) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_4K ) )
  {
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(blockAddr, buffer, 16);
  }
  else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL )
  {
    byte buffer2[16];
    byte size2 = sizeof(buffer2);
    if (writeBlock == -1) {
      memset(buffer2, 0, size2);
      memcpy(buffer2, buffer, 4);
      status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(8, buffer2, 16);

      memset(buffer2, 0, size2);
      memcpy(buffer2, buffer + 4, 4);
      status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(9, buffer2, 16);

      memset(buffer2, 0, size2);
      memcpy(buffer2, buffer + 8, 4);
      status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(10, buffer2, 16);

      memset(buffer2, 0, size2);
      memcpy(buffer2, buffer + 12, 4);
      status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(11, buffer2, 16);
    } else if (writeBlock <= 11 && writeBlock >= 8) {
      memset(buffer2, 0, size2);
      memcpy(buffer2, buffer, 4);
      status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(writeBlock, buffer2, 16);
    }
  }

  if (status != MFRC522::STATUS_OK) {
#if defined DEBUG
    Serial.print(F("MIFARE_Write failed "));
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    if (feedback) {
      mp3.playMp3FolderTrack(401);
    }
  }
  else if (feedback) {
    mp3.playMp3FolderTrack(400);
    waitForTrackToFinish();
  }
#if defined DEBUG
  Serial.println();
#endif
  //delay(1000);
}
//////////////////////////////////////////////////////////////////////////
/**
  Helper routine to dump a byte array as hex values to Serial.
*/
#if defined DEBUG
void dump_byte_array(byte * buffer, byte bufferSize) {

  for (byte i = 0; i < bufferSize; i++) {

    Serial.print(buffer[i] < 0x10 ? "0" : " ");
    Serial.print(buffer[i], HEX );
    Serial.print(" ");

  }
  Serial.println();
}
#endif
///////////////////////////////////////// Check Bytes   ///////////////////////////////////
/*bool checkTwo ( uint8_t a[], uint8_t b[] ) {
  for ( uint8_t k = 0; k < 4; k++ ) {   // Loop 4 times
    if ( a[k] != b[k] ) {     // IF a != b then false, because: one fails, all fail
      return false;
    }
  }
  return true;
  }*/
//////////////////////////////////////////////////////////////////////////

bool setupModifier(folderSettings * tmpFolderSettings) {

  tmpFolderSettings->folder = ModifierMode;
  tmpFolderSettings->mode = voiceMenu(10, 966, 966, false, 0, 0, true);

  if (tmpFolderSettings->mode != ModifierMode) {
    if (tmpFolderSettings->mode == SleepTimerMod) {
      switch (voiceMenu(4, 960, 960)) {
        case 1: tmpFolderSettings->special = 5; break;
        case 2: tmpFolderSettings->special = 15; break;
        case 3: tmpFolderSettings->special = 30; break;
        case 4: tmpFolderSettings->special = 60; break;
      }
      tmpFolderSettings->special2 = 0x00;
    } else if (tmpFolderSettings->mode == PuzzleGameMod) {
      //Save first part?
      tmpFolderSettings->special = voiceMenu(2, 979, 933) - 1;
      tmpFolderSettings->special2 = 0x00;
    }
    else if (tmpFolderSettings->mode == QuizGameMod) {
      //Set Folder
      tmpFolderSettings->special =  voiceMenu(99, 301, 0, true, 0, 0, true);
      tmpFolderSettings->special2 = 0x00;
    }
    else if (tmpFolderSettings->mode == ButtonSmashMod) {
      //Set Folder
      tmpFolderSettings->special =  voiceMenu(99, 301, 0, true, 0, 0, true);
      //Set Volume
      tmpFolderSettings->special2 =  voiceMenu(30, 904, false, false, 0, volume);
    }
    else if (tmpFolderSettings->mode == CalculateMod) {
      //Set Oprator
      tmpFolderSettings->special =  voiceMenu(5, 423, 423, false, 0, 0, true);
      //Set max number
      tmpFolderSettings->special2 =  voiceMenu(255, 429, 0, false, 0, 0, true);
    }

    //Save Modifier in EEPROM?
    tmpFolderSettings->special3 = voiceMenu(2, 980, 933, false, false, 0) - 1;
    tmpFolderSettings->special4 = 0x00;
    return true;
  }
  return false;
}

bool SetModifier (folderSettings * tmpFolderSettings) {
  if (activeModifier != NULL) {
    if (activeModifier->getActive() == tmpFolderSettings->mode) {
      return RemoveModifier();
    }
  }

#if defined DEBUG
  Serial.print(F("set modifier: "));
  Serial.println(tmpFolderSettings->mode);
#endif

  if (tmpFolderSettings->mode != 0 && tmpFolderSettings->mode != AdminMenuMod) {
    if (isPlaying()) {
      mp3.playAdvertisement(260);
    }
    else {
      mp3.playMp3FolderTrack(260);
    }
    waitForTrackToFinish();
  }


  switch (tmpFolderSettings->mode) {
    case 0:
    case AdminMenuMod:
      mfrc522.PICC_HaltA(); mfrc522.PCD_StopCrypto1(); adminMenu(true);  break;
    case SleepTimerMod: activeModifier = new SleepTimer(tmpFolderSettings->special); break;
    case FreezeDanceMod: activeModifier = new FreezeDance(); break;
    case LockedMod: activeModifier = new Locked(); break;
    case ToddlerModeMod: activeModifier = new ToddlerMode(); break;
    case KindergardenModeMod: activeModifier = new KindergardenMode(); break;
    case RepeatSingleMod: activeModifier = new RepeatSingleModifier(); break;
    case PuzzleGameMod: activeModifier = new PuzzleGame(tmpFolderSettings->special); break;
    case QuizGameMod: activeModifier = new QuizGame(tmpFolderSettings->special); break;
    case ButtonSmashMod: activeModifier = new ButtonSmash(tmpFolderSettings->special, tmpFolderSettings->special2); break;
    //case TheQuestMod: activeModifier = new TheQuest(tmpFolderSettings->special, tmpFolderSettings->special2); break;
    case CalculateMod: activeModifier = new Calculate (tmpFolderSettings->special, tmpFolderSettings->special2); break;
  }
  if (tmpFolderSettings->special3 == 1) {
    mySettings.savedModifier = *tmpFolderSettings;
    writeSettings();
  }
  return false;
}

bool RemoveModifier() {
  if (activeModifier != NULL) {
    activeModifier->deleteModifier();
    activeModifier = NULL;
  }


  _lastTrackFinished = 0;

  if (mySettings.savedModifier.folder != 0 ||  mySettings.savedModifier.mode != 0) {
    mySettings.savedModifier.folder = 0;
    mySettings.savedModifier.mode = 0;
    writeSettings();
  }

#if defined DEBUG
  Serial.println(F("modifier removed"));
#endif
  if (isPlaying()) {
    mp3.playAdvertisement(261);
  }
  else {
    mp3Pause();
    mp3.playMp3FolderTrack(261);
    waitForTrackToFinish();
  }
  return false;
}

void checkForUnwrittenTrack() {
  if (trackToStoreOnCard > 0 && !hasCard && activeShortCut < 0) {
    //ungespeicherter Track vorhanden und keine Karte vorhanden
    if (isPlaying()) {
      mp3.playAdvertisement(981);
    } else {
      mp3.playMp3FolderTrack(981);
    }
    waitForTrackToFinish();
    delay(100);
  }
}

void writeCardMemory (uint8_t track, bool announcement = false) {
#if defined DEBUG || defined SHORTCUTS_PRINT
  Serial.println(F("write memory..."));
#endif

  if (knownCard && hasCard) {
#if defined DEBUG
    Serial.println(F("on card"));
#endif
    //Karte aufgelegt  & aktiv
    trackToStoreOnCard = 0;
    myCard.nfcFolderSettings.special3 = track;
    writeCard(myCard, 10, false);
  } else if (knownCard && !hasCard) {
#if defined DEBUG
    Serial.println(F("in variable (no card)"));
#endif
    //Karte nicht mehr vorhanden aber aktiv, kein schreiben möglich. Track merken
    trackToStoreOnCard = track;
    if (announcement) {
      checkForUnwrittenTrack();
    }
  } else if (activeShortCut >= 0) {
#if defined DEBUG || defined SHORTCUTS_PRINT
    Serial.println(F("in EEPROM (shortcut)"));
#endif
    updateShortCutTrackMemory(track);
  }
}

//Um festzustellen ob eine Karte entfernt wurde, muss der MFRC regelmäßig ausgelesen werden
uint8_t pollCard() {
  const byte maxRetries = 2;

  if (!hasCard)  {
    if (mfrc522.PICC_IsNewCardPresent()) {
      if (mfrc522.PICC_ReadCardSerial()) {
#if defined DEBUG
        Serial.println(F("ReadCardSerial finished"));
#endif
        if (readCard(&myCard))  {
          bool bSameUID = !memcmp(lastCardUid, mfrc522.uid.uidByte, 4);
          // store info about current card
          memcpy(lastCardUid, mfrc522.uid.uidByte, 4);
          lastCardWasUL = mfrc522.PICC_GetType(mfrc522.uid.sak) == MFRC522::PICC_TYPE_MIFARE_UL;

          retries = maxRetries;
          hasCard = true;
          return bSameUID ? PCS_CARD_IS_BACK : PCS_NEW_CARD;
        }
        else {//readCard war nicht erfolgreich
          mfrc522.PICC_HaltA();
          mfrc522.PCD_StopCrypto1();
#if defined DEBUG
          Serial.println(F("cant read card"));
#endif
        }
      }
    }
    return PCS_NO_CHANGE;
  }
  else // hasCard
  {
    // perform a dummy read command just to see whether the card is in range
    byte buffer[18];
    byte size = sizeof(buffer);

    if (mfrc522.MIFARE_Read(lastCardWasUL ? 8 : blockAddr, buffer, &size) != MFRC522::STATUS_OK) {
      if (retries > 0)
        retries--;
      else {
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
        hasCard = false;
        return PCS_CARD_GONE;
      }
    }
    else
      retries = maxRetries;
  }
  return PCS_NO_CHANGE;
}

Enum_PCS handleCardReader() {
  bool tmpStopWhenCardAway = mySettings.stopWhenCardAway;

  static unsigned long lastCardPoll = 0;
  unsigned long now = millis();
  // poll card only every 100ms
  if (static_cast<unsigned long>(now - lastCardPoll) > 100)  {
    lastCardPoll = now;

    if (activeModifier != NULL) {
      tmpStopWhenCardAway &= !activeModifier->handleStopWhenCardAway();
    }

    switch (pollCard()) {
      case PCS_NEW_CARD:
#if defined DEBUG
        Serial.println(F("new card"));
#endif
        onNewCard();
        return PCS_NEW_CARD;
        break;
      case PCS_CARD_GONE:
#if defined DEBUG
        Serial.println(F("card gone"));
#endif
        if (tmpStopWhenCardAway) {
          knownCard = false;
          myTriggerEnable.pauseTrack |= true;
          myTrigger.pauseTrack |= true;
          pauseAction();
          //mp3Pause();
          //setstandbyTimer();
        }
        return PCS_CARD_GONE;
        break;
      case PCS_CARD_IS_BACK:
#if defined DEBUG
        Serial.println(F("same card"));
#endif
        if (tmpStopWhenCardAway) {

          //nur weiterspielen wenn vorher nicht konfiguriert wurde
          //if (!forgetLastCard) {
          knownCard = true;
          myTriggerEnable.pauseTrack |= true;
          myTrigger.pauseTrack |= true;
          pauseAction();
          //mp3.start();
          //disablestandbyTimer();
          /*}
            else
            onNewCard();*/
        } else if (trackToStoreOnCard == 0) {
          onNewCard();
        }

        if (trackToStoreOnCard > 0) {
          writeCardMemory(trackToStoreOnCard);
          trackToStoreOnCard = 0;
          if (!isPlaying()) {
            mp3.start();
            delay(150);
            mp3.playAdvertisement(982);
            waitForTrackToFinish();
            delay(100);
            mp3.pause();
          }
        }
        return PCS_CARD_IS_BACK;
        break;
    }
  }
}

void onNewCard() {
  //forgetLastCard = false;
  //make random a little bit more "random"
  //randomSeed(millis() + random(1000));
  trackToStoreOnCard = 0;
  if (myCard.cookie == cardCookie && myCard.nfcFolderSettings.folder != 0 && myCard.nfcFolderSettings.mode != 0) {
#if defined DEBUG
    Serial.println(F("onNewCard()->playFolder"));
#endif
    knownCard = true;
    activeShortCut = -1;
    playFolder();
  }

  // Neue Karte konfigurieren
  else if (myCard.cookie != cardCookie) {
    mp3.playMp3FolderTrack(300);
    waitForTrackToFinish();
    setupCard();
  }
}

/// Funktionen für den Standby Timer (z.B. über Pololu-Switch oder Mosfet)

void setstandbyTimer() {
#if defined DEBUG
  Serial.println(F("set standby timer"));
#endif
#if defined AiO
  // verstärker aus
  digitalWrite(8, HIGH);
  delay(100);
#elif defined SPEAKER_SWITCH
  digitalWrite(SpeakerOnPin, LOW);
  delay(100);
#endif
  if (mySettings.standbyTimer != 0)
    sleepAtMillis = millis() + (mySettings.standbyTimer * 60 * 1000);
  else
    sleepAtMillis = 0;
#if defined DEBUG
  Serial.print(F("milis: "));
  Serial.println(sleepAtMillis);
#endif
}
//////////////////////////////////////////////////////////////////////////
void disablestandbyTimer() {
#if defined DEBUG
  Serial.println(F("disable standby timer"));
#endif
#if defined AiO
  // verstärker an
  digitalWrite(8, LOW);
  delay(100);
#elif defined SPEAKER_SWITCH
  digitalWrite(SpeakerOnPin, HIGH);
  delay(100);
#endif
  sleepAtMillis = 0;
}
//////////////////////////////////////////////////////////////////////////
void checkStandbyAtMillis() {
  if (sleepAtMillis != 0 && millis() > sleepAtMillis) {
#if defined DEBUG
    Serial.println(F("standby active"));
#endif
    shutDown();
  }
}
//////////////////////////////////////////////////////////////////////////
void shutDown() {
#if defined PUSH_ON_OFF
#if defined DEBUG
  Serial.println("Shut Down");
#endif

  mp3Pause();

#if defined AiO
  // verstärker an
  digitalWrite(8, LOW);

#elif defined SPEAKER_SWITCH
  digitalWrite(SpeakerOnPin, HIGH);
#endif

#if defined POWER_ON_LED
  digitalWrite(PowerOnLEDPin, LOW);
#endif

  if (volume > mySettings.initVolume)
    volume = mySettings.initVolume;

  mp3.setVolume(volume);
  delay(100);

  knownCard = false;
  activeShortCut = -1;
  mp3.playMp3FolderTrack(265);
  waitForTrackToFinish();

#if defined AiO
  // verstärker aus
  digitalWrite(8, HIGH);

#elif defined SPEAKER_SWITCH
  digitalWrite(SpeakerOnPin, LOW);
#endif

#if not defined AiO
  digitalWrite(shutdownPin, HIGH);

  // enter sleep state
  // http://discourse.voss.earth/t/intenso-s10000-powerbank-automatische-abschaltung-software-only/805
  // powerdown to 27mA (powerbank switches off after 30-60s)
  mfrc522.PCD_AntennaOff();
  mfrc522.PCD_SoftPowerDown();
  //mp3.sleep();

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  cli();  // Disable interrupts
  sleep_mode();
#else
  digitalWrite(shutdownPin, LOW);
#endif
#endif
}

//////////////////////////////////////////////////////////////////////////
#if defined FADING_LED
// fade in/out status led while beeing idle, during playback set to full brightness
void fadeStatusLed(bool isPlaying) {
  static bool statusLedDirection = false;
  static int8_t statusLedValue = 255;
  static unsigned long statusLedOldMillis;
  static int8_t statusLedDeltaValuePause = 500;
  static int8_t statusLedDeltaValuePlay = 1;
  static int8_t statusLedDeltaValue = 10;

  if (isPlaying) {
    statusLedDeltaValue = statusLedDeltaValuePlay;
  }
  else {
    statusLedDeltaValue = statusLedDeltaValuePause;
  }

  if ((millis() - statusLedOldMillis) >= 100) {
    statusLedOldMillis = millis();
    if (statusLedDirection) {
      statusLedValue += statusLedDeltaValue;
      if (statusLedValue >= 255) {
        statusLedValue = 255;
        statusLedDirection = !statusLedDirection;
      }
    }
    else {
      statusLedValue -= statusLedDeltaValue;
      if (statusLedValue <= 0) {
        statusLedValue = 0;
        statusLedDirection = !statusLedDirection;
      }
    }
    analogWrite(POWER_ON_LED_PIN, statusLedValue);
  }
}
#endif
//////////////////////////////////////////////////////////////////////////
