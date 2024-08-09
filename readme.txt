1. Overview

The IoT-based Smart Energy Meter is designed to monitor and manage electrical energy consumption in real-time. It measures key parameters like current, voltage, power, and energy consumption (kWh) using sensors connected to an ESP32 microcontroller. The data is stored locally on an SD card and transmitted to a web interface where users can monitor usage, set thresholds, and control connected devices. This system provides users with insights into their energy usage patterns and helps in optimizing energy consumption.

2. Methodology

2.1. Hardware Components

ESP32 Microcontroller: Central unit for data processing, communication, and control.
Current and Voltage Sensors (e.g., PZEM-004T): Measure the electrical parameters such as current, voltage, and power.
SD Card Module: For local data storage of daily, weekly, and monthly energy usage.
Relay Modules: For controlling the power supply to connected devices.
Power Supply: To power the ESP32 and sensors.
LCD Display (Optional): For local display of real-time data.
2.2. Software Components

Arduino IDE: Used for programming the ESP32.
Web Interface (HTML/CSS/JavaScript): Displays real-time data, historical usage, and control options.
Firebase or Local Database: To store and retrieve data for remote monitoring and control.
Embedded C/C++: For writing the firmware to handle sensor data acquisition, storage, and communication.
2.3. Data Acquisition

Sensor Integration: The current and voltage sensors are interfaced with the ESP32 to measure real-time electrical parameters.
Data Processing: The ESP32 processes the raw sensor data to calculate power and energy consumption (kWh).
Data Logging: Every 5 minutes, the calculated kWh and other relevant data are logged onto the SD card in a readable format.
2.4. Communication

Wi-Fi Module (ESP32): Facilitates the sending of data to the web interface and receiving commands for device control.
Web Interface: Displays real-time data and provides options to view daily, weekly, and monthly energy usage. Users can also set thresholds for alerts and control relays.
2.5. Control Mechanism

Relay Control: The web interface allows users to toggle power to devices via relays based on usage patterns or alerts.
Threshold Alerts: The system sends alerts if certain parameters exceed the predefined thresholds (e.g., high current, voltage, or power consumption).
2.6. Data Visualization

Real-time Monitoring: The web interface continuously updates with real-time data.
Historical Data: Users can view daily, weekly, and monthly usage trends and compare them over time.
Alerts and Notifications: Alerts are displayed on the web interface, and notifications can be sent to users if thresholds are exceeded.
3. Usage
3.1. Home Energy Management

Users can monitor their household energy consumption in real-time.
Helps in identifying high-energy-consuming devices and optimizing their usage.
Enables remote control of appliances based on usage patterns to save energy.
3.2. Industrial Monitoring

Provides insights into the energy usage of machinery and equipment.
Helps in preventing energy wastage by identifying inefficiencies.
Alerts management in case of unusual energy consumption patterns, reducing the risk of equipment failure.
3.3. Utility Management

Can be integrated into utility management systems to provide detailed insights into energy usage patterns.
Helps in accurate billing based on real-time consumption.
Assists in demand-side management by controlling peak loads and distributing energy efficiently.
3.4. Research and Development

Useful in R&D environments for testing and validating energy consumption models.
Enables the simulation of various load conditions and their impact on energy consumption.
Assists in the development of energy-efficient technologies and smart grids.