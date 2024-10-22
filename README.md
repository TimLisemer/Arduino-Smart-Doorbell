# Smart Doorbell Integration with Arduino Uno

This project enhances my traditional (dumb) doorbell system by integrating it with smart home automation. 
Using an Arduino Uno with a relay shield and Ethernet shield, the system communicates with Home Assistant via MQTT. 
This setup allows me to control my existing dumb doorbell chimes through Home Assistant, while also detecting button presses 
on both the smart and dumb doorbells.

## Project Overview

### Problem:
- Original setup: A dumb doorbell system with two chimes, one transformer, and a standard button. 
  Pressing the button would close the circuit and make the chimes ring.

### Solution:
- **Make the dumb doorbell smart:** By adding an Arduino Uno to the system, I can now control the doorbell chimes through Home Assistant. 
  Additionally, pressing the dumb doorbell button can trigger automations in my Home Assistant setup.

- **Note:** The original doorbell circuit remains undisturbed. This means that the old dumb doorbell system will still work 
  even if the Arduino loses power or is disconnected.

## Hardware Setup

- Arduino Uno
- Relay Shield for Arduino
- Ethernet Shield for Arduino
- AC/DC Step-down converter (8V AC to 3.3V DC)
- Cat 5 cable (or similar with multiple cores)
- MQTT broker (configured on Home Assistant)

## Circuit Design

### Original Setup:
- The system consists of a transformer powering two doorbell chimes, with a button to complete the circuit when pressed, causing the chimes to ring.

### Modified Setup:
- I cut the wire going to one of the chimes and placed the Arduino into the circuit.
- A previously unused core in the Cat 5 cable was used to carry a permanent 8V AC supply from the transformer to the Arduino's relay.
- The relay shield now controls whether the circuit is closed, which determines whether the chimes ring.

### Button Press Detection:
- I detect button presses by wiring the doorbell button through an AC/DC step-down converter. 
  The converter reduces the 8V AC signal to 3.3V DC, which is read by the Arduino on pin 3.

## Features

1. **Control Dumb Chimes via MQTT:**
   - Home Assistant can send MQTT messages to control the relay. When the relay closes, it rings the dumb doorbell chimes.
    
2. **Detect Dumb Button Presses:**
   - When the dumb doorbell button is pressed, the Arduino detects this and sends an MQTT message to Home Assistant, 
     allowing for smart automations to be triggered.

## Software Details

1. **Libraries Required:**
   - PubSubClient for MQTT communication.
   - Ethernet for Ethernet shield handling.

2. **MQTT Topics Used:**
   - `home/doorbell/chime/set` (to control chime ON/OFF)
   - `home/doorbell/chime/available` (to report Arduino availability)
   - `home/doorbell/button` (to report the state of the dumb doorbell button)

## Code Overview

- **Relay Control:** The relay is controlled via the MQTT topic `home/doorbell/chime/set`. 
  When the topic receives an "ON" message, the relay closes, ringing the chimes. 
  An "OFF" message opens the relay.

- **Button Detection:** The Arduino detects the state of the dumb doorbell button using pin 3. 
  It sends an MQTT message to `home/doorbell/button` when the button is pressed or released.

- **Ethernet and MQTT Setup:** 
  The Ethernet shield is configured with a static IP, and the Arduino connects to the MQTT broker to 
  publish and subscribe to relevant topics.

