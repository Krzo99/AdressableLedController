#include "SoundDetector.h"


LedMode LedModes[] = {
	LedMode(0, "Beat", 1, 0), 
	LedMode(1, "Rise", 1, 1), 
	LedMode(2, "Bullet", 1, 1), 
	LedMode(3, "Rainbow", 0, 1), 
	LedMode(4, "Breathing", 0, 1), 
	LedMode(5, "Fade", 0, 1),
	LedMode(6, "Move", 0, 1) };
Detector::Detector(int Env, int Gate, int Aud)
{
	EnvPin = Env;
	GatePin = Gate;
	AudPin = Aud;
}

DetectorResults Detector::DetectorTask()
{
	int CurrentVal = ReadEnv();
	DetectorResults result;
	result.val = CurrentVal;
	
	if (CurrentVal < lastVal)
	{
		int LastExtremeVal = LastExtreme.Val;
		LastExtreme = Extreme(1, lastVal);

		int JumpSize = lastVal - LastExtremeVal;

		result.Jump = JumpSize;
		result.bBigJump = false;
		
		if (JumpSize > BigJumpVal)
		{
			result.bBigJump = true;
		}
		
	}
	
	lastVal = CurrentVal;
	return result;
}

int Detector::ReadEnv()
{
	int val = analogRead(ENV_PIN);
	return val;
}

Extreme::Extreme(int typeInp, int ValInp)
{
	type = typeInp;
	Val = ValInp;
}

LedController::LedController()
{
	ChangeLedMode(0);
	Detect = Detector();
}

void LedController::ControllerTask()
{
	DetectorResults DetectorCheck = Detect.DetectorTask();
	SwitchModesTask();
	if (CurrentLedMode->IsContinious)
	{
		if (CurrentLedMode->ModeName == "Rise")
		{
			fillArraySameColor(CRGB(0, 0, 0));
			static CRGB color = CRGB(random(255), random(255), random(255));
			int lvl = DetectorCheck.val;
			int deltaPerLed = MaxVolLevel / 150;
			

			if (lvl > MaxVolLevel)
			{
				lvl = MaxVolLevel;
			}
			if (DetectorCheck.bBigJump) {
				color = CRGB(random(255), random(255), random(255));
			}
			for (int i = 0; i < lvl / deltaPerLed; i++)
			{
				leds[i] = color;
			}
			/*for (int i = 0; i < 150; i++)
			{
				if (leds[i].r != 0 || leds[i].g != 0 || leds[i].b != 0)
				{
					Serial.printf("Ta ni: %d\n", i);
				}
			}*/

			FastLED.show();
			delay(3);
		}
		else if (CurrentLedMode->ModeName == "Rainbow")
		{
			byte *c;
			uint16_t i;

				for (i = 0; i < NUM_LEDS; i++) {
					c = Wheel(((i * 256 / NUM_LEDS) + RainbowCurrentLocation) & 255);
					leds[i] = CRGB(*c, *(c + 1), *(c + 2));
				}

				RainbowCurrentLocation < 256 * 5 ? RainbowCurrentLocation++ : RainbowCurrentLocation = 0;
				FastLED.show();
				delay(RainbowDelay);


		}
		else if (CurrentLedMode->ModeName == "Bullet")
		{
			fillArraySameColor(CRGB(0, 0, 0));
			for (int i = 0; i < MaxBullets; i++)
			{
				if (ActiveBullets[i] != nullptr)
				{
					ActiveBullets[i]->ChangeBrightness();
					for (int u = 1; u < ActiveBullets[i]->Length + 1; u++)
					{
						//Serial.printf("Loc: %d, Len: %d, i: %d, u: %d\n", ActiveBullets[i]->Loc, ActiveBullets[i]->Length, i, u);
						int Loc = ActiveBullets[i]->Loc;
						if (Loc + u < NUM_LEDS)
						{
							leds[Loc + u] = ActiveBullets[i]->Color;
						}
						else
						{
							delete ActiveBullets[i];
							ActiveBullets[i] = nullptr;
							return;
						}
					}

					if (ActiveBullets[i] != nullptr)
					{
						ActiveBullets[i]->MoveBullet();
					}
				}
			}
			FastLED.show();
			delay(BulletSpeedDelay);
		}
		else if (CurrentLedMode->ModeName == "Breathing")
		{
			static CRGB BreathColor = CRGB(random(255), random(255), random(255));
			static int state = 0; //State gre od 0 do 100 in nazaj na 0
			static bool bRising = true;

			fillArraySameColor(GetColorState(BreathColor, state));

			if (state >= 100) {
				bRising = false;
			}
			else if (state <= 0) {
				bRising = true;
				BreathColor = CRGB(random(255), random(255), random(255));
			}

			if (bRising) {
				state++;
			}
			else {
				state--;
			}

			FastLED.show();
			delay(BreathingDelay);
			
		}
		else if (CurrentLedMode->ModeName == "Fade")
		{
			static byte Pos = 0;
			byte* c = Wheel(Pos);
			CRGB Color = CRGB(*c, *(c + 1), *(c + 2));
			fillArraySameColor(Color);
			FastLED.show();

			Pos++;

			delay(FadeMoveDelay);

		}
		else if (CurrentLedMode->ModeName == "Move")
		{
			static bool bSoda = true;
			static CRGB Color = CRGB(random(255), random(255), random(255));
			static int LastChange = millis();

			FastLED.clear();
			for (int i = (bSoda ? 0 : 1); i < NUM_LEDS; i += 2)
			{
				leds[i] = Color;

			}
			bSoda = !bSoda;

			if (millis() - LastChange > 20 * 1000)
			{
				Color = CRGB(random(255), random(255), random(255));
				LastChange = millis();
			}

			FastLED.show();
			delay(MoveDelay);

			
		}
	}
	if (DetectorCheck.bBigJump)
	{
		if (CurrentLedMode->ModeName == "Beat")
		{
			LedEvent(0);
		}
		else if (CurrentLedMode->ModeName == "Bullet")
		{
			LedEvent(1);
		}
	}
}

void LedController::ChangeLedMode(int id)
{
	CurrentLedMode = &LedModes[id];
}

void LedController::LedEvent(int type)
{
	static int lastEvent = millis();
	if (CurrentLedMode != nullptr && type != -1 && millis() - lastEvent > EventDelay)
	{
		if (type == 0)
		{
			fillArraySameColor(CRGB(random(255), random(255), random(255)));
			FastLED.show();
		}
		else if (type == 1)
		{
			//Loc
			int BullLoc = 0;

			//Length
			int BullLeng = BulletLength;
			if (BulletLength <= 0)
			{
				BullLeng = random(2) + 2;
			}

			//Color
			CRGB BullCol = CRGB(random(255), random(255), random(255));

			//Life
			int BullLife = BulletLife;
			if (BulletLife <= 0)
			{
				BullLife = random(100) + 1;
			}
			int LastLed = (float) ((float) BullLife / 100) * NUM_LEDS;

			float BrightnessChangeStep = 1 - (1 / (float)LastLed);


			int PlaceForNewBullet = IndexOfFreeBullet();
			if (PlaceForNewBullet >= 0)
			{
				ActiveBullets[PlaceForNewBullet] = new Bullet(BullLoc, BullLeng, BullCol, BullLife, BrightnessChangeStep);
			}

		}

		lastEvent = millis();
	}
}

int LedController::IndexOfFreeBullet()
{
	for (int i = 0; i < MaxBullets; i++)
	{
		if (ActiveBullets[i] == nullptr)
		{
			return i;
		}
	}
	return -1;
}

CRGB LedController::GetColorState(CRGB FullColor, int state)
{
	float stateFloat = (float) state / 100;
	CRGB Color = CRGB(FullColor.r * stateFloat, FullColor.g * stateFloat, FullColor.b * stateFloat);
	return Color;
}

void LedController::SwitchModesTask()
{

	static int lastSwitch = millis();
	if (millis() - lastSwitch > 20 * 1000)
	{
		if (bAutoMusicMode)
		{
			int NewMode = CurrentLedMode->ModeId;
			while (NewMode == CurrentLedMode->ModeId)
			{
				NewMode = random(3);
			}

			ChangeLedMode(NewMode);
		}
		else if (bAutoNonMusicMode)
		{
			int NewMode = CurrentLedMode->ModeId;
			while (NewMode == CurrentLedMode->ModeId)
			{
				NewMode = random(4) + 3;
			}

			ChangeLedMode(NewMode);
		}
		if (bAutoMusicMode || bAutoNonMusicMode)
		{
			lastSwitch = millis();
		}
	}
}


void LedController::fillArraySameColor(CRGB color)
{
	for (int i = 0; i < NUM_LEDS; i++)
	{
		leds[i] = color;
	}
}

byte* LedController::Wheel(byte WheelPos) {
	static byte c[3];

	if (WheelPos < 85) {
		c[0] = WheelPos * 3;
		c[1] = 255 - WheelPos * 3;
		c[2] = 0;
	}
	else if (WheelPos < 170) {
		WheelPos -= 85;
		c[0] = 255 - WheelPos * 3;
		c[1] = 0;
		c[2] = WheelPos * 3;
	}
	else {
		WheelPos -= 170;
		c[0] = 0;
		c[1] = WheelPos * 3;
		c[2] = 255 - WheelPos * 3;
	}

	return c;
}
LedMode::LedMode(int id, char * name, bool bSoundResponsive, bool bContinious)
{
	ModeId = id;
	ModeName = name;
	IsSoundResponsive = bSoundResponsive;
	IsContinious = bContinious;
}

Bullet::Bullet(int LocCon, int LenCon, CRGB ColorCon, int LifeCon, float BrightChangeCon)
{
	Length = LenCon;
	Loc = LocCon;
	Color = ColorCon;
	Life = LifeCon;
	BrightnessChangePerStep = BrightChangeCon;
}

void Bullet::MoveBullet(int kok)
{
	Loc += kok;
}

void Bullet::ChangeBrightness()
{
	Color.r = Color.r * BrightnessChangePerStep;
	Color.g = Color.g * BrightnessChangePerStep;
	Color.b = Color.b * BrightnessChangePerStep;
}
