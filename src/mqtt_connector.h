/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <mosquitto.h>
#include <mqtt_protocol.h>
#include <rbus/rbus.h>
#include <rbus/rbus_object.h>
#include <rbus/rbus_property.h>
#include <rbus/rbus_value.h>
#include <cimplog.h>

#ifdef BUILD_YOCTO
#define DEVICE_PROPS_FILE       "/etc/device.properties"
#else
#define DEVICE_PROPS_FILE       "/tmp/device.properties"
#endif

#define pComponentName "mqttCM"

#define MQTT_CONFIG_FILE     "/tmp/.mqttconfig"
#define MOSQ_TLS_VERSION     "tlsv1.2"
#define OPENSYNC_CERT        "/etc/mqttcm/mqtt_cert_init.sh"
#define KEEPALIVE            60
#define MQTT_PORT            443
#define MAX_MQTT_LEN         128
#define SINGLE_CONN_ELEMENTS 12
#define MAX_BUF_SIZE 255
#define maxParamLen 128

#define MQTT_SUBSCRIBE_TOPIC_PREFIX "x/to/"
#define MQTT_PUBLISH_GET_TOPIC_PREFIX "x/fr/get/chi/"
#define MQTT_PUBLISH_NOTIFY_TOPIC_PREFIX "x/fr/poke/chi/"

#define MQTT_LOCATIONID_PARAM  "Device.X_RDK_MQTT.LocationID"
#define MQTT_BROKER_PARAM      "Device.X_RDK_MQTT.BrokerURL"
#define MQTT_CLIENTID_PARAM    "Device.X_RDK_MQTT.ClientID"
#define MQTT_PORT_PARAM        "Device.X_RDK_MQTT.Port"

#define MQTT_CONNECTMODE_PARAM    "Device.X_RDK_MQTT.ConnectionMode"
#define MQTT_CONNECT_PARAM        "Device.X_RDK_MQTT.Connect"
#define MQTT_SUBSCRIBE_PARAM      "Device.X_RDK_MQTT.Subscribe"
#define MQTT_PUBLISHGET_PARAM     "Device.X_RDK_MQTT.Webconfig.PublishGET"
#define MQTT_PUBLISHNOTIF_PARAM   "Device.X_RDK_MQTT.WebConfig.PublishNotification"

#define WEBCFG_MQTT_ONCONNECT_CALLBACK "Device.X_RDK_MQTT.Webconfig.OnConnectCallback"
#define WEBCFG_MQTT_SUBSCRIBE_CALLBACK "Device.X_RDK_MQTT.Webconfig.OnSubcribeCallback"
#define WEBCFG_MQTT_ONMESSAGE_CALLBACK "Device.X_RDK_MQTT.Webconfig.OnMessageCallback"
#define WEBCFG_MQTT_ONPUBLISH_CALLBACK "Device.X_RDK_MQTT.Webconfig.OnPublishCallback"

#define MAX_MQTT_RETRY 8
#define MQTT_RETRY_ERR -1
#define MQTT_RETRY_SHUTDOWN 1
#define MQTT_DELAY_TAKEN 0
#define UNUSED(x) (void )(x)

typedef struct {
  struct timespec ts;
  int count;
  int max_count;
  int delay;
} mqtt_timer_t;

void on_connect(struct mosquitto *mosq, void *obj, int reason_code, int flag, const mosquitto_property *props);
void on_disconnect(struct mosquitto *mosq, void *obj, int reason_code, const mosquitto_property *props);
void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos, const mosquitto_property *props);
void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg, const mosquitto_property *props);
void on_publish(struct mosquitto *mosq, void *obj, int mid, int reason_code, const mosquitto_property *props);

int writeToDBFile(char *db_file_path, char *data, size_t size);
bool webcfg_mqtt_init();
void get_from_file(char *key, char **val, char *filepath);
void publish_notify_mqtt(char *pub_topic, void *payload, ssize_t len);
char * createMqttPubHeader(char * payload, char * dest, ssize_t * payload_len);
int get_global_mqtt_connected();
void reset_global_mqttConnected();
void set_global_mqttConnected();
int createMqttHeader(char **header_list);
int triggerBootupSync();
void checkMqttParamSet();
pthread_mutex_t *get_global_mqtt_retry_mut(void);
pthread_cond_t *get_global_mqtt_retry_cond(void);
int validateForMqttInit();
pthread_cond_t *get_global_mqtt_cond(void);
pthread_mutex_t *get_global_mqtt_mut(void);
int regMqttDataModel();
void execute_mqtt_script(char *name);
int getHostIPFromInterface(char *interface, char **ip);
void mqtt_subscribe();
int mqttCMRbusInit();
void mqttCMRbus_Uninit();
bool cm_mqtt_init();
