#include <ZMPT101B.h>
#include <SD.h>
#include <SPI.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define SENSITIVITY 570.0f
#define INTERVAL 300 // 5 minutes interval in seconds

// Define pin numbers for the sensors
const int currentSensorPin1 = 34;  // Pin for the first ACS712 current sensor
const int voltageSensorPin1 = 35;  // Pin for the first ZMPT101B voltage sensor
const int currentSensorPin2 = 32;  // Pin for the second ACS712 current sensor
const int voltageSensorPin2 = 33;  // Pin for the second ZMPT101B voltage sensor
const int relayPin1 = 2;
//const int relayPin2 = 36;
const int relayPin2 = 3;

// Define the SD card CS pin
const int chipSelect = 5;

int mVperAmp = 100;

double Voltage1 = 0;
double VRMS1 = 0;
double AmpsRMS1 = 0;
double voltage1 = 0;
double Power1 = 0;
double Energy1 = 0;

double Voltage2 = 0;
double VRMS2 = 0;
double AmpsRMS2 = 0;
double voltage2 = 0;
double Power2 = 0;
double Energy2 = 0;

float ReachedEngVal1 = 0;
float ReachedEngVal2 = 0;
float ThreshVal1 = 0.4;
float ThreshVal2 = 0.5;

unsigned long previousMillis = 0;
unsigned long intervalMillis = INTERVAL * 1000;

ZMPT101B voltageSensor1(voltageSensorPin1, 50.0);
ZMPT101B voltageSensor2(voltageSensorPin2, 50.0);
 
const char *ssid = "Dialog 4G 107";
const char *password = "7dBA6De3";

AsyncWebServer server(80);

// NTP client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);
String alertMessage = "";

String getCurrentReadings();
String calculateEnergy(String date, String endTime, int device);
String calculateWeeklyEnergy(int device);
String calculateMonthlyEnergy(int device);
float getVPP(int pin);
void thresholdReached(int deviceId);
// HTML content
const char *index_html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>ESP32 Data</title>



  <style>
    body { background-color: bisque; }
    h1 { color: #333; 
        background-color: yellowgreen;
    }
    button { padding: 10px 20px; margin: 5px; }
    pre { white-space: pre-wrap; }
    .centered-text {
    text-align: center; /* Horizontally center text */
  }
  </style>

</head>
<body>
  <h1 class="centered-text"><b>SMART ENERGY METER</b></h1>

<table border="1" cellpadding="10" cellspacing="0">
  <tr>
    <th>Instant Data:</th>
    <th>Daily Usage:</th>
    <th>Weekly Usage Data:</th>
    <th>monthly Usage Data:</th>
  </tr>
  <tr>
    <td> <pre id="instantData"></pre></td>
    <td><label for="date">Date:</label>
  <input type="date" id="date" name="date" required><br><br>
  
  <label for="endTime">End Time:</label>
  <input type="time" id="endTime" name="endTime" required><br><br>
  
  <label for="dailyDevice">Device:</label>
  <select id="dailyDevice" name="dailyDevice" required>
    <option value="1">Device 1</option>
    <option value="2">Device 2</option>
  </select><br><br>
  
  <button type="button"  class="btn btn-info" onclick="getDailyData()">Get Daily Data</button>
  
  <h2>Daily Usage Data:</h2>
  <pre id="dailyUsage"></pre>
</td>
    <td><label for="weeklyDevice">Choose Device:</label>
  <select id="weeklyDevice" name="weeklyDevice" required>
    <option value="1">Device 1</option>
    <option value="2">Device 2</option>
  </select><br><br>
  
  <input type="button"  class="btn btn-warning" value="Get Weekly Data" onclick="fetchWeeklyUsage()">
  
  <h2>Weekly Usage Data:</h2>
  <pre id="weeklyUsage"></pre>
</td>
    <td> <label for="monthlyDevice">Choose Device:</label>
  <select id="monthlyDevice" name="monthlyDevice" required>
    <option value="1">Device 1</option>
    <option value="2">Device 2</option>
  </select><br><br>
  
  <input type="button" class="btn btn-success"  value="Get Monthly Data" onclick="fetchMonthlyUsage()">
  
  <h2>Monthly Usage Data:</h2>
  <pre id="monthlyUsage"></pre></td>
  </tr>
 
</table>




  


  
  <p>Set Thresholds:</p>
  <label for="threshold1">Device 1 Threshold (kWh):</label>
  <input type="number" id="threshold1" step="0.01" min="0" value="10">
  <button  class="btn btn-primary" onclick="setThreshold(1)">Set Threshold 1</button> <br><br>
  
  <label for="threshold2">Device 2 Threshold (kWh):</label>
  <input type="number" id="threshold2" step="0.01" min="0" value="10">
  <button   class="btn btn-primary"  onclick="setThreshold(2)">Set Threshold 2</button>
  
  <p id="alertMessage"></p>
  
  <button  class="btn btn-danger"  onclick="toggleRelay(1)">Toggle Device 1 Power</button>
  <button  class="btn btn-danger"  onclick="toggleRelay(2)">Toggle Device 2 Power</button>
  
  <script>
    function fetchInstantData() {
      fetch('/instant')
        .then(response => response.text())
        .then(text => document.getElementById('instantData').textContent = text)
        .catch(error => console.error('Error fetching instant data:', error));
    }
    setInterval(fetchInstantData, 2000); // Refresh every 2 seconds
    fetchInstantData(); // Initial fetch

    function getDailyData() {
      const date = document.getElementById("date").value;
      const endTime = document.getElementById("endTime").value;
      const device = document.getElementById("dailyDevice").value;
      const queryString = `/daily?date=${date}&endTime=${endTime}&device=${device}`;
      
      fetch(queryString)
        .then(response => response.text())
        .then(text => document.getElementById('dailyUsage').textContent = text)
        .catch(error => console.error('Error fetching daily data:', error));
    }

    function fetchWeeklyUsage() {
      const device = document.getElementById("weeklyDevice").value;
      const queryString = `/weekly?device=${device}`;
      
      fetch(queryString)
        .then(response => response.text())
        .then(text => document.getElementById('weeklyUsage').textContent = text)
        .catch(error => console.error('Error fetching weekly data:', error));
    }

    function fetchMonthlyUsage() {
      const device = document.getElementById("monthlyDevice").value;
      const queryString = `/monthly?device=${device}`;
      
      fetch(queryString)
        .then(response => response.text())
        .then(text => document.getElementById('monthlyUsage').textContent = text)
        .catch(error => console.error('Error fetching monthly data:', error));
    }

    function setThreshold(device) {
      const threshold = document.getElementById(`threshold${device}`).value;
      fetch(`/setThreshold?device=${device}&value=${threshold}`)
        .catch(error => console.error('Error setting threshold:', error));
    }

    function toggleRelay(device) {
      fetch(`/toggleRelay?device=${device}`)
        .catch(error => console.error('Error toggling relay:', error));
    }

    function fetchAlertMessage() {
      fetch('/alert')
        .then(response => response.text())
        .then(text => document.getElementById('alertMessage').textContent = text)
        .catch(error => console.error('Error fetching alert message:', error));
    }
    setInterval(fetchAlertMessage, 5000); // Fetch alert message every 5 seconds
    fetchAlertMessage(); // Initial fetch
  </script>
</body>
</html>


)rawliteral";


void setup() {
  Serial.begin(115200);
  Serial.println("ACS712 and ZMPT101B Sensors");
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin1, HIGH);
  digitalWrite(relayPin2, HIGH);

  // Set sensitivity for voltage sensors
  voltageSensor1.setSensitivity(SENSITIVITY);
  voltageSensor2.setSensitivity(SENSITIVITY);

    // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize SD card
  if (!SD.begin(chipSelect)) {
    Serial.println("Card Mount Failed");
    return;
  }

  // Check if the log file exists, if not, create it and write headers
  if (!SD.exists("/energy_log.txt")) {
    File dataFile = SD.open("/energy_log.txt", FILE_WRITE);
    if (dataFile) {
      //dataFile.println("Date, Time, Energy1 (Wh), Energy2 (Wh)");
      dataFile.close();
      Serial.println("Log file created with headers.");
    } else {
      Serial.println("Error creating energy_log.txt");
      return;
    }
  }

  // Check if the threshold file exists, if not, create it and write headers
  if (!SD.exists("/thresh_val.txt")) {
    File threshFile = SD.open("/thresh_val.txt", FILE_WRITE);
    if (threshFile) {
      //threshFile.println("Date, Time, ReachedEngVal1 (Wh), ReachedEngVal2 (Wh)");
      threshFile.close();
      Serial.println("Threshold file created with headers.");
    } else {
      Serial.println("Error creating thresh_val.txt");
      return;
    }
  }



  // Initialize NTP client
  timeClient.begin();
  timeClient.setTimeOffset(19800); // Adjust this offset for your timezone

  // Serve the index HTML file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html); // Make sure `index_html` is defined or loaded elsewhere
  });

  // Handle HTTP GET requests for current readings
  server.on("/instant", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = getCurrentReadings();
    request->send(200, "text/plain", response);
  });

  // Handle HTTP GET requests for daily energy
  server.on("/daily", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("date") && request->hasParam("endTime") && request->hasParam("device")) {
      String date = request->getParam("date")->value();
      String endTime = request->getParam("endTime")->value();
      int device = request->getParam("device")->value().toInt();
      String data = calculateEnergy(date, endTime, device);
      request->send(200, "text/plain", data);
    } else {
      request->send(400, "text/plain", "Missing parameters");
    }
  });

  // Handle HTTP GET requests for weekly energy
  server.on("/weekly", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("device")) {
      int device = request->getParam("device")->value().toInt();
      String data = calculateWeeklyEnergy(device);
      request->send(200, "text/plain", data);
    } else {
      request->send(400, "text/plain", "Missing device parameter");
    }
  });

  // Handle HTTP GET requests for monthly energy
  server.on("/monthly", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("device")) {
      int device = request->getParam("device")->value().toInt();
      String data = calculateMonthlyEnergy(device);
      request->send(200, "text/plain", data);
    } else {
      request->send(400, "text/plain", "Missing device parameter");
    }
  });

  // Handle HTTP GET requests for setting threshold
  server.on("/setThreshold", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("device") && request->hasParam("value")) {
      int device = request->getParam("device")->value().toInt();
      float value = request->getParam("value")->value().toFloat();
      if (device == 1) {
        ThreshVal1 = value;
      } else if (device == 2) {
        ThreshVal1 = value;
      }
      request->send(200, "text/plain", "Threshold set.");
    } else {
      request->send(400, "text/plain", "Missing parameters");
    }
  });

  // Handle HTTP GET requests for toggling relay
  server.on("/toggleRelay", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("device")) {
      int device = request->getParam("device")->value().toInt();
      if (device == 1) {
        digitalWrite(relayPin1, !digitalRead(relayPin1));
      } else if (device == 2) {
        digitalWrite(relayPin2, !digitalRead(relayPin2));
      }
      request->send(200, "text/plain", "Relay toggled.");
    } else {
      request->send(400, "text/plain", "Missing device parameter");
    }
  });

  // Handle HTTP GET requests for alerts
  server.on("/alert", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", alertMessage);
  });

  // Start the server
  server.begin();
  Serial.println("HTTP server started");

}

void loop() {
  unsigned long currentMillis = millis();

  // Get voltage reading for the first pair
  voltage1 = voltageSensor1.getRmsVoltage()-20;
  // Serial.print("Voltage 1: ");
  // Serial.print(voltage1);
  // Serial.println(" V");

  // Get current reading for the first pair
  Voltage1 = getVPP(currentSensorPin1);
  VRMS1 = (Voltage1 / 2.0) * 0.707;
  AmpsRMS1 = (((VRMS1 * 1000) / mVperAmp) - 0.18) * 10;
  // Serial.print("Current 1: ");
  // Serial.print(AmpsRMS1);
  // Serial.println(" A");

  // Calculate power for the first pair
  Power1 = voltage1 * AmpsRMS1;
  // Serial.print("Power 1: ");
  // Serial.print(Power1);
  // Serial.println(" W");

  // Get voltage reading for the second pair
  voltage2 = voltageSensor2.getRmsVoltage();
  // Serial.print("Voltage 2: ");
  // Serial.print(voltage2);
  // Serial.println(" V");

  // Get current reading for the second pair
  Voltage2 = getVPP(currentSensorPin2);
  VRMS2 = (Voltage2 / 2.0) * 0.707;
  AmpsRMS2 = (((((VRMS2 * 1000) / mVperAmp) - 0.18) * 10))/100;
  // Serial.print("Current 2: ");
  // Serial.print(AmpsRMS2);
  // Serial.println(" A");

  // Calculate power for the second pair
  Power2 = voltage2 * AmpsRMS2;
  // Serial.print("Power 2: ");
  // Serial.print(Power2);
  // Serial.println(" W");

  // Accumulate energy for each pair
  Energy1 += Power1 / 3600; // Convert power from watts to watt-hours
  Energy2 += Power2 / 3600;

  // Update the reached energy values
  ReachedEngVal1 += Energy1;
  ReachedEngVal2 += Energy2;

  // Check if the threshold value is reached
  if (ReachedEngVal1 >= ThreshVal1)
  {
    thresholdReached(1);
  }
  if (ReachedEngVal2 >= ThreshVal2) 
  {
    thresholdReached(2);
  }

  // Check if 5 minutes have passed
  if (currentMillis - previousMillis >= intervalMillis) {
    previousMillis = currentMillis;

    // Update time from NTP server
    timeClient.update();

    // Get the current date and time
    time_t rawtime = timeClient.getEpochTime();
    struct tm * ti;
    ti = localtime(&rawtime);

    String timestamp = timeClient.getFormattedTime();
    char dateBuffer[20];
    sprintf(dateBuffer, "%04d-%02d-%02d", ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday);
    String date = String(dateBuffer);

    Serial.print("Current date and time: ");
    Serial.print(date);
    Serial.print(" ");
    Serial.println(timestamp);

    // Store the energy values in the SD card
    File dataFile = SD.open("/energy_log.txt", FILE_APPEND);
    if (dataFile) {
      dataFile.print(date);
      dataFile.print(", ");
      dataFile.print(timestamp);
      dataFile.print(", ");
      dataFile.print(Energy1);
      dataFile.print(", ");
      dataFile.println(Energy2);
      dataFile.close();
      Serial.println("Data written to SD card.");
    } else {
      Serial.println("Error opening energy_log.txt");
    }

    // Store the reached energy values in the threshold file
    File threshFile = SD.open("/thresh_val.txt", FILE_APPEND);
    if (threshFile) {
      threshFile.print(date);
      threshFile.print(", ");
      threshFile.print(timestamp);
      threshFile.print(", ");
      threshFile.print(ReachedEngVal1);
      threshFile.print(", ");
      threshFile.println(ReachedEngVal2);
      threshFile.close();
      Serial.println("Threshold values written to SD card.");
    } else {
      Serial.println("Error opening thresh_val.txt");
    }

    // Reset energy accumulation for the next interval
    Energy1 = 0;
    Energy2 = 0;
  }

  // if (Serial.available() > 0) 
  // {
  //   char command = Serial.read();
  //   if (command == 'c') {
  //     double en0 = calculateEnergy("2024-08-05", "06:59:55", 1);
  //     double en1 = calculateWeeklyEnergy(1);
  //     double en2 = calculateMonthlyEnergy(1);
  //     double en3 = calculateEnergy("2024-08-05", "06:59:55", 2);
  //     double en4 = calculateWeeklyEnergy(2);
  //     double en5 = calculateMonthlyEnergy(2);
  //     String values = getCurrentReadings();
  //     Serial.println(en0);
  //     Serial.println(en1);
  //     Serial.println(en2);
  //     Serial.println(en3);
  //     Serial.println(en4);
  //     Serial.println(en5);
  //     Serial.println(values);
  //     delay(10000);
  //   }
  // }

  delay(1000);
}

// Function to get the peak-to-peak voltage
  float getVPP(int pin) {
  float result;
  int readValue;
  int maxValue = 0;
  int minValue = 4096;  // max ADC value for ESP32

  uint32_t start_time = millis();
  while ((millis() - start_time) < 1000) { // sample for 1 sec
    readValue = analogRead(pin);
    if (readValue > maxValue) {
      maxValue = readValue;
    }
    if (readValue < minValue) {
      minValue = readValue;
    }
  }

  result = ((maxValue - minValue) * 3.3) / 4096.0;
  return result;
}
//Function to calculate energy consumed by a device from 12.00 a.m. to a specified time
String calculateEnergy(String date, String endTime, int device)
{
  double totalEnergy = 0.0;
  File dataFile = SD.open("/energy_log.txt");

  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      if (line.length() > 0) {
        int firstComma = line.indexOf(',');
        int secondComma = line.indexOf(',', firstComma + 1);
        String logDate = line.substring(0, firstComma);
        String logTime = line.substring(firstComma + 2, secondComma);

        if (logDate == date && logTime <= endTime) {
          int thirdComma = line.indexOf(',', secondComma + 1);
          String energy1Str = line.substring(secondComma + 2, thirdComma);
          String energy2Str = line.substring(thirdComma + 2);

          double energy1 = energy1Str.toDouble();
          double energy2 = energy2Str.toDouble();

          if (device == 1) {
            totalEnergy += energy1;
          } else if (device == 2) {
            totalEnergy += energy2;
          }
        }
      }
    }
    dataFile.close();
  } else {
    Serial.println("Error opening energy_log.txt for reading");
  }
  String totalEnergyStr = String( totalEnergy);
  return totalEnergyStr;
}
//Function to calculate energy consumed by a device throughout a week
String calculateWeeklyEnergy(int device)
{
  double totalEnergy = 0.0;
  File dataFile = SD.open("/energy_log.txt");

  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      if (line.length() > 0) {
        int firstComma = line.indexOf(',');
        int secondComma = line.indexOf(',', firstComma + 1);
        String logDate = line.substring(0, firstComma);
        int year, month, day;
        sscanf(logDate.c_str(), "%d-%d-%d", &year, &month, &day);
        struct tm logTime;
        logTime.tm_year = year - 1900;
        logTime.tm_mon = month - 1;
        logTime.tm_mday = day;
        logTime.tm_hour = 0;
        logTime.tm_min = 0;
        logTime.tm_sec = 0;
        time_t logEpoch = mktime(&logTime);

        time_t currentEpoch = timeClient.getEpochTime();
        double secondsInDay = 24 * 3600;
        double daysDifference = difftime(currentEpoch, logEpoch) / secondsInDay;

        if (daysDifference < 7) {
          int thirdComma = line.indexOf(',', secondComma + 1);
          String energy1Str = line.substring(secondComma + 2, thirdComma);
          String energy2Str = line.substring(thirdComma + 2);

          double energy1 = energy1Str.toDouble();
          double energy2 = energy2Str.toDouble();

          if (device == 1) {
            totalEnergy += energy1;
          } else if (device == 2) {
            totalEnergy += energy2;
          }
        }
      }
    }
    dataFile.close();
  } else {
    Serial.println("Error opening energy_log.txt for reading");
  }
  String totalEnergyStr = String(totalEnergy);
  return totalEnergyStr;
}

//Function to calculate energy consumed by a device throughuot a month
String calculateMonthlyEnergy(int device)
{
  double totalEnergy = 0.0;
  File dataFile = SD.open("/energy_log.txt");

  if (dataFile) {
    while (dataFile.available()) {
      String line = dataFile.readStringUntil('\n');
      if (line.length() > 0) {
        int firstComma = line.indexOf(',');
        int secondComma = line.indexOf(',', firstComma + 1);
        String logDate = line.substring(0, firstComma);

        // Extract year and month from the date
        int year, month, day;
        sscanf(logDate.c_str(), "%d-%d-%d", &year, &month, &day);

        // Get the current time
        timeClient.update();
        time_t currentEpoch = timeClient.getEpochTime();
        struct tm *ti;
        ti = localtime(&currentEpoch);
        int currentYear = ti->tm_year + 1900;
        int currentMonth = ti->tm_mon + 1;

        // Check if the log entry is from the current month and year
        if (year == currentYear && month == currentMonth) {
          int thirdComma = line.indexOf(',', secondComma + 1);
          String energy1Str = line.substring(secondComma + 2, thirdComma);
          String energy2Str = line.substring(thirdComma + 2);

          double energy1 = energy1Str.toDouble();
          double energy2 = energy2Str.toDouble();

          if (device == 1) {
            totalEnergy += energy1;
          } else if (device == 2) {
            totalEnergy += energy2;
          }
        }
      }
    }
    dataFile.close();
  } else {
    Serial.println("Error opening energy_log.txt for reading");
  }
  String totalEnergyStr = String(totalEnergy);
  return totalEnergyStr;
}

String getCurrentReadings() {
  String readings = "";
  readings += "Voltage 1: " + String(voltage1) + " V\n";
  readings += "Current 1: " + String(AmpsRMS1) + " A\n";
  readings += "Power 1: " + String(Power1) + " W\n";
  readings += "Energy 1: " + String(Energy1) + " Wh\n";
  readings += "Voltage 2: " + String(voltage2) + " V\n";
  readings += "Current 2: " + String(AmpsRMS2) + " A\n";
  readings += "Power 2: " + String(Power2) + " W\n";
  readings += "Energy 2: " + String(Energy2) + " Wh\n";
  return readings;
}

void thresholdReached(int deviceId)
{
 
  if (deviceId == 1) 
  {
    String alertMessage = "Energy threshold for device 1 reached. Would you like to switch off device 1?";
    ReachedEngVal1 = 0;
     //server.send(200, "text/plain", alertMessage);
  } else if (deviceId == 2) 
  {
     String alertMessage = "Energy threshold for device 2 reached. Would you like to switch off device 1?";
     ReachedEngVal2 = 0;
     //server.send(200, "text/plain", alertMessage);
  }

  // Reset the reached energy value
  // if (deviceId == 1) {
  //   ReachedEngVal1 = 0;
  // } else if (deviceId == 2) {
  //   ReachedEngVal2 = 0;
  // }
}
