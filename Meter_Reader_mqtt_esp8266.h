
#define OTA



#define DIGITAL_INPUT_SENSOR 2  // The digital input you attached your light sensor.  (Only 2 and 3 generates interrupt!)
#define PULSE_FACTOR 1000       // Nummber of blinks per KWH of your meeter
#define MAX_WATT 10000          // Max watt value to report. This filetrs outliers.


// wifi settings
const char* ssid     = "your_ssid";
const char* password = "your_password";

// mqtt server settings
const char* mqtt_server   = "10.0.1.2";
const int mqtt_port       = 1883;
const char* mqtt_username = "mqtt_user";
const char* mqtt_password = "mqtt_password";

// mqtt client settings



const char* client_id                   = "meter-reader"; // Must be unique on the MQTT network
const char* metereader_status_topic      = "house/meter/status";
const char* metereader_debug_topic        = "house/meter/debug";
const char* metereader_debug_set_topic    = "house/meter/debug/set";


// pinouts
const int blueLedPin = 2; // Onboard LED = digital pin 0 (blue LED on adafruit ESP8266 huzzah)

// sketch settings
const unsigned int SEND_ROOM_TEMP_INTERVAL_MS = 5000;
const unsigned int READ_ROOM_TEMP_INTERVAL_MS = 1000;
