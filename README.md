# MediBox: Smart Medicine Storage System

Welcome to the MediBox repository! This project aims to provide a smart solution for storing medicines securely while also monitoring environmental conditions such as temperature and humidity. The system consists of a hardware setup using ESP32 module, DHT22 sensor, OLED display, and push buttons, along with a Node-RED dashboard for data visualization and control.

## Hardware Components

- **ESP32 Module:** Acts as the main controller for the system, handling sensor data and user inputs.
- **DHT22 Sensor:** Measures temperature and humidity levels inside the MediBox.
- **OLED Display:** Provides visual feedback to the user, displaying information such as current time, set alarms, and environmental data.
- **Push Buttons:** Used for user interaction, allowing users to set time, alarms, and navigate through the menu.

## MediBox Circuit Setup

Here is the circuit setup for the MediBox:

![MediBox Circuit Setup](https://github.com/RPX2001/Medibox-Project/blob/main/image.png)

## Functionality

- **Set Time:** Users can set the current time using the push buttons.
- **Set Alarms:** Up to 3 alarms can be set for medicine reminders. Users can adjust the alarm times using the push buttons.
- **Temperature and Humidity Monitoring:** The DHT22 sensor continuously monitors the temperature and humidity levels inside the MediBox. If the levels are outside the specified range, appropriate alerts are displayed.
- **Node-RED Dashboard Integration:** Data from the MediBox, including temperature and humidity readings, are sent to the Node-RED dashboard using MQTT protocol. Users can visualize the data and adjust the light condition in the MediBox as needed through the dashboard.

## Repository Structure

- `hardware/`: Contains the code for the ESP32 module and hardware setup.
- `node-red/`: Includes the Node-RED flows and configuration files for the dashboard setup.
- `docs/`: Documentation files, including this README and any additional guides or manuals.

## Getting Started

To replicate this project, you will need the following:

- ESP32 module
- DHT22 sensor
- OLED display
- Push buttons
- Wokwi platform for hardware simulation
- Node-RED for setting up the dashboard

Follow the instructions in the respective directories (`hardware/` and `node-red/`) to set up and configure the hardware and Node-RED dashboard.

## Contributors

- [Raveen Pramuditha](https://github.com/RPX2001)
  
## License

This project is licensed under the [MIT License](LICENSE).
