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

#include <rbus.h>
#include "mqttcm_webcfg.h"
#include "mqttcm_log.h"
#include "mqttcm_connect.h"

int webcfg_subscribe = 0;
int webcfg_onmessage = 0;
int webcfg_onpublish = 0;

void sendRusEventWebcfgOnSubscribe()
{
	if(webcfg_subscribe)
	{
		rbusEvent_t event = {0};
		rbusObject_t data;
		rbusValue_t value;

		MqttCMInfo("publishing Event\n");

		rbusValue_Init(&value);
		rbusValue_SetString(value, "success");

		rbusObject_Init(&data, NULL);
		rbusObject_SetValue(data, "value", value);

		event.name = WEBCFG_MQTT_SUBSCRIBE_CALLBACK;
		event.data = data;
		event.type = RBUS_EVENT_GENERAL;

		rbusError_t rc = rbusEvent_Publish(get_global_rbus_handle(), &event);

		rbusValue_Release(value);
		rbusObject_Release(data);

		if(rc != RBUS_ERROR_SUCCESS)
			MqttCMError("provider: rbusEvent_Publish onsubscribe event failed: %d\n", rc);
	}
}

void sendRusEventWebcfgOnMessage(char *mqttdata, int dataSize)
{
	if(webcfg_onmessage)
	{
		rbusEvent_t event = {0};
		rbusObject_t dataIn;
		rbusValue_t value;

		MqttCMInfo("publishing onmessafe event1\n");

		rbusValue_Init(&value);
		rbusValue_SetBytes(value, (uint8_t*)mqttdata, dataSize);

		rbusObject_Init(&dataIn, NULL);
		rbusObject_SetValue(dataIn, "value", value);

		event.name = WEBCFG_MQTT_ONMESSAGE_CALLBACK;
		event.data = dataIn;
		event.type = RBUS_EVENT_GENERAL;

		MqttCMInfo("Before rbusEvent_Publish\n");
		rbusError_t rc = rbusEvent_Publish(get_global_rbus_handle(), &event);
		MqttCMInfo("After rbusEvent_Publish\n");

		rbusValue_Release(value);
		rbusObject_Release(dataIn);

		if(rc != RBUS_ERROR_SUCCESS)
			MqttCMError("provider: rbusEvent_Publish onmessage event failed: %d\n", rc);
		else
			MqttCMInfo("publishing onmessafe event success with rc %d\n", rc);
	}
}

void sendRusEventWebcfgOnPublish(int mid)
{
	if(webcfg_onpublish)
	{
		char msg[256] = { 0 };
		rbusEvent_t event = {0};
		rbusObject_t data;
		rbusValue_t value;

		snprintf(msg, MAX_MQTT_LEN, "Message with mid %d has been published.", mid);

		MqttCMInfo("publishing Event\n");

		rbusValue_Init(&value);
		rbusValue_SetString(value, msg);

		rbusObject_Init(&data, NULL);
		rbusObject_SetValue(data, "value", value);

		event.name = WEBCFG_MQTT_ONPUBLISH_CALLBACK;
		event.data = data;
		event.type = RBUS_EVENT_GENERAL;

		rbusError_t rc = rbusEvent_Publish(get_global_rbus_handle(), &event);

		rbusValue_Release(value);
		rbusObject_Release(data);

		if(rc != RBUS_ERROR_SUCCESS)
			MqttCMError("provider: rbusEvent_Publish onpublish event failed: %d\n", rc);
	}
}
	
rbusError_t webcfgMqttSubscribeHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char* eventName, rbusFilter_t filter, int32_t interval, bool* autoPublish)
{
    (void)handle;
    (void)filter;
    (void)autoPublish;
    (void)interval;

    MqttCMInfo(
        "webcfgMqttSubscribeHandler called:\n" \
        "\taction=%s\n" \
        "\teventName=%s\n",
        action == RBUS_EVENT_ACTION_SUBSCRIBE ? "subscribe" : "unsubscribe",
        eventName);

    if(!strcmp(WEBCFG_MQTT_SUBSCRIBE_CALLBACK, eventName))
    {
        webcfg_subscribe = action == RBUS_EVENT_ACTION_SUBSCRIBE ? 1 : 0;
    }
    else
    {
        MqttCMError("provider: webcfgMqttSubscribeHandler unexpected eventName %s\n", eventName);
    }

    return RBUS_ERROR_SUCCESS;
}

rbusError_t webcfgMqttOnMessageHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char* eventName, rbusFilter_t filter, int32_t interval, bool* autoPublish)
{
    (void)handle;
    (void)filter;
    (void)autoPublish;
    (void)interval;

    MqttCMInfo(
        "webcfgMqttOnMessageHandler called:\n" \
        "\taction=%s\n" \
        "\teventName=%s\n",
        action == RBUS_EVENT_ACTION_SUBSCRIBE ? "subscribe" : "unsubscribe",
        eventName);

    if(!strcmp(WEBCFG_MQTT_ONMESSAGE_CALLBACK, eventName))
    {
        webcfg_onmessage = action == RBUS_EVENT_ACTION_SUBSCRIBE ? 1 : 0;
    }
    else
    {
        MqttCMError("provider: webcfgMqttOnMessageHandler unexpected eventName %s\n", eventName);
    }

    return RBUS_ERROR_SUCCESS;
}

rbusError_t webcfgMqttOnPublishHandler(rbusHandle_t handle, rbusEventSubAction_t action, const char* eventName, rbusFilter_t filter, int32_t interval, bool* autoPublish)
{
    (void)handle;
    (void)filter;
    (void)autoPublish;
    (void)interval;

    MqttCMInfo(
        "webcfgMqttOnPublishHandler called:\n" \
        "\taction=%s\n" \
        "\teventName=%s\n",
        action == RBUS_EVENT_ACTION_SUBSCRIBE ? "subscribe" : "unsubscribe",
        eventName);

    if(!strcmp(WEBCFG_MQTT_ONPUBLISH_CALLBACK, eventName))
    {
        webcfg_onpublish = action == RBUS_EVENT_ACTION_SUBSCRIBE ? 1 : 0;
    }
    else
    {
        MqttCMError("provider: webcfgMqttOnPublishHandler unexpected eventName %s\n", eventName);
    }

    return RBUS_ERROR_SUCCESS;
}

int rbusRegWebcfgDataElements()
{
	rbusError_t ret = RBUS_ERROR_SUCCESS;
	rbusDataElement_t webcfgDataElements[WEBCFG_ELEMENTS] = {
		{WEBCFG_MQTT_SUBSCRIBE_CALLBACK, RBUS_ELEMENT_TYPE_EVENT, {NULL, NULL, NULL, NULL, webcfgMqttSubscribeHandler, NULL}},
		{WEBCFG_MQTT_ONMESSAGE_CALLBACK, RBUS_ELEMENT_TYPE_EVENT, {NULL, NULL, NULL, NULL, webcfgMqttOnMessageHandler, NULL}},
		{WEBCFG_MQTT_ONPUBLISH_CALLBACK, RBUS_ELEMENT_TYPE_EVENT, {NULL, NULL, NULL, NULL, webcfgMqttOnPublishHandler, NULL}},
	};

	ret = rbus_regDataElements(get_global_rbus_handle(), WEBCFG_ELEMENTS, webcfgDataElements);
	if(ret == RBUS_ERROR_SUCCESS)
	{
		MqttCMInfo("Registered webcfg rbus callback events\n");
	}
	else
	{
		MqttCMError("Failed to register webcfg rbus callback events %d\n", ret);
	}
	return ret;
}
