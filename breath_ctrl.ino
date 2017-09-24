#define ENCODER_OPTIMIZE_INTERRUPTS
#include <Encoder.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET);

int cnt = 0;

Encoder enc(8, 9);

void setup()   {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  display.clearDisplay();
  display.setCursor(0, 0);
  
  display.setTextColor(WHITE);
  display.println("Hello Taras!");
  display.display();
  delay(2000);
  display.setTextSize(3);
}


void loop() {

  cnt = enc.read();
  
  display.clearDisplay();
  display.setCursor(30, 30);
  display.print(cnt / 4);
  display.display();
}
