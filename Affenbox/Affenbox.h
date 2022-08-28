#include <Arduino.h>
#include "Configuration.h"
#include <DFMiniMp3.h>
#include <EEPROM.h>
#include <JC_Button.h>
#include <AceButton.h>
#include <MFRC522.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#if not defined AiO
#include <avr/sleep.h>
#endif
#if defined ROTARY_ENCODER
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>
#endif
#if defined IRREMOTE
#include <IRremote.h>
#endif
#if defined AiO
#include "Emulated_EEPROM.h"
#endif

using ace_button::AceButton;
using ace_button::ButtonConfig;
using ace_button::LadderButtonConfig;

///////// General globals ////////////////////////////////////////////////
unsigned long sleepAtMillis = 0;
bool standbyTimerSet = false;
static const uint8_t openAnalogPin = A7; //Default A7, muss ein unbelegeter, analoger Eingang sein
//////////////////////////////////////////////////////////////////////////

////////// Button timings ////////////////////////////////////////////////
static const uint8_t SHORT_PRESS = 50;
static const uint16_t LONG_PRESS = 750;
static const uint16_t LONGER_PRESS = 1500;
//////////////////////////////////////////////////////////////////////////

///////// MFRC522 ////////////////////////////////////////////////////////
static const uint8_t RST_PIN = 9;                 // Configurable, see typical pin layout above
static const uint8_t SS_PIN = 10;               // Configurable, see typical pin layout above
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
#if defined AiO
static const uint16_t EEPROM_size = 512;
#endif
static const uint16_t EEPROM_settingsStartAdress = 1;

#if defined ANALOG_INPUT
static const uint8_t availableShortCuts = 2 + ANALOG_INPUT_BUTTON_COUNT; //Die Anzahl Inputs einer 3x4 Matrix oder eines 0-9,#,* Tastenfeldes
#else
static const uint8_t availableShortCuts = 2 ; //
#endif
///////// setup buttons //////////////////////////////////////////////////
Button pauseButton(buttonPause);
#if defined buttonPower
  Button powerButton(buttonPower);
#endif
Button upButton(buttonUp);
Button downButton(buttonDown);
#if defined FIVEBUTTONS
Button buttonFour(buttonFourPin);
Button buttonFive(buttonFivePin);
#endif
//////////////////////////////////////////////////////////////////////////

enum Enum_Trigger {
  NoTrigger = 0,
  PauseTrackTrigger,
  NextTrigger,
  NextPlusTenTrigger,
  PreviousTrigger,
  PreviousPlusTenTrigger,
  VolumeUpTrigger,
  VolumeDownTrigger,
  AbortTrigger,
  ShutDownTrigger,
  AdminMenuTrigger,
  ResetTrackTrigger,
  TrackFinishedTrigger,
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
  bool shutDown;
  bool adminMenu;
  bool resetTrack;
  bool trackFinished;
  bool shortCutNo [availableShortCuts]; //muss an letzter Stelle stehen bleiben!
};

static inputTrigger myTrigger;
static inputTrigger myTriggerEnable;

static const uint8_t sizeOfInputTrigger = sizeof(inputTrigger);

#if defined AiO
const uint8_t folderMemoryCount = 3; //limited EEPROM space for the LGT8F328P
#else
const uint8_t folderMemoryCount = 5;
#endif

//////// IR Remote ////////////////////////////////////////////////////
#if defined IRREMOTE
const uint8_t irRemoteCodeCount = sizeOfInputTrigger;
#endif
//////////////////////////////////////////////////////////////////////////

//////// Rotary Encoder ///////////////////////////////////////////////
#if defined ROTARY_ENCODER
Encoder myEnc(ROTARY_ENCODER_PIN_A, ROTARY_ENCODER_PIN_B);
#endif
//////////////////////////////////////////////////////////////////////////

///////// DFPlayer Mini //////////////////////////////////////////////////
SoftwareSerial mySoftwareSerial(2, 3); // RX, TX
static const uint8_t busyPin = 4;
uint8_t numTracksInFolder;
uint8_t currentTrack;
uint8_t firstTrack;
uint8_t queue[255];
uint8_t volume;
static uint8_t _lastTrackFinished;
//////////////////////////////////////////////////////////////////////////

//////// analog input /////////////////////////////////////////////////
#if defined ANALOG_INPUT
static AceButton analogInputButtons[ANALOG_INPUT_BUTTON_COUNT];
static AceButton* analogInputButtonsPointer[ANALOG_INPUT_BUTTON_COUNT];
LadderButtonConfig *analogInputButtonConfig;
#endif
//////////////////////////////////////////////////////////////////////////

///////// Enums //////////////////////////////////////////////////////////
enum Enum_PlayMode
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
enum Enum_Modifier
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

enum Enum_AdminMenuOptions
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
  SetupIRRemote,
  ResetEEPROM,
  LockAdminMenu
};
enum Enum_PCS
{
  PCS_NO_CHANGE     = 0, // no change detected since last pollCard() call
  PCS_NEW_CARD      = 1, // card with new UID detected (had no card or other card before)
  PCS_CARD_GONE     = 2, // card is not reachable anymore
  PCS_CARD_IS_BACK  = 3 // card was gone, and is now back again
};

enum Enum_NfcGain
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
  uint8_t userAge; // Reserviert für zukünftige Funktion
  uint16_t irRemoteUserCodes[sizeOfInputTrigger];//Ein Slot pro möglichem Trigger
};
adminSettings mySettings;
uint16_t sizeOfAdminSettings = sizeof(adminSettings);
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
static bool hasCard = false;
static byte lastCardUid[4];
static byte retries;
static bool lastCardWasUL;
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
void SetVolume(uint8_t volume);
void PlayMp3FolderTrack(uint16_t Track);
void playFolder();
void activateShortCut (uint8_t shortCutNo);
static void nextTrack(uint8_t track, bool force = false) ;
static void previousTrack();
bool isPlaying();
void mp3Pause(uint16_t delayTime = 100);
void waitForTrackToFinish (bool interruptByTrigger = true, uint16_t timeOut = 5000);
void waitForTrackToStart(uint16_t timeOut = 5000);
void setup();
void readButtons(bool invertVolumeButtons = false);
#if defined IRREMOTE
void readIr ();
#endif
#if defined ANALOG_INPUT
void readAnalogIn(AceButton* button, uint8_t eventType, uint8_t buttonState);
#endif
#if defined ROTARY_ENCODER
void RotEncSetVolume ();
#endif
void checkNoTrigger ();
void readTrigger(bool invertVolumeButtons = false);
void resetTrigger();
void resetTriggerEnable();
void volumeUpAction(bool rapidFire = false);
void volumeDownAction(bool rapidFire = false);
void nextAction();
void previousAction();
void pauseAction(bool fromCard = false);
void loop();
void adminMenu(bool fromCard = false);
uint8_t voiceMenu(int16_t numberOfOptions, uint16_t startMessage, int messageOffset,
                  bool preview = false, uint8_t previewFromFolder = 0, int defaultValue = 0,
                  bool enableSkipTen = true);
bool setupFolder(folderSettings * theFolder) ;
void setupShortCut(int8_t shortCutNumber = -1);
void setupCard();
bool readCard(nfcTagObject * nfcTag);
void resetCard();
void resetCurrentCard();
void writeCard(nfcTagObject nfcTag, int8_t writeBlock = -1, bool feedback = true);
void dump_byte_array(byte * buffer, byte bufferSize);
bool setupModifier(folderSettings * tmpFolderSettings);
bool SetModifier (folderSettings * tmpFolderSettings);
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
class Mp3Notify;

static DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(mySoftwareSerial);
