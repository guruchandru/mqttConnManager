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



/*----------------------------------------------------------------------------*/
/*                             Test Functions                             */
/*----------------------------------------------------------------------------*/


// Test function for MqttLocationIdGet_SetHandler
void test_MqttLocationIdGet_SetHandler(void)
{
    mqttCMRbusInit("componentName");
    regMqttDataModel();

    rbusValue_t value;
    rbusValue_Init(&value);
    rbusValue_SetString(value, "Test");

    rbusSetOptions_t opts;
    opts.commit = true;

    rbusError_t rc = rbus_set(get_global_rbus_handle(), MQTT_LOCATIONID_PARAM, value, &opts);
    CU_ASSERT_EQUAL(rc, RBUS_ERROR_SUCCESS);

    rbusError_t rc_get = rbus_get(get_global_rbus_handle(), MQTT_LOCATIONID_PARAM, &value);
    CU_ASSERT_EQUAL(rc_get, RBUS_ERROR_SUCCESS);
    const char* retrievedValue = NULL;
    retrievedValue = rbusValue_GetString(value, NULL);
    CU_ASSERT_STRING_EQUAL(retrievedValue, "Test");
    rbusValue_Release(value);
    mqttCMRbus_Uninit();
}

// Test function for MqttBrokerGet_SetHandler
void test_MqttBrokerGet_SetHandler(void)
{
    mqttCMRbusInit("componentName");
    regMqttDataModel();

    rbusValue_t value;
    rbusValue_Init(&value);
    rbusValue_SetString(value, "Localhost1");

    rbusSetOptions_t opts;
    opts.commit = true;

    rbusError_t rc = rbus_set(get_global_rbus_handle(), MQTT_BROKER_PARAM, value, &opts);
    CU_ASSERT_EQUAL(rc, RBUS_ERROR_SUCCESS);

    rbusError_t rc_get = rbus_get(get_global_rbus_handle(), MQTT_BROKER_PARAM, &value);
    CU_ASSERT_EQUAL(rc_get, RBUS_ERROR_SUCCESS);
    const char* retrievedValue = NULL;
    retrievedValue = rbusValue_GetString(value, NULL);
    CU_ASSERT_STRING_EQUAL(retrievedValue, "Localhost1");
    rbusValue_Release(value);
    mqttCMRbus_Uninit();
}

// Test function for MqttPortGet_SetHandler
void test_MqttPortGet_SetHandler(void)
{
    mqttCMRbusInit("componentName");
    regMqttDataModel();

    rbusValue_t value;
    rbusValue_Init(&value);
    rbusValue_SetString(value, "630");

    rbusSetOptions_t opts;
    opts.commit = true;

    rbusError_t rc = rbus_set(get_global_rbus_handle(), MQTT_PORT_PARAM, value, &opts);
    CU_ASSERT_EQUAL(rc, RBUS_ERROR_SUCCESS);

    rbusError_t rc_get = rbus_get(get_global_rbus_handle(), MQTT_PORT_PARAM, &value);
    CU_ASSERT_EQUAL(rc_get, RBUS_ERROR_SUCCESS);
    const char* retrievedValue = NULL;
    retrievedValue = rbusValue_GetString(value, NULL);
    CU_ASSERT_STRING_EQUAL(retrievedValue, "630");
    rbusValue_Release(value);
    mqttCMRbus_Uninit();
}

// Test function for MqttConnModeGet_SetHandler
void test_MqttConnModeGet_SetHandler(void)
{
    mqttCMRbusInit("componentName");
    regMqttDataModel();

    rbusValue_t value;
    rbusValue_Init(&value);
    rbusValue_SetString(value, "Single");

    rbusSetOptions_t opts;
    opts.commit = true;

    rbusError_t rc = rbus_set(get_global_rbus_handle(), MQTT_CONNECTMODE_PARAM, value, &opts);
    CU_ASSERT_EQUAL(rc, RBUS_ERROR_SUCCESS);

    rbusError_t rc_get = rbus_get(get_global_rbus_handle(), MQTT_CONNECTMODE_PARAM, &value);
    CU_ASSERT_EQUAL(rc_get, RBUS_ERROR_SUCCESS);
    const char* retrievedValue = NULL;
    retrievedValue = rbusValue_GetString(value, NULL);
    CU_ASSERT_STRING_EQUAL(retrievedValue, "Single");
    rbusValue_Release(value);
    mqttCMRbus_Uninit();
}

// Test function for MqttSubscribe_MethodHandler
void test_MqttSubscribe_MethodHandler(void)
{
    mqttCMRbusInit("componentName");
    regMqttDataModel();

    rbusObject_t inParams;
    rbusObject_Init(&inParams, NULL);

    rbusValue_t compnameValue;
    rbusValue_Init(&compnameValue);
    rbusValue_SetString(compnameValue, "ComponentNameValue");
    rbusObject_SetValue(inParams, "compname", compnameValue);

    rbusValue_t topicValue;
    rbusValue_Init(&topicValue);
    rbusValue_SetString(topicValue, "TopicValue");
    rbusObject_SetValue(inParams, "topic", topicValue);

    rbusObject_t outParams;
    rbusObject_Init(&outParams, NULL);

    rbusError_t rc = rbusMethod_Invoke(get_global_rbus_handle(), MQTT_SUBSCRIBE_PARAM, inParams, &outParams);
    CU_ASSERT_EQUAL(rc, RBUS_ERROR_SUCCESS);

    rbusObject_Release(inParams);
    rbusObject_Release(outParams);
    rbusValue_Release(compnameValue);
    mqttCMRbus_Uninit();
}

// Test function for MqttPublish_MethodHandler
void test_MqttPublish_MethodHandler(void)
{
    mqttCMRbusInit("componentName");
    regMqttDataModel();

    rbusObject_t inParams;
    rbusObject_Init(&inParams, NULL);

    rbusValue_t compnameValue;
    rbusValue_Init(&compnameValue);
    rbusValue_SetString(compnameValue, "PayloadValue");
    rbusObject_SetValue(inParams, "payload", compnameValue);

    rbusValue_t topicValue;
    rbusValue_Init(&topicValue);
    rbusValue_SetString(topicValue, "TopicValue");
    rbusObject_SetValue(inParams, "topic", topicValue);

    rbusValue_t qosValue;
    rbusValue_Init(&qosValue);
    rbusValue_SetString(topicValue, "QosValue");
    rbusObject_SetValue(inParams, "qos", qosValue);

    rbusObject_t outParams;
    rbusObject_Init(&outParams, NULL);

    rbusError_t rc = rbusMethod_Invoke(get_global_rbus_handle(), MQTT_PUBLISH_PARAM, inParams, &outParams);
    CU_ASSERT_EQUAL(rc, RBUS_ERROR_SUCCESS);

    rbusObject_Release(inParams);
    rbusObject_Release(outParams);
    rbusValue_Release(compnameValue);
    mqttCMRbus_Uninit();
}

// Test function MqttConnStatus
void test_MqttConnStatusGet_Handler(void)
{
    mqttCMRbusInit("componentName");
    regMqttDataModel();

    rbusValue_t value;
    rbusValue_Init(&value);

    rbusError_t rc_get = rbus_get(get_global_rbus_handle(), MQTT_CONNSTATUS_PARAM, &value);
    CU_ASSERT_EQUAL(rc_get, RBUS_ERROR_SUCCESS);

    rbusValue_Release(value);
    mqttCMRbus_Uninit();
}

// Test function for regMqttDataModel
void test_regMqttDataModel_success()
{
    mqttCMRbusInit("componentName");
    int result = regMqttDataModel();
    CU_ASSERT_EQUAL(result, 0);
}

void test_regMqttDataModel_failure()
{
    int result = regMqttDataModel();
    CU_ASSERT_NOT_EQUAL(result, 0);
    mqttCMRbus_Uninit();
}

void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "tests", NULL, NULL );
    CU_add_test( *suite, "test test_MqttLocationIdGet_SetHandler", test_MqttLocationIdGet_SetHandler);
    CU_add_test( *suite, "test MqttBrokerGet_SetHandler", test_MqttBrokerGet_SetHandler);
    CU_add_test( *suite, "test MqttPortGet_SetHandler", test_MqttPortGet_SetHandler);
    CU_add_test( *suite, "test MqttConnModeGet_SetHandler", test_MqttConnModeGet_SetHandler);
    CU_add_test( *suite, "test MqttSubscribe_MethodHandler", test_MqttSubscribe_MethodHandler);
    CU_add_test( *suite, "test MqttPublish_MethodHandler", test_MqttPublish_MethodHandler);
    CU_add_test( *suite, "test MqttConnStatusGet_Handler", test_MqttConnStatusGet_Handler);
    CU_add_test( *suite, "test regMqttDataModel", test_regMqttDataModel_success);
    CU_add_test( *suite, "test regMqttDataModel", test_regMqttDataModel_failure);
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
