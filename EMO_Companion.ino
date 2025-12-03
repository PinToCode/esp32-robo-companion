#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <ESP32Servo.h>
#include <VL53L0X.h>  // Add VL53L0X library

/* Uncomment the initialize the I2C address , uncomment only one, If you get a totally blank screen try the other*/
#define i2c_Address 0x3c  //initialize with the I2C addr 0x3C Typically eBay OLED's
//#define i2c_Address 0x3d //initialize with the I2C addr 0x3D Typically Adafruit OLED's

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     //   QT-PY / XIAO

// Built-in LED pin (usually GPIO 2 for most ESP32 boards)
#define LED_BUILTIN 2

// VL53L0X Distance sensor
VL53L0X distanceSensor;

// Distance detection variables
#define DETECTION_THRESHOLD 250  // Distance threshold in mm
bool personDetected = false;
bool previousPersonState = false;
unsigned long personDetectedTime = 0;
unsigned long lastDistanceCheck = 0;
const unsigned long distanceCheckInterval = 100;  // Check distance every 100ms
const unsigned long greetingDisplayTime = 5000;   // Show greeting for 5 seconds

Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#include <FluxGarage_RoboEyes.h>
roboEyes roboEyes;  // create RoboEyes instance

// Eye structure for coordinates
struct Eye {
  int x, y;
};
Eye leftEye = { 20, 20 };
Eye rightEye = { 70, 20 };

// WiFi credentials - CHANGE THESE TO YOUR NETWORK
const char* ssid = "ssid";
const char* password = "password";

// Use OpenMeteo - completely free, no API key needed
const char* weatherApiKey = "";         // Not needed for OpenMeteo
const char* weatherCity = "your_city";  // Just for display
const char* weatherUrl = "http://api.open-meteo.com/v1/forecast?latitude=12.87&longitude=74.88&current_weather=true&temperature_unit=celsius";

// Time is already set for India (IST)
const long gmtOffset_sec = 19800;  // IST = GMT+5:30
const int daylightOffset_sec = 0;  // No daylight saving in India
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const char* ntpServer3 = "time.google.com";

Servo myServo;

int servoPin = 13;

// Servo movement variables
int servoPos = 30;
int servoDirection = 1;
unsigned long lastServoUpdate = 0;
const int servoDelay = 1;

// Servo rest period to prevent wear
bool servoActive = true;
unsigned long servoActiveStartTime = 0;
const unsigned long servoActiveTime = 300000;  // 5 minutes active
const unsigned long servoRestTime = 60000;     // 1 minute rest

// WiFi and data variables
bool wifiConnected = false;
bool wifiStatusShown = false;    // Flag to track if WiFi status was already shown
bool previousWifiState = false;  // Track previous WiFi state
char currentWeather[32] = "";    // Changed to char array
char temperature[8] = "";        // Changed to char array
int currentQuoteIndex = 0;
unsigned long lastWeatherUpdate = 0;
const unsigned long weatherUpdateInterval = 600000;  // Update weather every 10 minutes

// Memory monitoring
unsigned long lastMemCheck = 0;
const unsigned long memCheckInterval = 60000;  // Check every minute

// Time sync variables
unsigned long lastTimeSync = 0;
const unsigned long timeSyncInterval = 3600000;  // Sync every hour

// Greeting messages array for when person is detected
const char* greetingMessages[] = {
  "Hi!",
  "How are you?",
  "Happy to see you!",
  "Hello there!",
  "Welcome!",
  "Nice to meet you!",
  "Good to see you!",
  "Greetings!",
  "Hey friend!",
  "What's up?"
};
const int numGreetings = sizeof(greetingMessages) / sizeof(greetingMessages[0]);

// Funny quotes array
const char* funnyQuotes[] = {
  "Still here?",
  "Bored much?",
  "Coffee time!",
  "Yawn...",
  "Really?",
  "Again?",
  "Seriously?",
  "Nope!",
  "Meh.",
  "Whatever!",
  "Ugh, humans!",
  "Not today!",
  "Maybe later!",
  "Sure, sure...",
  "Oh please!",
  "Beep!",
  "Hi there!",
  "What's up?",
  "Bye!",
  "Cool!",
  "Awesome!",
  "Nice!",
  "Sweet!",
  "Neat!",
  "Wow!"
};
const int numQuotes = sizeof(funnyQuotes) / sizeof(funnyQuotes[0]);

// Static JSON document to prevent repeated allocations
StaticJsonDocument<1024> weatherDoc;

// VL53L0X Distance Sensor Functions
bool initializeDistanceSensor() {
  if (!distanceSensor.init()) {
    Serial.println("Failed to initialize VL53L0X sensor!");
    return false;
  }

  Serial.println("VL53L0X sensor initialized successfully!");

  // Start continuous ranging measurements
  distanceSensor.startContinuous();

  // Optional: Set measurement timing budget (higher = more accurate but slower)
  // distanceSensor.setMeasurementTimingBudget(50000); // 50ms

  return true;
}

int readDistance() {
  uint16_t distance = distanceSensor.readRangeContinuousMillimeters();

  if (distanceSensor.timeoutOccurred()) {
    Serial.println("VL53L0X timeout occurred!");
    return -1;  // Return -1 to indicate error
  }

  return distance;
}

void checkPersonPresence() {
  unsigned long currentTime = millis();

  // Only check distance at specified intervals
  if (currentTime - lastDistanceCheck < distanceCheckInterval) {
    return;
  }

  lastDistanceCheck = currentTime;

  int distance = readDistance();

  if (distance == -1) {
    return;  // Sensor error, skip this reading
  }

  // Print distance for debugging
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" mm");

  // Check if person is within detection threshold
  previousPersonState = personDetected;
  personDetected = (distance < DETECTION_THRESHOLD);

  // If person just entered detection zone
  if (personDetected && !previousPersonState) {
    personDetectedTime = currentTime;
    Serial.println("Person detected! Showing greeting...");

    // Set excited mood for the robot eyes
    roboEyes.setMood(HAPPY);
    roboEyes.anim_laugh();  // Play laugh animation
  }
  // If person just left detection zone
  else if (!personDetected && previousPersonState) {
    Serial.println("Person left detection zone");
  }
}

void displayGreeting() {
  updateServo();

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);

  // Select random greeting message
  int greetingIndex = random(0, numGreetings);
  const char* greeting = greetingMessages[greetingIndex];
  display.setCursor(0, 0);
  // Center the text
  // int textWidth = strlen(greeting) * 12; // Approximate width for size 2 text
  // int x = (SCREEN_WIDTH - textWidth) / 2;
  // int y = 20;

  // display.setCursor(x, y);
  display.println(greeting);

  // Add distance info
  int distance = readDistance();
  // if (distance != -1) {
  //   display.setCursor(0, 0);
  //   display.setTextSize(1);
  //   display.print("Dist: ");
  //   display.print(distance);
  //   display.println("mm");
  // }

  display.display();
}

const char* getTimeBasedQuote() {
  if (!wifiConnected) {
    return funnyQuotes[currentQuoteIndex];  // Fallback to current system
  }

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return funnyQuotes[currentQuoteIndex];  // Fallback if time not available
  }

  int hour = timeinfo.tm_hour;

  // Morning quotes (6 AM - 12 PM)
  if (hour >= 6 && hour < 12) {
    const char* morningQuotes[] = {
      "Morning!",
      "Awake yet?",
      "Coffee first!",
      "Too early!",
      "Yawn!"
    };
    int index = random(0, 5);
    return morningQuotes[index];
  }
  // Afternoon quotes (12 PM - 6 PM)
  else if (hour >= 12 && hour < 18) {
    const char* afternoonQuotes[] = {
      "Lunch?",
      "Nap time!",
      "Sleepy?",
      "Hot day!",
      "Tired yet?"
    };
    int index = random(0, 5);
    return afternoonQuotes[index];
  }
  // Evening/Night quotes (6 PM - 6 AM)
  else {
    const char* eveningQuotes[] = {
      "Bedtime!",
      "Sleep now!",
      "Night night!",
      "Too late!",
      "Go sleep!"
    };
    int index = random(0, 5);
    return eveningQuotes[index];
  }
}

// Missing function definitions
void drawEyes(int leftX, int leftY, int rightX, int rightY) {
  display.clearDisplay();
  display.fillCircle(leftX + 15, leftY + 15, 15, SH110X_WHITE);
  display.fillCircle(rightX + 15, rightY + 15, 15, SH110X_WHITE);
  display.fillCircle(leftX + 15, leftY + 15, 8, SH110X_BLACK);
  display.fillCircle(rightX + 15, rightY + 15, 8, SH110X_BLACK);
  display.display();
}

void drawWiFiSymbol(int x, int y, bool connected) {
  if (connected) {
    display.drawCircle(x, y, 4, SH110X_WHITE);
    display.drawCircle(x, y, 8, SH110X_WHITE);
    display.drawCircle(x, y, 12, SH110X_WHITE);
  } else {
    display.drawLine(x - 6, y - 6, x + 6, y + 6, SH110X_WHITE);
    display.drawLine(x - 6, y + 6, x + 6, y - 6, SH110X_WHITE);
  }
}

// Function to continuously update servo position with rest periods
void updateServo() {
  unsigned long currentTime = millis();

  // Check if servo needs rest
  if (servoActive && (currentTime - servoActiveStartTime >= servoActiveTime)) {
    servoActive = false;
    servoActiveStartTime = currentTime;
    Serial.println("Servo entering rest period");
    return;
  } else if (!servoActive && (currentTime - servoActiveStartTime >= servoRestTime)) {
    servoActive = true;
    servoActiveStartTime = currentTime;
    Serial.println("Servo resuming activity");
  }

  // Only move servo if active
  if (!servoActive) {
    return;
  }

  if (currentTime - lastServoUpdate >= servoDelay) {
    servoPos += servoDirection;

    // Strict limit enforcement - don't exceed 30-80 range
    if (servoPos >= 80) {
      servoDirection = -1;
      servoPos = 80;
    } else if (servoPos <= 30) {
      servoDirection = 1;
      servoPos = 30;
    }

    // Double check limits before writing
    if (servoPos >= 30 && servoPos <= 80) {
      myServo.write(servoPos);
    }
    lastServoUpdate = currentTime;
  }
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
    yield();
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println();
    Serial.println("WiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    wifiConnected = false;
    Serial.println();
    Serial.println("WiFi connection failed!");
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void checkWiFiStatusChange() {
  bool currentWiFiState = (WiFi.status() == WL_CONNECTED);

  if (currentWiFiState != previousWifiState) {
    previousWifiState = currentWiFiState;
    wifiConnected = currentWiFiState;

    if (wifiConnected) {
      Serial.println("WiFi reconnected!");
      digitalWrite(LED_BUILTIN, HIGH);
      configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);
      getWeatherData();
    } else {
      Serial.println("WiFi disconnected!");
      digitalWrite(LED_BUILTIN, LOW);
    }
    wifiStatusShown = false;
  }

  if (!wifiConnected && !wifiStatusShown) {
    showWiFiDisconnectedEyes();
    wifiStatusShown = true;
  }
}

void showWiFiDisconnectedEyes() {
  updateServo();

  for (int frame = 0; frame < 60; frame++) {
    display.clearDisplay();

    // Sad eyes
    display.fillRoundRect(leftEye.x, 32, 35, 20, 10, SH110X_WHITE);
    display.fillRoundRect(rightEye.x, 32, 35, 20, 10, SH110X_WHITE);

    // Sad eye shape (curved top)
    display.fillRect(leftEye.x + 3, 28, 29, 6, SH110X_BLACK);
    display.fillRect(rightEye.x + 3, 28, 29, 6, SH110X_BLACK);

    // WiFi disconnected symbol
    drawWiFiSymbol(64, 50, false);

    display.display();
    delay(50);
    yield();  // Allow ESP32 to handle background tasks
  }
}

void displayStartupEyes() {
  for (int i = 0; i < 3; i++) {
    drawEyes(20, 24, 70, 24);
    delay(500);
    display.clearDisplay();
    display.display();
    delay(300);
    yield();
  }
}

const char* getWeatherDescription(int code) {
  switch (code) {
    case 0: return "Clear sky";
    case 1:
    case 2:
    case 3: return "Partly cloudy";
    case 45:
    case 48: return "Fog";
    case 51:
    case 53:
    case 55: return "Drizzle";
    case 61:
    case 63:
    case 65: return "Rain";
    case 71:
    case 73:
    case 75: return "Snow";
    case 80:
    case 81:
    case 82: return "Showers";
    case 95: return "Thunderstorm";
    default: return "Unknown";
  }
}

void displayTime() {
  updateServo();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  if (wifiConnected) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      // Format time string
      char timeStr[32];
      strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

      // Format date string
      char dateStr[32];
      strftime(dateStr, sizeof(dateStr), "%d/%m/%Y", &timeinfo);

      // Format day string
      char dayStr[16];
      strftime(dayStr, sizeof(dayStr), "%A", &timeinfo);

      // Display time (large)
      display.setTextSize(2);
      display.setCursor(15, 20);
      display.println(timeStr);

      // Display date
      display.setTextSize(1);
      display.setCursor(25, 45);
      display.println(dateStr);

      // Display day
      display.setCursor(35, 55);
      display.println(dayStr);

      // Title
      display.setCursor(45, 5);
      display.println("Time");
    } else {
      display.setCursor(10, 20);
      display.println("Time sync failed");
    }
  } else {
    display.setCursor(10, 20);
    display.println("No WiFi connection");
    display.setCursor(10, 35);
    display.println("Time unavailable");
  }

  display.display();
}

void displayFunnyQuote() {
  updateServo();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  const char* quote = getTimeBasedQuote();

  // Title
  display.setCursor(35, 5);
  display.println("Message:");

  // Display quote with text wrapping
  displayWrappedText(quote, 10, 25, SCREEN_WIDTH - 10);

  display.display();

  // Move to next quote for next time
  currentQuoteIndex = (currentQuoteIndex + 1) % numQuotes;
}

void displayWeather() {
  updateServo();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  if (strlen(currentWeather) > 0 && strlen(temperature) > 0) {
    // Title
    display.setCursor(35, 5);
    display.println("Weather:");

    // City name
    display.setCursor(10, 20);
    display.println(weatherCity);

    // Temperature
    display.setTextSize(2);
    display.setCursor(10, 35);
    display.print(temperature);
    display.println("C");

    // Weather description
    display.setTextSize(1);
    display.setCursor(10, 55);
    display.println(currentWeather);
  } else {
    display.setCursor(10, 20);
    display.println("Weather data");
    display.setCursor(10, 35);
    display.println("not available");
  }

  display.display();
}

void displayWrappedText(const char* text, int x, int y, int maxWidth) {
  int currentX = x;
  int currentY = y;
  int spaceWidth = 6;  // Approximate width of a space character
  int charWidth = 6;   // Approximate width of characters

  char words[20][20];  // Max 20 words, each max 20 chars
  int wordCount = 0;

  // Split text into words
  int lastSpace = 0;
  int textLen = strlen(text);
  for (int i = 0; i <= textLen; i++) {
    if (text[i] == ' ' || i == textLen) {
      if (i > lastSpace) {
        int wordLen = i - lastSpace;
        if (wordLen > 19) wordLen = 19;  // Prevent overflow
        strncpy(words[wordCount], &text[lastSpace], wordLen);
        words[wordCount][wordLen] = '\0';
        wordCount++;
        if (wordCount >= 20) break;  // Prevent overflow
      }
      lastSpace = i + 1;
    }
  }

  // Display words with wrapping
  for (int i = 0; i < wordCount; i++) {
    int wordWidth = strlen(words[i]) * charWidth;

    if (currentX + wordWidth > maxWidth && currentX > x) {
      currentY += 12;  // Move to next line
      currentX = x;
    }

    display.setCursor(currentX, currentY);
    display.print(words[i]);
    currentX += wordWidth + spaceWidth;
  }
}

void getWeatherData() {
  if (!wifiConnected) return;

  HTTPClient http;
  http.setTimeout(10000);  // 10 second timeout

  http.begin(weatherUrl);
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    String payload = http.getString();

    // Clear previous data
    weatherDoc.clear();

    // Parse JSON response for OpenMeteo API
    DeserializationError error = deserializeJson(weatherDoc, payload);

    if (!error) {
      float temp = weatherDoc["current_weather"]["temperature"];
      snprintf(temperature, sizeof(temperature), "%.1f", temp);

      // Map weather codes to descriptions (OpenMeteo weather codes)
      int weatherCode = weatherDoc["current_weather"]["weathercode"];
      const char* weatherDesc = getWeatherDescription(weatherCode);
      strncpy(currentWeather, weatherDesc, sizeof(currentWeather) - 1);
      currentWeather[sizeof(currentWeather) - 1] = '\0';

      Serial.print("Weather updated: ");
      Serial.print(temperature);
      Serial.print("°C, ");
      Serial.println(currentWeather);
      lastWeatherUpdate = millis();
    } else {
      Serial.println("JSON parsing failed");
    }
  } else {
    Serial.print("Error getting weather data: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  yield();
}

void monitorMemory() {
  unsigned long currentTime = millis();
  if (currentTime - lastMemCheck >= memCheckInterval) {
    Serial.print("Free heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.print(" bytes, Min free heap: ");
    Serial.print(ESP.getMinFreeHeap());
    Serial.println(" bytes");
    lastMemCheck = currentTime;
  }
}

void syncTimeIfNeeded() {
  unsigned long currentTime = millis();
  if (wifiConnected && (currentTime - lastTimeSync >= timeSyncInterval)) {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);
    lastTimeSync = currentTime;
    Serial.println("Time re-synced with NTP server");
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);  // Start with LED off
  Serial.println("Built-in LED initialized");

  delay(250);                        // wait for the OLED to power up
  display.begin(i2c_Address, true);  // Address 0x3C default
  display.clearDisplay();
  display.display();

  // Initialize I2C for VL53L0X (same I2C bus as OLED)
  Wire.begin();
  Serial.println("I2C initialized for sensors");

  // Initialize VL53L0X distance sensor
  if (!initializeDistanceSensor()) {
    Serial.println("Warning: VL53L0X sensor not found, continuing without distance sensing");
  }

  // Initialize servo
  myServo.attach(servoPin);
  myServo.write(servoPos);
  servoActiveStartTime = millis();

  // Show startup eyes
  displayStartupEyes();

  // Connect to WiFi
  connectToWiFi();

  // Initialize time
  if (wifiConnected) {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2, ntpServer3);
    lastTimeSync = millis();

    // Wait for time synchronization
    delay(2000);

    // Verify time sync with retry
    struct tm timeinfo;
    int retries = 0;
    while (!getLocalTime(&timeinfo) && retries < 10) {
      delay(1000);
      retries++;
      display.setTextSize(2);
      display.setCursor(0, 0);
      display.println("Wait for me");
      Serial.println("Waiting for time sync...");
      yield();
    }

    getWeatherData();
  }

  Serial.println("Enhanced Eyes System with VL53L0X Ready");
  Serial.print("Initial free heap: ");
  Serial.println(ESP.getFreeHeap());
  delay(2000);

  // Startup robo eyes
  roboEyes.begin(SCREEN_WIDTH, SCREEN_HEIGHT, 100);  // screen-width, screen-height, max framerate

  // Define some automated eyes behaviour
  roboEyes.setAutoblinker(ON, 3, 2);  // Start auto blinker animation cycle
  roboEyes.setIdleMode(ON, 2, 2);     // Start idle animation cycle

  // Define eye shapes, all values in pixels
  roboEyes.setWidth(36, 36);       // byte leftEye, byte rightEye
  roboEyes.setHeight(36, 36);      // byte leftEye, byte rightEye
  roboEyes.setBorderradius(8, 8);  // byte leftEye, byte rightEye
  roboEyes.setSpacebetween(10);    // int space

  // Define mood, curiosity and position
  roboEyes.setMood(DEFAULT);      // mood expressions
  roboEyes.setPosition(DEFAULT);  // cardinal directions
  roboEyes.setCuriosity(ON);      // bool on/off

  // Play prebuilt oneshot animations
  roboEyes.anim_confused();  // confused - eyes shaking left and right
  roboEyes.anim_laugh();     // laughing - eyes shaking up and down

  roboEyes.setMood(TIRED);
  roboEyes.setMood(ANGRY);
  roboEyes.setMood(HAPPY);

}  // end of setup

void loop() {
  // Yield to allow ESP32 to handle background tasks
  yield();

  // Check for person presence using VL53L0X sensor
  checkPersonPresence();

  // If person is detected and within greeting display time, show greeting
  if (personDetected && (millis() - personDetectedTime < greetingDisplayTime)) {
    displayGreeting();
    delay(500);  // Keep greeting visible
    return;      // Skip normal display cycle while showing greeting
  }

  // Monitor memory usage
  monitorMemory();

  // Time sync check
  syncTimeIfNeeded();

  // Check for WiFi status changes
  checkWiFiStatusChange();

  // Update servo position continuously
  updateServo();

  // Change to random mood every 3-7 seconds
  static unsigned long lastMoodChange = 0;
  static unsigned long nextMoodInterval = 10000;

  if (millis() - lastMoodChange > nextMoodInterval) {
    int randomMood = random(0, 4);
    switch (randomMood) {
      case 0: roboEyes.setMood(DEFAULT); break;
      case 1: roboEyes.setMood(TIRED); break;
      case 2: roboEyes.setMood(ANGRY); break;
      case 3: roboEyes.setMood(HAPPY); break;
    }

    lastMoodChange = millis();
    nextMoodInterval = random(3000, 7000);  // Next change in 3-7 seconds
  }

  // Show RoboEyes with current mood for 10 seconds
  unsigned long eyesStartTime = millis();
  while (millis() - eyesStartTime < 10000) {
    // Check for person during eyes display
    checkPersonPresence();
    if (personDetected && (millis() - personDetectedTime < greetingDisplayTime)) {
      break;  // Exit eyes display to show greeting
    }

    roboEyes.update();  // update eyes drawings
    updateServo();
    delay(50);
    yield();
  }

  // Show time page (unless person detected)
  if (!personDetected || (millis() - personDetectedTime >= greetingDisplayTime)) {
    displayTime();
    delay(3000);
  }

  // Show funny quote (unless person detected)
  if (!personDetected || (millis() - personDetectedTime >= greetingDisplayTime)) {
    displayFunnyQuote();
    delay(3000);
  }

  // Show weather (unless person detected)
  if (wifiConnected && (!personDetected || (millis() - personDetectedTime >= greetingDisplayTime))) {
    displayWeather();
    delay(2000);
  }

  // Update weather data periodically
  if (wifiConnected && (millis() - lastWeatherUpdate > weatherUpdateInterval)) {
    getWeatherData();
  }
}
