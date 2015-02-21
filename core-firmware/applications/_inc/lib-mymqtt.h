#include "lib-mqttMessage.h"

void noop();
void mqttLoop();
void mqttConnect();
void mqttDisconnected();
void defaultOnConnect();
void defaultOnDisconnect();
void mqttSubscribe(char* topic);
void mqttAction(char* action, char* payload);
void mqttPublish(char* topic, char* payload);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void mqttCustomMessageHandler(MqttMessage msg);
void mqttDefaultMessageHandler(MqttMessage msg);
void mqttStatus(char* component, char* attribute, int payload);
void mqttStatus(char* component, char* attribute, char* payload);
void mqttStatus(char* component, char* payload);
void mqttLog(char* topic, char* msg);
void mqttLog(char* topic);

TCPClient tcp;
SimpleTimer timer;
int connectTimer;
callback connectCallback;
callback disconnectCallback;

char DEVICE_ID[25];
bool DEBUG_LOG = false;
bool BROKER_CONNECTED = false;
bool SERVER_CONNECTED = false;
bool IS_CONNECTED = false;
char* DEVICE_TYPE = "";

String MQTT_DEVICE_PREFIX;
const int DEVICE_PREFIX_LENGTH = 30;
char MQTT_DEVICE_PREFIX_CHAR[DEVICE_PREFIX_LENGTH];
PubSubClient mqtt(MQTT_HOST, 1883, mqttCallback, tcp);


void mqttSetup(char* deviceType, callback onConnect, callback onDisconnect) {

    String deviceID = Spark.deviceID();

    DEVICE_TYPE = deviceType;
    MQTT_DEVICE_PREFIX = "dev/" + deviceID + "/";
    MQTT_DEVICE_PREFIX.toCharArray(MQTT_DEVICE_PREFIX_CHAR, DEVICE_PREFIX_LENGTH);
    deviceID.toCharArray(DEVICE_ID, 25);

    connectCallback = onConnect;
    disconnectCallback = onDisconnect;
    connectTimer = timer.setInterval(2000, mqttConnect);

    mqttConnect();
}

void mqttSetup(char* deviceType) { mqttSetup(deviceType, noop, noop); }

void mqttLoop() {
    mqtt.loop();
    timer.run();
    if (mqtt.connected()) {
        timer.disable(connectTimer);
        if (!BROKER_CONNECTED) {

            // At this point, the device has connected
            // to the MQTT message broker, but we still
            // don't know if the device server is ready.

            // This initiates a handshake with the device
            // server, which should respond with an event
            // topic of "command/network/connect" = "SYNACK".

            mqttAction("network/connect", "SYN");

            // Find the rest of the handshake process in
            // the mqttDefaultMessageHandler().

        }
        BROKER_CONNECTED = true;
    } else {
        timer.enable(connectTimer);
        if (BROKER_CONNECTED) {
            disconnectCallback();
        }
        BROKER_CONNECTED = false;
    }
}

void mqttDisconnected() {
    IS_CONNECTED = false;
    SERVER_CONNECTED = false;
    disconnectCallback();
}

void mqttConnect() {

    char msg[] = "1";
    String willStr = MQTT_DEVICE_PREFIX + "action/network/disconnect";
    int len = willStr.length() + 1;
    char willChar[len];
    willStr.toCharArray(willChar, len);

    if (!mqtt.connected()) {
        if (mqtt.connect(DEVICE_ID, willChar, 1, 0, msg)) {
            mqttSubscribe("setup/#");
            mqttSubscribe("command/#");
            mqtt.subscribe("dev/all/#");
        }
    }

    // mqttAction("network/connect", "SYN");
}

void mqttSubscribe(char* topic) {

    // This makes it so that every event that is subscribed
    // is prefixed in the /dev/{DEVICE_ID}/ namespace.

    String fullTopic = MQTT_DEVICE_PREFIX + String(topic);
    char charTopic[fullTopic.length() + 1];
    fullTopic.toCharArray(charTopic, fullTopic.length() + 1);
    mqtt.subscribe(charTopic);
}

void mqttPublish(char* topic, char* payload) {

    // This makes it so that every event that is published
    // is prefixed in the /dev/{DEVICE_ID}/ namespace.

    String fullTopic = MQTT_DEVICE_PREFIX + String(topic);
    char charTopic[fullTopic.length() + 1];
    fullTopic.toCharArray(charTopic, fullTopic.length() + 1);
    mqtt.publish(charTopic, payload);

}

void mqttAction(char* topic, char* payload) {
    String fullTopic = "action/" + String(topic);
    char charTopic[fullTopic.length() + 1];
    fullTopic.toCharArray(charTopic, fullTopic.length() + 1);
    mqttPublish(charTopic, payload);
}

void mqttStatus(char* component, char* attribute, char* payload) {
    String fullTopic = "status/" + String(component) + "/" + String(attribute);
    char charTopic[fullTopic.length() + 1];
    fullTopic.toCharArray(charTopic, fullTopic.length() + 1);
    mqttPublish(charTopic, payload);
}

void mqttStatus(char* component, char* attribute, int payload) {
    char str[5] = "";
    itoa(payload, str, 10);
    mqttStatus(component, attribute, str);
}

void mqttStatus(char* component, char* payload) {
    mqttStatus(component, "", payload);
}




void mqttCallback(char* incomingTopic, byte* payload, unsigned int length) {

    if (length == 0) { return; }

    char msg[length];
    int topicCount = 0;
    char* topicChunk;
    char fullTopic[strlen(incomingTopic)];
    char shortTopic[strlen(incomingTopic)];
    strcpy(fullTopic, incomingTopic);

    String payloadStr = String((const char*) payload);
    payloadStr.toCharArray(msg, length + 1);
    topicChunk = strpbrk(incomingTopic, SLASH);

    while (topicChunk != NULL) {
        topicChunk = strpbrk (topicChunk+1, SLASH);
        if (topicCount == 0) strcpy(shortTopic, topicChunk);
        topicCount++;
    }

    // this trims the slash from the front of the topic
    memmove(shortTopic, shortTopic+1, strlen(shortTopic));

    // split all the parts into an array
    char* topicParts[topicCount];

    if (topicCount > 0) {
        strtok(incomingTopic, SLASH); // dev
        strtok(NULL, SLASH);  // {deviceID}
        for(int i = 1; i < topicCount; i++) {
            topicParts[i] = strtok (NULL, SLASH);
            mqttLog("topic/part", topicParts[i]);
        }
    }

    mqttLog("topic/short", shortTopic);
    mqttLog("topic/msg", msg);

    MqttMessage mqttMsg(fullTopic, shortTopic, topicParts, topicCount, msg);

    mqttDefaultMessageHandler(mqttMsg);
    mqttCustomMessageHandler(mqttMsg);

}

void mqttDefaultMessageHandler(MqttMessage msg) {

    // If the server broadcasts a message to all devices
    // asking them to connect, then initiate a handshake.

    if (equals(msg.fullTopic(), "dev/all/network/connect")) {
        mqttAction("network/connect", "SYN");
        return;
    }

    // If the server broadcasts a message to all devices
    // to disconnect, then fire disconnect callback.

    if (equals(msg.fullTopic(), "dev/all/network/disconnect")) {
        mqttDisconnected();
        return;
    }

    // Default setup actions that apply to all devices,
    // made up mostly of setup and configuration overrides.

    if (equals(msg.topic(), "setup/debug")) {
        DEBUG_LOG = msg.boolVal();
        return;
    }

    // *********************
    // DEVICE IDENTIFICATION
    // *********************

    if (equals(msg.topic(), "command/identify")) {
        mqttAction("identify", DEVICE_TYPE);
    }

    // *********************
    // CUSTOM SETUP
    // *********************

    if (equals(msg.topic(), "command/configure")) {
        mqttAction("configure", "true");
        return;
    }


    // This is the second part of the handshake handler.
    // When the server responds with a SYNACK, then the
    // device and the server are both alive.  Send a
    // final acknowledgement to the server via ACK.

    if (equals(msg.topic(), "command/network/connect") && equals(msg.charVal(), "SYNACK")) {
        mqttAction("network/connect", "ACK");
        SERVER_CONNECTED = true;
        IS_CONNECTED = true;
        connectCallback();
        return;
    }
}

void mqttLog(char* topic, char* msg) {
    if (DEBUG_LOG) {
        char log[strlen(topic) + 4];
        strcpy(log, "log/");
        strcat(log, topic);
        mqttPublish(log, msg);
    }
}

void mqttLog(char* topic) {
    mqttLog(topic, "...");
}