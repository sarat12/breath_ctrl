#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>

const String MIDI_CHANNEL_HEADER = String("CC:");
const String LOW_OFFSET_HEADER = String("LO:");
const String HIGH_OFFSET_HEADER = String("HO:");

const byte NONE_MENU = 255;
const byte MC_MENU = 0;
const byte LO_MENU = 1;
const byte HO_MENU = 2;

const byte MIDI_MODE = 0;
const byte MENU_MODE = 1;
const byte VALUE_MODE = 2;

#define SENSOR_PIN 0

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

byte lastBtnStatus = LOW;
byte mode = MIDI_MODE;
byte selectedMenu = NONE_MENU;

struct CFG_t {
  byte midiCc;
  int lowOffset;
  int highOffset;
} cfg;

int lastCcValue = 0;

Encoder enc(8, 9);

void setup()   {
  EEPROM_readAnything(0, cfg);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pinMode(7, INPUT);
  
  initScreen();
  display.display();
}

void loop() {
  if (buttonPressed()) {
    switch (mode){
      case MIDI_MODE : {
        mode = MENU_MODE;
        selectedMenu = MC_MENU;
        refreshMenuHeaders();
      } break;
      case MENU_MODE : {
        mode = VALUE_MODE;
        switch (selectedMenu) {
          case MC_MENU : enc.write(cfg.midiCc * 4); break;
          case LO_MENU : enc.write(cfg.lowOffset * 4); break;
          case HO_MENU : enc.write(cfg.highOffset * 4); break;
        }
        refreshSelectedMenuValue();
      } break;
      case VALUE_MODE : {
        mode = MIDI_MODE;
        EEPROM_writeAnything(0, cfg);
        initScreen();
      } break;
    }
    display.display();
  }

  if (mode == MENU_MODE) {
    long ev = enc.read() / 4;
    long cnt = constrain(ev, MC_MENU, HO_MENU);
    if (ev != cnt)
      enc.write(cnt * 4);

    if (selectedMenu != cnt) {
      selectedMenu = cnt;
      refreshMenuHeaders();
      display.display();
    }
  }

  if (mode == VALUE_MODE) {
    switch (selectedMenu) {
      case MC_MENU : {
        long ev = enc.read() / 4;
        long cnt = constrain(ev, 1, 127);
        if (ev != cnt)
          enc.write(cnt * 4);

        if (cfg.midiCc != cnt) {
          cfg.midiCc = cnt;
          refreshSelectedMenuValue();
          display.display();
        }
      } break;

      case LO_MENU : {
        long ev = enc.read() / 4;
        long cnt = constrain(ev, 0, cfg.highOffset);
        if (ev != cnt)
          enc.write(cnt * 4);

        if (cfg.lowOffset != cnt) {
          cfg.lowOffset = cnt;
          refreshSelectedMenuValue();
          display.display();
        }
      } break;

      case HO_MENU : {
        long ev = enc.read() / 4;
        long cnt = constrain(ev, cfg.lowOffset, 1023);
        if (ev != cnt)
          enc.write(cnt * 4);

        if (cfg.highOffset != cnt) {
          cfg.highOffset = cnt;
          refreshSelectedMenuValue();
          display.display();
        }
      } break;
    }
  }

  if (mode == MIDI_MODE) {
    int constrained = constrain(analogRead(SENSOR_PIN), cfg.lowOffset, cfg.highOffset);
    int val = map(constrained, cfg.lowOffset, cfg.highOffset, 0, 127);
    if (lastCcValue != val) {
      lastCcValue = val;
      usbMIDI.sendControlChange(cfg.midiCc, lastCcValue, 1);
    }
  }
}

//----------------------------------------------FUNCTIONS-----------------------------------------------------

bool buttonPressed() {
  byte curBtnStatus = digitalRead(7);
  bool result = false;
  if (curBtnStatus != lastBtnStatus) {
    if (curBtnStatus == HIGH)
      result = true;
    lastBtnStatus = curBtnStatus;
  }
  return result;
}

void initScreen() {
  display.clearDisplay();

  selectedMenu = NONE_MENU;
  refreshMenuHeaders();
  refreshMenuValues();
}

void refreshMenuHeaders() {
	display.setTextSize(2);
  display.setCursor(20, 8);
	setColor(selectedMenu == MC_MENU);
  display.print(MIDI_CHANNEL_HEADER);
  display.setCursor(20, 26);
	setColor(selectedMenu == LO_MENU);
  display.print(LOW_OFFSET_HEADER);
  display.setCursor(20, 44);
	setColor(selectedMenu == HO_MENU);
  display.print(HIGH_OFFSET_HEADER);
}

void refreshMenuValues() {
  display.setTextSize(2);
  display.setCursor(60, 8);
	setColor(selectedMenu == MC_MENU);
  display.print(formatNumber(cfg.midiCc));
  display.setCursor(60, 26);
	setColor(selectedMenu == LO_MENU);
  display.print(formatNumber(cfg.lowOffset));
  display.setCursor(60, 44);
	setColor(selectedMenu == HO_MENU);
  display.print(formatNumber(cfg.highOffset));
}

void refreshSelectedMenuValue() {
	display.setTextSize(2);
	setColor(true);
	switch (selectedMenu)
	{
    case MC_MENU: {
      display.setCursor(60, 8);
      display.print(formatNumber(cfg.midiCc));
    } break;
    case LO_MENU: {
      display.setCursor(60, 26);
      display.print(formatNumber(cfg.lowOffset));
    } break;
    case HO_MENU: {
      display.setCursor(60, 44);
      display.print(formatNumber(cfg.highOffset));
    } break;
	}
}

void setColor(bool invert) {
  if (invert)
    display.setTextColor(BLACK, WHITE);
  else
    display.setTextColor(WHITE, BLACK);
}

String formatNumber(int num) {
  String out = String("");
  if (num < 10) return String("   ") + num;
  if (num < 100) return String("  ") + num;
  if (num < 1000) return String(" ") + num;
  return num;
}

template <class T> int EEPROM_writeAnything(int ee, const T& value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
    byte* p = (byte*)(void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}



