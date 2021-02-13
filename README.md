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

![](https://user-images.githubusercontent.com/78769090/107862982-b0d8bb00-6e1e-11eb-85fd-d1b5bc89c99e.PNG)

## Schematic 

![finalSchematic](https://user-images.githubusercontent.com/78769090/107863370-edf27c80-6e21-11eb-80d8-1cb7a4440dd2.PNG)

## Electrical Setup 


1. ![](https://user-images.githubusercontent.com/78769090/107863342-ac61d180-6e21-11eb-9471-06d0a86a2e5e.jpg)



1. ![](https://user-images.githubusercontent.com/78769090/107863346-bb488400-6e21-11eb-92a3-bbda369875b2.jpg)



## Readings Published on ThingSpeak.com


![Temperature & Humidity readings](https://user-images.githubusercontent.com/78769090/107863492-c6e87a80-6e22-11eb-9f63-fde7c162b2c3.PNG)
