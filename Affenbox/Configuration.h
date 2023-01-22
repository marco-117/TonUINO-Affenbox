
//===========================================================================
//================================ All in One ===============================
//===========================================================================
/*
*Bei Verwendung der All in One Plus Baugruppe, (genannt AiO+) aktivieren
*https://www.leiterkartenpiraten.de/produkt/tonuino-all-in-one-plus-platine/
*/

#define AiO_Plus

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//===========================================================================
//============================= Hardware Features ===========================
//===========================================================================
/** 
*Nachfolgend sind alle möglichen Hardware erweiterungen aufgelistet und die dazugehörigen Konfigurationsmöglichkeiten.
*Um eine Funktion zu aktivieren das entsprechende "#define" auskommentiern. Dazu die beiden "//" davor entfernen.
*Alles weiter steht bei den entsprechenden Punkten
*/

//===========================================================================
//================================= Buttons =================================
//===========================================================================

/** 
 * Konfiguration der Standard Button Pins 
 */

/**
*Sample All in One Plus
*/

 #define buttonPause A0
 #define buttonUp A4
 #define buttonDown A3

/** 
 * Konfiguration eines separten power buttons
 */
//#define buttonPower A0

//===========================================================================
//============================= FIVEBUTTONS =================================
//===========================================================================
/**
*Für die Verwendung von fünf Buttons, statt drei.
*/
//#define FIVEBUTTONS    

#if defined FIVEBUTTONS
/**
*Sample All in One
*/
#define buttonFourPin A2
#define buttonFivePin A1

#endif


//===========================================================================
//============================= ANALOG_INPUT ================================
//===========================================================================
/**
*Gibt die Möglichkeit die Affenbox mit mehreren Tastern über einen analogen Eingang zu steuern.
*Hierzu wird extra Hardware benötigt.
*z.B.:
*TonUINO Button Board (bereits vorkonfiguriert)
*https://www.leiterkartenpiraten.de/produkt/tonuino-button-board-3x3/
*
*Für eigene Aufbauten weitere Informationen:
*https://github.com/bxparks/AceButton/tree/develop/examples/LadderButtons
*
*Bitte beachten: die AiO verwendet andere Referenspegel als der Arduino Nano!
*/

//#define ANALOG_INPUT

#if defined ANALOG_INPUT

/**
*Angabe des Anschlusspins für die analogen Signale.
*Es kann jeder freie analoge Eingagn gewählt werden.
*Ausgeschlossen sind folgende Pins: A7, sowie bereits durch andere Funktionen vergeneben Pins.
 */
#define ANALOG_INPUT_PIN A3 //Forward Button Pin, used by 3x3 Button Board

/** 
*Menge der analogen Signale/Buttons.
 */
#define ANALOG_INPUT_BUTTON_COUNT 9

/** 
*Liste der analogen Werte für jeden Button.
*Es muss für jeden Button ein Wert angegeben werden, plus der Wert wenn kein Button gedrückt ist.
*Dieser Nullwert muss an erster Stelle stehen.
*Die hier in der {}-Klammer schon aufgelisteten Werte, sind die Standardwerte für das TonUINO Button Board. Sowohl für die AiO als auch für die klassische Variante. 
*Bei eigenen Aufbauten kann alles in der {}-Klammer gelöscht und ersetzt werden. 
*!!Zur erfassung der Werte hilft "#define ANALOG_INPUT_PRINT"!!
 */
static const uint16_t ANALOG_INPUT_LEVELS[ANALOG_INPUT_BUTTON_COUNT + 1 ] = {
  0,
  92,
  194,
  307,
  405,
  512,
  608,
  698,
  773,
  1023
};

/** 
*Liste der Funktion für jeden analogen Wert.
*Es muss für jeden analogen Wert eine Funktion festgelegt werden, inklusive dem Nullwert.
*Die Reihenfolge der Funktionen muss der Reihenfolge der analogen Werte entsprechen.
*D.h. Analogwert 3 == Funktionswert 3
*
*Die hier in der {}-Klammer schon aufgelisteten Werte, sind die Standardwerte für das TonUINO Button Board. Sowohl für die AiO als auch für die klassische Variante. 
*Bei eigenen Aufbauten kann alles in der {}-Klammer gelöscht und ersetzt werden. 
*
* Folgende Werte können angegeben werden:
   Null                = 0,
   Pause               = 1,
   Next                = 2,
   Next +10            = 4,
   Previous            = 5,
   Previous +10        = 6,
   VolumeUp            = 7,
   VolumeDown          = 8,
   Abort/Shutdown      = 9,
   AdminMenu           = 10,
   Reset Playback Mode = 11,
   Shortcut No.1 to 12 = 12 bis 24
 */
//#define ANALOG_INPUT_SUPPLY_PIN 6 //Nur verwenden, wenn es unbedingt nötig ist!Der Referenzpegel kann auch von einem IO Pin kommen.
static const uint8_t ANALOG_INPUT_BUTTON_MAP[ANALOG_INPUT_BUTTON_COUNT + 1 ] = {
  0,  //Null
  12, //Shortcut No. 1
  13, //Shortcut No. 2
  14, 
  15,
  16,
  17,
  18,
  19,
  20  //Shortcut No. 9
};

/*
*Für genauere Debugausgaben im seriellen Monitor.
*!Achtung! Benötigt viel Speicher, eventuell müssen andere Funktionen temporär deaktivert werden!
*/
//#define ANALOG_INPUT_PRINT

/*
*Gibt die analogen Werte so schnell wieder wie möglich. 
*Falls die analogen Werte der Taste nicht bekannt sind können sie hiermit ermittelt werden.
*/
//#define ANALOG_INPUT_PRINT_ANALOGREAD

#endif

//===========================================================================
//============================= IRREMOTE ====================================
//===========================================================================
/**
*Gibt die Möglichkeit die Affenbox mit einer IR Fernbedienung zu steuern.
*Hierzu wird extra Hardware benötigt.
*z.B.:
*IR Infrarot Remote Receiver / Empfänger TSOP38238 
*https://www.berrybase.de/bauelemente/aktive-bauelemente/leds/infrarot-leds/ir-infrarot-remote-receiver/empf-228-nger-tsop38238

*Infrarot Fernbedienung mit 17 Tasten (bereits vorkonfiguriert)
*https://www.berrybase.de/raspberry-pi/raspberry-pi-computer/eingabegeraete/infrarot-fernbedienung-mit-17-tasten
*/

//#define IRREMOTE

#if defined IRREMOTE

/* 
*Angabe des Anschlusspins für den Data Pin des IR Empfängers.
*Es kann jeder freie digtale Eingang geählt werden.
*Ausgeschlossen sind folgende Pins: 4, 9, 10, sowie bereits durch andere Funktionen vergeneben Pins.
 */
#define IRREMOTE_PIN 18


/*
*Falls das Protokoll der gewünschten Fernbedienung beakannt ist, sollte NUR DAS ENTSPRECHENDE PROTOKOLL auskommentiert werden. 
*Das spart wertovllen Speicher.
*Falls das Protokoll nicht bekannt ist, keines der Prtoolle auskommentiern!
*"#define IRREMOTE_PRINT" hilft bei der identifizierung des Protokolls.
*/
//#define DECODE_DENON        // Includes Sharp
//#define DECODE_JVC
//#define DECODE_KASEIKYO
//#define DECODE_PANASONIC    // the same as DECODE_KASEIKYO
//#define DECODE_LG
#define DECODE_NEC          // Includes Apple and Onkyo
//#define DECODE_SAMSUNG
//#define DECODE_SONY
//#define DECODE_RC5
//#define DECODE_RC6

//#define DECODE_BOSEWAVE
//#define DECODE_LEGO_PF
//#define DECODE_MAGIQUEST
//#define DECODE_WHYNTER

//#define DECODE_HASH         // special decoder for all protocols


/*
*Für genauere Debugausgaben im seriellen Monitor.
*!Achtung! Benötigt viel Speicher, eventuell müssen andere Funktionen temporär deaktivert werden!
*/
//#define IRREMOTE_PRINT

#endif


//===========================================================================
//============================= ROTARY_ENCODER ==============================
//===========================================================================
/**
*Gibt die Möglichkeit die Lautstärke der Affenbox über eine Drehencoder zu steuern.
*Hierzu wird extra Hardware benötigt.
*z.B.:
*KY-040 Drehwinkelgeber/Drehgeber
*https://www.az-delivery.de/products/drehimpulsgeber-modul?_pos=1&_sid=85bb51922&_ss=r
*/

//#define ROTARY_ENCODER   

#if defined ROTARY_ENCODER

/* 
*Angabe der Anschlusspins für den Data und Clock Pin des Drehgebers.
*Es kann jeder freie  Pin geählt werden, auch Analoge.
*Ausgeschlossen sind folgende Pins: 4, 9, 10, A7, sowie bereits durch andere Funktionen vergeneben Pins.
 */
#define ROTARY_ENCODER_PIN_A A9 
#define ROTARY_ENCODER_PIN_B A10

/* 
*Anzahl erkannter Steps pro Klick des Drehgebers.
*Falls der Wert falsch eingestellt ist, werden eventuell mehr als ein Schritt pro Klick erkannt.
 */
#define ROTARY_ENCODER_STEPS 4

/*
*Für genauere Debugausgaben im seriellen Monitor.
*!Achtung! Benötigt viel Speicher, eventuell müssen andere Funktionen temporär deaktivert werden!
*/
//#define ROTARY_ENCODER_PRINT

#endif

//===========================================================================
//============================ 7-Segment-Anzeige ============================
//===========================================================================
/**
*Gibt die Möglichkeit Informationen über eine 7-Segmentanzeige auszugeben
*Hierzu wird extra Hardware benötigt, z.B.:
TM1637 4 Digit 7-Segment Display
https://www.az-delivery.de/products/4-digit-display
*/

#define DISPLAY   

#if defined DISPLAY

/* 
*Angabe der Anschlusspins für den DIO und CLK des Displays
*Es kann jeder freie  Pin gewählt werden, auch analoge.
*Ausgeschlossen sind folgende Pins: 4, 9, 10, A7, sowie bereits durch andere Funktionen vergebene Pins.
 */
#define DISPLAY_PIN_A A15 //CLK
#define DISPLAY_PIN_B A13 //DIO

/* 
Helligkeit des Displays
 */
#define DISPLAY_BRIGHTNESS 2

#endif

//===========================================================================
//===================== POWER_ON_LED(EXPERIMENTEL!!) ========================
//===========================================================================
//--------POWER_ON_LED(EXPERIMENTEL!!)--------
/*
*Gibt die Möglichkeit eine simple Status LED einzufügen.
*Als zusätzliche Funktion, kann diese durch fading den Staus genauer wiedergeben.
*/

//#define POWER_ON_LED    

#if defined POWER_ON_LED

/* 
*Angabe des Anschlusspins für die LED.
*Es kann jeder freie  Pin geählt werden, auch Analoge.
*Ausgeschlossen sind folgende Pins: 4, 9, 10, A7, sowie bereits durch andere Funktionen vergeneben Pins.
 */
#define POWER_ON_LED_PIN 36

//#define FADING_LED 

#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//===========================================================================
//============================= NFCgain =====================================
//===========================================================================
/**
*Um Probleme bei der Erkennung von RFID Tags zu beheben, kann die Verstärkung des RFID Reader/Writer geändert werden.
*Ein unnötiges Hochsetzen der Empfindlichkeit kann ebenfalls zu Problemen bei der Erkennung führen.
*/
//#define NFCgain_max   // Maximale Empfindlichkeit
#define NFCgain_avg   // Mittlere Empfindlichkeit
//#define NFCgain_min   // Minimale Empfindlichkeit


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//===========================================================================
//============================= Debugging ===================================
//===========================================================================
/*
*Aktiviert Standard Debug Asugaben
*/
#define DEBUG  

/*
*Aktiviert Shortcut Debug Ausgaben
*/
//#define SHORTCUTS_PRINT

/*
*Aktiviert die Ausgabe der Queue
*/
//#define QUEUE_PRINT 

/*
*Aktiviert DFPlayer Debug Ausgaben
*/ 
//#define DFPLAYER_PRINT

/*
*Aktiviert die Ausgabe des EEPROMS zu beginn
*/
//#define EEPROM_PRINT

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
