#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

byte mac[] = { 0x52, 0x1A, 0x3B, 0x4C, 0x5D, 0x6E };
IPAddress ip(10, 0, 0, 80);
EthernetClient ethClient;
PubSubClient client(ethClient);

const int relayPin = 4;  // Only one relay is used now
boolean relayStatus = false;
const int buttonPin = 3; // Digital pin for listening to 3.3V signal
boolean lastButtonState = LOW;
boolean optimisticMode = false; // Set optimistic mode by default

void setup() {
  Ethernet.begin(mac, ip);

  if (Serial) {
    Serial.begin(115200);  // Increased baud rate for debugging
  }

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Initially turn off the relay
  relayStatus = false; // Initialize relayStatus

  pinMode(buttonPin, INPUT); // Set up button input pin
  
  client.setServer("10.0.0.2", 1883);
  client.setCallback(callback);

  if (Serial) {
    Serial.println("Connecting to MQTT broker...");
  }

  while (!client.connected()) {
    if (client.connect("arduinoClient")) {
      if (Serial) {
        Serial.println("Connected to MQTT broker.");
      }
      client.subscribe("home/doorbell/chime/set");

      if (optimisticMode) {
        client.publish("home/doorbell/chime", relayStatus ? "ON" : "OFF", true);
      }

      client.publish("home/doorbell/chime/available", "online", true);
      
    } else {
      // MQTT connection failed, print error and try ping tests
      if (Serial) {
        Serial.print("Failed, rc=");
        Serial.print(client.state());
        Serial.println(" Retrying in 5 seconds...");

        // Perform ping test to Google (8.8.8.8) and Cloudflare (1.1.1.1)
        pingTest();
      }
      delay(5000);  // Retry in 5 seconds
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Check button state and send MQTT message if changed
  boolean currentButtonState = digitalRead(buttonPin);
  if (currentButtonState != lastButtonState) {
    if (currentButtonState == HIGH) {
      client.publish("home/doorbell/button", "ON", true);
    } else {
      client.publish("home/doorbell/button", "OFF", true);
    }
    delay(1000);
    lastButtonState = currentButtonState;
  }
}

// Callback function to handle incoming messages
void callback(char* topic, byte* payload, unsigned int length) {
  if (Serial) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
  }

  // Ensure the payload is a valid string
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0'; // Null-terminate the string

  if (Serial) {
    Serial.println(message); // Print the payload
  }

  // Handle the message: turn the relay ON or OFF
  if (strcmp(topic, "home/doorbell/chime/set") == 0) {
    if (strcmp(message, "ON") == 0) {
      digitalWrite(relayPin, HIGH);
      relayStatus = true;

      if (Serial) {
        Serial.println("Relay switched ON");
      }

    } else if (strcmp(message, "OFF") == 0) {
      digitalWrite(relayPin, LOW);
      relayStatus = false;

      if (Serial) {
        Serial.println("Relay switched OFF");
      }
    }

    // Publish state if not in optimistic mode
    if (!optimisticMode) {
      client.publish("home/doorbell/chime", relayStatus ? "ON" : "OFF", true);
    }
  }
}

// Reconnect to the MQTT broker if disconnected
void reconnect() {
  while (!client.connected()) {
    if (Serial) {
      Serial.print("Attempting MQTT connection...");
    }

    if (client.connect("arduinoClient")) {
      if (Serial) {
        Serial.println("connected");
      }

      client.subscribe("home/doorbell/chime/set");

      if (optimisticMode) {
        client.publish("home/doorbell/chime", relayStatus ? "ON" : "OFF", true);
      }

      client.publish("home/doorbell/chime/available", "online", true);
      
    } else {
      if (Serial) {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");

        // Perform ping test to Google (8.8.8.8) and Cloudflare (1.1.1.1)
        pingTest();
      }
      delay(5000);
    }
  }
}

// Function to test connectivity to Google and Cloudflare IPs
void pingTest() {
  IPAddress google(8, 8, 8, 8);
  IPAddress cloudflare(1, 1, 1, 1);

  Serial.println("Performing ping test...");

  if (ethClient.connect(google, 80)) {
    Serial.println("Ping to Google (8.8.8.8) successful.");
    ethClient.stop();
  } else {
    Serial.println("Ping to Google (8.8.8.8) failed.");
  }

  if (ethClient.connect(cloudflare, 80)) {
    Serial.println("Ping to Cloudflare (1.1.1.1) successful.");
    ethClient.stop();
  } else {
    Serial.println("Ping to Cloudflare (1.1.1.1) failed.");
  }
}
