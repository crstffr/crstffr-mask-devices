#ifndef MqttMessage_h
#define MqttMessage_h

class MqttMessage
{
    public:
        MqttMessage(char* fullTopic, char* shortTopic, char** parts, int count, char* val);
        char** topicParts();
        char* fullTopic();
        char* topic();
        int intVal();
        bool boolVal();
        char* charVal();
        bool topicIs(char* what);
        bool isStatusRequest();
        bool is(char* what);
        bool isFor(char* who);
        bool isSetup(char* which);
        bool isCommand(char* which);

    private:
        char* _topic;
        char* _short;
        char** _parts;
        char* _val;
        int _count;
};

MqttMessage::MqttMessage(char* topic, char* shortTopic, char** parts, int count, char* val) {

    _short = shortTopic;
    _topic = topic;
    _count = count;
    _parts = parts;
    _val = val;

}

bool MqttMessage::topicIs(char* what) {
    return equals(_short, what);
}

char** MqttMessage::topicParts() {
    return _parts;
}

char* MqttMessage::fullTopic() {
    return _topic;
}

char* MqttMessage::topic() {
    return _short;
}

char* MqttMessage::charVal() {
    return _val;
}

int MqttMessage::intVal() {
    return atoi(_val);
}

bool MqttMessage::boolVal() {
    return equals(_val, "true");
}

bool MqttMessage::isStatusRequest() {
    return equals(_parts[1], "command") && equals(_parts[2], "status");
}

bool MqttMessage::is(char* what) {
    return equals(what, _val);
}

bool MqttMessage::isFor(char* who) {
    char* part = (isStatusRequest()) ? _parts[3] : _parts[2];
    return equals(part, who) || equals(part, "all");
}

bool MqttMessage::isSetup(char* which) {
    return equals(_parts[1], "setup") && equals(_parts[3], which);
}

bool MqttMessage::isCommand(char* which) {
    return equals(_parts[1], "command") && equals(_parts[3], which);
}


#endif
