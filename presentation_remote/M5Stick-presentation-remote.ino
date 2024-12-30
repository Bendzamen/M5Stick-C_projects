#include <M5StickC.h>
#include <BleKeyboard.h>

// the main button sends right arrow key to go forward in a presentation
// the side button sends left arrow key to go backwards in a presentation

//Se the name of the bluetooth keyboard (that shows up in the bluetooth menu of your device)
BleKeyboard bleKeyboard("m5 remote");

int M5_BUTTON_B = G39;

float v;
int batpercent = 100;
bool blestate = false;

void showstate(char *txt) {
    M5.Lcd.setCursor(0, 50);
    M5.Lcd.fillRect(0, 50, 160, 10, BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.printf(txt);
    Serial.println(txt);
}

void batStatus(){
  if (v!=M5.Axp.GetBatVoltage()) {
    M5.Lcd.setCursor(135, 5);
    M5.Lcd.fillRect(135, 0, 50, 15, TFT_BLACK);
    M5.Lcd.setTextSize(1);
    batpercent = (M5.Axp.GetBatVoltage() - 3.6) * 166.666; // random constants to calculate actual voltage
    if (batpercent<=15){
        M5.Lcd.setTextColor(TFT_RED);
    }
    else {
        M5.Lcd.setTextColor(TFT_WHITE);
    }

    M5.Lcd.print(batpercent);
    M5.Lcd.print("%");
    //M5.Lcd.printf(ASCIIpercent);
    Serial.print(batpercent);
    Serial.println("%");
    v = M5.Axp.GetBatVoltage();
    }
}

void lowBatOff(){
  if (batpercent <= 2) {
    M5.Lcd.fillScreen(RED);
    M5.Lcd.setCursor(20,28);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(TFT_BLACK);
    M5.Lcd.printf("LOW BATTERY");
    M5.Lcd.setCursor(20,45);
    M5.Lcd.setTextSize(2);
    M5.Lcd.printf("Turning OFF");
    delay(7000);
    M5.Axp.PowerOff();
  }
}

void setup() {
  //Start the Serial communication (with the computer at 115200 bits per second)
  Serial.begin(115200);
  M5.begin();
  //Send this message to the computer
  Serial.println("Starting BLE!");
  //Begin the BLE keyboard/start advertising the keyboard (so phones can find it)
  bleKeyboard.begin();
  pinMode(M5_BUTTON_HOME, INPUT);
  pinMode(M5_BUTTON_B, INPUT);
  M5.Axp.ScreenBreath(8);
  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_WHITE);
  M5.Lcd.println("BT name: m5 remote");
  
  M5.Lcd.setCursor(0, 40);
  M5.Lcd.println("Bluetooth:");
  showstate("Disconnected");
  v = M5.Axp.GetBatVoltage();
  Serial.println("starting");
}

void loop() {
  batStatus();
  lowBatOff();
  
  if (bleKeyboard.isConnected()) {
    //if the keyboard is connected to a device
    if (!blestate) {
        blestate = true;
        showstate("Connected");
    }
    
    if (digitalRead(M5_BUTTON_HOME) == LOW) {
        bleKeyboard.write(KEY_RIGHT_ARROW);
        bleKeyboard.releaseAll();
        delay(300);
    }
   else if (digitalRead(M5_BUTTON_B) == LOW){
    bleKeyboard.write(KEY_LEFT_ARROW);
    bleKeyboard.releaseAll();
    delay(300);
   }
  }
  
  else {
    if (blestate) {
        blestate = false;
        showstate("Disconnected");
    }
  }
  delay(10); 
}
