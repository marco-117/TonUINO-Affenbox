
#include "Affenbox.h"

class Mp3Notify
{
  public:
    static void OnError(uint16_t errorCode)
    {
#if defined DFPLAYER_PRINT
      switch (errorCode)
      {
        case DfMp3_Error_Busy:
          {
            Serial.println(F("busy"));
            break;
          }
        case DfMp3_Error_Sleeping:
          {
            Serial.println(F("sleep"));
            break;
          }
        case DfMp3_Error_SerialWrongStack:
          {
            Serial.println(F("serial stack"));
            break;
          }
        case DfMp3_Error_CheckSumNotMatch:
          {
            Serial.println(F("checksum"));
            break;
          }
        case DfMp3_Error_FileIndexOut:
          {
            Serial.println(F("file index"));
            break;
          }
        case DfMp3_Error_FileMismatch:
          {
            Serial.println(F("file mismatch"));
            break;
          }
        case DfMp3_Error_Advertise:
          {
            Serial.println(F("advertise"));
            break;
          }
        case DfMp3_Error_RxTimeout:
          {
            Serial.println(F("rx timeout"));
            break;
          }
        case DfMp3_Error_PacketSize:
          {
            Serial.println(F("packet size"));
            break;
          }
        case DfMp3_Error_PacketHeader:
          {
            Serial.println(F("packet header"));
            break;
          }
        case DfMp3_Error_PacketChecksum:
          {
            Serial.println(F("packet checksum"));
            break;
          }
        case DfMp3_Error_General:
          {
            Serial.println(F("general"));
            break;
          }
        default:
          {
            Serial.println(F("unknown"));
            break;
          }
      }
#endif
    }

    static void PrintlnSourceAction(DfMp3_PlaySources source, const char *action)
    {
#if defined DFPLAYER_PRINT
      if (source & DfMp3_PlaySources_Sd)
        Serial.print("SD ");
      if (source & DfMp3_PlaySources_Usb)
        Serial.print("USB ");
      if (source & DfMp3_PlaySources_Flash)
        Serial.print("Flash ");
      Serial.println(action);
#endif
    }

    static void OnPlaySourceOnline(DfMp3_PlaySources source)
    {
#if defined DFPLAYER_PRINT
      PrintlnSourceAction(source, "online");
#endif
    }
    static void OnPlaySourceInserted(DfMp3_PlaySources source)
    {
#if defined DFPLAYER_PRINT
      PrintlnSourceAction(source, "ready");
#endif
    }
    static void OnPlaySourceRemoved(DfMp3_PlaySources source)
    {
#if defined DFPLAYER_PRINT
      PrintlnSourceAction(source, "removed");
#endif
    }
    static void OnPlayFinished(DfMp3_PlaySources source, uint16_t track)
    {
      myTrigger.trackFinished |= true;
      nextTrack(track);
    }
};

//////////////////////////////////////////////////////////////////////////
void shuffleQueue(uint8_t startTrack /* = firstTrack */, uint8_t endTrack /*  = numTracksInFolder */)
{

#if defined DEBUG || defined QUEUE_PRINT
  Serial.println(F("shuffleQ"));
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
    uint8_t j = random(0, endTrack - startTrack + 1);
    uint8_t t = queue[i];
    queue[i] = queue[j];
    queue[j] = t;
  }
#ifdef QUEUE_PRINT
  Serial.println(F("Queue :"));
  for (uint8_t x = 0; x < endTrack - startTrack + 1; x++)
  {
    Serial.print(x + 1);
    Serial.print(". : ");
    Serial.println(queue[x]);
  }
#endif
}
//////////////////////////////////////////////////////////////////////////
void writeSettings()
{

#if defined DEBUG
  Serial.println(F("writeSettings"));
#endif

  EEPROM.put(EEPROM_settingsStartAdress, mySettings);

}

void getSettings()
{

  EEPROM.get(EEPROM_settingsStartAdress, mySettings);

  if (mySettings.version != 3 || mySettings.cookie != cardCookie)
  {
    resetSettings();
  }

  if (mySettings.irRemoteUserCodes[0] != 0)
  {
    mySettings.irRemoteUserCodes[0] = 0;
  }

#if defined EEPROM_PRINT

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

  Serial.print(F("Saved Modifier "));
  Serial.println(mySettings.savedModifier.mode);
#endif
#if defined IRREMOTE_PRINT && defined EEPROM_PRINT
  for (uint8_t i = 0; i < sizeOfInputTrigger; i++)
  {
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

void resetSettings()
{
#if defined DEBUG
  Serial.println(F("rsetSettings"));
#endif
  mySettings.cookie = cardCookie;
  mySettings.version = 3;
  mySettings.maxVolume = 25;
  mySettings.minVolume = 1;
  mySettings.initVolume = 18;
  mySettings.eq = 1;
  mySettings.locked = false;
  mySettings.standbyTimer = 1;
  mySettings.invertVolumeButtons = false;
  mySettings.adminMenuLocked = 0;
  mySettings.savedModifier.folder = 0;
  mySettings.savedModifier.special = 0;
  mySettings.savedModifier.special2 = 0;
  mySettings.savedModifier.mode = 0;
  mySettings.stopWhenCardAway = false;
  mySettings.userAge = 0;

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
void resetShortCuts()
{
#if defined DEBUG
  Serial.println(F("rset short cuts"));
#endif
  for (uint8_t i = 0; i < availableShortCuts; i++)
  {
    shortCuts[i].folder = 0;
    shortCuts[i].mode = 0;
    shortCuts[i].special = 0;
    shortCuts[i].special2 = 0;
    shortCuts[i].special3 = 0;
  }
  writeShortCuts();
}
void writeShortCuts()
{
#if defined SHORTCUTS_PRINT
  Serial.print(F("write short cuts at "));
  Serial.println(EEPROM_settingsStartAdress + sizeOfAdminSettings);
#endif

  EEPROM.put(EEPROM_settingsStartAdress + sizeOfAdminSettings, shortCuts);

}

void getShortCuts()
{
#if defined SHORTCUTS_PRINT
  Serial.print(F("get short cuts from "));
  Serial.println(EEPROM_settingsStartAdress + sizeOfAdminSettings);
#endif

  EEPROM.get(EEPROM_settingsStartAdress + sizeOfAdminSettings, shortCuts);

#if defined SHORTCUTS_PRINT
  for (uint8_t i = 0; i < availableShortCuts; i++)
  {
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
void updateShortCutTrackMemory(uint8_t track)
{
#if defined SHORTCUTS_PRINT
  Serial.print(F("update short cuts at "));
  Serial.print(EEPROM_settingsStartAdress + sizeOfAdminSettings + (sizeOfFolderSettings * activeShortCut) - 1);
  Serial.print(F(" with track "));
  Serial.println(track);
#endif
  shortCuts[activeShortCut].special3 = track;

  EEPROM.put(EEPROM_settingsStartAdress + sizeOfAdminSettings + (sizeOfFolderSettings * activeShortCut), shortCuts[activeShortCut]);

}
//////////////////////////////////////////////////////////////////////////
class Modifier
{
  public:
    virtual void loop() {}
    virtual bool handlePause()
    {
      return false;
    }
    virtual bool handleNext()
    {
      return false;
    }
    virtual bool handlePrevious()
    {
      return false;
    }
    virtual bool handleNextAction()
    {
      return false;
    }
    virtual bool handlePreviousAction()
    {
      return false;
    }
    virtual bool handleVolumeUpAction()
    {
      return false;
    }
    virtual bool handleVolumeDownAction()
    {
      return false;
    }
    virtual bool handleShortCut(uint8_t shortCutNo)
    {
      return false;
    }
    virtual bool handleShutDown()
    {
      return false;
    }
    virtual bool handleRFID(nfcTagObject *newCard)
    {
      return false;
    }
    virtual bool handleStopWhenCardAway()
    {
      return false;
    }
    virtual bool handleSaveModifier()
    {
      return false;
    }
    virtual bool handleAdminMenu()
    {
      return false;
    }
    virtual uint8_t getActive()
    {
      return 0;
    }
    virtual ~Modifier()
    {
      delete this;
    }
    Modifier()
    {
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
class SleepTimer : public Modifier
{
  private:
    unsigned long sleepAtMillis = 0;

  public:
    void loop()
    {
      if (this->sleepAtMillis != 0 && millis() > this->sleepAtMillis)
      {
#if defined DEBUG
        Serial.println(F("SleepTimer sleep"));
#endif
        mp3Pause();
        //setstandbyTimer();
        activeModifier = NULL;
        delete this;
      }
    }

    SleepTimer(uint8_t minutes)
    {
#if defined DEBUG
      Serial.print(F("SleepTimer minutes "));
      Serial.println(minutes);
#endif
      this->sleepAtMillis = millis() + minutes * 60000;
    }
    uint8_t getActive()
    {
      return SleepTimerMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class FreezeDance : public Modifier
{
  private:
    unsigned long nextStopAtMillis = 0;
    const uint8_t minSecondsBetweenStops = 5;
    const uint8_t maxSecondsBetweenStops = 30;

    void setNextStopAtMillis()
    {
      uint8_t seconds = random(this->minSecondsBetweenStops, this->maxSecondsBetweenStops + 1);
#if defined DEBUG
      Serial.print(F("FreezeDance next stop "));
      Serial.print(seconds);
      Serial.println(F(" s"));
#endif
      this->nextStopAtMillis = millis() + seconds * 1000;
    }

  public:
    void loop()
    {
      if (this->nextStopAtMillis != 0 && millis() > this->nextStopAtMillis)
      {
#if defined DEBUG
        Serial.println(F("FreezeDance freeze"));
#endif
        if (isPlaying())
        {
          mp3.playAdvertisement(301);
          delay(500);
        }
        setNextStopAtMillis();
      }
    }
    FreezeDance(void)
    {
#if defined DEBUG
      Serial.println(F("FreezeDance"));
#endif
      if (isPlaying())
      {
        delay(1000);
        mp3.playAdvertisement(300);
        delay(500);
      }
      setNextStopAtMillis();
    }
    uint8_t getActive()
    {
      return FreezeDanceMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class PuzzleGame : public Modifier
{
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

    void Success()
    {
      PartOneSaved = false;
      PartTwoSaved = false;
#if defined DEBUG
      Serial.println(F("Puzzle right"));
#endif

      PlayMp3FolderTrack(256); //Toll gemacht! Das ist richtig.
      waitForTrackToFinish();
    }

    void Failure()
    {
      if (mode == 0)
      {
        PartOneSaved = false;
      }
      PartTwoSaved = false;

#if defined DEBUG
      Serial.println(F("Puzzle err"));
#endif

      PlayMp3FolderTrack(257);
      waitForTrackToFinish();
    }

    void CompareParts()
    {
      if ((PartOneSpecial == PartTwoSpecial) && (PartOneFolder == PartTwoFolder) && (PartOneSpecial2 == PartTwoSpecial2))
      {
        PartTwoSaved = false;
        return;
      }
      else if (((PartOneSpecial != PartTwoSpecial) || (PartOneFolder != PartTwoFolder)) && (PartOneSpecial2 == PartTwoSpecial2))
      {
        Success();
      }
      else
      {
        Failure();
      }
    }

  public:
    void loop()
    {
      if (PartOneSaved && PartTwoSaved)
      {
        if (!isPlaying())
        {
          CompareParts();
        }
      }
      if (myTrigger.resetTrack)
      {
#if defined DEBUG
        Serial.println(F("Puzzle del part"));
#endif
        mp3Pause();
        PlayMp3FolderTrack(298); //Okay, lass uns was anderes probieren.
        PartOneSaved = false;
        PartTwoSaved = false;
      }
    }

    PuzzleGame(uint8_t special)
    {
      mp3.loop();
#if defined DEBUG
      Serial.println(F("Puzzle"));
#endif
      mp3Pause();
      knownCard = false;
      activeShortCut = -1;

      PlayMp3FolderTrack(407); // intro
      waitForTrackToFinish();
      mode = special;
    }

    virtual bool handlePause()
    {
      return false;
    }

    virtual bool handleNext()
    {
      return true;
    }

    virtual bool handlePrevious()
    {
      return true;
    }

    virtual bool handleNextAction()
    {
      return true;
    }

    virtual bool handlePreviousAction()
    {
      return true;
    }

    virtual bool handleShutDown()
    {
      return true;
    }

    virtual bool handleShortCut(uint8_t shortCutNo)
    {
      return true;
    }

    virtual bool handleRFID(nfcTagObject *newCard)
    {

      //this->tmpCard = *newCard;
      if (newCard->nfcFolderSettings.mode != PuzzlePart)
      {
#if defined DEBUG
        Serial.println(F("Puzzle no valid part"));
#endif
        mp3Pause();
        PlayMp3FolderTrack(258); //Diese Karte ist nicht Teil des Spiels. Probiere eine Andere.
        return true;
      }
      else
      {
        if (!PartOneSaved)
        {
          PartOneSpecial = newCard->nfcFolderSettings.special;
          PartOneFolder = newCard->nfcFolderSettings.folder;
          PartOneSpecial2 = newCard->nfcFolderSettings.special2;
          PartOneSaved = true;
        }
        else if (PartOneSaved && !PartTwoSaved)
        {
          PartTwoSpecial = newCard->nfcFolderSettings.special;
          PartTwoFolder = newCard->nfcFolderSettings.folder;
          PartTwoSpecial2 = newCard->nfcFolderSettings.special2;
          PartTwoSaved = true;
        }
        return false;
      }
    }
    virtual bool handleStopWhenCardAway()
    {
      return true;
    }
    uint8_t getActive()
    {
      return PuzzleGameMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class QuizGame : public Modifier
{
  private:
    uint8_t PartOneSpecial = 0;
    uint8_t PartOneFolder = 0;
    uint8_t PartOneSpecial2 = 0;

    uint8_t PartTwoSpecial = 0;
    uint8_t PartTwoFolder = 0;
    uint8_t PartTwoSpecial2 = 0;

    bool PartOneSaved = false;
    bool PartTwoSaved = false;

    void Success()
    {
#if defined DEBUG
      Serial.println(F("Quiz right"));
#endif
      PlayMp3FolderTrack(256);
      waitForTrackToFinish();
      this->PartOneSaved = false;
      this->PartTwoSaved = false;
      next();
    }

    void Failure()
    {
      PartTwoSaved = false;
#if defined DEBUG
      Serial.println(F("Quiz err"));
#endif
      PlayMp3FolderTrack(257);
      waitForTrackToFinish();
    }

    void CompareParts()
    {
      if ((this->PartOneSpecial == this->PartTwoSpecial) && (this->PartOneFolder == this->PartTwoFolder) && (this->PartOneSpecial2 == this->PartTwoSpecial2))
      {
        PartTwoSaved = false;
        return;
      }
      else if (((this->PartOneSpecial != this->PartTwoSpecial) || (this->PartOneFolder != this->PartTwoFolder)) && (this->PartOneSpecial2 == this->PartTwoSpecial2))
      {
        Success();
      }
      else
      {
        Failure();
      }
    }

  public:
    void loop()
    {
      if (this->PartOneSaved && this->PartTwoSaved)
      {
        if (!isPlaying())
        {
          CompareParts();
        }
      }

      if (myTrigger.resetTrack)
      {
#if defined DEBUG
        Serial.println(F("Quiz del part"));
#endif
        mp3Pause();
        PlayMp3FolderTrack(298); //Okay, lass uns was anderes probieren.
        waitForTrackToFinish();
        this->PartOneSaved = false;
        this->PartTwoSaved = false;
        next();
      }
    }

    QuizGame(uint8_t special)
    {
      mp3.loop();
#if defined DEBUG
      Serial.println(F("Quiz"));
#endif
      mp3Pause();
      knownCard = false;
      activeShortCut = -1;

      PlayMp3FolderTrack(408); // intro
      waitForTrackToFinish();

      this->PartOneFolder = special;
      numTracksInFolder = mp3.getFolderTrackCount(this->PartOneFolder);
      firstTrack = 1;
      shuffleQueue();
      currentTrack = 0;

#if defined DEBUG
      Serial.println(F("Quiz Q set"));
#endif
      next();
    }

    void next()
    {
      //numTracksInFolder = mp3.getFolderTrackCount(this->PartOneFolder);

      if (currentTrack < numTracksInFolder)
      {
#if defined DEBUG
        Serial.println(F("Quiz next"));
#endif
        currentTrack++;
      }
      else
      {
#if defined DEBUG
        Serial.println(F("Quiz end of Q"));
#endif
        mp3Pause();
        PlayMp3FolderTrack(409);
        waitForTrackToFinish();
        RemoveModifier();
        return;
      }
      this->PartOneSaved = true;
      this->PartOneSpecial = queue[currentTrack - 1];
      this->PartOneSpecial2 = this->PartOneSpecial;
#if defined DEBUG
      Serial.print(F("Part "));
      Serial.println(this->PartOneSpecial);
#endif
      mp3.playFolderTrack(this->PartOneFolder, this->PartOneSpecial);
    }

    virtual bool handleNext()
    {
      return true;
    }

    virtual bool handlePrevious()
    {
      return true;
    }
    virtual bool handlePause()
    {
#if defined DEBUG
      Serial.println(F("Quiz pause repeat "));
#endif
      if (isPlaying())
      {
        mp3Pause();
      }
      else
      {
        mp3.playFolderTrack(this->PartOneFolder, this->PartOneSpecial);
      }
      return true;
    }

    virtual bool handleNextAction()
    {
      return true;
    }

    virtual bool handlePreviousAction()
    {
      return true;
    }

    virtual bool handleShutDown()
    {
      return true;
    }

    virtual bool handleShortCut(uint8_t shortCutNo)
    {
      return true;
    }

    virtual bool handleRFID(nfcTagObject *newCard)
    {

      if (newCard->nfcFolderSettings.mode != PuzzlePart)
      {
#if defined DEBUG
        Serial.println(F("Quiz no valid part"));
#endif
        mp3Pause();
        PlayMp3FolderTrack(258); //Diese Karte ist nicht Teil des Spiels. Probiere eine Andere.
        return true;
      }
      else
      {

        if (this->PartOneSaved && !this->PartTwoSaved)
        {
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
    virtual bool handleStopWhenCardAway()
    {
      return true;
    }
    uint8_t getActive()
    {
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
    PlayMp3FolderTrack(297);
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
      PlayMp3FolderTrack(298); //Okay, lass uns was anderes probieren.
        waitForTrackToFinish();
        this->PartOneSaved = false;
        next();
      }
      //Steuerung über Pause Button
      //2s Druck: Wiederholen der Questaufgabe
      if (pauseButton.pressedFor(LONGER_PRESS)) {
        if (pauseButton.wasReleased()) {
  #if defined DEBUG
          Serial.println(F("Quiz > pause and repeat "));
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
class ButtonSmash : public Modifier
{
  private:
    uint8_t Folder;

    nfcTagObject tmpCard;

  public:
    void loop()
    {
    }

    ButtonSmash(uint8_t special, uint8_t special2)
    {
      mp3.loop();
#if defined DEBUG
      Serial.println(F("Smash"));
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

    void next()
    {
      mp3Pause();
      numTracksInFolder = mp3.getFolderTrackCount(this->Folder);

      if (currentTrack != numTracksInFolder - firstTrack + 1)
      {
#if defined DEBUG
        Serial.println(F("Smash Q nxt"));
#endif
        currentTrack++;
      }
      else
      {
#if defined DEBUG
        Serial.println(F("Smash repeat Q"));
#endif
        firstTrack = 1;
        shuffleQueue();
        currentTrack = 1;
      }
      mp3.playFolderTrack(this->Folder, queue[currentTrack - 1]);
      waitForTrackToFinish();
    }

    virtual bool handleNext()
    {
      return true;
    }

    virtual bool handlePrevious()
    {
      return true;
    }

    virtual bool handlePause()
    {
      next();
      return true;
    }
    virtual bool handleNextAction()
    {
      next();
      return true;
    }
    virtual bool handlePreviousAction()
    {
      next();
      return true;
    }
    virtual bool handleVolumeUpAction()
    {
      next();
      return true;
    }
    virtual bool handleVolumeDownAction()
    {
      next();
      return true;
    }
    virtual bool handleShortCut(uint8_t shortCutNo)
    {
      next();
      return true;
    }
    virtual bool handleShutDown()
    {
      next();
      return true;
    }
    virtual bool handleAdminMenu()
    {
      next();
      return true;
    }
    virtual bool handleRFID(nfcTagObject *newCard)
    {
      next();
      return true;
    }
    virtual bool handleStopWhenCardAway()
    {
      return true;
    }
    uint8_t getActive()
    {
      return ButtonSmashMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class Calculate : public Modifier
{
  private:
    uint8_t mode = 1;       // learn mode (1 = addition, 2 = subtraction, 3 = multiplication, 4 = division, 5 = mixed)
    uint8_t upperBound = 0; // max number (can not exceed 254)
    uint8_t opA = 0;
    uint8_t opB = 0;
    int16_t answer = 0;
    uint8_t answerOld = 0;
    uint8_t result = 0;
    uint8_t opr = 0; // operator, see mode (0..3)
    bool setAnswer = false;
    bool invertVolumeButtonsMemory = false;

    void nextQuestion(bool repeat = false)
    {
#if defined DEBUG
      Serial.println(F("Calc nxt Question"));
#endif
      this->answer = 0;
      this->answerOld = 0;

      if (!repeat)
      {
        if (this->mode == 5)
        {
          this->opr = random(1, 5);
        }
        else
        {
          this->opr = this->mode;
        }
        this->result = 0;

        switch (this->opr)
        {
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
              do
              {
                this->opB = random(1, this->upperBound);
              } while (this->opA % this->opB != 0);
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

      switch (this->opr)
      {
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

      PlayMp3FolderTrack(411); // question "how much is"
      waitForTrackToFinish();
      PlayMp3FolderTrack(this->opA); // 1..254
      waitForTrackToFinish();
      PlayMp3FolderTrack(411 + this->opr); // 402, 403, 404, 405
      waitForTrackToFinish();
      PlayMp3FolderTrack(this->opB); // 1..254
      waitForTrackToFinish();
    }

    void readAnswer()
    {
      if (this->answer != this->answerOld)
      {
        this->answerOld = this->answer;
        PlayMp3FolderTrack(this->answer);
        waitForTrackToStart();
      }
    }

  public:
    void loop()
    {
      this->readAnswer();
      if (this->answer == -1)
      {
        this->answer = 0;
        this->answerOld = 0;
        PlayMp3FolderTrack(298); // next Question
        waitForTrackToFinish();
        this->nextQuestion();
        return;
      }
      else if (this->answer >= 1 && this->setAnswer)
      {
        setAnswer = false;
        if (this->result == this->answer)
        {
#if defined DEBUG
          Serial.println(F("Calc right"));
#endif

          PlayMp3FolderTrack(420); // richtig
          waitForTrackToFinish();
          this->nextQuestion();
        }
        else
        {
#if defined DEBUG
          Serial.println(F("Calc wrong"));
#endif

          PlayMp3FolderTrack(421); // falsch
          waitForTrackToFinish();
          this->nextQuestion(true); // repeat question
        }
      }
      return;
    }
    virtual bool handlePause()
    {
      if (this->answer != 0)
      {
        this->setAnswer = true;
#if defined DEBUG
        Serial.print(F("= "));
        Serial.println(this->answer);
#endif
        mp3Pause();
      }
      return true;
    }
    virtual bool handleNext()
    {
      return true;
    }
    virtual bool handlePrevious()
    {
      return true;
    }
    virtual bool handleNextAction()
    {
      this->answer++;
      this->answer = min(this->answer, this->upperBound);
      return true;
    }
    virtual bool handlePreviousAction()
    {
      this->answer--;
      this->answer = max(this->answer, 1);
      return true;
    }
    virtual bool handleShortCut(uint8_t shortCutNo)
    {
      return true;
    }
    virtual bool handleVolumeUpAction()
    {
      this->answer += 10;
      this->answer = min(this->answer, this->upperBound);
      return true;
    }
    virtual bool handleVolumeDownAction()
    {
      this->answer -= 10;
      this->answer = max(this->answer, 1);
      return true;
    }
    virtual bool handleShutDown()
    {
      this->answer = -1;
      return true;
    }
    virtual bool handleRFID(nfcTagObject *newCard)
    {
      return true;
    }
    virtual bool handleAdminMenu()
    {
      return true;
    }
    Calculate(uint8_t special, uint8_t special2)
    {
#if defined DEBUG
      Serial.println(F("Calc"));
#endif
      //delay(150); //Laufzeit Advert Modifier Jingle, nötig, damit laufender Track mit der folgenden Pause auch pausiert wird
      mp3.loop();
      knownCard = false;
      mp3Pause();
      this->invertVolumeButtonsMemory = mySettings.invertVolumeButtons;
      mySettings.invertVolumeButtons = false;
      this->mode = special;
      this->upperBound = special2;

      if (this->mode == 5)
      {
        this->opr = random(1, 5);
      }
      else
      {
        this->opr = this->mode;
      }
      //waitForTrackToFinish();
      PlayMp3FolderTrack(410); // intro
      waitForTrackToFinish();
      this->nextQuestion();
    }

    uint8_t getActive()
    {
      return CalculateMod;
    }
    virtual void deleteModifier()
    {
      mySettings.invertVolumeButtons = this->invertVolumeButtonsMemory;
      delete this;
    }
};
//////////////////////////////////////////////////////////////////////////
class Locked : public Modifier
{
  public:
    virtual bool handlePause()
    {
      return true;
    }
    virtual bool handleNextAction()
    {
      return true;
    }
    virtual bool handlePreviousAction()
    {
      return true;
    }
    virtual bool handleVolumeUpAction()
    {
      return true;
    }
    virtual bool handleVolumeDownAction()
    {
      return true;
    }
    virtual bool handleShutDown()
    {
      return true;
    }
    virtual bool handleShortCut(uint8_t shortCutNo)
    {
      return true;
    }
    virtual bool handleAdminMenu()
    {
      return true;
    }

    virtual bool handleRFID(nfcTagObject *newCard)
    {
      return true;
    }
    Locked(void)
    {
#if defined DEBUG
      Serial.println(F("Lckd Modifier"));
#endif
    }
    uint8_t getActive()
    {
      return LockedMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class ToddlerMode : public Modifier
{
  public:
    virtual bool handlePause()
    {
      return true;
    }
    virtual bool handleNextAction()
    {
      return true;
    }
    virtual bool handlePreviousAction()
    {
      return true;
    }
    virtual bool handleVolumeUpAction()
    {
      return true;
    }
    virtual bool handleVolumeDownAction()
    {
      return true;
    }
    virtual bool handleShutDown()
    {
      return true;
    }
    //    virtual bool handleShortCut(uint8_t shortCutNo) {
    //      return true;
    //    }
    virtual bool handleAdminMenu()
    {
      return true;
    }

    ToddlerMode()
    {
#if defined DEBUG
      Serial.println(F("ToddlerMode"));
#endif
    }

    uint8_t getActive()
    {
      return ToddlerModeMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class KindergardenMode : public Modifier
{
  private:
    nfcTagObject nextCard;
    bool cardQueued = false;

  public:
    virtual bool handleNext()
    {
#if defined DEBUG
      Serial.println(F("Kindergarden nxt"));
#endif
      if (this->cardQueued == true)
      {
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
    virtual bool handleNextAction()
    {
      return true;
    }
    virtual bool handlePreviousAction()
    {
      return true;
    }
    virtual bool handleShutDown()
    {
      return true;
    }
    virtual bool handleAdminMenu()
    {
      return true;
    }

    virtual bool handleShortCut(uint8_t shortCutNo)
    {
      if (isPlaying())
        return true;
      else
        return false;
    }

    virtual bool handleRFID(nfcTagObject *newCard)
    { // lot of work to do!
#if defined DEBUG
      Serial.println(F("Kindergarden RFID queued"));
#endif
      if (knownCard)
      {
        this->nextCard = *newCard;
        this->cardQueued = true;
        if (!isPlaying())
        {
          handleNext();
        }
      }
      else
      {
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
    KindergardenMode()
    {
#if defined DEBUG
      Serial.println(F("Kindergarden"));
#endif
    }
    uint8_t getActive()
    {
      return KindergardenModeMod;
    }
};
//////////////////////////////////////////////////////////////////////////
class RepeatSingleModifier : public Modifier
{
  public:
    virtual bool handleNext()
    {
#if defined DEBUG
      Serial.println(F("RepeatSingle repeat"));
#endif
      delay(50);
      if (isPlaying())
        return true;
      if (myFolder->mode == Party || myFolder->mode == Party_Section)
      {
        mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
      }
      else
      {
        mp3.playFolderTrack(myFolder->folder, currentTrack);
      }

      _lastTrackFinished = 0;
      return true;
    }
    RepeatSingleModifier()
    {
#if defined DEBUG
      Serial.println(F("RepeatSingle"));
#endif
    }
    uint8_t getActive()
    {
      return RepeatSingleMod;
    }
    virtual bool handleNextAction()
    {
      nextTrack(random(65536), true);
      return false;
    }
};
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
void SetVolume(uint8_t volume) {
  mp3.setVolume(volume);
  delay(100);
}

void PlayMp3FolderTrack(uint16_t Track)
{
  disablestandbyTimer();
  mp3.playMp3FolderTrack(Track);
}

void playFolder()
{
  // uint8_t counter = 0;
  bool queueTrack = false;

  mp3.loop();
  _lastTrackFinished = 0;
  numTracksInFolder = mp3.getFolderTrackCount(myFolder->folder);
  firstTrack = 1;

  switch (myFolder->mode)
  {
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
      if (myFolder->special3 > 0)
      {
        if (myFolder->special3 < numTracksInFolder)
        {
          currentTrack = myFolder->special3 + 1;
        }
        else
        {
          currentTrack = 1;
        }
        writeCardMemory(currentTrack);
      }
      else
      {
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
      if (myFolder->special3 > 0)
      {
        if (myFolder->special3 < myFolder->special2)
        {
          currentTrack = myFolder->special3 + 1;
        }
        else
        {
          currentTrack = myFolder->special;
        }
        writeCardMemory(currentTrack);
      }
      else
      {
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
      if (currentTrack == 0 || currentTrack > numTracksInFolder)
      {
        currentTrack = 1;
      }
      break;
    case AudioBook_Section:
#if defined DEBUG
      Serial.println(F("Audio Book section"));
#endif
      firstTrack = myFolder->special;
      currentTrack = myFolder->special3;
      if (currentTrack < firstTrack || currentTrack >= firstTrack + numTracksInFolder)
      {
        currentTrack = firstTrack;
#if defined DEBUG
        Serial.println(F("start from beginn"));
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

  disablestandbyTimer();

#ifdef DEBUG
  Serial.print(numTracksInFolder);
  Serial.print(F(" mp3 in folder "));
  Serial.println(myFolder->folder);
  Serial.print(F("play mp3 "));
#endif

  if (queueTrack)
  {
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
#ifdef DEBUG
    Serial.println(queue[currentTrack - 1]);
#endif
  }
  else
  {
    mp3.playFolderTrack(myFolder->folder, currentTrack);
#ifdef DEBUG
    Serial.println(currentTrack);
#endif
  }

  waitForTrackToStart();
#if defined DISPLAY  
  showTrackOnDisplay();
#endif  
}

void activateShortCut(uint8_t shortCutNo)
{
  trackToStoreOnCard = 0;
#if defined DEBUG || defined SHORTCUTS_PRINT
  Serial.print("play short cut no ");
  Serial.println(shortCutNo);
#endif
  if (shortCuts[shortCutNo].mode != 0)
  {
    knownCard = false;
    switch (shortCuts[shortCutNo].folder)
    {
      case ModifierMode:
        SetModifier(&shortCuts[shortCutNo]);
        break;
      default:
        if (activeModifier != NULL)
        {
          if (activeModifier->handleShortCut(shortCutNo) == true)
          {
#if defined DEBUG ^ defined SHORTCUTS_PRINT
            Serial.println(F("lckd"));
#endif
            return;
          }
        }
        myFolder = &shortCuts[shortCutNo];
        activeShortCut = shortCutNo;
        playFolder();
        break;
    }
  } /*else {
    mp3Pause();
  PlayMp3FolderTrack(297);
    waitForTrackToFinish();
    setupShortCut(shortCutNo);
  }*/
}
//////////////////////////////////////////////////////////////////////////
// Leider kann das Modul selbst keine Queue abspielen, daher müssen wir selbst die Queue verwalten
//////////////////////////////////////////////////////////////////////////
static void nextTrack(uint8_t track, bool force /* = false */)
{
  bool queueTrack = false;

#ifdef DEBUG
  Serial.println(F("nxt track"));
#endif
  if (!force && (track == _lastTrackFinished || (knownCard == false && activeShortCut < 0)))
  {
#ifdef DEBUG
    Serial.println(F("abort"));
#endif
    return;
  }
  _lastTrackFinished = track;

  if (activeModifier != NULL && !force)
  {
    if (activeModifier->handleNext() == true)
    {
#ifdef DEBUG
      Serial.println(F("lckd"));
#endif
      return;
    }
  }

  switch (myFolder->mode)
  {
    case AudioDrama:
    case AudioDrama_Section:
      resetCurrentCard();
      return;
      break;
    case Album:
      if (currentTrack < numTracksInFolder)
      {
        currentTrack = currentTrack + 1;
        if (myFolder->special3 > 0 && currentTrack != 0)
        {
          writeCardMemory(currentTrack);
        }
      }
      else
      {
        Serial.println(F("album end"));
        return;
      }
      break;
    case Album_Section:
      if (currentTrack < myFolder->special2)
      {
        currentTrack = currentTrack + 1;
        if (myFolder->special3 > 0 && currentTrack != 0)
        {
          writeCardMemory(currentTrack);
        }
      }
      else
      {
        Serial.println(F("album end"));
        return;
      }
      break;

    case Party:
      if (currentTrack < numTracksInFolder)
      {
        currentTrack = currentTrack + 1;
      }
      else
      {
        currentTrack = 1;
      }
      queueTrack = true;
      break;
    case Party_Section:
      if (currentTrack < (myFolder->special2 - firstTrack + 1))
      {
        currentTrack = currentTrack + 1;
      }
      else
      {
        currentTrack = 1;
      }
      queueTrack = true;
      break;

    case AudioBook:
      if (currentTrack < numTracksInFolder)
      {
        currentTrack = currentTrack + 1;
        writeCardMemory(currentTrack);
      }
      else
      {
        writeCardMemory(firstTrack, !isPlaying());    
        Serial.println(F("audiobook end"));
        return;
      }

      break;
    case AudioBook_Section:
      if (currentTrack < myFolder->special2)
      {
        currentTrack = currentTrack + 1;
        writeCardMemory(currentTrack);
      }
      else
      {
        writeCardMemory(firstTrack, !isPlaying());   
        Serial.println(F("audiobook end"));     
        return;
      }
      break;

    default:
#ifdef DEBUG
      Serial.println(F("no nxt track"));
#endif
      return;
      break;
  }
#if defined DEBUG
  Serial.print("track ");
#endif
  if (queueTrack)
  {
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
#if defined DEBUG
    Serial.println(queue[currentTrack - 1]);
#endif
  }
  else
  {
    mp3.playFolderTrack(myFolder->folder, currentTrack);
#if defined DEBUG
    Serial.println(currentTrack);
#endif
  }
#if defined DISPLAY
  showTrackOnDisplay();
#endif
}

static void previousTrack()
{
  bool queueTrack = false;

#ifdef DEBUG
  Serial.println(F("prev track"));
#endif

  if (knownCard == false && activeShortCut < 0)
  {
#ifdef DEBUG
    Serial.println(F("abort"));
#endif
    return;
  }

  if (activeModifier != NULL)
  {
    if (activeModifier->handlePrevious() == true)
    {
#ifdef DEBUG
      Serial.println(F("lckd"));
#endif
      return;
    }
  }

  _lastTrackFinished = 0;

  switch (myFolder->mode)
  {
    case Album:
    case Album_Section:
      if (currentTrack > firstTrack)
      {
        currentTrack = currentTrack - 1;
        if (myFolder->special3 > 0 && currentTrack != 0)
        {
          writeCardMemory(currentTrack);
        }
      }
      else
      {
        return;
        Serial.println(F("album start"));
      }

      break;

    case Party:
      if (currentTrack > 1)
      {
        currentTrack = currentTrack - 1;
      }
      else
      {
        currentTrack = numTracksInFolder;
      }
      queueTrack = true;
      break;
    case Party_Section:
      if (currentTrack > 1)
      {
        currentTrack = currentTrack - 1;
      }
      else
      {
        currentTrack = myFolder->special2 - firstTrack + 1;
      }
      queueTrack = true;
      break;

    case AudioBook:
      /*if (currentTrack > 1)
      {
        currentTrack = currentTrack - 1;
      }
      else
      {
        currentTrack = 1;
      }
      // Fortschritt im EEPROM abspeichern
      writeCardMemory(currentTrack);
      break;*/

    case AudioBook_Section:
      if (currentTrack > firstTrack)
      {
        currentTrack = currentTrack - 1;
      }
      else
      {
        currentTrack = firstTrack;
       Serial.println(F("audiobook start"));
      }
      // Fortschritt im EEPROM abspeichern
      writeCardMemory(currentTrack);
      break;

    default:
#if defined DEBUG
      Serial.println(F("no prev Track"));
#endif
      //setstandbyTimer();
      return;
      break;
  }

#if defined DEBUG
  Serial.print("track ");
#endif

  if (queueTrack)
  {
    mp3.playFolderTrack(myFolder->folder, queue[currentTrack - 1]);
#if defined DEBUG
    Serial.println(queue[currentTrack - 1]);
#endif
  }
  else
  {
    mp3.playFolderTrack(myFolder->folder, currentTrack);
#if defined DEBUG
    Serial.println(currentTrack);
#endif
  }
#if defined DISPLAY
  showTrackOnDisplay();
#endif
}
//////////////////////////////////////////////////////////////////////////
bool isPlaying()
{
  return !digitalRead(busyPin);
}

void mp3Pause(uint16_t delayTime /* = 100 */)
{
  mp3.pause();
  delay(delayTime);
}

void waitForTrackToFinish(bool interruptByTrigger /* = true */, uint16_t timeOut /* = 5000 */)
{
#if defined DFPLAYER_PRINT
  Serial.println(F("waitForTrackToFinish"));
#endif
  waitForTrackToStart(timeOut);
  while (isPlaying())
  {
    mp3.loop();
    readTrigger();
    if (myTrigger.pauseTrack && myTriggerEnable.pauseTrack)
    {
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

void waitForTrackToStart(uint16_t timeOut /* = 5000 */)
{
#if defined DFPLAYER_PRINT
  Serial.println(F("waitForTrackToStart"));
#endif
  unsigned long waitForTrackToStartMillis = millis();
  delay(100);
  while (!isPlaying())
  {
    mp3.loop();
    if (millis() - waitForTrackToStartMillis >= timeOut)
    {
#ifdef DFPLAYER_PRINT
      Serial.println(F("track not starting"));
#endif
      return;
    }
  }

#if defined DFPLAYER_PRINT
  Serial.println(F("track startet"));
#endif
}
//////////////////////////////////////////////////////////////////////////
void setup()
{
#if defined DEBUG || defined ANALOG_INPUT_PRINT || defined ROTARY_ENCODER_PRINT || defined SHORTCUTS_PRINT || defined QUEUE_PRINT || defined DFPLAYER_PRINT || defined IRREMOTE_PRINT
  Serial.begin(115200); // Es gibt ein paar Debug Ausgaben über die serielle Schnittstelle
  // Dieser Hinweis darf nicht entfernt werden
  Serial.println(F("Affenbox for AiO+ v2.20 by Marco Schulz"));
  Serial.println(F("forked from TonUINO by Thorsten Voß; licensed under GNU/GPL."));
  Serial.println(F("Information and contribution https://tonuino.de.\n"));
#endif

  // spannung einschalten
  pinMode(powerPin, OUTPUT);
  digitalWrite(powerPin, HIGH);

  // sd karten zugang aus
  pinMode(usbSwitchPin, OUTPUT);
  digitalWrite(usbSwitchPin, LOW);

  // verstärker aus
  pinMode(ampPin, OUTPUT);
  digitalWrite(ampPin, HIGH);

  mp3.begin();
  delay(500);

  // Busy Pin
  pinMode(busyPin, INPUT);

  // NFC Leser initialisieren
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
#if defined DEBUG
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader
#endif
  for (byte i = 0; i < 6; i++)
  {
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
#if defined buttonPower
  pinMode(buttonPower, INPUT_PULLUP);
#endif
  pinMode(buttonUp, INPUT_PULLUP);
  pinMode(buttonDown, INPUT_PULLUP);
#if defined FIVEBUTTONS
  pinMode(buttonFourPin, INPUT_PULLUP);
  pinMode(buttonFivePin, INPUT_PULLUP);
#endif

#if defined DISPLAY
  myDisplay.init();
  myDisplay.set(DISPLAY_BRIGHTNESS);
  myDisplay.point(0);
  //show "--" on display to give some feedback during startup
  myDisplay.display(1, 0x34);
  myDisplay.display(2, 0x34);
#endif

#if defined IRREMOTE
  IrReceiver.begin(IRREMOTE_PIN, DISABLE_LED_FEEDBACK);
#endif

#if defined POWER_ON_LED
  pinMode(POWER_ON_LED_PIN, OUTPUT);
  digitalWrite(POWER_ON_LED_PIN, LOW);
#endif

#if defined ANALOG_INPUT
  // Don't use internal pull-up resistor.
  pinMode(ANALOG_INPUT_PIN, INPUT);

  for (uint8_t i = 0; i < ANALOG_INPUT_BUTTON_COUNT; i++)
  {
    analogInputButtons[i].init(i);
    analogInputButtonsPointer[i] = &analogInputButtons[i];
  }
  analogInputButtonConfig = new LadderButtonConfig(ANALOG_INPUT_PIN, ANALOG_INPUT_BUTTON_COUNT + 1, ANALOG_INPUT_LEVELS /*mySettings.analogInUserValues*/, ANALOG_INPUT_BUTTON_COUNT, analogInputButtonsPointer);
  analogInputButtonConfig->setEventHandler(readAnalogIn);
  analogInputButtonConfig->setFeature(ButtonConfig::kFeatureClick);
  analogInputButtonConfig->setFeature(ButtonConfig::kFeatureLongPress);
#endif

  // Wert für randomSeed() erzeugen durch das mehrfache Sammeln von rauschenden LSBs eines offenen Analogeingangs
  uint32_t ADC_LSB;
  uint32_t ADCSeed;
  for (uint8_t i = 0; i < 128; i++)
  {
    ADC_LSB = analogRead(openAnalogPin) & 0x1;
    ADCSeed ^= ADC_LSB << (i % 32);
  }
  randomSeed(ADCSeed); // Zufallsgenerator initialisieren

  resetTriggerEnable();

  // RESET --- ALLE DREI KNÖPFE BEIM STARTEN GEDRÜCKT HALTEN > alle EINSTELLUNGEN werden gelöscht
  if (digitalRead(buttonPause) == LOW && digitalRead(buttonUp) == LOW &&
      digitalRead(buttonDown) == LOW)
  {
    Serial.println(F("del EEPROM"));
    for (uint16_t i = 0; i < EEPROM.length(); i++)
    {
      EEPROM.update(i, 0);
    }
    resetSettings();
    resetShortCuts();
  }
  // load settings & sort cuts from EEPROM
  getSettings();
  getShortCuts();

  delay(1500);

  //  verstärker an
  digitalWrite(ampPin, LOW);

#if defined POWER_ON_LED
  digitalWrite(POWER_ON_LED_PIN, HIGH);
#endif

  delay(500);

  mp3.loop();
  volume = mySettings.initVolume;
  SetVolume(volume);
  mp3.setEq((DfMp3_Eq)(mySettings.eq - 1));

  PlayMp3FolderTrack(264);
  waitForTrackToFinish();

  // activate standby timer
  //setstandbyTimer();
  sleepAtMillis = 0;

  // Set saved Modifier
  if (mySettings.savedModifier.mode != 0)
    SetModifier(&mySettings.savedModifier);
}
//////////////////////////////////////////////////////////////////////////
void readButtons(bool invertVolumeButtons = false)
{
  pauseButton.read();
  upButton.read();
  downButton.read();
#if defined FIVEBUTTONS
  buttonFour.read();
  buttonFive.read();
#endif
#if defined buttonPower
 powerButton.read();
 #endif
 
  myTrigger.cancel |= pauseButton.pressedFor(LONGER_PRESS) &&
                      !upButton.isPressed() && !downButton.isPressed();
  myTrigger.pauseTrack |= pauseButton.wasReleased() ;//&& myTriggerEnable.cancel;
  
#if defined buttonPower
  myTrigger.shutDown |= powerButton.pressedFor(LONGER_PRESS);
#else
  myTrigger.shutDown |= myTrigger.cancel;
#endif

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
  if (invertVolumeButtons)
  {
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
  }
  else
  {
    //3 Buttons nicht invertiert
#if not defined ROTARY_ENCODER
    myTrigger.volumeUp |= upButton.pressedFor(LONG_PRESS);     //&& myTriggerEnable.shortCutNo[0];
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

  myTrigger.adminMenu |= (pauseButton.pressedFor(LONG_PRESS) || upButton.pressedFor(LONG_PRESS) || downButton.pressedFor(LONG_PRESS)) &&
                         pauseButton.isPressed() && upButton.isPressed() && downButton.isPressed();
  myTrigger.resetTrack |= (upButton.pressedFor(LONGER_PRESS) || downButton.pressedFor(LONGER_PRESS)) && !pauseButton.isPressed() && upButton.isPressed() && downButton.isPressed();
}

#if defined IRREMOTE
void readIr()
{
  Enum_Trigger irRemoteEvent = NoTrigger;
  static unsigned long irRemoteOldMillis;

  if (IrReceiver.decode())
  {
#if defined IRREMOTE_PRINT
    IrReceiver.printIRResultShort(&Serial);
#endif
    for (uint8_t i = 0; i < irRemoteCodeCount; i++)
    {
      //if we have a match, temporally populate irRemoteEvent and break
      if (IrReceiver.decodedIRData.command == mySettings.irRemoteUserCodes[i])
      {
        irRemoteEvent = (Enum_Trigger)i;
        break;
      }
    }
    // if the inner loop had a match, populate inputEvent and break
    // ir remote key presses are debounced by 250ms
    if (irRemoteEvent != 0 && millis() - irRemoteOldMillis >= 250)
    {
#if defined IRREMOTE_PRINT
      Serial.print(F("IR debunced remote event: "));
      Serial.println(irRemoteEvent);
#endif
      irRemoteOldMillis = millis();
      switch (irRemoteEvent)
      {
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
        case ShutDownTrigger:
          myTrigger.shutDown |= true;
          break;
        case AdminMenuTrigger:
          myTrigger.adminMenu |= true;
          break;
        case ResetTrackTrigger:
          myTrigger.resetTrack |= true;
          break;
        default:
          if (irRemoteEvent >= ShortcutTrigger)
          {
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

void checkAceLadderButtons()
{
  static unsigned long prev = millis();

  // DO NOT USE delay(5) to do this.
  unsigned long now = millis();
  if (now - prev > 5)
  {
#if defined ANALOG_INPUT_PRINT_ANALOGREAD
    Serial.println(analogRead(ANALOG_INPUT_PIN));
#endif
    analogInputButtonConfig->checkButtons();
    prev = now;
  }
}

void readAnalogIn(AceButton *button, uint8_t eventType, uint8_t buttonState)
{
  int8_t buttonPin = button->getPin();

#if defined ANALOG_INPUT_PRINT
  Serial.print(F("getPin = "));
  Serial.println(buttonPin);
#endif
  switch (eventType)
  {
    case AceButton::kEventClicked:
      {
        switch (ANALOG_INPUT_BUTTON_MAP[buttonPin + 1])
        {
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
          case ShutDownTrigger:
            myTrigger.shutDown |= true;
            break;
          case AdminMenuTrigger:
            myTrigger.adminMenu |= true;
            break;
          case ResetTrackTrigger:
            myTrigger.resetTrack |= true;
            break;
          default:
            if (ANALOG_INPUT_BUTTON_MAP[buttonPin + 1] >= ShortcutTrigger)
            {
              myTrigger.shortCutNo[ANALOG_INPUT_BUTTON_MAP[buttonPin + 1] - ShortcutTrigger] |= true;
            }
            break;
        }
      }
  }
}
#endif

#if defined ROTARY_ENCODER
int8_t currentPosition = 0;

void RotEncSetVolume()
{
  int8_t newPosition = myEnc.read() / ROTARY_ENCODER_STEPS;

  if (newPosition != currentPosition)
  {
    if (newPosition > currentPosition)
    {
#if defined ROTARY_ENCODER_PRINT
      Serial.println(F("encoder direction clockwise"));
#endif
      myTrigger.volumeUp |= true;
    }
    else if (newPosition < currentPosition)
    {
#if defined ROTARY_ENCODER_PRINT
      Serial.println(F("encoder direction conterclockwise"));
#endif
      myTrigger.volumeDown |= true;
    }
    currentPosition = 0;
    myEnc.write(currentPosition);
  }
}
#endif

void readTrigger(bool invertVolumeButtons /* = false */)
{
  resetTrigger();

  readButtons(invertVolumeButtons);

#if defined IRREMOTE
  readIr();
#endif

#if defined ANALOG_INPUT
  checkAceLadderButtons();
#endif

#if defined ROTARY_ENCODER
  RotEncSetVolume();
#endif

  checkNoTrigger();

  if (myTrigger.noTrigger && myTriggerEnable.noTrigger)
  {
    resetTriggerEnable();
  }
  else
  {
    myTriggerEnable.noTrigger = true;
  }
}

void checkNoTrigger()
{
  myTrigger.noTrigger = !(myTrigger.pauseTrack || myTrigger.next || myTrigger.nextPlusTen || myTrigger.previous || myTrigger.previousPlusTen || myTrigger.volumeUp || myTrigger.volumeDown || myTrigger.pauseTrack || myTrigger.cancel || myTrigger.shutDown || myTrigger.adminMenu || myTrigger.resetTrack);

  for (uint8_t i = 0; i < availableShortCuts; i++)
  {
    myTrigger.noTrigger &= !myTrigger.shortCutNo[i];
  }
}

void resetTrigger()
{
  myTrigger.noTrigger = true;
  myTrigger.pauseTrack = false;
  myTrigger.next = false;
  myTrigger.nextPlusTen = false;
  myTrigger.previous = false;
  myTrigger.previousPlusTen = false;
  myTrigger.volumeUp = false;
  myTrigger.volumeDown = false;
  for (uint8_t i = 0; i < availableShortCuts; i++)
  {
    myTrigger.shortCutNo[i] = false;
  }
  myTrigger.cancel = false;
  myTrigger.shutDown = false;
  myTrigger.adminMenu = false;
  myTrigger.resetTrack = false;
}

void resetTriggerEnable()
{
  myTriggerEnable.noTrigger = false;
  myTriggerEnable.pauseTrack = true;
  myTriggerEnable.next = true;
  myTriggerEnable.nextPlusTen = true;
  myTriggerEnable.previous = true;
  myTriggerEnable.previousPlusTen = true;
  myTriggerEnable.volumeUp = true;
  myTriggerEnable.volumeDown = true;
  for (uint8_t i = 0; i < availableShortCuts; i++)
  {
    myTriggerEnable.shortCutNo[i] = true;
  }
  myTriggerEnable.cancel = true;
  myTriggerEnable.shutDown = true;
  myTriggerEnable.adminMenu = true;
  myTriggerEnable.resetTrack = true;
}
//////////////////////////////////////////////////////////////////////////
void volumeUpAction(bool rapidFire /* = false */)
{
  //Wenn Volume auf kurzem Tastendruck bei 3 Tasten liegt, kann sonst kein next/previous ausgefürt werden
  if (/*mySettings.invertVolumeButtons == 1 && */ myTriggerEnable.volumeUp == true || rapidFire)
  {
    myTriggerEnable.volumeUp = false || rapidFire;
    if (activeModifier != NULL)
    {
      if (activeModifier->handleVolumeUpAction() == true)
      {
#if defined DEBUG
        Serial.println(F("vol act lckd"));
#endif
        return;
      }
    }
    if (isPlaying())
    {
      if (volume < mySettings.maxVolume)
      {
        mp3.increaseVolume();
        delay(100);
        volume++;
        if (rapidFire)
        {
          delay(75);
        }
      }
#if defined DISPLAY
      showVolumeOnDisplay(volume);
#endif
#if defined DEBUG
      Serial.print(F("vol up "));
      Serial.println(volume);
#endif
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void volumeDownAction(bool rapidFire /* = false */)
{
  //Wenn Volume auf kurzem Tastendruck bei 3 Tasten liegt, kann sonst kein next/previous ausgefürt werden
  if (/*mySettings.invertVolumeButtons == 1 && */ myTriggerEnable.volumeDown == true || rapidFire)
  {
    myTriggerEnable.volumeDown = false || rapidFire;
    if (activeModifier != NULL)
    {
      if (activeModifier->handleVolumeDownAction() == true)
      {
#if defined DEBUG
        Serial.println(F("vol act lckd"));
#endif
        return;
      }
    }
    if (isPlaying())
    {
      if (volume > mySettings.minVolume && volume > 1)
      {
        mp3.decreaseVolume();
        delay(100);
        volume--;
        if (rapidFire)
        {
          delay(75);
        }
      }
#if defined DISPLAY
      showVolumeOnDisplay(volume);
#endif
#if defined DEBUG
      Serial.print(F("vol down "));
      Serial.println(volume);
#endif
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void nextAction()
{
  if (myTriggerEnable.next == true)
  {
    myTriggerEnable.next = false;
    if (activeModifier != NULL)
    {
      if (activeModifier->handleNextAction() == true)
      {
#if defined DEBUG
        Serial.println(F("nxt act lckd"));
#endif
        return;
      }
    }
    if (isPlaying())
    {
      nextTrack(random(65536));
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void previousAction()
{
  if (myTriggerEnable.previous == true)
  {
    myTriggerEnable.previous = false;
    if (activeModifier != NULL)
    {
      if (activeModifier->handlePreviousAction() == true)
      {
#if defined DEBUG
        Serial.println(F("prev act lckd"));
#endif
        return;
      }
    }
    if (isPlaying())
    {
      previousTrack();
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void pauseAction(bool fromCard /* = false */)
{
  if (myTriggerEnable.pauseTrack == true)
  {
    myTriggerEnable.pauseTrack = false;
    if (activeModifier != NULL)
    {
      if (activeModifier->handlePause() == true && !fromCard)
      {
#if defined DEBUG
        Serial.println(F("pause act lckd"));
#endif
        return;
      }
    }
    if (isPlaying())
    {
#if defined DEBUG
      Serial.println(F("pause"));
#endif
      checkForUnwrittenTrack();
      mp3Pause();
      //setstandbyTimer();
    }
    else if (knownCard || activeShortCut >= 0)
    {
#if defined DEBUG
      Serial.println(F("play"));
#endif
      mp3.start();
      //disablestandbyTimer();
    }
  }
}

//////////////////////////////////////////////////////////////////////////
void shutDownAction()
{  
  if (myTriggerEnable.shutDown == true)
  {
    myTriggerEnable.shutDown = false;
    if (activeModifier != NULL)
    {
      if (activeModifier->handleShutDown() == true)
      {
#if defined DEBUG
        Serial.println(F("shut down act lckd"));
#endif
        return;
      }
    }
    shutDown();
  }
}

//////////////////////////////////////////////////////////////////////////
void shortCutAction(uint8_t shortCutNo)
{
  if (myTriggerEnable.shortCutNo[shortCutNo] == true)
  {
    myTriggerEnable.shortCutNo[shortCutNo] = false;
    if (activeModifier != NULL)
    {
      if (activeModifier->handleShortCut(shortCutNo) == true)
      {
#if defined DEBUG ^ defined SHORTCUTS_PRINT
        Serial.println(F("short cut act lckd"));
#endif
        return;
      }
    }
    if (!isPlaying())
    {
      activateShortCut(shortCutNo);
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void adminMenuAction()
{
  if (myTriggerEnable.adminMenu == true)
  {
    myTriggerEnable.adminMenu = false;
    if (activeModifier != NULL)
    {
      if (activeModifier->handleAdminMenu() == true)
      {
#if defined DEBUG
        Serial.println(F("admin menu act lckd"));
#endif
        return;
      }
    }
    adminMenu();
  }
}
//////////////////////////////////////////////////////////////////////////
void loop()
{
  //19.8.22 Change order of mp3.loop and checkStanbyAtMillis to prevent unexpected shutdowns
  mp3.loop();
  checkStandbyAtMillis();  

#if defined FADING_LED
  fadeStatusLed(isPlaying());
#endif

#if defined DISPLAY
  if(volumeIsShown && millis() > volumeTimer){
    //switch back from volume to track
    volumeIsShown = false;
    showTrackOnDisplay();
  }
#endif

  readTrigger(mySettings.invertVolumeButtons);

  if (activeModifier != NULL)
  {
    activeModifier->loop();
  }

  // admin menu
  if (myTrigger.adminMenu)
  {
    myTriggerEnable.adminMenu = false;
    mp3Pause();
    do
    {
      readTrigger();
      if (myTrigger.noTrigger && myTriggerEnable.noTrigger)
      {
        myTriggerEnable.noTrigger = false;
        break;
      }

    } while (true);
    adminMenuAction();
  }

  //Springe zum ersten Titel zurück
  if (myTrigger.resetTrack && myTriggerEnable.resetTrack)
  {
    myTriggerEnable.resetTrack = false;
    mp3Pause();
    do
    {
      readTrigger();
      if (myTrigger.noTrigger && myTriggerEnable.noTrigger)
      {
        myTriggerEnable.noTrigger = false;
        break;
      }

    } while (true);
#if defined DEBUG
    Serial.println(F("rset tracks"));
#endif
    if (currentTrack != firstTrack)
    {
      currentTrack = firstTrack;
      if (myFolder->special3 != 0)
      {
#if defined DEBUG
        Serial.println(F("rset mem"));
#endif
        writeCardMemory(currentTrack);
      }
    }
    playFolder();
  }

  if (myTrigger.pauseTrack)
  {
    pauseAction();
  }

  if (myTrigger.shutDown)
  {
    shutDownAction();
  }

  if (myTrigger.volumeUp)
  {
#if defined FIVEBUTTONS || defined ROTARY_ENCODER
    volumeUpAction(false);
#else
    volumeUpAction(!mySettings.invertVolumeButtons);
#endif
  }

  if (myTrigger.volumeDown)
  {
#if defined FIVEBUTTONS || defined ROTARY_ENCODER
    volumeDownAction(false);
#else
    volumeDownAction(!mySettings.invertVolumeButtons);
#endif
  }

  if (myTrigger.next)
  {
    nextAction();
  }

  if (myTrigger.previous)
  {
    previousAction();
  }

  for (uint8_t i = 0; i < availableShortCuts; i++)
  {
    if (myTrigger.shortCutNo[i])
    {
      shortCutAction(i);
    }
  }

  handleCardReader();
}
//////////////////////////////////////////////////////////////////////////
void adminMenu(bool fromCard /* = false */)
{
  uint8_t subMenu = 0;
#if defined DEBUG
  Serial.println(F("adminMenu"));
#endif
  resetCurrentCard();

  mp3Pause();
  disablestandbyTimer();

  if (fromCard == false)
  {
    // Admin menu has been locked - it still can be trigged via admin card
    if (mySettings.adminMenuLocked == 1)
    {
      return;
    }
  }

  do
  {
    subMenu = voiceMenu(13, 900, 900, false, false, 0);
    if (subMenu == Exit)
    {
      writeSettings();
      waitForTrackToFinish();
      return;
    }
    else if (subMenu == ResetCard)
    {
      resetCard();
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
    else if (subMenu == MaxVolume)
    {
      // Maximum Volume
      mySettings.maxVolume = voiceMenu(30 - mySettings.minVolume, 930, mySettings.minVolume, false, false, mySettings.maxVolume - mySettings.minVolume) + mySettings.minVolume;
    }
    else if (subMenu == MinVolume)
    {
      // Minimum Volume
      mySettings.minVolume = voiceMenu(mySettings.maxVolume - 1, 931, 0, false, false, mySettings.minVolume);
    }
    else if (subMenu == InitVolume)
    {
      // Initial Volume
      mySettings.initVolume = voiceMenu(mySettings.maxVolume - mySettings.minVolume + 1, 932, mySettings.minVolume - 1, false, false, mySettings.initVolume - mySettings.minVolume + 1) + mySettings.minVolume - 1;
    }
    else if (subMenu == EQ)
    {
      // EQ
      mySettings.eq = voiceMenu(6, 920, 920);
      mp3.setEq((DfMp3_Eq)(mySettings.eq - 1));
    }
    else if (subMenu == SetupShortCuts)
    {
      setupShortCut();
    }
    else if (subMenu == SetupStandbyTimer)
    {
      switch (voiceMenu(5, 960, 960))
      {
        case 1:
          mySettings.standbyTimer = 5;
          break;
        case 2:
          mySettings.standbyTimer = 15;
          break;
        case 3:
          mySettings.standbyTimer = 30;
          break;
        case 4:
          mySettings.standbyTimer = 60;
          break;
        case 5:
          mySettings.standbyTimer = 0;
          break;
      }
    }
    else if (subMenu == CreateFolderCards)
    {
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

      PlayMp3FolderTrack(936);
      waitForTrackToFinish();
      for (uint8_t x = special; x <= special2; x++)
      {
        PlayMp3FolderTrack(x);
        tempCard.nfcFolderSettings.special = x;
#if defined DEBUG
        Serial.print(x);
        Serial.println(F(" place Tag"));
#endif
        do
        {
          readTrigger();
          if (myTrigger.cancel)
          {
#if defined DEBUG
            Serial.println(F("abort!"));
#endif
            myTriggerEnable.cancel = false;
            PlayMp3FolderTrack(802);
            return;
          }
        } while (!mfrc522.PICC_IsNewCardPresent());

        // RFID Karte wurde aufgelegt
        if (mfrc522.PICC_ReadCardSerial())
        {
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
    else if (subMenu == InvertButtons)
    {
      // Invert Functions for Up/Down Buttons
#if defined FIVEBUTTONS || defined ROTARY_ENCODER
      PlayMp3FolderTrack(992);
      waitForTrackToFinish();
#else
      switch (voiceMenu(2, 989, 989, false))
      {
        case 1:
          mySettings.invertVolumeButtons = false;
          break;
        case 2:
          mySettings.invertVolumeButtons = true;
          break;
      }
#endif
    }
    else if (subMenu == StopWhenCardAway)
    {
      switch (voiceMenu(2, 937, 933, false))
      {
        case 1:
          mySettings.stopWhenCardAway = false;
          break;
        case 2:
          mySettings.stopWhenCardAway = true;
          break;
      }
    }
    else if (subMenu == SetupIRRemote)
    {
#if defined IRREMOTE
#if defined IRREMOTE_PRINT
      Serial.println(F("IRRemote start learning"));
#endif
      if (voiceMenu(2, 826, 933, false, false, 0) == 2)
      {
#if defined IRREMOTE_PRINT
        Serial.println(F("IRRemote reset settings"));
#endif
        for (uint8_t i = 0; i < sizeOfInputTrigger; i++)
        {
          mySettings.irRemoteUserCodes[i] = 0;
        }
        PlayMp3FolderTrack(999);
        waitForTrackToFinish();
      }
      mp3Pause();
      PlayMp3FolderTrack(832);
      waitForTrackToFinish();
      bool exitWhile = true;
      uint8_t triggerNo = 0;
      do
      {
        mp3.loop();

        triggerNo = voiceMenu((sizeOfInputTrigger - availableShortCuts) + 1, 834, 834, false, false, 0);
        if (triggerNo == 0)
        {
          exitWhile = false;
          break;
        }
        else if (triggerNo >= (sizeOfInputTrigger - availableShortCuts) + 1)
        {
          triggerNo += voiceMenu(availableShortCuts, 940, 0) - 1;
          if (triggerNo < availableShortCuts)
          {
            exitWhile = false;
            break;
          }
        }
#if defined IRREMOTE_PRINT
        Serial.print(F("IRRemote set trigger no.: "));
        Serial.println(triggerNo);
#endif
        mp3Pause();
        PlayMp3FolderTrack(833);
        // clear ir receive buffer
        IrReceiver.resume();
        // wait for ir signal
        while (!IrReceiver.decode())
        {
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
      PlayMp3FolderTrack(831);
      waitForTrackToFinish();
#endif
    }
    else if (subMenu == ResetEEPROM)
    {
      if (voiceMenu(2, 825, 933, false, false, 0) == 2)
      {

        for (uint16_t i = 0; i < EEPROM.length(); i++)
        {
          EEPROM.update(i, 0);
        }
        resetSettings();
        PlayMp3FolderTrack(999);
      }
    }
    else if (subMenu == LockAdminMenu)
    {
      switch (voiceMenu(2, 986, 986))
      {
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

}
//////////////////////////////////////////////////////////////////////////
uint8_t voiceMenu(int16_t numberOfOptions, uint16_t startMessage, int messageOffset,
                  bool preview /* = false */, uint8_t previewFromFolder /* = 0 */, int defaultValue /* = 0 */,
                  bool enableSkipTen /* = false */)
{
  int16_t returnValue = -1;
  int16_t currentValue = defaultValue;
  int16_t currentValueOld = currentValue;

  disablestandbyTimer();
  mp3Pause();

  if (startMessage != 0)
  {
    PlayMp3FolderTrack(startMessage);
    waitForTrackToStart();
  }

#if defined DEBUG
  Serial.print(F("voiceMenu "));
  Serial.print(numberOfOptions);
  Serial.println(F(" Opt"));
#endif

  do
  {
    readTrigger();
    mp3.loop();

    if (myTrigger.nextPlusTen && numberOfOptions > 10 && enableSkipTen && myTriggerEnable.nextPlusTen)
    {
      myTriggerEnable.nextPlusTen = false;
      currentValue += 10;
      if (currentValue == 0)
      {
        currentValue = 1;
      }
    }
    if (myTrigger.next && myTriggerEnable.next)
    {
      myTriggerEnable.next = false;
      currentValue++;
      if (currentValue == 0)
      {
        currentValue = 1;
      }
    }
    if (currentValue > numberOfOptions)
    {
      currentValue = currentValue - numberOfOptions;
    }

    if (myTrigger.previousPlusTen && numberOfOptions > 10 && enableSkipTen && myTriggerEnable.previousPlusTen)
    {
      myTriggerEnable.previousPlusTen = false;
      currentValue -= 10;
      if (currentValue == 0)
      {
        currentValue = -1;
      }
    }
    if (myTrigger.previous && myTriggerEnable.previous)
    {
      myTriggerEnable.previous = false;
      currentValue--;
      if (currentValue == 0)
      {
        currentValue = -1;
      }
    }
    if (currentValue == -1)
    {
      currentValue = numberOfOptions;
    }
    else if (currentValue < -1)
    {
      currentValue = numberOfOptions + currentValue;
    }

    if (currentValue != currentValueOld)
    {
      currentValueOld = currentValue;
#if defined DEBUG
      Serial.println(currentValue);
#endif
      PlayMp3FolderTrack(messageOffset + currentValue);
      if (preview)
      {
        waitForTrackToFinish(); // mit preview Track (in der Regel Nummer) komplett Spielen, da es sonst zu abgehakten Anssagen kommt.
        if (previewFromFolder == 0)
        {
          mp3.playFolderTrack(currentValue, 1);
        }
        else
        {
          mp3.playFolderTrack(previewFromFolder, currentValue);
        }
      }
      else
      {
        waitForTrackToStart(); // ohne preview Track nur anspielen um Menü flüssiger zu gestallten
      }
    }

    if (myTrigger.cancel && myTriggerEnable.cancel)
    {
      myTriggerEnable.cancel = false;

      PlayMp3FolderTrack(802);
      waitForTrackToFinish();
      returnValue = defaultValue;
    }
    if (myTrigger.pauseTrack && myTriggerEnable.pauseTrack)
    {
      myTriggerEnable.pauseTrack = false;
      if (currentValue != 0)
      {
        returnValue = currentValue;
      }
    }

#if defined DEBUG
    if (Serial.available() > 0)
    {
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
bool setupFolder(folderSettings *theFolder)
{
  uint8_t enableMemory = 0;

  theFolder->folder = 0;
  theFolder->mode = 0;
  theFolder->special = 0;
  theFolder->special2 = 0;
  theFolder->special3 = 0;

  // Ordner abfragen
  theFolder->folder = voiceMenu(99, 301, 0, true, 0, 0, true);
  if (theFolder->folder == 0)
    return false;

  // Wiedergabemodus abfragen
  theFolder->mode = voiceMenu(11, 305, 305);
  if (theFolder->mode == 0)
    return false;
#if defined DEBUG
  Serial.println(F("Setup fldr"));
  Serial.print(F("Fldr: "));
  Serial.println(theFolder->folder);
  Serial.print(F("Mode: "));
  Serial.println(theFolder->mode);
#endif
  //  // Hörbuchmodus > Fortschritt im EEPROM auf 1 setzen
  //  writeCardMemory (myFolder->folder,myFolder->special3, 1);

  switch (theFolder->mode)
  {
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
      if (theFolder->mode == Album || theFolder->mode == Album_Section)
      {
        enableMemory = voiceMenu(2, 983, 933, false, false, 0) - 1;
        if (enableMemory == 1)
        {
          theFolder->special3 = 255; // Muss mit letztem möglichen Track initialisert werden, da bei start der Karte, der Track um eins erhöht wird. So kann nie Track 1 gespielt werden.
        }
      }
      else if (theFolder->mode == AudioBook || theFolder->mode == AudioBook_Section)
      {
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
  Serial.println(F("Setup ok"));
#endif
  return true;
}
//////////////////////////////////////////////////////////////////////////
void setupShortCut(int8_t shortCutNo /* = -1 */)
{
  bool returnValue = true;
  mp3Pause();

  if (shortCutNo == -1)
  {
    shortCutNo = voiceMenu(availableShortCuts, 940, 0);
  }
  switch (voiceMenu(2, 941, 941))
  {
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
  if (returnValue)
  {
    PlayMp3FolderTrack(400);
    waitForTrackToStart();
    writeShortCuts();
  }
}

void setupCard()
{
  bool returnValue = true;
  mp3Pause();

  nfcTagObject newCard;

  switch (voiceMenu(2, 941, 941))
  {
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
  if (returnValue)
  {
    // Karte ist konfiguriert > speichern
    mp3Pause();
    resetCurrentCard();
    writeCard(newCard);
  }
}

bool readCard(nfcTagObject *nfcTag)
{
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
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K))
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
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL)
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the tempCard

    // Authenticate using key A
#if defined DEBUG
    Serial.println(F("Auth. MIFARE UL"));
#endif
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK)
  {
#if defined DEBUG
    Serial.print(F("PCD_Auth. failed "));
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    return false;
  }

  // Show the whole sector as it currently is
  // Serial.println(F("Data in sect:"));
  // mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  // Serial.println();

  // Read data from the block
  if ((piccType == MFRC522::PICC_TYPE_MIFARE_MINI) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_1K) ||
      (piccType == MFRC522::PICC_TYPE_MIFARE_4K))
  {
#if defined DEBUG
    Serial.print(F("Read block"));
    Serial.println(blockAddr);
#endif
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK)
    {
#if defined DEBUG
      Serial.print(F("MIFARE_Read1 err "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
  }
  else if (piccType == MFRC522::PICC_TYPE_MIFARE_UL)
  {
    byte buffer2[18];
    byte size2 = sizeof(buffer2);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(8, buffer2, &size2);
    if (status != MFRC522::STATUS_OK)
    {
#if defined DEBUG
      Serial.print(F("MIFARE_Read err "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(9, buffer2, &size2);
    if (status != MFRC522::STATUS_OK)
    {
#if defined DEBUG
      Serial.print(F("MIFARE_Read2 err "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 4, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(10, buffer2, &size2);
    if (status != MFRC522::STATUS_OK)
    {
#if defined DEBUG
      Serial.print(F("MIFARE_Read3 err "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 8, buffer2, 4);

    status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(11, buffer2, &size2);
    if (status != MFRC522::STATUS_OK)
    {
#if defined DEBUG
      Serial.print(F("MIFARE_Read4 err "));
      Serial.println(mfrc522.GetStatusCodeName(status));
#endif
      return false;
    }
    memcpy(buffer + 12, buffer2, 4);
  }

#if defined DEBUG
  Serial.println(F("data on card "));
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
  if (tempCard.cookie == cardCookie)
  {

    if (activeModifier != NULL && tempCard.nfcFolderSettings.folder != 0)
    {
      if (activeModifier->handleRFID(&tempCard) == true)
      {
#if defined DEBUG
        Serial.println(F("RFID lckd"));
#endif
        return false;
      }
    }
    if (tempCard.nfcFolderSettings.folder == 0)
      return SetModifier(&tempCard.nfcFolderSettings);
    else
    {
      memcpy(nfcTag, &tempCard, sizeof(nfcTagObject));
#if defined DEBUG
      Serial.println(nfcTag->nfcFolderSettings.folder);
#endif
      myFolder = &nfcTag->nfcFolderSettings;
#if defined DEBUG
      Serial.println(myFolder->folder);
#endif
    }
    return true;
  }
  else
  {
    memcpy(nfcTag, &tempCard, sizeof(nfcTagObject));
    return true;
  }
}

void resetCard()
{
  PlayMp3FolderTrack(800);
  do
  {
    readTrigger();
    if (myTrigger.cancel)
    {
#if defined DEBUG
      Serial.print(F("abort"));
#endif
      myTriggerEnable.cancel = false;
      PlayMp3FolderTrack(802);
      return;
    }
  } while (!mfrc522.PICC_IsNewCardPresent());

  if (!mfrc522.PICC_ReadCardSerial())
    return;

#if defined DEBUG
  Serial.print(F("rset tag"));
#endif
  setupCard();
}
//////////////////////////////////////////////////////////////////////////
void writeCard(nfcTagObject nfcTag, int8_t writeBlock /* = -1 */, bool feedback /* = true */)
{
  MFRC522::PICC_Type mifareType;
  byte buffer[16] = {0x13, 0x37, 0xb3, 0x47, // 0x1337 0xb347 magic cookie to
                     // identify our nfc tags
                     0x02,                             // version 1
                     nfcTag.nfcFolderSettings.folder,  // the folder picked by the user
                     nfcTag.nfcFolderSettings.mode,    // the playback mode picked by the user
                     nfcTag.nfcFolderSettings.special, // track or function for admin cards
                     nfcTag.nfcFolderSettings.special2,
                     nfcTag.nfcFolderSettings.special3,
                     nfcTag.nfcFolderSettings.special4,
                     0x00, 0x00, 0x00, 0x00, 0x00
                    };

  // byte size = sizeof(buffer);

#if defined DEBUG
  Serial.println(F("write Card"));
#endif
  mifareType = mfrc522.PICC_GetType(mfrc522.uid.sak);

  // Authenticate using key B
  //authentificate with the card and set card specific parameters
  if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_1K) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_4K))
  {
#if defined DEBUG
    Serial.println(F("Auth again w. key A"));
#endif
    status = mfrc522.PCD_Authenticate(
               MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  }
  else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL)
  {
    byte pACK[] = {0, 0}; //16 bit PassWord ACK returned by the NFCtag

    // Authenticate using key A
#if defined DEBUG
    Serial.println(F("Auth UL"));
#endif
    status = mfrc522.PCD_NTAG216_AUTH(key.keyByte, pACK);
  }

  if (status != MFRC522::STATUS_OK)
  {
#if defined DEBUG
    Serial.print(F("PCDAuth err "));
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    if (feedback)
    {
      PlayMp3FolderTrack(401);
      waitForTrackToFinish();
    }
    return;
  }

  // Write data to the block
#if defined DEBUG
  Serial.print(F("write data "));
  Serial.print(blockAddr);
  Serial.println(F("..."));
  dump_byte_array(buffer, 16);
  Serial.println();
#endif

  if ((mifareType == MFRC522::PICC_TYPE_MIFARE_MINI) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_1K) ||
      (mifareType == MFRC522::PICC_TYPE_MIFARE_4K))
  {
    status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(blockAddr, buffer, 16);
  }
  else if (mifareType == MFRC522::PICC_TYPE_MIFARE_UL)
  {
    byte buffer2[16];
    byte size2 = sizeof(buffer2);
    if (writeBlock == -1)
    {
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
    }
    else if (writeBlock <= 11 && writeBlock >= 8)
    {
      memset(buffer2, 0, size2);
      memcpy(buffer2, buffer, 4);
      status = (MFRC522::StatusCode)mfrc522.MIFARE_Write(writeBlock, buffer2, 16);
    }
  }

  if (status != MFRC522::STATUS_OK)
  {
#if defined DEBUG
    Serial.print(F("MIFAREWrite err "));
    Serial.println(mfrc522.GetStatusCodeName(status));
#endif
    if (feedback)
    {
      PlayMp3FolderTrack(401);
    }
  }
  else if (feedback)
  {
    PlayMp3FolderTrack(400);
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
void dump_byte_array(byte *buffer, byte bufferSize)
{

  for (byte i = 0; i < bufferSize; i++)
  {

    Serial.print(buffer[i] < 0x10 ? "0" : " ");
    Serial.print(buffer[i], HEX);
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

bool setupModifier(folderSettings *tmpFolderSettings)
{

  tmpFolderSettings->folder = ModifierMode;
  tmpFolderSettings->mode = voiceMenu(10, 966, 966, false, 0, 0, true);

  if (tmpFolderSettings->mode != ModifierMode)
  {
    if (tmpFolderSettings->mode == SleepTimerMod)
    {
      switch (voiceMenu(4, 960, 960))
      {
        case 1:
          tmpFolderSettings->special = 5;
          break;
        case 2:
          tmpFolderSettings->special = 15;
          break;
        case 3:
          tmpFolderSettings->special = 30;
          break;
        case 4:
          tmpFolderSettings->special = 60;
          break;
      }
      tmpFolderSettings->special2 = 0x00;
    }
    else if (tmpFolderSettings->mode == PuzzleGameMod)
    {
      //Save first part?
      tmpFolderSettings->special = voiceMenu(2, 979, 933) - 1;
      tmpFolderSettings->special2 = 0x00;
    }
    else if (tmpFolderSettings->mode == QuizGameMod)
    {
      //Set Folder
      tmpFolderSettings->special = voiceMenu(99, 301, 0, true, 0, 0, true);
      tmpFolderSettings->special2 = 0x00;
    }
    else if (tmpFolderSettings->mode == ButtonSmashMod)
    {
      //Set Folder
      tmpFolderSettings->special = voiceMenu(99, 301, 0, true, 0, 0, true);
      //Set Volume
      tmpFolderSettings->special2 = voiceMenu(30, 904, false, false, 0, volume);
    }
    else if (tmpFolderSettings->mode == CalculateMod)
    {
      //Set Oprator
      tmpFolderSettings->special = voiceMenu(5, 423, 423, false, 0, 0, true);
      //Set max number
      tmpFolderSettings->special2 = voiceMenu(255, 429, 0, false, 0, 0, true);
    }

    //Save Modifier in EEPROM?
    tmpFolderSettings->special3 = voiceMenu(2, 980, 933, false, false, 0) - 1;
    tmpFolderSettings->special4 = 0x00;
    return true;
  }
  return false;
}

bool SetModifier(folderSettings *tmpFolderSettings)
{
  if (activeModifier != NULL)
  {
    if (activeModifier->getActive() == tmpFolderSettings->mode)
    {
      return RemoveModifier();
    }
  }

#if defined DEBUG
  Serial.print(F("set modifier: "));
  Serial.println(tmpFolderSettings->mode);
#endif

  if (tmpFolderSettings->mode != 0 && tmpFolderSettings->mode != AdminMenuMod)
  {
    if (isPlaying())
    {
      mp3.playAdvertisement(260);
    }
    else
    {
      PlayMp3FolderTrack(260);
    }
    waitForTrackToFinish();
  }

  switch (tmpFolderSettings->mode)
  {
    case 0:
    case AdminMenuMod:
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      adminMenu(true);
      break;
    case SleepTimerMod:
      activeModifier = new SleepTimer(tmpFolderSettings->special);
      break;
    case FreezeDanceMod:
      activeModifier = new FreezeDance();
      break;
    case LockedMod:
      activeModifier = new Locked();
      break;
    case ToddlerModeMod:
      activeModifier = new ToddlerMode();
      break;
    case KindergardenModeMod:
      activeModifier = new KindergardenMode();
      break;
    case RepeatSingleMod:
      activeModifier = new RepeatSingleModifier();
      break;
    case PuzzleGameMod:
      activeModifier = new PuzzleGame(tmpFolderSettings->special);
      break;
    case QuizGameMod:
      activeModifier = new QuizGame(tmpFolderSettings->special);
      break;
    case ButtonSmashMod:
      activeModifier = new ButtonSmash(tmpFolderSettings->special, tmpFolderSettings->special2);
      break;
    //case TheQuestMod: activeModifier = new TheQuest(tmpFolderSettings->special, tmpFolderSettings->special2); break;
    case CalculateMod:
      activeModifier = new Calculate(tmpFolderSettings->special, tmpFolderSettings->special2);
      break;
  }
  if (tmpFolderSettings->special3 == 1)
  {
    mySettings.savedModifier = *tmpFolderSettings;
    writeSettings();
  }
  return false;
}

bool RemoveModifier()
{
  if (activeModifier != NULL)
  {
    activeModifier = NULL;
  }

  _lastTrackFinished = 0;

  if (mySettings.savedModifier.folder != 0 || mySettings.savedModifier.mode != 0)
  {
    mySettings.savedModifier.folder = 0;
    mySettings.savedModifier.mode = 0;
    writeSettings();
  }

#if defined DEBUG
  Serial.println(F("modifier remvd"));
#endif
  if (isPlaying())
  {
    mp3.playAdvertisement(261);
  }
  else
  {
    mp3Pause();
    PlayMp3FolderTrack(261);
    waitForTrackToFinish();
  }
  return false;
}

void checkForUnwrittenTrack()
{
  if (trackToStoreOnCard > 0 && !hasCard && activeShortCut < 0)
  {
    //ungespeicherter Track vorhanden und keine Karte vorhanden
    if (isPlaying())
    {
      mp3.playAdvertisement(981);
    }
    else
    {
      PlayMp3FolderTrack(981);
    }
    waitForTrackToFinish();
    delay(100);
  }
}

void writeCardMemory(uint8_t track, bool announcement /* = false */)
{
#if defined DEBUG || defined SHORTCUTS_PRINT
  Serial.println(F("write mem"));
#endif

  if (knownCard && hasCard)
  {
#if defined DEBUG
    Serial.println(F("on card"));
#endif
    //Karte aufgelegt  & aktiv
    trackToStoreOnCard = 0;
    myCard.nfcFolderSettings.special3 = track;
    writeCard(myCard, 10, false);
  }
  else if (knownCard && !hasCard)
  {
#if defined DEBUG
    Serial.println(F("in var (no card)"));
#endif
    //Karte nicht mehr vorhanden aber aktiv, kein schreiben möglich. Track merken
    trackToStoreOnCard = track;
    if (announcement)
    {
      checkForUnwrittenTrack();
    }
  }
  else if (activeShortCut >= 0)
  {
#if defined DEBUG || defined SHORTCUTS_PRINT
    Serial.println(F("in EEPROM (shortcut)"));
#endif
    updateShortCutTrackMemory(track);
  }
}

//Um festzustellen ob eine Karte entfernt wurde, muss der MFRC regelmäßig ausgelesen werden
uint8_t pollCard()
{
  const byte maxRetries = 2;

  if (!hasCard)
  {
    if (mfrc522.PICC_IsNewCardPresent())
    {
      if (mfrc522.PICC_ReadCardSerial())
      {
#if defined DEBUG
        Serial.println(F("ReadCardSerial fin"));
#endif
        if (readCard(&myCard))
        {
          bool bSameUID = !memcmp(lastCardUid, mfrc522.uid.uidByte, 4);
          // store info about current card
          memcpy(lastCardUid, mfrc522.uid.uidByte, 4);
          lastCardWasUL = mfrc522.PICC_GetType(mfrc522.uid.sak) == MFRC522::PICC_TYPE_MIFARE_UL;

          retries = maxRetries;
          hasCard = true;
          return bSameUID ? PCS_CARD_IS_BACK : PCS_NEW_CARD;
        }
        else
        { //readCard war nicht erfolgreich
          mfrc522.PICC_HaltA();
          mfrc522.PCD_StopCrypto1();
#if defined DEBUG
          Serial.println(F("read card err"));
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

    if (mfrc522.MIFARE_Read(lastCardWasUL ? 8 : blockAddr, buffer, &size) != MFRC522::STATUS_OK)
    {
      if (retries > 0)
        retries--;
      else
      {
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

Enum_PCS handleCardReader()
{
  bool tmpStopWhenCardAway = mySettings.stopWhenCardAway;

  static unsigned long lastCardPoll = 0;
  unsigned long now = millis();
  // poll card only every 100ms
  if (static_cast<unsigned long>(now - lastCardPoll) > 100)
  {
    lastCardPoll = now;

    if (activeModifier != NULL)
    {
      tmpStopWhenCardAway &= !activeModifier->handleStopWhenCardAway();
    }

    switch (pollCard())
    {
      case PCS_NEW_CARD:
#if defined DEBUG
        Serial.println(F("new tag"));
#endif
        onNewCard();
        return PCS_NEW_CARD;
        break;
      case PCS_CARD_GONE:
#if defined DEBUG
        Serial.println(F("tag gone"));
#endif
        if (tmpStopWhenCardAway)
        {
          knownCard = false;
          myTriggerEnable.pauseTrack |= true;
          myTrigger.pauseTrack |= true;
          pauseAction(true);
          //mp3Pause();
          //setstandbyTimer();
        }
        return PCS_CARD_GONE;
        break;
      case PCS_CARD_IS_BACK:
#if defined DEBUG
        Serial.println(F("same tag"));
#endif
        if (tmpStopWhenCardAway)
        {

          //nur weiterspielen wenn vorher nicht konfiguriert wurde

          knownCard = true;
          myTriggerEnable.pauseTrack |= true;
          myTrigger.pauseTrack |= true;
          pauseAction(true);
        }
        else if (trackToStoreOnCard == 0)
        {
          onNewCard();
        }

        if (trackToStoreOnCard > 0)
        {
          writeCardMemory(trackToStoreOnCard);
          trackToStoreOnCard = 0;
          if (!isPlaying())
          {
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
  return PCS_NO_CHANGE;
}

void onNewCard()
{
  //forgetLastCard = false;
  //make random a little bit more "random"
  //randomSeed(millis() + random(1000));
  trackToStoreOnCard = 0;
  if (myCard.cookie == cardCookie && myCard.nfcFolderSettings.folder != 0 && myCard.nfcFolderSettings.mode != 0)
  {
    knownCard = true;
    activeShortCut = -1;
    playFolder();
  }

  // Neue Karte konfigurieren
  else if (myCard.cookie != cardCookie)
  {
    PlayMp3FolderTrack(300);
    waitForTrackToFinish();
    setupCard();
  }
}

void resetCurrentCard()
{
  knownCard = false;
  memset(lastCardUid, 0, sizeof(lastCardUid));
  activeShortCut = -1;
}

/// Funktionen für den Standby Timer (z.B. über Pololu-Switch oder Mosfet)

void setstandbyTimer()
{
  if (!standbyTimerSet)
  {
#if defined DEBUG
    Serial.println(F("set stby timer"));
#endif
    standbyTimerSet = true;

    // verstärker aus
    digitalWrite(ampPin, HIGH);
    delay(100);

    if (mySettings.standbyTimer != 0)
      sleepAtMillis = millis() + (mySettings.standbyTimer * 60 * 1000);
    else
      sleepAtMillis = 0;
#if defined DEBUG
    Serial.print(F("milis "));
    Serial.println(sleepAtMillis);
#endif
  }
}
//////////////////////////////////////////////////////////////////////////
void disablestandbyTimer()
{
  if (standbyTimerSet)
  {
#if defined DEBUG
    Serial.println(F("disable stby timer"));
#endif
    standbyTimerSet = false;

    // verstärker an
    digitalWrite(ampPin, LOW);
    delay(25);

    sleepAtMillis = 0;
  }
}
//////////////////////////////////////////////////////////////////////////
void checkStandbyAtMillis()
{
  if (sleepAtMillis != 0)
  {
    if (millis() > sleepAtMillis)
    {
#if defined DEBUG
      Serial.println(F("standby act"));
#endif
      sleepAtMillis = 0;
      shutDown();
    }
    if (isPlaying())
    {
      disablestandbyTimer();
    }
  } else
  {
    if (!isPlaying())
    {
      setstandbyTimer();
    } else
    {
      disablestandbyTimer();
    }
  }
}
//////////////////////////////////////////////////////////////////////////
void shutDown()
{
#if defined DEBUG
  Serial.println("Shut Down");
#endif

  mp3Pause();

#if defined POWER_ON_LED_PIN
  digitalWrite(POWER_ON_LED_PIN, LOW);
#endif

  if (volume > mySettings.initVolume)
    volume = mySettings.initVolume;

  SetVolume(volume);

  knownCard = false;
  activeShortCut = -1;
  PlayMp3FolderTrack(265);
  delay(1500);
  waitForTrackToFinish();


  // verstärker aus
  digitalWrite(ampPin, HIGH);

digitalWrite(powerPin, LOW);

}

//////////////////////////////////////////////////////////////////////////
#if defined FADING_LED
// fade in/out status led while beeing idle, during playback set to full brightness
void fadeStatusLed(bool isPlaying)
{
  static bool statusLedDirection = false;
  static int16_t statusLedValue = 255;
  static unsigned long statusLedOldMillis;
  static int16_t statusLedDeltaValuePause = 500;
  static int16_t statusLedDeltaValuePlay = 1;
  static int16_t statusLedDeltaValue = 10;

  if (isPlaying)
  {
    statusLedDeltaValue = statusLedDeltaValuePlay;
  }
  else
  {
    statusLedDeltaValue = statusLedDeltaValuePause;
  }

  if ((millis() - statusLedOldMillis) >= 100)
  {
    statusLedOldMillis = millis();
    if (statusLedDirection)
    {
      statusLedValue += statusLedDeltaValue;
      if (statusLedValue >= 255)
      {
        statusLedValue = 255;
        statusLedDirection = !statusLedDirection;
      }
    }
    else
    {
      statusLedValue -= statusLedDeltaValue;
      if (statusLedValue <= 0)
      {
        statusLedValue = 0;
        statusLedDirection = !statusLedDirection;
      }
    }
    analogWrite(POWER_ON_LED_PIN, statusLedValue);
  }
}
#endif
//////////////////////////////////////////////////////////////////////////
#if defined DISPLAY
void showTrackOnDisplay(){
  //clear
  myDisplay.clearDisplay();
  myDisplay.point(0);

  //write new track
  int digit0 = currentTrack / 100;
  int digit1 = currentTrack / 100;
  int digit2 = currentTrack / 10;
  int digit3 = currentTrack % 10;

  if(digit0 > 0){
    myDisplay.display(0, digit0);
  }
  if(digit1 > 0){
    myDisplay.display(1, digit1);
  }
  if(digit2 > 0){
    myDisplay.display(2, digit2);
  }
  myDisplay.display(3, digit3);
} 

void showVolumeOnDisplay(int volume){
  //clear
  myDisplay.clearDisplay();
  myDisplay.point(0);

  //write new volume
  myDisplay.display(0, volume / 10);
  myDisplay.display(1, volume % 10);

  //start timer for 1 second, so display will switch back to currentTrack
  volumeIsShown = true;
  volumeTimer = millis() + 1000; // 1000ms = 1s 
} 
#endif
//////////////////////////////////////////////////////////////////////////
