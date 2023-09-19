/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 Comcast Cable Communications Management, LLC
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
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <rbus/rbus.h>
#include <stdlib.h>
#include <ifaddrs.h>
#include <CUnit/Basic.h>
#include <sys/stat.h>
#include "../src/mqttcm_connect.h"
#include "../src/mqttcm_generic.h"
#include "../src/mqttcm_generic.h"
#include "../src/mqttcm_log.h"
#include "../src/mqttcm_privilege.h"
#include "../src/mqttcm_webcfg.h"

#define CONSUMER_COMPONENT_NAME "consumercomp"
/*----------------------------------------------------------------------------*/
/*                             Test Functions                             */
/*----------------------------------------------------------------------------*/
rbusHandle_t handle;

void rbushandleclose(char * name)
{
	rbusEvent_Unsubscribe(handle, name);
	rbus_close(handle);
}

static void subscribeEventSuccessCallbackHandler(
    rbusHandle_t handle,
    rbusEvent_t const* event,
    rbusEventSubscription_t* subscription)
{
    rbusValue_t incoming_value;

    incoming_value = rbusObject_GetValue(event->data, "value");

    if(incoming_value)
    {
        char * inVal = (char *)rbusValue_GetString(incoming_value, NULL);
	MqttCMInfo("inVal is %s\n", inVal);
        if(strncmp(inVal, "success", 7) == 0)
        {
		MqttCMInfo("rbusEvent_OnSubscribe callback received successfully\n");
		CU_ASSERT(1);
	}
    }
    (void)handle;
}

static void messageEventSuccessCallbackHandler(
    rbusHandle_t handle,
    rbusEvent_t const* event,
    rbusEventSubscription_t* subscription)
{
    rbusValue_t incoming_value;

    incoming_value = rbusObject_GetValue(event->data, "value");

     if(incoming_value != NULL)
    {
	int len = 0;
        char * inVal = (char *)rbusValue_GetBytes(incoming_value, &len);
	MqttCMInfo("inVal is %.*s\n", 8, inVal);
	MqttCMInfo("rbusEvent_OnMessage callback received successfully\n");
	CU_ASSERT(1);
    }
    (void)handle;
}

static void publishEventSuccessCallbackHandler(
    rbusHandle_t handle,
    rbusEvent_t const* event,
    rbusEventSubscription_t* subscription)
{
    rbusValue_t incoming_value;

    incoming_value = rbusObject_GetValue(event->data, "value");

    if(incoming_value)
    {
        char * inVal = (char *)rbusValue_GetString(incoming_value, NULL);
	MqttCMInfo("inVal is %s\n", inVal);
	if(strncmp(inVal, "Message with mid 1 has been published.", 38) == 0)
	{
		MqttCMInfo("rbusEvent_OnPublish callback received successfully with message %s\n", inVal);
		CU_ASSERT(1);
	}
    }
    (void)handle;
}

//Function to subscribe to events
void subscribe_to_event(char * eventname)
{
	int rc = RBUS_ERROR_SUCCESS;

	MqttCMInfo("rbus_open for component %s\n", CONSUMER_COMPONENT_NAME);
	rc = rbus_open(&handle, CONSUMER_COMPONENT_NAME);
	if(rc != RBUS_ERROR_SUCCESS)
	{
		CU_FAIL("rbus_open failed for subscribe_to_event");
	}

	if(strncmp(eventname, WEBCFG_MQTT_SUBSCRIBE_CALLBACK, strlen(WEBCFG_MQTT_SUBSCRIBE_CALLBACK)) == 0)
	{
		MqttCMInfo("Inside subscribeEventSuccessCallbackHandler for %s and eventname is %s\n", WEBCFG_MQTT_SUBSCRIBE_CALLBACK, eventname);
		rc = rbusEvent_Subscribe(handle, eventname, subscribeEventSuccessCallbackHandler, NULL, 0);
	}
	else if(strncmp(eventname, WEBCFG_MQTT_ONMESSAGE_CALLBACK, strlen(WEBCFG_MQTT_ONMESSAGE_CALLBACK)) == 0)
	{
		MqttCMInfo("Inside messageEventSuccessCallbackHandler for %s and eventname is %s\n", WEBCFG_MQTT_ONMESSAGE_CALLBACK, eventname);
		rc = rbusEvent_Subscribe(handle, eventname, messageEventSuccessCallbackHandler, NULL, 0);
	}
	else if(strncmp(eventname, WEBCFG_MQTT_ONPUBLISH_CALLBACK, strlen(WEBCFG_MQTT_ONPUBLISH_CALLBACK)) == 0)
	{
		MqttCMInfo("Inside publishEventSuccessCallbackHandler for %s and eventname is %s\n", WEBCFG_MQTT_ONPUBLISH_CALLBACK, eventname);
		rc = rbusEvent_Subscribe(handle, eventname, publishEventSuccessCallbackHandler, NULL, 0);
	}

	if(rc != RBUS_ERROR_SUCCESS)
		CU_FAIL("subscribe_to_event onsubscribe event failed");
}

//Test case for rbusRegWebcfgDataElements success
void test_rbusRegWebcfgDataElements_success()
{
	mqttCMRbusInit("test_mqttcm_webcfg_component");
	int result = rbusRegWebcfgDataElements();
	CU_ASSERT_EQUAL(result, 0);
}

//Test case for sendRbusEventWebcfgOnSubscribe Success
void test_sendRbusEventWebcfgOnSubscribe_success()
{
	subscribe_to_event(WEBCFG_MQTT_SUBSCRIBE_CALLBACK);
	rbusError_t ret = sendRbusEventWebcfgOnSubscribe();
	CU_ASSERT_EQUAL(ret, 0);
	rbushandleclose(WEBCFG_MQTT_SUBSCRIBE_CALLBACK);
}

//Test case for sendRbusEventWebcfgOnMessage Success
void test_sendRbusEventWebcfgOnMessage_success()
{
	subscribe_to_event(WEBCFG_MQTT_ONMESSAGE_CALLBACK);
	rbusError_t ret = sendRbusEventWebcfgOnMessage("mqttdata", 8, "testtopic");
	CU_ASSERT_EQUAL(ret, 0);
	rbushandleclose(WEBCFG_MQTT_ONMESSAGE_CALLBACK);
}

//Test case for sendRbusEventWebcfgOnPublish Success
void test_sendRbusEventWebcfgOnPublish_success()
{
	subscribe_to_event(WEBCFG_MQTT_ONPUBLISH_CALLBACK);
	rbusError_t ret = sendRbusEventWebcfgOnPublish(1);
	CU_ASSERT_EQUAL(ret, 0);
	rbushandleclose(WEBCFG_MQTT_ONPUBLISH_CALLBACK);
}

//Test case for sendRbusEventWebcfgOnSubscribe failure
void test_sendRbusEventWebcfgOnSubscribe_failure()
{
	rbusError_t ret = sendRbusEventWebcfgOnSubscribe();
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

//Test case for sendRbusEventWebcfgOnMessage failure
void test_sendRbusEventWebcfgOnMessage_failure()
{
	rbusError_t ret = sendRbusEventWebcfgOnMessage("mqttdata", 8, "testtopic");
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

//Test case for sendRbusEventWebcfgOnPublish failure
void test_sendRbusEventWebcfgOnPublish_failure()
{
	rbusError_t ret = sendRbusEventWebcfgOnPublish(1);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

//Test case for sendRbusErrorToMqtt Publish failure
void test_sendRbusErrorToMqtt_publish_failure()
{
	int ret = sendRbusErrorToMqtt(56, "webconfig");
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

//Test case for sendRbusErrorToMqtt Topic NULL Check
void test_sendRbusErrorToMqtt_topic_Null()
{
	int ret = sendRbusErrorToMqtt(56, NULL);
	CU_ASSERT_NOT_EQUAL(ret, 0);
}

//Test case for createMqttPubHeader success
void test_createMqttPubHeader_success()
{
	ssize_t payload_len = 0;
	createMqttPubHeader("testpayload", &payload_len);
	CU_ASSERT_NOT_EQUAL(payload_len, 0);
}

//Test case for createMqttPubHeader failure
void test_createMqttPubHeader_failure()
{
	ssize_t payload_len = 0;
	createMqttPubHeader(NULL, &payload_len);
	CU_ASSERT_EQUAL(payload_len, 0);
}

//Test case for createcJsonSchema success
void test_createcJsonSchema_success()
{
	char * ret = createcJsonSchema(7, "testtopic");
	CU_ASSERT_PTR_NOT_NULL(ret);
}

//Test function for webcfgMqttSubscribeHandler
void test_webcfgMqttSubscribeHandler()
{
	rbusError_t ret = RBUS_ERROR_BUS_ERROR;

	ret = webcfgMqttSubscribeHandler(handle, RBUS_EVENT_ACTION_SUBSCRIBE , WEBCFG_MQTT_SUBSCRIBE_CALLBACK, NULL, 0, false);
	CU_ASSERT_EQUAL(ret, 0);

	ret = webcfgMqttSubscribeHandler(handle, RBUS_EVENT_ACTION_SUBSCRIBE , WEBCFG_MQTT_ONMESSAGE_CALLBACK, NULL, 0, false);
	CU_ASSERT_EQUAL(ret, 0);
}

//Test function for webcfgMqttOnMessageHandler
void test_webcfgMqttOnMessageHandler()
{
	rbusError_t ret = RBUS_ERROR_BUS_ERROR;

	ret = webcfgMqttOnMessageHandler(handle, RBUS_EVENT_ACTION_SUBSCRIBE , WEBCFG_MQTT_ONMESSAGE_CALLBACK, NULL, 0, false);
	CU_ASSERT_EQUAL(ret, 0);

	ret = webcfgMqttOnMessageHandler(handle, RBUS_EVENT_ACTION_SUBSCRIBE , WEBCFG_MQTT_SUBSCRIBE_CALLBACK, NULL, 0, false);
	CU_ASSERT_EQUAL(ret, 0);
}

//Test function for webcfgMqttOnPublishHandler
void test_webcfgMqttOnPublishHandler()
{
	rbusError_t ret = RBUS_ERROR_BUS_ERROR;

	ret = webcfgMqttOnPublishHandler(handle, RBUS_EVENT_ACTION_SUBSCRIBE , WEBCFG_MQTT_ONPUBLISH_CALLBACK, NULL, 0, false);
	CU_ASSERT_EQUAL(ret, 0);

	ret = webcfgMqttOnPublishHandler(handle, RBUS_EVENT_ACTION_SUBSCRIBE , WEBCFG_MQTT_ONMESSAGE_CALLBACK, NULL, 0, false);
	CU_ASSERT_EQUAL(ret, 0);
}

void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "tests", NULL, NULL );
    CU_add_test( *suite, "test rbusRegWebcfgDataElements_success", test_rbusRegWebcfgDataElements_success);
    CU_add_test( *suite, "test sendRbusEventWebcfgOnSubscribe_success", test_sendRbusEventWebcfgOnSubscribe_success);
    CU_add_test( *suite, "test sendRbusEventWebcfgOnSubscribe_failure", test_sendRbusEventWebcfgOnSubscribe_failure);
    CU_add_test( *suite, "test sendRbusEventWebcfgOnMessage_success", test_sendRbusEventWebcfgOnMessage_success);
    CU_add_test( *suite, "test sendRbusEventWebcfgOnMessage_failure", test_sendRbusEventWebcfgOnMessage_failure);
    CU_add_test( *suite, "test sendRbusEventWebcfgOnPublish_success", test_sendRbusEventWebcfgOnPublish_success);
    CU_add_test( *suite, "test sendRbusEventWebcfgOnPublish_failure", test_sendRbusEventWebcfgOnPublish_failure);
    CU_add_test( *suite, "test sendRbusErrorToMqtt_publish_failure", test_sendRbusErrorToMqtt_publish_failure);
    CU_add_test( *suite, "test sendRbusErrorToMqtt_topic_Null", test_sendRbusErrorToMqtt_topic_Null);
    CU_add_test( *suite, "test createMqttPubHeader_success", test_createMqttPubHeader_success);
    CU_add_test( *suite, "test createMqttPubHeader_failure", test_createMqttPubHeader_failure);
    CU_add_test( *suite, "test createcJsonSchema_success", test_createcJsonSchema_success);
    CU_add_test( *suite, "test webcfgMqttSubscribeHandler", test_webcfgMqttSubscribeHandler);
    CU_add_test( *suite, "test webcfgMqttOnMessageHandler", test_webcfgMqttOnMessageHandler);
    CU_add_test( *suite, "test webcfgMqttOnPublishHandler", test_webcfgMqttOnPublishHandler);
}

int main( int argc, char *argv[] )
{
    unsigned rv = 1;
    CU_pSuite suite = NULL;

    (void ) argc;
    (void ) argv;

    if( CUE_SUCCESS == CU_initialize_registry() ) {
        add_suites( &suite );
        if( NULL != suite ) {
            CU_basic_set_mode( CU_BRM_VERBOSE );
            CU_basic_run_tests();
            printf( "\n" );
            CU_basic_show_failures( CU_get_failure_list() );
            printf( "\n\n" );
            rv = CU_get_number_of_tests_failed();
        }

        CU_cleanup_registry();

    }
    return rv;
}
