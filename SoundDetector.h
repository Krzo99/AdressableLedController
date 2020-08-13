#include "Stdio.h"
#include <EEPROM.h>
#include "Arduino.h"
#include <FastLED.h>

#define NUM_LEDS 150
#define LED_PIN 27
#define ENV_PIN 35
#define EEPROM_SIZE 512

//Extreme of analogRead
//type is Min or Max
//Val is val in extreme
//DiffFromLastExtreme is diff from last extreme
//BigJumpVal can be set, and tells you at what peak diff lights should change, or whatever the effect is.
class Extreme
{
public:
	Extreme(int typeInp = 0, int ValInp = 0);

	int type;
	int Val;

};

class Bullet
{
public:
	Bullet(int LocCon, int LenCon, CRGB ColorCon, int LifeCon, float BrightChangeCon);
	int Length = 0;
	int Loc = 0;
	CRGB Color = 0;
	int Life = 0;
	float BrightnessChangePerStep = 0;

	void MoveBullet(int kok = 1);
	void ChangeBrightness();
};

class DetectorResults
{
public:

	int Jump = 0;
	bool bBigJump = 0;
	int val = 0;
};

class Detector
{
public:
	Detector(int EnvPin = ENV_PIN, int GatePin = 14, int AudPin = 12);

	//Settings
	int BigJumpVal = 300;

	//Pins
	int EnvPin;
	int GatePin;
	int AudPin;

	Extreme LastExtreme;
	int lastVal = 0;
	
	DetectorResults DetectorTask();
	int ReadEnv();
};


class LedMode
{
public:
	LedMode(int id, char* name, bool bSoundResponsive, bool bContinious);
	int ModeId;
	char* ModeName;
	bool IsSoundResponsive;
	bool IsContinious;
};

class LedController
{
public:

	LedController();
	LedMode* CurrentLedMode;
	Detector Detect;

	bool bSetupComplete = false;

	//
	//Settings
	//
	//General
	int EventDelay = 100;
	bool bJumpByAverage = false; //Do jumps fire based on average (aka, jump must be "BigJumpVal" more than average) or based on previous extreme, aka min.
	bool bAutoMusicMode = false;
	bool bAutoNonMusicMode = false;
	//Rise settings
	int MaxVolLevel = 1500;

	//Rainbow settings
	int RainbowDelay = 20;

	//Bullet settings
	int BulletSpeedDelay = 2;
	int BulletLength = 0;
	int BulletLife = 0;
	static const int MaxBullets = 8;

	//Breathing settings
	int BreathingDelay = 30;

	//Fade settings
	int FadeMoveDelay = 50;

	//Move settings
	int MoveDelay = 100;

	uint16_t RainbowCurrentLocation = 0;

	void ControllerTask();
	void ChangeLedMode(int id = 0);
	void LedEvent(int type = 0);
	void SetLeds();

	void fillArraySameColor(CRGB color);
	void fillArrayDifColor(CRGB colors[]);
	byte* Wheel(byte WheelPos);

	CRGB leds[NUM_LEDS];
	Bullet* ActiveBullets[MaxBullets];
	int IndexOfFreeBullet();

	CRGB GetColorState(CRGB FullColor, int state);

	void SwitchModesTask();
	
};
