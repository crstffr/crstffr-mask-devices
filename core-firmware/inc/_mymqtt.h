typedef void (*callback)(void);

void noop();
void mqttLoop();
void mqttConnect();
void mqttSubscribe(char* topic);
void mqttPublish(char* topic, char* payload);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void mqttCustomMessageHandler(char** topic, char* msg);
void mqttDefaultMessageHandler(char** topic, char* msg);
void mqttLog(char* topic, char* msg);
void mqttLog(char* topic);

TCPClient tcp;
SimpleTimer timer;
int connectTimer;
callback connectCallback;
callback disconnectCallback;

char SLASH[] = "/";
char DEVICE_ID[25];
bool DEBUG_LOG = false;
bool CONNECTED = false;

String MQTT_DEVICE_PREFIX;
PubSubClient mqtt(MQTT_HOST, 1883, mqttCallback, tcp);

void noop() {}

void mqttSetup(callback onConnect, callback onDisconnect) {
    MQTT_DEVICE_PREFIX = "dev/" + Spark.deviceID() + "/";
    String deviceID = Spark.deviceID();
    deviceID.toCharArray(DEVICE_ID, 25);

    connectCallback = onConnect;
    disconnectCallback = onDisconnect;

    connectTimer = timer.setInterval(1000, mqttConnect);
    mqttConnect();
}

void mqttSetup() { mqttSetup(noop, noop); }

void mqttLoop() {
    mqtt.loop();
    timer.run();
    if (mqtt.connected()) {
        timer.disable(connectTimer);
        if (!CONNECTED) {
            connectCallback();
        }
        CONNECTED = true;
    } else {
        timer.enable(connectTimer);
        if (CONNECTED) {
            disconnectCallback();
        }
        CONNECTED = false;
    }
}

void mqttConnect() {
    if (!mqtt.connected()) {
        if (mqtt.connect(DEVICE_ID)) {
            mqttSubscribe("setup/#");
            mqttSubscribe("control/#");
        }
    }
}

void mqttSubscribe(char* topic) {

    // This makes it so that every event that is subscribed
    // is prefixed in the /dev/{DEVICE_ID}/ namespace.

    String fullTopic = MQTT_DEVICE_PREFIX + String(topic);
    char charTopic[fullTopic.length() + 1];
    fullTopic.toCharArray(charTopic, fullTopic.length() + 1);
    mqtt.subscribe(charTopic, 1);
}

void mqttPublish(char* topic, char* payload) {

    // This makes it so that every event that is published
    // is prefixed in the /dev/{DEVICE_ID}/ namespace.

    String fullTopic = MQTT_DEVICE_PREFIX + String(topic);
    char charTopic[fullTopic.length() + 1];
    fullTopic.toCharArray(charTopic, fullTopic.length() + 1);
    mqtt.publish(charTopic, payload);

}


void mqttCallback(char* topic, byte* payload, unsigned int length) {

    if (length == 0) { return; }

    char msg[length];
    int topicCount = 0;
    char* topicChunk;

    String payloadStr = String((const char*) payload);
    payloadStr.toCharArray(msg, length + 1);
    topicChunk = strpbrk (topic, SLASH);

    while (topicChunk != NULL) {
        topicChunk = strpbrk (topicChunk+1, SLASH);
        topicCount++;
    }

    char* topicParts[topicCount];

    if (topicCount > 0) {
        topicParts[0] = strtok(topic, SLASH);
        for(int i = 1; i <= topicCount; i++) {
            topicParts[i] = strtok (NULL, SLASH);
            mqttLog("callback/topic", topicParts[i]);
        }
    }

    mqttLog("callback/message", msg);

    mqttDefaultMessageHandler(topicParts, msg);
    mqttCustomMessageHandler(topicParts, msg);

}

void mqttDefaultMessageHandler(char** topic, char* msg) {

    // Default setup actions that apply to all devices,
    // made up mostly of setup and configuration overrides.

    if ( equals(topic[2], "setup") ) {

        // DEVICE SETUP

        mqttLog("setup");

        if ( equals(topic[3], "debug") ) {
            mqttLog("setup/debug");
            if (equals(msg, "true")) {
                mqttLog("setup/debug/true");
                DEBUG_LOG = true;
            } else if (equals(msg, "false")) {
                mqttLog("setup/debug/false");
                DEBUG_LOG = false;
            }
        }

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