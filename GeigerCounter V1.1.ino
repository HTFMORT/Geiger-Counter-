#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h> 
#include <Math.h>  
#include <Preferences.h>
#include <icons.h>
#include <image.h>
#include <FS.h>
#include <LittleFS.h>




#define CS_PIN 22
#define LEDS_PIN 27

Preferences prefs;
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(CS_PIN);
TFT_eSprite sprite = TFT_eSprite(&tft);
TFT_eSprite iconSprite = TFT_eSprite(&tft);
TFT_eSprite overlaySprite = TFT_eSprite(&tft);
TFT_eSprite graphSprite = TFT_eSprite(&tft);
// ---------- LAYOUT ----------
#define SCREEN_W 480
#define SCREEN_H 320

#define BOX_X 40
#define BOX_Y 70
#define BOX_W 300
#define BOX_H 115

#define DOSE_X (BOX_X + 15)
#define DOSE_Y (BOX_Y + 15)
#define CPM_X  (BOX_X + 15)
#define CPM_Y  (BOX_Y + 60)

#define BAR_X  70
#define BAR_Y  190
#define BAR_W  250   
#define BAR_H  30

#define BTN_W  100
#define BTN_H  70
#define BTN_X  (SCREEN_W - BTN_W - 20)
#define BTN1_Y 70
#define BTN2_Y (BTN1_Y + BTN_H + 15)
#define BTN3_Y (BTN2_Y + BTN_H + 15)
#define BTN4_Y (BTN2_Y + BTN_H + 15)
#define BTN1_X 15
#define BTN2_X (BTN1_X + BTN_W + 15)
#define BTN3_X (BTN2_X + BTN_W + 15)

#define LOG_CURRENT_X 350
#define LOG_CURRENT_Y 170
#define LOG_CURRENT_W 100
#define LOG_CURRENT_H 50

#define OVERLAY_COLOR 0x4208  
#define OVERLAY_BORDER TFT_LIGHTGREY
#define OVERLAY_W 260
#define OVERLAY_H 60


#define GX 70
#define GY 245
#define GW 260
#define GH 120

#define SCREEN_MAIN  0
#define SCREEN_GRAPH 1
#define SCREEN_SETTINGS 2
#define SCREEN_CAPO 3


#define GRAPH_RESET_X 350
#define GRAPH_RESET_Y 270
#define GRAPH_RESET_W 100
#define GRAPH_RESET_H 35


#define PIN_LED      13  
#define PIN_GEIGER   34   
#define PIN_MOTORE   26  
#define PIN_BUZZER   25   
#define PIN_BATT     32  
#define PIN_ALIM     33  
#define INTERVALLO_MS 10000UL
#define DEBOUNCE_US 20000UL  
#define FATTORE_USV 153.8f   

volatile uint32_t contatore = 0;
volatile uint32_t contatoreXcpm = 0;
volatile bool nuovoImpulso = false;

const uint16_t* ICONS_64[4] = {
  icon_BuzzOn, icon_BuzzOff, icon_vibration, icon_combo
};
const uint16_t* OTHER_ICONS[2] = {
  icon_Graph, icon_Settings
};
const uint16_t* LED_ICONS[2] = {
  icon_LedOn, icon_LedOff
};
const uint16_t* ALARM_ICONS[2] = {
  icon_AlarmOn, icon_AlarmOff
};
const uint16_t* RESET_ICON[1] = {
  icon_Reset
};
const uint16_t* BAT_ICONS[3] = {
  icon_Bat, icon_BatCharge, icon_BatCharging
};

#define BAT_ICON_W 64
#define BAT_ICON_H 64

#define BAT_ICON_X (SCREEN_W - 76)
#define BAT_ICON_Y -8

#define CHARGER_DETECT_V 4.5f

int iconMode = 0; 
int alarmMode = 0;
int ledMode = 0;
bool reset = false;
bool capo = false;


bool overlayActive = false;
unsigned long overlayStart = 0;
const int overlayDuration = 1000;

char overlayText[32];


float dose = 0.12;
int cpm = 25;
unsigned long lastUpdate = 0;
const char* frame0[] = {
"           @@@@@@@@@@@@           ",
"       @@@@@@@@@@@@@@@@@@@@       ",
"     @@@@@@@@@@@@@@@@@@@@@@@@     ",
"   @@@@@   @@@@@@@@@@@@   @@@@@   ",
"  @@@@      @@@@@@@@@@      @@@@  ",
" @@@@        @@@@@@@@        @@@@ ",
" @@@          @@@@@@          @@@@",
"@@@           @@@@@@           @@@",
"@@@          @      @          @@@",
"@@@@@@@@@@@@@@      @@@@@@@@@@@@@@",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"@@@@@@@@@@@@@@      @@@@@@@@@@@@@@",
" @@@@@@@@@@@@        @@@@@@@@@@@@ ",
"  @@@@@@@@@@          @@@@@@@@@@  ",
"   @@@@@@@@            @@@@@@@@   ",
"     @@@@@@            @@@@@@     ",
"       @@@@@@@@@@@@@@@@@@@@       ",
"           @@@@@@@@@@@@           "
};

const char* frame1[] = {
"            @@@@@@@@@@@           ",
"       @@@@@@@@@@@@@@@@@@@@       ",
"     @@@@@@      @@@@@@@@@@@@     ",
"   @@@@@         @@@@@@@@@@@@@@   ",
"  @@@@           @@@@@@@@@@@@@@@  ",
" @@@@@           @@@@@@@@@@@@@@@@ ",
" @@@@@@@@        @@@@@@@@@    @@@@",
"@@@@@@@@@@@@  @@@@@@@@        @@@@",
"@@@@@@@@@@@@@@@     @          @@@",
"@@@@@@@@@@@@@@      @          @@@",
"@@@@@@@@@@@@  @@@@@@@@        @@@@",
"@@@@@@@@@        @@@@@@@@@    @@@@",
" @@@@@           @@@@@@@@@@@@@@@@ ",
"  @@@@           @@@@@@@@@@@@@@@  ",
"   @@@@@         @@@@@@@@@@@@@@   ",
"     @@@@@@      @@@@@@@@@@@@     ",
"       @@@@@@@@@@@@@@@@@@@@       ",
"           @@@@@@@@@@@@           "
};

const char* frame2[] = {
"            @@@@@@@@@@@           ",
"       @@@@@@@@@@@@@@@@@@@@       ",
"     @@@@@@            @@@@@@     ",
"   @@@@@@@@            @@@@@@@@   ",
"  @@@@@@@@@@          @@@@@@@@@@  ",
" @@@@@@@@@@@@        @@@@@@@@@@@@ ",
" @@@@@@@@@@@@@      @@@@@@@@@@@@@@",
"@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@",
"@@@@@@@@@@@@@@@     @@@@@@@@@@@@@@",
"@@@          @      @          @@@",
"@@@@          @@@@@@           @@@",
"@@@@          @@@@@@          @@@@",
" @@@@        @@@@@@@@        @@@@ ",
"  @@@@      @@@@@@@@@@      @@@@  ",
"   @@@@@   @@@@@@@@@@@@   @@@@@   ",
"     @@@@@@@@@@@@@@@@@@@@@@@@     ",
"       @@@@@@@@@@@@@@@@@@@@       ",
"           @@@@@@@@@@@@           "
};

const char* frame3[] = {
"            @@@@@@@@@@@           ",
"       @@@@@@@@@@@@@@@@@@@@       ",
"     @@@@@@@@@@@@      @@@@@@     ",
"   @@@@@@@@@@@@@@         @@@@@   ",
"  @@@@@@@@@@@@@@@           @@@@  ",
" @@@@@@@@@@@@@@@@           @@@@@ ",
" @@@    @@@@@@@@@        @@@@@@@@@",
"@@@@        @@@@@@@@  @@@@@@@@@@@@",
"@@@          @@     @@@@@@@@@@@@@@",
"@@@          @      @@@@@@@@@@@@@@",
"@@@@        @@@@@@@@  @@@@@@@@@@@@",
"@@@@     @@@@@@@@        @@@@@@@@@",
" @@@@@@@@@@@@@@@@           @@@@@ ",
"  @@@@@@@@@@@@@@@           @@@@  ",
"   @@@@@@@@@@@@@@         @@@@@   ",
"     @@@@@@@@@@@@      @@@@@@     ",
"       @@@@@@@@@@@@@@@@@@@@       ",
"           @@@@@@@@@@@@           "
};

const char** frames[] = {
  frame0,
  frame1,
  frame2,
  frame3
};

int len = 0;
uint8_t frameCounter = 4;
int rows = 18;
unsigned long startTime;
bool finished = false;
int frame = 0;
uint8_t CurrentScreen = 0;
bool touchNow = false;
bool touchWas = false;
bool touchReleased = false;



float xCalM = 0.0, yCalM = 0.0;
float xCalC = 0.0, yCalC = 0.0;

class ScreenPoint {
public:
  int16_t x;
  int16_t y;

  ScreenPoint(){
    
  }
  ScreenPoint(int16_t xIn, int16_t yIn){
    x = xIn;
    y = yIn;
  }
};

ScreenPoint touchPoint;
ScreenPoint releasedPoint;

struct Button {
  int x, y, w, h;
};

Button resetBtn = {20, 275, 100, 30};

Button backBtn  = {20, 20, 100, 40};
Button modeBtn  = {SCREEN_W - 120, 20, 100, 40};


bool isPressed = false;


  
#define SAMPLE_INTERVAL 1000




#define GRAPH_REALTIME 0
#define GRAPH_HISTORY  1

uint8_t graphMode = GRAPH_REALTIME;

float maxDose = 0.2;
float peakDose = 0.0;
float prevDose = 0;

unsigned long lastSample = 0;

bool graphInitialized = false;

#define MAX_POINTS 600
#define STATS_MAX_POINTS 3600

struct DataPoint {
  float dose;
  unsigned long time;
};


DataPoint statsHistory[STATS_MAX_POINTS];
int statsHistoryIndex = 0;
int statsValidPoints = 0;

DataPoint history[MAX_POINTS];
int historyIndex = 0;
int validPoints = 0;

const unsigned long windows[] = {
  60000,    // 1 min
  300000,   // 5 min
  600000    // 10 min
};

const char* windowLabels[] = {
  "1m",
  "5m",
  "10m"
};

int windowMode = 0;

unsigned long realtimeStart = 0;
int lastRealtimeX = -1;
int lastRealtimeY = -1;
unsigned long historyStart = 0;
bool historyRunning = false;
bool historyFinished = false;
// ---------- SETTINGS ----------
#define SETTINGS_MENU       0
#define SETTINGS_THRESHOLD  1
#define SETTINGS_DISPLAY    2

uint8_t settingsPage = SETTINGS_MENU;

// Main dose display mode
#define DOSE_REAL 0
#define DOSE_MAX  1
#define DOSE_MIN  2

uint8_t mainDoseMode = DOSE_REAL;

// Threshold for normal/elevated warning
float threshold = 0.30;

#define TIME_MINUTES 0
#define TIME_HOURS   1

uint16_t mainStatsIntervalValue = 1;   // 1, 2, 3...
uint8_t mainStatsTimeUnit = TIME_MINUTES;
bool mainStatsInfinite = false;

const char* doseModeLabels[] = {
  "REAL",
  "MAX",
  "MIN"
};




#define SETTINGS_DATA_LOG  3
#define SETTINGS_BATTERY   4
#define SETTINGS_CONTROL   5


uint8_t vibrationPower = 100;  // 10 to 100 %
uint8_t lightPower = 100;      // 10 to 100 %


// ---------- DATA LOGGING ----------
uint16_t logIntervalValue = 1;
uint8_t logIntervalUnit = TIME_MINUTES;   // TIME_MINUTES or TIME_HOURS

const char* LOG_FILE = "/geiger_log.txt";

#define LED_PWM_BITS      8
#define LED_PWM_MAX       255

#define MOTOR_PWM_BITS    12
#define MOTOR_PWM_MAX     4096
#define ALARM_MOTOR_POWER 100


bool buz, mot, alarmNot, leds;
bool alarmActive = false;
bool alarmWasActive = false;
// ---------- REAL GEIGER MEASUREMENTS ----------
uint32_t ultimoContatoreImpulsi = 0;
unsigned long lastGeigerCalc = 0;
float batteryVoltage = 0.0;
float alimVoltage = 0.0;
int batteryLevel = 0;

unsigned long lastBatteryUpdate = 0;
unsigned long lastChargerCheck = 0;

#define BATTERY_UPDATE_INTERVAL 120000UL  
#define CHARGER_CHECK_INTERVAL 500UL       


#define GEIGER_MOTOR_PULSE_MS 160UL

bool geigerMotorPulseActive = false;
unsigned long geigerMotorPulseEnd = 0;

void readBatteryNow();

unsigned long getGraphStartTime();

void clearGraphHistory();

void updateGraphMaxForWindow();

void drawBatteryIcon(int level);

void updateBatteryMeasurements();

void updateGeigerMotorPulse();

bool isAlarmEnabled();
bool isAlarmConditionActive();
void updateAlarmOutput();
void stopAlarmOutput();

void isrImpulso();
float lettureTensione(int j);
void handleGeigerPulseFeedback();
void updateGeigerMeasurements();
int batteryPercentFromVoltage(float voltage);


void drawControlScreen();
void setLogIntervalToCurrentRuntime();

void drawDataLogScreen();
void drawBatteryScreen();

int saveLogDataToFile();
void uploadLogDataToSerial();

unsigned long getLogIntervalMs();
void makeLogIntervalLabel(char* buffer, size_t size);

void drawStaticUI();
void drawButton(int x, int y, int w, int h, const char *label);

void updateDoseAndCPM(float dose, int cpm);
void updateIndicatorBar(float dose);

ScreenPoint getScreenCoords(int16_t x, int16_t y);
void calibrateTouchScreen();

void animation(const char* frame[], int posX, int posY, int scale);
void loading(int x, int y, int w, int h, int progress);

void drawBattery(int level);
void drawIconButton(int x, int y, int w, int h, const uint16_t* bmp);

bool getReleasedTouch(ScreenPoint &releasedPoint);
bool isInsideButton(ScreenPoint p, int x, int y, int w, int h);

void checkIconTouch();

void drawOverlay();
void clearOverlay();
void showNotification(const char* text);

void drawGraphScreen();
void drawGraph();
void handleGraphTouch();
void addDataPoint(float dose);

void drawGraphUI();
void drawXAxis();
void updateGraph(float d);

void resetRealtimeGraph();
void resetRealtimeOnly();
void resetHistoryOnly();

void updateTouch();
bool inside(int x, int y, int w, int h);

void GranCapo();

void saveSettings();
void loadSettings();

float getMainDoseValue();

void drawSettingsScreen();
void drawSettingsMenu();
void drawThresholdEditor();
void drawDisplayModeEditor();
void handleSettingsTouch();

unsigned long getMainStatsIntervalMs();
void makeStatsIntervalLabel(char* buffer, size_t size);
void addStatsDataPoint(float dose);

uint16_t getMaxStatsMinutes();
uint16_t getMaxStatsHours();



void setup() {
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_GEIGER, INPUT);
  pinMode(PIN_MOTORE, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  ledcAttach(PIN_LED, 5000, LED_PWM_BITS);
  ledcAttach(PIN_MOTORE, 5000, MOTOR_PWM_BITS);
  ledcAttach(LEDS_PIN, 5000, 12);

  attachInterrupt(digitalPinToInterrupt(PIN_GEIGER), isrImpulso, FALLING);

  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
  } else {
    Serial.println("LittleFS ready");
  }
  tft.init();
  tft.setRotation(1);
  ts.begin();
  ts.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  sprite.createSprite(240, 200);
  iconSprite.createSprite(BTN_W, BTN_H);
  overlaySprite.createSprite(OVERLAY_W, OVERLAY_H);
  sprite.setTextColor(TFT_YELLOW, TFT_BLACK);
  sprite.setTextSize(1);
  tft.setSwapBytes(true);
  iconSprite.setSwapBytes(true);
  prefs.begin("touch", true);

  bool done = prefs.getBool("done", false);

  if (done) {
    xCalM = prefs.getFloat("xCalM");
    xCalC = prefs.getFloat("xCalC");
    yCalM = prefs.getFloat("yCalM");
    yCalC = prefs.getFloat("yCalC");
  }

  prefs.end();
  prefs.begin("Icons", true);

  iconMode = prefs.getInt("iconMode");
  alarmMode = prefs.getInt("alarmMode");
  ledMode = prefs.getInt("ledMode");
  prefs.end();
  
  loadSettings();

  
  if (!done) {
    calibrateTouchScreen();
  }
  startTime = millis();

  int scale = 1;
  int x = 10;
  int y = 10;
  
  
  
  while(!finished){

    unsigned long elapsed = millis() - startTime;

    int progress = (elapsed * 100)/4000;

    if(progress >= 100){
      progress = 100;
      finished = true;
    }
    sprite.fillSprite(TFT_BLACK);

    animation(frames[frame], x, y, scale);
    frame = (frame + 1) % frameCounter;

    loading(10, 170, 200, 20, progress);

    sprite.pushSprite(
    (tft.width() -240)/2,
    (tft.height() - 160)/2);
    delay(60);

  }
  readBatteryNow();
  tft.fillScreen(TFT_BLACK);
  drawStaticUI();
  drawIconButton(BTN_X, BTN1_Y, BTN_W, BTN_H, ICONS_64[iconMode]);
  capo = false;
  noInterrupts();
  contatore = contatoreXcpm = 0;
  nuovoImpulso = false;
  interrupts();

  lastGeigerCalc = millis();
  Serial.println(F("Contatore Geiger integrato con UI pronto!"));


  

}


void loop() {

  updateTouch();
  

  

  if(ledMode == 0){
    int ledpower = map(lightPower, 0, 100, 0, 4096);
    ledcWrite(LEDS_PIN, ledpower);
  }
  else if(ledMode == 1){
    ledcWrite(LEDS_PIN, 0);
  }

  if(iconMode == 0){
    buz = true;
    mot = false;
  } //SUONO
  else if(iconMode == 1){
    buz = false;
    mot = false;
  } //SILENZIOSO
  else if(iconMode == 2){
    buz = false;
    mot = true;
  } //VIBRAZIONE
  else if(iconMode == 3){
    buz = true;
    mot = true;
  } //VIBRAZIONE + SUONO

  // ---------- TOUCH HANDLING ----------
  if (CurrentScreen == SCREEN_MAIN) {
    checkIconTouch();
  }
  else if (CurrentScreen == SCREEN_GRAPH) {
    handleGraphTouch();
  }
  else if (CurrentScreen == SCREEN_SETTINGS) {
    handleSettingsTouch();
  }
  else if (CurrentScreen == SCREEN_CAPO) {
    if (touchReleased) {
      ESP.restart();
    }
  }

  updateAlarmOutput();
  handleGeigerPulseFeedback();
  updateBatteryMeasurements();
  updateGeigerMotorPulse();
  // ---------- REAL GEIGER DATA UPDATE ----------
  updateGeigerMeasurements();

  // ---------- UI / GRAPH UPDATE ----------
  if (millis() - lastSample >= SAMPLE_INTERVAL) {

    lastSample = millis();
    
    if (reset) {
      noInterrupts();
      contatore = 0;
      contatoreXcpm = 0;
      interrupts();

      dose = 0.0;
      cpm = 0;
      ultimoContatoreImpulsi = 0;
      peakDose = 0.0;
      maxDose = 0.2;
      reset = false;
      stopAlarmOutput();
    }

    

    addDataPoint(dose);
    addStatsDataPoint(dose);

    if (CurrentScreen == SCREEN_MAIN) {
      float shownDose = getMainDoseValue();

      updateDoseAndCPM(shownDose, cpm);
      updateIndicatorBar(shownDose);

      if (overlayActive) {
        drawOverlay();
      }
    }

    if (CurrentScreen == SCREEN_GRAPH) {
      if (graphMode == GRAPH_REALTIME) {
        updateGraph(dose);
      } 
      else if (graphMode == GRAPH_HISTORY) {
        if (historyRunning) {
          drawGraph();
        }
      }
    }
  }

  // ---------- NOTIFICATION CLEAR ----------
  if (overlayActive && millis() - overlayStart > overlayDuration) {
    overlayActive = false;
    clearOverlay();
  }
}

// ---------- PARTI STATICI NEL UI ----------
void drawStaticUI() {

  
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 10);
  tft.print("MOMO, BIG, IL GRAN CAPO");

  tft.drawFastHLine(10, 40, SCREEN_W - 20, TFT_DARKGREY);

  
  tft.drawRoundRect(BOX_X, BOX_Y, BOX_W, BOX_H, 8, TFT_WHITE);
  
  
  drawIconButton(BTN_X, BTN1_Y, BTN_W, BTN_H, ICONS_64[iconMode]);


  drawIconButton(BTN_X, BTN2_Y, BTN_W, BTN_H, OTHER_ICONS[0]);
  drawIconButton(BTN_X, BTN3_Y, BTN_W, BTN_H, OTHER_ICONS[1]);
  drawIconButton(BTN1_X, BTN4_Y, BTN_W, BTN_H, ALARM_ICONS[alarmMode]);
  drawIconButton(BTN2_X, BTN4_Y, BTN_W, BTN_H, LED_ICONS[ledMode]);
  drawIconButton(BTN3_X, BTN4_Y, BTN_W, BTN_H, RESET_ICON[0]);
  
  drawBatteryIcon(batteryLevel);
  
}

void drawButton(int x, int y, int w, int h, const char *label) {

  tft.drawRoundRect(x, y, w, h, 8, TFT_CYAN);
  tft.setTextColor(TFT_CYAN);
  tft.setTextSize(2);
  int16_t tw = strlen(label) * 12;
  tft.setCursor(x + (w - tw) / 2, y + (h / 2) - 8);
  tft.print(label);
}

// ---------- AGGIORNAMENTI DINAMICI ----------
void updateDoseAndCPM(float dose, int cpm) {
  // clear rectangle interior
  tft.fillRect(BOX_X + 2, BOX_Y + 2, BOX_W - 4, BOX_H - 4, TFT_BLACK);

  // dose
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(4);
  tft.setCursor(DOSE_X, DOSE_Y);
  tft.printf("%.2f", dose);
  tft.setTextSize(2);
  tft.setCursor(DOSE_X + 150, DOSE_Y + 10);
  tft.print("uSv/h");

  // cpm
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(CPM_X, CPM_Y);
  tft.printf("CPM: %d", cpm);
}

void updateIndicatorBar(float dose) {
  const int segments = 15;
  const int gap = 3;
  const int segW = (BAR_W - (segments - 1) * gap) / segments;
  
  int barY = BOX_Y + 100 - 5;   
  int clearY = barY - 5;            
  float doseMax = 2.5;
  if (dose > doseMax) dose = doseMax;
  if (dose < 0.0) dose = 0.0;
  uint16_t color = TFT_GREEN;
  if (dose > threshold && dose <= 1.0) color = TFT_YELLOW;
  if (dose > 1.0) color = TFT_RED;
  float totalFill = (dose / doseMax) * segments;
  int filledSegments = (int)totalFill;
  float partial = totalFill - filledSegments;
  // --- clear ONLY the bar and below (no CPM overwrite) ---
  tft.fillRect(BAR_X - 5, clearY, BAR_W -1, BAR_H + 45, TFT_BLACK);

  // SEGMENTI
  for (int i = 0; i < segments; i++) {
    int x0 = BAR_X + i * (segW + gap);

    tft.drawRect(x0, barY, segW, BAR_H, TFT_DARKGREY);

    if (i < filledSegments)
      tft.fillRect(x0 + 1, barY + 1, segW - 2, BAR_H - 2, color);

    else if (i == filledSegments && partial > 0.01) {
      int pw = (int)((segW - 2) * partial);
      tft.fillRect(x0 + 1, barY + 1, pw, BAR_H - 2, color);
    }
  }

  // Triangolo
  float totalBarWidth = segments * (segW + gap) - gap;
  int triX = BAR_X + (dose / doseMax) * totalBarWidth;

  tft.fillTriangle(
    triX, barY - 4,
    triX - 7, barY - 18,
    triX + 7, barY - 18,
    TFT_WHITE
  );

  // Spazi Neri (COME ME)
  tft.setTextColor(TFT_LIGHTGREY);
  tft.setTextSize(1);

  for (int i = 0; i <= 5; i++) {
    float value = doseMax * i / 5.0;
    int lx = BAR_X + (int)(totalBarWidth * i / 5.0);

    tft.drawFastVLine(lx, barY + BAR_H + 1, 4, TFT_DARKGREY);

    char label[8];
    snprintf(label, sizeof(label), "%.1f", value);

    int tw = strlen(label) * 6;
    tft.setCursor(lx - tw / 2, barY + BAR_H + 8);
    tft.print(label);
  }

  // --- status text under the ticks ---
  tft.fillRect(BAR_X, barY + BAR_H + 20, 150, 20, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(BAR_W - 35, CPM_Y + 15);

  if (dose < threshold)
  tft.print("NORMAL");
  else if (dose < 1.0)
    tft.print("ELEVATED");
  else
    tft.print("ALERT");
}


ScreenPoint getScreenCoords(int16_t x, int16_t y){
  int16_t xCoord = round((x * xCalM) + xCalC);
  int16_t yCoord = round((y * yCalM) + yCalC);
  if(xCoord < 0) xCoord = 0;
  if(xCoord >= tft.width()) xCoord = tft.width() - 1;
  if(yCoord < 0) yCoord = 0;
  if(yCoord >= tft.height()) yCoord = tft.height() - 1;
  return(ScreenPoint(xCoord, yCoord));
}

void calibrateTouchScreen(){
  TS_Point p;
  int16_t x1,y1,x2,y2;

  tft.fillScreen(TFT_BLACK);
  // wait for no touch
  while(ts.touched());
  tft.drawFastHLine(10,20,20,TFT_RED);
  tft.drawFastVLine(20,10,20,TFT_RED);
  while(!ts.touched());
  delay(50);
  p = ts.getPoint();
  x1 = p.x;
  y1 = p.y;
  tft.drawFastHLine(10,20,20,TFT_BLACK);
  tft.drawFastVLine(20,10,20,TFT_BLACK);
  delay(500);
  while(ts.touched());
  tft.drawFastHLine(tft.width() - 30,tft.height() - 20,20,TFT_RED);
  tft.drawFastVLine(tft.width() - 20,tft.height() - 30,20,TFT_RED);
  while(!ts.touched());
  delay(50);
  p = ts.getPoint();
  x2 = p.x;
  y2 = p.y;
  tft.drawFastHLine(tft.width() - 30,tft.height() - 20,20,TFT_BLACK);
  tft.drawFastVLine(tft.width() - 20,tft.height() - 30,20,TFT_BLACK);

  int16_t xDist = tft.width() - 40;
  int16_t yDist = tft.height() - 40;



  xCalM = (float)xDist / (float)(x2 - x1);
  xCalC = 20.0 - ((float)x1 * xCalM);

  yCalM = (float)yDist / (float)(y2 - y1);
  yCalC = 20.0 - ((float)y1 * yCalM);

  prefs.begin("touch", false);

  prefs.putFloat("xCalM", xCalM);
  prefs.putFloat("xCalC", xCalC);
  prefs.putFloat("yCalM", yCalM);
  prefs.putFloat("yCalC", yCalC);
  prefs.putBool("done", true);
  
  prefs.end();

  Serial.print("x1 = ");Serial.print(x1);
  Serial.print(", y1 = ");Serial.print(y1);
  Serial.print("x2 = ");Serial.print(x2);
  Serial.print(", y2 = ");Serial.println(y2);
  Serial.print("xCalM = ");Serial.print(xCalM);
  Serial.print(", xCalC = ");Serial.print(xCalC);
  Serial.print("yCalM = ");Serial.print(yCalM);
  Serial.print(", yCalC = ");Serial.println(yCalC);



}


void animation(const char* frame[], int posX, int posY, int scale){

  

  uint8_t charW = 6 * scale;
  uint8_t charH = 8 * scale;
  int textW = strlen(frame[0])*charW;
  int textH = charH* rows;

  sprite.setTextSize(scale);


  for(int i = 0; i < rows; i++){
    
    sprite.setCursor(posX, posY + i * charH);
    sprite.print(frame[i]);
  }




}

void loading(int x, int y, int w, int h, int progress) {
  sprite.drawRect(x, y, w, h, TFT_WHITE);
  sprite.fillRect(x+2, y+2, (w-4) * progress / 100, h-4, TFT_WHITE);
}


void drawBattery(int level) {
  drawBatteryIcon(level);
}


void drawIconButton(int x, int y, int w, int h, const uint16_t* bmp) {


  const int iconW = 64;
  const int iconH = 64;
  iconSprite.fillSprite(TFT_BLACK);

  iconSprite.drawRoundRect(0, 0, w, h, 8, TFT_CYAN);

  int cx = (w / 2) - (iconW /2);
  int cy = (h / 2) - (iconH /2);

  
  iconSprite.pushImage(cx, cy, 64, 64, bmp);
  
  iconSprite.pushSprite(x, y);
}




bool getReleasedTouch(ScreenPoint &releasedPoint) {
  static bool wasTouched = false;
  static ScreenPoint lastPoint;

  bool touchedNow = ts.touched();

  if (touchedNow) {
    TS_Point raw = ts.getPoint();
    lastPoint = getScreenCoords(raw.x, raw.y);

    // Debug, remove later
    Serial.print("Touch X: ");
    Serial.print(lastPoint.x);
    Serial.print(" Y: ");
    Serial.println(lastPoint.y);
  }

  if (wasTouched && !touchedNow) {
    releasedPoint = lastPoint;
    wasTouched = touchedNow;
    return true;
  }

  wasTouched = touchedNow;
  return false;
}


bool isInsideButton(ScreenPoint p, int x, int y, int w, int h) {
  return (
    p.x >= x &&
    p.x <= x + w &&
    p.y >= y &&
    p.y <= y + h
  );
}



void checkIconTouch() {
  if (!touchReleased) return;

  const char* modeText[4] = {
    "Suono",
    "Silenzioso",
    "Vibrazione",
    "Vibrazione + Suono"
  };

  // BUTTONE MULTITASKING
  if (inside(BTN_X, BTN1_Y, BTN_W, BTN_H)) {
    iconMode = (iconMode + 1) % 4;

    drawIconButton(BTN_X, BTN1_Y, BTN_W, BTN_H, ICONS_64[iconMode]);
    showNotification(modeText[iconMode]);

    prefs.begin("Icons", false);
    prefs.putInt("iconMode", iconMode);
    prefs.end();

    return;
  }
  


  if (inside(BTN1_X, BTN4_Y, BTN_W, BTN_H)){
    alarmMode = (alarmMode +1) % 2;

    drawIconButton(BTN1_X, BTN4_Y, BTN_W, BTN_H, ALARM_ICONS[alarmMode]);

    prefs.begin("Icons", false);
    prefs.putInt("alarmMode", alarmMode);
    prefs.end();

    if (!isAlarmEnabled()){
    stopAlarmOutput();
    }

    return;

  }
  if (inside(BTN2_X, BTN4_Y, BTN_W, BTN_H)){
    ledMode = (ledMode +1) % 2;

    drawIconButton(BTN2_X, BTN4_Y, BTN_W, BTN_H, LED_ICONS[ledMode]);

    prefs.begin("Icons", false);
    prefs.putInt("ledMode", ledMode);
    prefs.end();

    return;

  }
  if (inside(BTN3_X, BTN4_Y, BTN_W, BTN_H)){
    reset = true;

    return;
    


  }
  if(inside(160, 20, 140, 20)){
    GranCapo();
    
    
  }


  // BUTTONE GRAFICO
  if (inside(BTN_X, BTN2_Y, BTN_W, BTN_H)) {
  CurrentScreen = SCREEN_GRAPH;
  graphMode = GRAPH_REALTIME;

  tft.fillScreen(TFT_BLACK);

  if (!graphInitialized) {
    graphSprite.setColorDepth(8);
    graphSprite.createSprite(GW, GH);

    if (!graphSprite.created()) {
      Serial.println("Graph sprite creation failed");
      CurrentScreen = SCREEN_MAIN;
      tft.fillScreen(TFT_BLACK);
      drawStaticUI();
      updateDoseAndCPM(dose, cpm);
      updateIndicatorBar(dose);
      return;
    }

    graphInitialized = true;
  }


  drawGraphUI();
  updateGraph(dose);

  return;
  }

  // BUTTONE IMPOSTAZIONE
  
  if (inside(BTN_X, BTN3_Y, BTN_W, BTN_H)) {
    CurrentScreen = SCREEN_SETTINGS;
    settingsPage = SETTINGS_MENU;

    tft.fillScreen(TFT_BLACK);
    drawSettingsScreen();

    return;
  }
}




void drawOverlay() {
  int x = (SCREEN_W - OVERLAY_W) / 2;
  int y = (SCREEN_H - OVERLAY_H) / 2;

  tft.fillRoundRect(x, y, OVERLAY_W, OVERLAY_H, 12, OVERLAY_COLOR);
  tft.drawRoundRect(x, y, OVERLAY_W, OVERLAY_H, 12, TFT_LIGHTGREY);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, OVERLAY_COLOR);

  int tw = strlen(overlayText) * 12;
  tft.setCursor(x + (OVERLAY_W - tw) / 2, y + (OVERLAY_H / 2) - 8);
  tft.print(overlayText);
}

void clearOverlay() {
  int x = (SCREEN_W - OVERLAY_W) / 2;
  int y = (SCREEN_H - OVERLAY_H) / 2;

  tft.fillRoundRect(x, y, OVERLAY_W, OVERLAY_H, 12, TFT_BLACK);

  if (CurrentScreen == SCREEN_MAIN) {
    tft.drawRoundRect(BOX_X, BOX_Y, BOX_W, BOX_H, 8, TFT_WHITE);
    updateDoseAndCPM(dose, cpm);
    updateIndicatorBar(dose);
    
    drawIconButton(BTN_X, BTN1_Y, BTN_W, BTN_H, ICONS_64[iconMode]);
    drawIconButton(BTN_X, BTN2_Y, BTN_W, BTN_H, OTHER_ICONS[0]);
    drawIconButton(BTN_X, BTN3_Y, BTN_W, BTN_H, OTHER_ICONS[1]);
  }
}

void showNotification(const char* text) {
  strncpy(overlayText, text, sizeof(overlayText));
  overlayText[sizeof(overlayText) - 1] = '\0';

  overlayActive = true;
  overlayStart = millis();

  drawOverlay();
}





void drawGraphScreen() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 10);
  tft.print("Dose Graph");

  // BUTTONE DI RITORNO
  drawButton(20, 260, 100, 40, "BACK");

  // BUTTONO CHE CAMBIA INTERVALLO
  drawButton(350, 260, 100, 40, "INTERVAL");
  tft.fillRect(40, 50, 380, 150, TFT_BLACK);
  drawGraph();
}

void drawGraph() {
  if (graphMode != GRAPH_HISTORY) return;

  updateGraphMaxForWindow();

  // Clear graph area
  tft.fillRect(GX + 1, GY - GH + 1, GW - 2, GH - 2, TFT_BLACK);

  // Redraw grid
  for (int i = 0; i <= 5; i++) {
    int y = GY - (GH * i) / 5;
    tft.drawFastHLine(GX, y, GW, TFT_DARKGREY);
  }

  tft.drawRect(GX, GY - GH, GW, GH, TFT_WHITE);

  if (validPoints < 1) return;

  unsigned long window = windows[windowMode];

  unsigned long startTime = getGraphStartTime();
  unsigned long endTime = startTime + window;

  int prevX = -1;
  int prevY = -1;

  for (int i = 0; i < validPoints; i++) {
    int idx = (historyIndex - validPoints + i + MAX_POINTS) % MAX_POINTS;

    unsigned long t = history[idx].time;

    if (t < startTime) continue;
    if (t > endTime) continue;

    float xNorm = (float)(t - startTime) / (float)window;

    if (xNorm < 0.0f || xNorm > 1.0f) continue;

    int x = GX + xNorm * GW;

    int y = GY - (history[idx].dose / maxDose) * GH;

    if (y < GY - GH) y = GY - GH;
    if (y > GY) y = GY;

    if (prevX != -1) {
      tft.drawLine(prevX, prevY, x, y, TFT_GREEN);
    } else {
      tft.drawPixel(x, y, TFT_GREEN);
    }

    prevX = x;
    prevY = y;
  }
}



void handleGraphTouch() {
  if (!touchReleased) return;

  // BACK
  if (inside(20, 15, 90, 35)) {

    if (graphInitialized) {
      graphSprite.deleteSprite();
      graphInitialized = false;
    }

    CurrentScreen = SCREEN_MAIN;

    tft.fillScreen(TFT_BLACK);
    drawStaticUI();
    updateDoseAndCPM(dose, cpm);
    updateIndicatorBar(dose);

    return;
  }

  // MODE BUTTON
  if (inside(360, 15, 100, 35)) {

  graphMode = (graphMode == GRAPH_REALTIME) ? GRAPH_HISTORY : GRAPH_REALTIME;

  tft.fillScreen(TFT_BLACK);
  drawGraphUI();

  if (graphMode == GRAPH_REALTIME) {
    updateGraph(dose);
  } else {
    drawGraph();
  }

  return;
}


  // WINDOW / INTERVAL
  if (inside(190, 270, 100, 35)) {

  windowMode = (windowMode + 1) % 3;

  tft.fillScreen(TFT_BLACK);
  drawGraphUI();

  if (graphMode == GRAPH_REALTIME) {
    updateGraph(dose);
  } else {
    drawGraph();
  }

  return;
}
  // RESET BUTTON
  if (graphMode == GRAPH_HISTORY &&
    inside(GRAPH_RESET_X, GRAPH_RESET_Y, GRAPH_RESET_W, GRAPH_RESET_H)) {

  clearGraphHistory();
  return;
}
}

void addDataPoint(float dose) {
  history[historyIndex].dose = dose;
  history[historyIndex].time = millis();

  historyIndex = (historyIndex + 1) % MAX_POINTS;

  if (validPoints < MAX_POINTS) validPoints++;
}


void drawGraphUI() {

  updateGraphMaxForWindow();
  tft.fillScreen(TFT_BLACK);
  
  // Title
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(145, 10);

  if (graphMode == GRAPH_REALTIME) {
    tft.print("REALTIME");
  } else {
    tft.print("HISTORY");
  }

  // Buttons
  drawButton(20, 15, 90, 35, "BACK");

  if (graphMode == GRAPH_REALTIME) {
    drawButton(360, 15, 100, 35, "HIST");
  } else {
    drawButton(360, 15, 100, 35, "REAL");
  }

  drawButton(190, 270, 100, 35, windowLabels[windowMode]);

  // Axis text
  tft.setTextSize(1);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);

  tft.setCursor(GX - 45, GY - GH - 12);
  tft.print("uSv/h");

  tft.setCursor(GX + GW - 30, GY + 25);
  tft.print("time");

  // Border
  tft.drawRect(GX, GY - GH, GW, GH, TFT_WHITE);

  // Grid + Y labels
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

  for (int i = 0; i <= 5; i++) {
    int y = GY - (GH * i) / 5;

    float value = maxDose * i / 5.0;

    char label[10];
    snprintf(label, sizeof(label), "%.1f", value);

    tft.setCursor(GX - 35, y - 3);
    tft.print(label);

    tft.drawFastHLine(GX, y, GW, TFT_DARKGREY);
  }

  drawXAxis();

  // Border again, on top of grid
  tft.drawRect(GX, GY - GH, GW, GH, TFT_WHITE);
  if (graphMode == GRAPH_HISTORY) {
    drawButton(GRAPH_RESET_X, GRAPH_RESET_Y, GRAPH_RESET_W, GRAPH_RESET_H, "RESET");
  }
}


void drawXAxis() {
  tft.setTextSize(1);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

  unsigned long windowSec = windows[windowMode] / 1000;

  for (int i = 0; i <= 5; i++) {
    int x = GX + (GW * i) / 5;

    tft.drawFastVLine(x, GY, 5, TFT_WHITE);

    unsigned long value = (windowSec * i) / 5;

    char label[12];

    if (value >= 60) {
      snprintf(label, sizeof(label), "%lum", value / 60);
    } else {
      snprintf(label, sizeof(label), "%lus", value);
    }

    tft.setCursor(x - 12, GY + 8);
    tft.print(label);
  }
}



void updateGraph(float d) {
  if (graphMode != GRAPH_REALTIME) return;
  if (!graphInitialized) return;

  float oldMax = maxDose;
  updateGraphMaxForWindow();

  // If Y scale changed, redraw the axes and labels
  if (abs(maxDose - oldMax) > 0.001f) {
    drawGraphUI();
  }

  graphSprite.fillSprite(TFT_BLACK);

  if (validPoints < 1) {
    graphSprite.pushSprite(GX, GY - GH);
    return;
  }

  unsigned long now = millis();
  unsigned long window = windows[windowMode];

  unsigned long startTime = getGraphStartTime();
  unsigned long endTime = startTime + window;

  int prevX = -1;
  int prevY = -1;

  for (int i = 0; i < validPoints; i++) {
    int idx = (historyIndex - validPoints + i + MAX_POINTS) % MAX_POINTS;

    unsigned long t = history[idx].time;

    if (t < startTime) continue;
    if (t > endTime) continue;

    float xNorm = (float)(t - startTime) / (float)window;

    if (xNorm < 0.0f || xNorm > 1.0f) continue;

    int x = xNorm * (GW - 1);

    int y = GH - (history[idx].dose / maxDose) * GH;

    if (y < 0) y = 0;
    if (y >= GH) y = GH - 1;

    if (prevX != -1) {
      graphSprite.drawLine(prevX, prevY, x, y, TFT_GREEN);
    } else {
      graphSprite.drawPixel(x, y, TFT_GREEN);
    }

    prevX = x;
    prevY = y;
  }

  graphSprite.pushSprite(GX, GY - GH);
}






void resetRealtimeGraph() {
  lastRealtimeX = -1;
  lastRealtimeY = -1;
  prevDose = dose;

  if (graphInitialized) {
    graphSprite.fillSprite(TFT_BLACK);
    graphSprite.pushSprite(GX, GY - GH);
  }
}










void updateTouch() {
  touchReleased = false;

  touchNow = ts.touched();

  if (touchNow) {
    TS_Point raw = ts.getPoint();
    touchPoint = getScreenCoords(raw.x, raw.y);
  }

  if (touchWas && !touchNow) {
    releasedPoint = touchPoint;
    touchReleased = true;

    Serial.print("RELEASE x=");
    Serial.print(releasedPoint.x);
    Serial.print(" y=");
    Serial.println(releasedPoint.y);
  }

  touchWas = touchNow;
}









bool inside(int x, int y, int w, int h) {
  bool hit =
    releasedPoint.x >= x &&
    releasedPoint.x <= x + w &&
    releasedPoint.y >= y &&
    releasedPoint.y <= y + h;

  Serial.print("Checking box x=");
  Serial.print(x);
  Serial.print("-");
  Serial.print(x + w);
  Serial.print(" y=");
  Serial.print(y);
  Serial.print("-");
  Serial.print(y + h);
  Serial.print(" | released x=");
  Serial.print(releasedPoint.x);
  Serial.print(" y=");
  Serial.print(releasedPoint.y);
  Serial.print(" | hit=");
  Serial.println(hit ? "YES" : "NO");

  return hit;
}


void GranCapo(){
  CurrentScreen = SCREEN_CAPO;
  tft.fillScreen(TFT_BLACK);
  tft.pushImage(0, 0, 240, 320, dellana);
  tft.pushImage(240, 0, 240, 320, dellana);
  capo = true;
  
 
  
}


void resetRealtimeOnly() {
  prevDose = dose;

  if (graphInitialized) {
    graphSprite.fillSprite(TFT_BLACK);
    graphSprite.pushSprite(GX, GY - GH);
  }
}


void resetHistoryOnly() {
  historyRunning = true;
  historyFinished = false;

  tft.fillScreen(TFT_BLACK);
  drawGraphUI();
  drawGraph();
}



void saveSettings() {
  prefs.begin("settings", false);

  prefs.putFloat("threshold", threshold);
  prefs.putUChar("doseMode", mainDoseMode);

  prefs.putUShort("statVal", mainStatsIntervalValue);
  prefs.putUChar("statUnit", mainStatsTimeUnit);
  prefs.putBool("statInf", mainStatsInfinite);

  prefs.putUChar("vibPower", vibrationPower);
  prefs.putUChar("lightPower", lightPower);

  prefs.end();
}



void loadSettings() {
  prefs.begin("settings", true);

  threshold = prefs.getFloat("threshold", 0.30);
  mainDoseMode = prefs.getUChar("doseMode", DOSE_REAL);

  mainStatsIntervalValue = prefs.getUShort("statVal", 1);
  mainStatsTimeUnit = prefs.getUChar("statUnit", TIME_MINUTES);
  mainStatsInfinite = prefs.getBool("statInf", false);

  vibrationPower = prefs.getUChar("vibPower", 100);
  lightPower = prefs.getUChar("lightPower", 100);

  prefs.end();

  if (mainDoseMode > DOSE_MIN) mainDoseMode = DOSE_REAL;

  if (mainStatsIntervalValue < 1) mainStatsIntervalValue = 1;

  if (mainStatsTimeUnit > TIME_HOURS) {
    mainStatsTimeUnit = TIME_MINUTES;
  }


  if (mainStatsTimeUnit == TIME_MINUTES) {
  uint16_t maxMin = getMaxStatsMinutes();

  if (mainStatsIntervalValue > maxMin) {
    mainStatsIntervalValue = maxMin;
  }
}

if (mainStatsTimeUnit == TIME_HOURS) {
  uint16_t maxHours = getMaxStatsHours();

  if (maxHours < 1) {
    maxHours = 1;
  }

  if (mainStatsIntervalValue > maxHours) {
    mainStatsIntervalValue = maxHours;
  }
}

if (mainStatsIntervalValue < 1) {
  mainStatsIntervalValue = 1;
}
  if (vibrationPower < 10) vibrationPower = 10;
  if (vibrationPower > 100) vibrationPower = 100;

  if (lightPower < 10) lightPower = 10;
  if (lightPower > 100) lightPower = 100;

}



float getMainDoseValue() {
  if (mainDoseMode == DOSE_REAL) {
    return dose;
  }

  if (statsValidPoints < 1) {
    return dose;
  }

  unsigned long now = millis();
  unsigned long intervalMs = getMainStatsIntervalMs();

  bool found = false;
  float result = dose;

  for (int i = 0; i < statsValidPoints; i++) {
    int idx = (statsHistoryIndex - statsValidPoints + i + STATS_MAX_POINTS) % STATS_MAX_POINTS;

    unsigned long t = statsHistory[idx].time;

    if (!mainStatsInfinite) {
      if (now - t > intervalMs) continue;
    }

    float d = statsHistory[idx].dose;

    if (!found) {
      result = d;
      found = true;
    } else {
      if (mainDoseMode == DOSE_MAX && d > result) {
        result = d;
      }

      if (mainDoseMode == DOSE_MIN && d < result) {
        result = d;
      }
    }
  }

  if (!found) {
    return dose;
  }

  return result;
}



void drawSettingsScreen() {
  if (settingsPage == SETTINGS_MENU) {
    drawSettingsMenu();
  } 
  else if (settingsPage == SETTINGS_THRESHOLD) {
    drawThresholdEditor();
  } 
  else if (settingsPage == SETTINGS_DISPLAY) {
    drawDisplayModeEditor();
  }
  else if (settingsPage == SETTINGS_DATA_LOG) {
    drawDataLogScreen();
  }
  else if (settingsPage == SETTINGS_BATTERY) {
    drawBatteryScreen();
  }
  else if (settingsPage == SETTINGS_CONTROL) {
  drawControlScreen();
  }
}



void drawSettingsMenu() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(170, 10);
  tft.print("Impostazione");

  drawButton(20, 15, 90, 35, "BACK");

  drawButton(40, 70, 180, 45, "CALIBRATE");
  drawButton(260, 70, 180, 45, "Soglia");

  drawButton(40, 140, 400, 45, "Modalita Dose");

  drawButton(40, 260, 180, 40, "DATA LOG");

  drawButton(260, 260, 180, 40, "BATTERY");

  drawButton(285, 205, 155, 40, "CONTROL"); 

  tft.setTextSize(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);

  tft.setCursor(50, 205);
  tft.print("Soglia: ");
  tft.print(threshold, 2);
  tft.print(" uSv/h");

  tft.setCursor(50, 225);
  tft.print("Main dose: ");
  tft.print(doseModeLabels[mainDoseMode]);

  if (mainDoseMode != DOSE_REAL) {
    char label[20];
    makeStatsIntervalLabel(label, sizeof(label));

    tft.print(" / ");
    tft.print(label);
  }
}



void drawThresholdEditor() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(150, 10);
  tft.print("Soglia");

  drawButton(20, 15, 90, 35, "BACK");

  tft.setTextSize(4);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(150, 95);
  tft.printf("%.2f", threshold);

  tft.setTextSize(2);
  tft.setCursor(285, 110);
  tft.print("uSv/h");

  drawButton(100, 200, 90, 60, "-");
  drawButton(290, 200, 90, 60, "+");
}



void drawDisplayModeEditor() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(150, 10);
  tft.print("Modalita Dose");

  drawButton(20, 15, 90, 35, "BACK");

  // Mode button
  drawButton(130, 65, 220, 45, doseModeLabels[mainDoseMode]);

  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(60, 120);
  tft.print("Valore sUllo schermo Principale");

  if (mainDoseMode == DOSE_REAL) {
    tft.setTextSize(2);
    tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft.setCursor(120, 190);
    tft.print("Dose tempo reale");
    return;
  }

  char label[20];
  makeStatsIntervalLabel(label, sizeof(label));

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  tft.setCursor(80, 165);
  tft.print("Interval:");

  tft.setTextSize(3);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(190, 155);
  tft.print(label);

  // Minus / plus
  drawButton(60, 230, 70, 55, "-");
  drawButton(150, 230, 70, 55, "+");

  // Unit button
  if (mainStatsTimeUnit == TIME_MINUTES) {
  drawButton(250, 230, 90, 55, "MIN");
} else {
  drawButton(250, 230, 90, 55, "HOUR");
}

drawButton(360, 230, 90, 55, mainStatsInfinite ? "FINITE" : "INF");
}







void handleSettingsTouch() {
  if (!touchReleased) return;

  // ---------- SETTINGS MENU ----------
  if (settingsPage == SETTINGS_MENU) {

    // BACK to main
    if (inside(20, 15, 90, 35)) {
      CurrentScreen = SCREEN_MAIN;

      tft.fillScreen(TFT_BLACK);
      drawStaticUI();
      updateDoseAndCPM(getMainDoseValue(), cpm);
      updateIndicatorBar(getMainDoseValue());

      return;
    }

    // CALIBRATE
    if (inside(40, 65, 180, 40)) {
      tft.fillScreen(TFT_BLACK);

      prefs.begin("touch", false);
      prefs.clear();
      prefs.end();

      delay(300);
      calibrateTouchScreen();

      tft.fillScreen(TFT_BLACK);
      drawSettingsScreen();

      return;
    }

    // THRESHOLD
    if (inside(260, 65, 180, 40)) {
      settingsPage = SETTINGS_THRESHOLD;
      drawSettingsScreen();
      return;
    }

    // DOSE MODE
    if (inside(40, 140, 400, 45)) {
      settingsPage = SETTINGS_DISPLAY;
      drawSettingsScreen();
      return;
    }

    // DATA LOG
    if (inside(40, 260, 180, 40)) {
      settingsPage = SETTINGS_DATA_LOG;
      drawSettingsScreen();
      return;
    }

    // BATTERY
    if (inside(260, 260, 180, 40)) {
      settingsPage = SETTINGS_BATTERY;
      drawSettingsScreen();
      return;
    }
    // CONTROL
    if (inside(285, 205, 155, 40)) {
    settingsPage = SETTINGS_CONTROL;
    drawSettingsScreen();
    return;
    }
  }

  // ---------- THRESHOLD EDITOR ----------
  else if (settingsPage == SETTINGS_THRESHOLD) {

    // BACK to settings menu
    if (inside(20, 15, 90, 35)) {
      settingsPage = SETTINGS_MENU;
      saveSettings();
      drawSettingsScreen();
      return;
    }

    // Minus threshold
    if (inside(100, 200, 90, 60)) {
      threshold -= 0.05;
      if (threshold < 0.05) threshold = 0.05;

      saveSettings();
      drawThresholdEditor();
      return;
    }

    // Plus threshold
    if (inside(290, 200, 90, 60)) {
      threshold += 0.05;
      if (threshold > 5.00) threshold = 5.00;

      saveSettings();
      drawThresholdEditor();
      return;
    }
  }

  // ---------- DISPLAY MODE EDITOR ----------
  else if (settingsPage == SETTINGS_DISPLAY) {

    // BACK to settings menu
    if (inside(20, 15, 90, 40)) {
      settingsPage = SETTINGS_MENU;
      saveSettings();
      drawSettingsScreen();
      return;
    }

    // Mode button: REAL -> MAX -> MIN
    if (inside(130, 65, 220, 45)) {
      mainDoseMode = (mainDoseMode + 1) % 3;

      saveSettings();
      drawDisplayModeEditor();
      return;
    }

    // If REAL, interval controls are disabled
    if (mainDoseMode == DOSE_REAL) {
      return;
    }

    // Minus interval
    if (inside(60, 230, 70, 55)) {

      if (mainStatsInfinite) {
        mainStatsInfinite = false;
        mainStatsIntervalValue = 1;
      } 
      else {
        if (mainStatsIntervalValue > 1) {
          mainStatsIntervalValue--;
        }
      }

      saveSettings();
      drawDisplayModeEditor();
      return;
    }

    // Plus interval
    if (inside(150, 230, 70, 55)) {

      if (mainStatsInfinite) {
        mainStatsInfinite = false;
        mainStatsIntervalValue = 1;
      } 
      else {
        mainStatsIntervalValue++;

        if (mainStatsTimeUnit == TIME_MINUTES) {
          uint16_t maxMin = getMaxStatsMinutes();

          if (mainStatsIntervalValue > maxMin) {
            mainStatsIntervalValue = maxMin;
          }
        } 
        else if (mainStatsTimeUnit == TIME_HOURS) {
          uint16_t maxHours = getMaxStatsHours();

          if (maxHours < 1) maxHours = 1;

          if (mainStatsIntervalValue > maxHours) {
            mainStatsIntervalValue = maxHours;
          }
        }
      }

      saveSettings();
      drawDisplayModeEditor();
      return;
    }

    // Unit button: MIN <-> HOUR
    if (inside(250, 230, 90, 55)) {

      mainStatsInfinite = false;

      if (mainStatsTimeUnit == TIME_MINUTES) {
        mainStatsTimeUnit = TIME_HOURS;

        uint16_t maxHours = getMaxStatsHours();
        if (maxHours < 1) maxHours = 1;

        if (mainStatsIntervalValue > maxHours) {
          mainStatsIntervalValue = maxHours;
        }
      } 
      else {
        mainStatsTimeUnit = TIME_MINUTES;

        uint16_t maxMin = getMaxStatsMinutes();
        if (maxMin < 1) maxMin = 1;

        if (mainStatsIntervalValue > maxMin) {
          mainStatsIntervalValue = maxMin;
        }
      }

      saveSettings();
      drawDisplayModeEditor();
      return;
    }

    // Infinite button
    if (inside(360, 230, 90, 55)) {

      mainStatsInfinite = !mainStatsInfinite;

      if (!mainStatsInfinite && mainStatsIntervalValue < 1) {
        mainStatsIntervalValue = 1;
      }

      saveSettings();
      drawDisplayModeEditor();
      return;
    }
  }

  // ---------- DATA LOG SCREEN ----------
  else if (settingsPage == SETTINGS_DATA_LOG) {

    // BACK
    if (inside(20, 15, 90, 35)) {
      settingsPage = SETTINGS_MENU;
      drawSettingsScreen();
      return;
    }

    // Minus interval
    if (inside(60, 170, 70, 50)) {
      if (logIntervalValue > 1) {
        logIntervalValue--;
      }

      drawDataLogScreen();
      return;
    }

    // Plus interval
    if (inside(150, 170, 70, 50)) {
      logIntervalValue++;

      if (logIntervalUnit == TIME_MINUTES) {
        uint16_t maxMin = getMaxStatsMinutes();
        if (maxMin < 1) maxMin = 1;

        if (logIntervalValue > maxMin) {
          logIntervalValue = maxMin;
        }
      } 
      else {
        uint16_t maxHours = getMaxStatsHours();
        if (maxHours < 1) maxHours = 1;

        if (logIntervalValue > maxHours) {
          logIntervalValue = maxHours;
        }
      }

      drawDataLogScreen();
      return;
    }

    // Unit toggle
    if (inside(250, 170, 90, 50)) {

      if (logIntervalUnit == TIME_MINUTES) {
        logIntervalUnit = TIME_HOURS;

        uint16_t maxHours = getMaxStatsHours();
        if (maxHours < 1) maxHours = 1;

        if (logIntervalValue > maxHours) {
          logIntervalValue = maxHours;
        }
      } 
      else {
        logIntervalUnit = TIME_MINUTES;

        uint16_t maxMin = getMaxStatsMinutes();
        if (maxMin < 1) maxMin = 1;

        if (logIntervalValue > maxMin) {
          logIntervalValue = maxMin;
        }
      }

      drawDataLogScreen();
      return;
    }

    //Current

    if (inside(LOG_CURRENT_X, LOG_CURRENT_Y, LOG_CURRENT_W, LOG_CURRENT_H)) {
      setLogIntervalToCurrentRuntime();

      Serial.print("CURRENT selected interval: ");
      Serial.print(logIntervalValue);
      Serial.print(logIntervalUnit == TIME_MINUTES ? " min" : " hour");
      Serial.println();

      drawDataLogScreen();

      return;
    }

    // SAVE
    if (inside(60, 250, 150, 45)) {
      Serial.println("SAVE BUTTON MATCHED");

      int rows = saveLogDataToFile();

      tft.fillRect(50, 225, 220, 18, TFT_BLACK);
      tft.setTextSize(1);

      if (rows >= 0) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setCursor(80, 225);
        tft.print("Saved rows: ");
        tft.print(rows);
      } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setCursor(80, 225);
        tft.print("Save failed");
      }

      Serial.flush();
      return;
    }

    // UPLOAD
    if (inside(270, 250, 150, 45)) {
      Serial.println("UPLOAD BUTTON MATCHED");

      uploadLogDataToSerial();

      tft.fillRect(260, 225, 200, 18, TFT_BLACK);
      tft.setTextSize(1);
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.setCursor(275, 225);
      tft.print("Sent to Serial");

      Serial.flush();
      return;
    }
  }

  // ---------- BATTERY SCREEN ----------
  else if (settingsPage == SETTINGS_BATTERY) {

    // BACK
    if (inside(20, 15, 90, 35)) {
      settingsPage = SETTINGS_MENU;
      drawSettingsScreen();
      return;
    }
  }

  // ---------- CONTROL SCREEN ----------
  else if (settingsPage == SETTINGS_CONTROL) {

    // BACK
  if (inside(20, 15, 90, 35)) {
    settingsPage = SETTINGS_MENU;
    saveSettings();
    drawSettingsScreen();
    return;
  }

  // Vibration minus
  if (inside(80, 120, 80, 50)) {
    if (vibrationPower > 10) {
      vibrationPower -= 10;
    }

    saveSettings();
    drawControlScreen();
    return;
  }

  // Vibration plus
  if (inside(280, 120, 80, 50)) {
    if (vibrationPower < 100) {
      vibrationPower += 10;
    }

    saveSettings();
    drawControlScreen();
    return;
  }

  // Light minus
  if (inside(80, 240, 80, 50)) {
    if (lightPower > 10) {
      lightPower -= 10;
    }

    saveSettings();
    drawControlScreen();
    return;
  }

  // Light plus
  if (inside(280, 240, 80, 50)) {
    if (lightPower < 100) {
      lightPower += 10;
    }

    saveSettings();
    drawControlScreen();
    return;
  }
}
}






unsigned long getMainStatsIntervalMs() {
  if (mainStatsInfinite) {
    return 0; // 0 means infinite / since stored history started
  }

  if (mainStatsTimeUnit == TIME_MINUTES) {
    return (unsigned long)mainStatsIntervalValue * 60000UL;
  } else {
    return (unsigned long)mainStatsIntervalValue * 3600000UL;
  }
}


void makeStatsIntervalLabel(char* buffer, size_t size) {
  if (mainStatsInfinite) {
    snprintf(buffer, size, "INF");
    return;
  }

  if (mainStatsTimeUnit == TIME_MINUTES) {
    snprintf(buffer, size, "%u min", mainStatsIntervalValue);
  } else {
    snprintf(buffer, size, "%u h", mainStatsIntervalValue);
  }
}



void addStatsDataPoint(float dose) {
  statsHistory[statsHistoryIndex].dose = dose;
  statsHistory[statsHistoryIndex].time = millis();

  statsHistoryIndex = (statsHistoryIndex + 1) % STATS_MAX_POINTS;

  if (statsValidPoints < STATS_MAX_POINTS) {
    statsValidPoints++;
  }
}




uint16_t getMaxStatsMinutes() {
  return STATS_MAX_POINTS / 60;
}

uint16_t getMaxStatsHours() {
  return STATS_MAX_POINTS / 3600;
}




unsigned long getLogIntervalMs() {
  if (logIntervalUnit == TIME_MINUTES) {
    return (unsigned long)logIntervalValue * 60000UL;
  } else {
    return (unsigned long)logIntervalValue * 3600000UL;
  }
}


void makeLogIntervalLabel(char* buffer, size_t size) {
  if (logIntervalUnit == TIME_MINUTES) {
    snprintf(buffer, size, "%u min", logIntervalValue);
  } else {
    snprintf(buffer, size, "%u h", logIntervalValue);
  }
}


void drawDataLogScreen() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(140, 10);
  tft.print("DATA LOG");

  drawButton(20, 15, 90, 35, "BACK");

  char label[20];
  makeLogIntervalLabel(label, sizeof(label));

  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(85, 80);
  tft.print("Save interval:");

  tft.setTextSize(3);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(190, 115);
  tft.print(label);

  drawButton(60, 170, 70, 50, "-");
  drawButton(150, 170, 70, 50, "+");

  if (logIntervalUnit == TIME_MINUTES) {
    drawButton(250, 170, 90, 50, "MIN");
  } else {
    drawButton(250, 170, 90, 50, "HOUR");
  }
  drawButton(LOG_CURRENT_X, LOG_CURRENT_Y, LOG_CURRENT_W, LOG_CURRENT_H, "NOW");

  drawButton(60, 250, 150, 45, "SAVE");
  drawButton(270, 250, 150, 45, "UPLOAD");
}





int saveLogDataToFile() {
  Serial.println("SAVE FUNCTION STARTED");

  fs::File file = LittleFS.open(LOG_FILE, "w");

  if (!file) {
    Serial.println("ERROR: Could not open log file for writing");
    Serial.flush();
    return -1;
  }

  unsigned long now = millis();
  unsigned long intervalMs = getLogIntervalMs();
  unsigned long startTime = now - intervalMs;

  int savedRows = 0;

  file.println("Geiger Counter Log");

  char label[20];
  makeLogIntervalLabel(label, sizeof(label));

  file.print("Interval: ");
  file.println(label);

  file.println("time_ago_s,dose_uSv_h");

  for (int i = 0; i < statsValidPoints; i++) {
    int idx = (statsHistoryIndex - statsValidPoints + i + STATS_MAX_POINTS) % STATS_MAX_POINTS;

    unsigned long t = statsHistory[idx].time;

    if (t < startTime) continue;

    unsigned long timeAgo = (now - t) / 1000UL;

    file.print(timeAgo);
    file.print(",");
    file.println(statsHistory[idx].dose, 3);

    savedRows++;
  }

  file.close();

  Serial.print("SAVE DONE. Rows saved: ");
  Serial.println(savedRows);
  Serial.flush();

  return savedRows;
}



void uploadLogDataToSerial() {
  Serial.println();
  Serial.println("UPLOAD FUNCTION STARTED");

  if (!LittleFS.exists(LOG_FILE)) {
    Serial.println("ERROR: Log file does not exist");
    Serial.flush();
    return;
  }

  fs::File file = LittleFS.open(LOG_FILE, "r");

  if (!file) {
    Serial.println("ERROR: Could not open log file for reading");
    Serial.flush();
    return;
  }

  Serial.println("----- BEGIN GEIGER LOG -----");

  int lineCount = 0;

  while (file.available()) {
    String line = file.readStringUntil('\n');
    Serial.println(line);

    lineCount++;

    // Give Serial/USB time to breathe, because apparently even text needs life support
    if (lineCount % 20 == 0) {
      Serial.flush();
      delay(10);
    }
  }

  file.close();

  Serial.println("------ END GEIGER LOG ------");
  Serial.print("Uploaded lines: ");
  Serial.println(lineCount);
  Serial.println("UPLOAD DONE");
  Serial.flush();
}


void drawBatteryScreen() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(140, 10);
  tft.print("BATTERY");

  drawButton(20, 15, 90, 35, "BACK");

  int level = batteryLevel;

  tft.setTextSize(4);

  if (alimVoltage > CHARGER_DETECT_V) {
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
  } else if (level < 25) {
    tft.setTextColor(TFT_RED, TFT_BLACK);
  } else {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
  }

  tft.setCursor(165, 90);
  tft.printf("%d%%", level);

  tft.setTextSize(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setCursor(95, 150);

  if (alimVoltage > CHARGER_DETECT_V) {
    tft.print("Charging");
  } else if (level < 7) {
    tft.print("Critical battery");
  } else if (level < 25) {
    tft.print("Low battery");
  } else {
    tft.print("Battery status");
  }

  drawBatteryIconAt(205, 190, level);
}



void setLogIntervalToCurrentRuntime() {
  unsigned long elapsedSec = millis() / 1000UL;

  // Minimum = 1 minute
  if (elapsedSec < 60UL) {
    logIntervalUnit = TIME_MINUTES;
    logIntervalValue = 1;
    return;
  }

  // Maximum = 1 hour
  if (elapsedSec >= 3600UL) {
    logIntervalUnit = TIME_HOURS;
    logIntervalValue = 1;
    return;
  }

  // Between 1 minute and 59 minutes:
  // floor the minutes, so 5m 2s -> 5 min
  unsigned long minutes = elapsedSec / 60UL;

  if (minutes < 1) minutes = 1;
  if (minutes > 59) minutes = 59;

  logIntervalUnit = TIME_MINUTES;
  logIntervalValue = minutes;
}




void drawControlScreen() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(170, 10);
  tft.print("CONTROL");

  drawButton(20, 15, 90, 35, "BACK");

  // ---------- VIBRATION ----------
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(60, 80);
  tft.print("Vibration:");

  tft.setTextSize(3);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(240, 72);
  tft.print(vibrationPower);
  tft.print("%");

  drawButton(80, 120, 80, 50, "-");
  drawButton(280, 120, 80, 50, "+");

  // ---------- LIGHT ----------
  tft.setTextSize(2);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.setCursor(60, 200);
  tft.print("Light:");

  tft.setTextSize(3);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setCursor(240, 192);
  tft.print(lightPower);
  tft.print("%");

  drawButton(80, 240, 80, 50, "-");
  drawButton(280, 240, 80, 50, "+");
}







float lettureTensione(int j) {
  // j = 0 -> Batteria, j = 1 -> Vin caricatore
  int somma = 0;
  int pin = (j == 0) ? PIN_BATT : PIN_ALIM;

  for (int i = 0; i < 5; i++) {
    somma += analogReadMilliVolts(pin);
    delay(5);   
  }

  float v = (somma / 5.0f) / 1000.0f;
  v *= 2.0f;    // your voltage divider compensation

  if (v < 1.5f) {
    v = 0.0f;
  }

  return v;
}

int batteryPercentFromVoltage(float voltage) {
  // Basic 1-cell Li-ion estimate. Change these if your battery chemistry is different.
  if (voltage <= 3.30f) return 0;
  if (voltage >= 4.20f) return 100;
  return (int)((voltage - 3.30f) * 100.0f / (4.20f - 3.30f));
}

void handleGeigerPulseFeedback() {
  if (!nuovoImpulso) return;

  nuovoImpulso = false;

  noInterrupts();
  uint32_t cnt = contatore;
  interrupts();

  if (cnt == ultimoContatoreImpulsi) return;
  ultimoContatoreImpulsi = cnt;

  Serial.print(F("Impulso #"));
  Serial.println(cnt);

  // If alarm is active, it already controls motor and sound.
  if (isAlarmConditionActive()) return;

  int vibPWM = map(vibrationPower, 0, 100, 0, MOTOR_PWM_MAX);
  int ledPWM = LED_PWM_MAX;

  unsigned long now = millis();

  if (mot && !geigerMotorPulseActive) {
    // Small startup kick so the motor actually starts
    ledcWrite(PIN_MOTORE, MOTOR_PWM_MAX);
    delay(20);

    ledcWrite(PIN_MOTORE, vibPWM);

    geigerMotorPulseActive = true;
    geigerMotorPulseEnd = millis() + GEIGER_MOTOR_PULSE_MS;

    Serial.print("Motor pulse started, PWM = ");
    Serial.println(vibPWM);
  }


  ledcWrite(PIN_LED, ledPWM);


  if (buz) {
    tone(PIN_BUZZER, 2300, 2);
  }

  delay(5);

  ledcWrite(PIN_LED, 0);
}

void updateGeigerMeasurements() {
  unsigned long now = millis();

  if (now - lastGeigerCalc < INTERVALLO_MS) return;
  lastGeigerCalc = now;

  noInterrupts();
  uint32_t campione = contatoreXcpm;
  contatoreXcpm = 0;
  interrupts();

  float cpmFloat = (float)campione * (60000.0f / (float)INTERVALLO_MS);
  float uSv = cpmFloat / FATTORE_USV;

  cpm = (int)(cpmFloat + 0.5f);
  dose = uSv;

  

  Serial.print(F("CPM: "));
  Serial.print(cpmFloat, 1);
  Serial.print(F("  |  uSv/h: "));
  Serial.println(dose, 4);

  
}

void updateBatteryMeasurements() {
  unsigned long now = millis();

  static bool lastChargingState = false;
  static bool initialized = false;

  if (!initialized) {
    readBatteryNow();

    lastChargingState = alimVoltage > CHARGER_DETECT_V;
    initialized = true;

    if (CurrentScreen == SCREEN_MAIN) {
      drawBatteryIcon(batteryLevel);
    }

    if (CurrentScreen == SCREEN_SETTINGS && settingsPage == SETTINGS_BATTERY) {
      drawBatteryScreen();
    }

    return;
  }

  // ---------- FAST CHARGER CHECK ----------
  if (now - lastChargerCheck >= CHARGER_CHECK_INTERVAL) {
    lastChargerCheck = now;

    alimVoltage = lettureTensione(1);

    bool chargingNow = alimVoltage > CHARGER_DETECT_V;

    if (chargingNow != lastChargingState) {
      lastChargingState = chargingNow;

      if (CurrentScreen == SCREEN_MAIN) {
        drawBatteryIcon(batteryLevel);
      }

      if (CurrentScreen == SCREEN_SETTINGS && settingsPage == SETTINGS_BATTERY) {
        drawBatteryScreen();
      }
    }
  }

  // ---------- SLOW BATTERY PERCENTAGE UPDATE ----------
  if (now - lastBatteryUpdate >= BATTERY_UPDATE_INTERVAL) {
    lastBatteryUpdate = now;

    batteryVoltage = lettureTensione(0);
    batteryLevel = batteryPercentFromVoltage(batteryVoltage);

    if (CurrentScreen == SCREEN_MAIN) {
      drawBatteryIcon(batteryLevel);
    }

    if (CurrentScreen == SCREEN_SETTINGS && settingsPage == SETTINGS_BATTERY) {
      drawBatteryScreen();
    }
  }
}

void isrImpulso() {
  static unsigned long ultimoMicros = 0;
  unsigned long adesso = micros();

  if (adesso - ultimoMicros > DEBOUNCE_US) {
    ultimoMicros = adesso;
    contatore++;
    contatoreXcpm++;
    nuovoImpulso = true;
  }
}

bool isAlarmEnabled() {
  return alarmMode == 0;   
}

bool isAlarmConditionActive() {
  return isAlarmEnabled() && dose > threshold;
}

void updateAlarmOutput() {
  alarmActive = isAlarmConditionActive();

  if (alarmActive) {
    alarmWasActive = true;

    
    geigerMotorPulseActive = false;

    int alarmPWM = map(ALARM_MOTOR_POWER, 0, 100, 0, MOTOR_PWM_MAX);

    ledcWrite(PIN_MOTORE, alarmPWM);
    tone(PIN_BUZZER, 2300);
  } 
  else {
    // Spegne il motore quando lallarme è finito
    
    if (alarmWasActive) {
      ledcWrite(PIN_MOTORE, 0);
      noTone(PIN_BUZZER);

      alarmWasActive = false;
    }
  }
}

void stopAlarmOutput() {
  ledcWrite(PIN_MOTORE, 0);
  noTone(PIN_BUZZER);
}

void updateGeigerMotorPulse() {
  
  if (isAlarmConditionActive()) {
    geigerMotorPulseActive = false;
    return;
  }

  if (geigerMotorPulseActive && millis() >= geigerMotorPulseEnd) {
    ledcWrite(PIN_MOTORE, 0);
    geigerMotorPulseActive = false;

    Serial.println("Motor pulse ended");
  }
}


void drawBatteryIcon(int level) {
  if (level < 0) level = 0;
  if (level > 100) level = 100;

  bool charging = alimVoltage > CHARGER_DETECT_V;

  uint16_t textColor = TFT_GREEN;

  if (charging) {
    textColor = TFT_CYAN;
  } 
  else if (level < 25) {
    textColor = TFT_RED;
  }

  // Clear icon + percentage area
  // This clears the top-right area where the battery icon and text are drawn
  tft.fillRect(SCREEN_W - 125, 0, 125, 45, TFT_BLACK);

  // Draw icon a bit higher
  int iconX = BAT_ICON_X;
  int iconY = BAT_ICON_Y - 8;

  drawBatteryIconAt(iconX, iconY, level);

  // Draw percentage on the LEFT of the icon
  tft.setTextSize(2);
  tft.setTextColor(textColor, TFT_BLACK);

  int textX = iconX - 48;
  int textY = iconY + 25;

  tft.setCursor(textX, textY);

  if (charging) {
    tft.print("CHG");
  } else {
    tft.printf("%d%%", level);
  }
}


void drawBatteryIconAt(int x, int y, int level) {
  if (level < 0) level = 0;
  if (level > 100) level = 100;

  bool charging = alimVoltage > CHARGER_DETECT_V;

  tft.fillRect(x - 5, y - 5, BAT_ICON_W + 20, BAT_ICON_H + 20, TFT_BLACK);

  if (charging) {
    tft.pushImage(x, y, BAT_ICON_W, BAT_ICON_H, BAT_ICONS[2]);
    return;
  }

  if (level < 7) {
    tft.pushImage(x, y, BAT_ICON_W, BAT_ICON_H, BAT_ICONS[1]);
    return;
  }

  tft.pushImage(x, y, BAT_ICON_W, BAT_ICON_H, BAT_ICONS[0]);

  uint16_t fillColor = TFT_GREEN;
  if (level < 25) fillColor = TFT_RED;

  int fillX = x + 4;
  int fillY = y + 20;
  int fillW = 52;
  int fillH = 24;

  int filledW = map(level, 0, 100, 0, fillW);

  if (filledW > 0) {
    tft.fillRoundRect(fillX, fillY, filledW, fillH, 2, fillColor);
    
  }
}


void updateGraphMaxForWindow() {
  // Get latest graph value
  float current = dose;

  if (validPoints > 0) {
    int lastIdx = (historyIndex - 1 + MAX_POINTS) % MAX_POINTS;
    current = history[lastIdx].dose;
  }

  if (current < 0.0f) current = 0.0f;

  // Store all-time peak since program start/reset
  if (current > peakDose) {
    peakDose = current;
  }

  // Y axis max = peak + 0.1, rounded by tenths
  // Example:
  // peak 0.6 -> maxDose 0.7
  // peak 0.7 -> maxDose 0.8
  maxDose = (floor(peakDose * 10.0f + 0.0001f) / 10.0f) + 0.1f;

  // Minimum scale
  if (maxDose < 0.2f) maxDose = 0.2f;

  // Safety limit
  if (maxDose > 10.0f) maxDose = 10.0f;
}




void clearGraphHistory() {
  historyIndex = 0;
  validPoints = 0;
  peakDose = 0.0;
  
  maxDose = 0.2;

  historyRunning = true;
  historyFinished = false;

  if (graphInitialized) {
    graphSprite.fillSprite(TFT_BLACK);
    graphSprite.pushSprite(GX, GY - GH);
  }

  tft.fillScreen(TFT_BLACK);
  drawGraphUI();
}



unsigned long getGraphStartTime() {
  unsigned long now = millis();
  unsigned long window = windows[windowMode];

  if (validPoints < 1) {
    return now;
  }

  bool found = false;
  unsigned long oldestVisible = now;

  for (int i = 0; i < validPoints; i++) {
    int idx = (historyIndex - validPoints + i + MAX_POINTS) % MAX_POINTS;

    unsigned long t = history[idx].time;

    // Ignore points older than the selected window
    if (now - t > window) continue;

    if (!found || t < oldestVisible) {
      oldestVisible = t;
      found = true;
    }
  }

  if (!found) {
    return now;
  }

  return oldestVisible;
}


void readBatteryNow() {
  batteryVoltage = lettureTensione(0);
  alimVoltage = lettureTensione(1);
  batteryLevel = batteryPercentFromVoltage(batteryVoltage);

  lastBatteryUpdate = millis();
  lastChargerCheck = millis();
}