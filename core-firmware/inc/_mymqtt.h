typedef void (*callback)(void);

void noop();
void mqttLoop();
void mqttConnect();
void mqttSubscribe(char* topic);
void mqttPublish(char* topic, char* payload);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void mqttCustomMessageHandler(char* topic, char** topicParts, int topicCount, char* msg);
void mqttDefaultMessageHandler(char* topic, char** topicParts, int topicCount, char* msg);
void mqttLog(char* topic, char* msg);
void mqttLog(char* topic);

TCPClient tcp;
SimpleTimer timer;
int connectTimer;
callback connectCallback;
callback disconnectCallback;

const char SLASH[] = "/";
char DEVICE_ID[25];
bool DEBUG_LOG = false;
bool CONNECTED = false;

String MQTT_DEVICE_PREFIX;
const int DEVICE_PREFIX_LENGTH = 30;
char MQTT_DEVICE_PREFIX_CHAR[DEVICE_PREFIX_LENGTH];
PubSubClient mqtt(MQTT_HOST, 1883, mqttCallback, tcp);

void noop() {}

void mqttSetup(callback onConnect, callback onDisconnect) {
    MQTT_DEVICE_PREFIX = "dev/" + Spark.deviceID() + "/";
    MQTT_DEVICE_PREFIX.toCharArray(MQTT_DEVICE_PREFIX_CHAR, DEVICE_PREFIX_LENGTH);

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


void mqttCallback(char* fullTopic, byte* payload, unsigned int length) {

    if (length == 0) { return; }

    char msg[length];
    int topicCount = 0;
    char* topicChunk;
    char topicShort[strlen(fullTopic)];

    String payloadStr = String((const char*) payload);
    payloadStr.toCharArray(msg, length + 1);
    topicChunk = strpbrk(fullTopic, SLASH);

    while (topicChunk != NULL) {
        topicChunk = strpbrk (topicChunk+1, SLASH);
        if (topicCount == 0) strcpy(topicShort, topicChunk);
        topicCount++;
    }

    // this trims the slash from the front of the topic
    memmove(topicShort, topicShort+1, strlen(topicShort));

    // split all the parts into an array
    char* topicParts[topicCount];

    if (topicCount > 0) {
        strtok(fullTopic, SLASH); // dev
        strtok(NULL, SLASH);  // {deviceID}
        for(int i = 1; i < topicCount; i++) {
            topicParts[i] = strtok (NULL, SLASH);
            mqttLog("callback/topicPart", topicParts[i]);
        }
    }

    mqttLog("callback/topic", topicShort);
    mqttLog("callback/message", msg);

    mqttDefaultMessageHandler(topicShort, topicParts, topicCount - 1, msg);
    mqttCustomMessageHandler(topicShort, topicParts, topicCount - 1, msg);

}

void mqttDefaultMessageHandler(char* topic, char** topicParts, int topicCount, char* msg) {

    //int intmsg = atoi(msg);
    bool boolmsg = equals(msg, "true");

    // Default setup actions that apply to all devices,
    // made up mostly of setup and configuration overrides.

    if (equals(topic, "setup/debug")) {
        DEBUG_LOG = boolmsg;
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