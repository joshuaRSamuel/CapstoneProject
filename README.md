# CapstoneProject
## Point of Use Temperature and Humidity Monitoring system
An electrical setup that tracks the heat and humidity transfer of the wall being measured. This is independently powered where sensors would calculate live data and processed data is sent to the mobile app via Wi-Fi module.
## Summary
The design of the product is simple and feasible. The system monitors real-time variations in humidity and temperature. It consists of a DHT22 temperature and humidity sensor interfaced with PIC18F45K22 microcontroller to initiate data transmission from the sensor. These live readings are then sent wirelessly to the user using Wi-Fi-module (ESP8266). This data is transmitted on the user’s terminal using a designated server and display variations in readable graphical format. 
## Scope 
  * Sensors used in the design are independently powered using AC power adapter of design decisions. Sensors are positioned across the wall to get required readings in the given requirements. Sensors can process the collected data using microcontroller procedures and wirelessly sending to the mobile app.

  * Sensors can produce and record temperature range from-50◦C - 80◦C and a relative humidity  range  from  30-100%.  Sensors  can operate  and  read  data  during  varied weather conditions and are also able to report data on a user specified interval. 

   * The collected data is exported in a suitable graphical format for user interface. HMI component is added for user end along with mentioned specifications to meet all customer requirements. This was all validated through test reports and documentation.
## Proposed Embodiment 
![prjctflowdiagramPNG.png](C:\Users\USER\Desktop\capstone_project)


