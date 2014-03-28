#include "_button.h"
#include "_utils.h"
#include "_timer.h"
#include "_led.h"
#include "_mqtt.h"

char MQTT_HOST[10] = "lilliput1";

int DEFAULT_RADIO_STATION = 879;
int DEFAULT_RADIO_VOLUME = 12;
int DEFAULT_LED_INTENSITY = 2;

int  MAX_LED_INTENSITY = 10;

bool AMP_POWER_ON_BOOT = true;
int  AMP_VOLUME_DEFAULT = 20;
int  AMP_VOLUME_HIGH = 45;
int  AMP_VOLUME_MED = 35;
int  AMP_VOLUME_LOW = 10;


#include "_mymqtt.h" // Leave this down here.