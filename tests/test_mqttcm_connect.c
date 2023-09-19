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

extern pthread_mutex_t mqtt_retry_mut;
extern pthread_cond_t mqtt_retry_con;
extern pthread_mutex_t mqtt_mut;
extern pthread_cond_t mqtt_con;
extern rbusHandle_t rbus_handle;

/*----------------------------------------------------------------------------*/
/*                             Test Functions                             */
/*----------------------------------------------------------------------------*/

// Create file
FILE* createFile(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
	    CU_FAIL("Failed to create file");
	    return NULL;
    }
    return file;
}

//Test case for validateForMqttInit
void test_validateForMqttInit_failure() 
{
    // Check failure case where the locationId,clientId,broker is NULL and function returns 1
    int result = validateForMqttInit();
    CU_ASSERT_EQUAL(result, 1);
}

void test_validateForMqttInit_success() 
{
    mqttCMRbusInit("componentName");
    regMqttDataModel();
    // Check Success case where the locationId,clientId,broker is set and function returns 0
    int res = validateForMqttInit();
    CU_ASSERT_EQUAL(res, 0);
    mqttCMRbus_Uninit();
}

// Test case for writeToDBFile
void test_writeToDBFile_success() 
{
    char* db_file_path = "test.txt";
    char* data = "HelloWorld";
    size_t size = strlen(data);
    
    FILE* file = createFile(db_file_path);
    fclose(file);
    // Successful file write
    CU_ASSERT_EQUAL(writeToDBFile(db_file_path, data, size), 1);   
    if (remove(db_file_path) != 0) {
    	    CU_FAIL("Failed to remove file");
    }
}

void test_writeToDBFile_failure() 
{
    char* db_file_path = "test.txt";
    char* data = NULL;
    size_t size = 0;
    // Failed file open
    CU_ASSERT_EQUAL(writeToDBFile("nonexistent.txt", data, size), 0);
    // NULL data pointer
    CU_ASSERT_EQUAL(writeToDBFile(db_file_path, data, size), 0);
}

// Test case for get_from_file
void test_get_from_file_success() 
{
    char* key = "mykey";
    char* val = NULL;
    char* filepath = "key.txt";    
    FILE* file = createFile(filepath);
    fputs("mykeymyvalue\n", file);
    fclose(file);
    // Key exists in the file
    get_from_file(key, &val, filepath);
    CU_ASSERT_PTR_NOT_NULL(val);
    if (val != NULL) {
        CU_ASSERT_STRING_EQUAL(val, "myvalue");
        free(val);
        val = NULL;
    }    
    if (remove(filepath) != 0) {
        CU_FAIL("Failed to remove file");
    }
}

void test_get_from_file_failure() 
{
    char* key = "mykey";
    char* val = NULL;
    char* filepath = "key.txt";
    // Key does not exist in the file
    get_from_file("nonexistent", &val, filepath);
    CU_ASSERT_PTR_NULL(val);
    // File does not exist
    get_from_file(key, &val, "nonexistent.txt");
    CU_ASSERT_PTR_NULL(val);
}

// Test case for checkMqttParamSet
void signalConditionVariable() 
{
    pthread_mutex_lock(&mqtt_mut);
    pthread_cond_signal(&mqtt_con);
    pthread_mutex_unlock(&mqtt_mut);
}

void test_checkMqttParamSet_failure() 
{
    pthread_t thread;
    pthread_create(&thread, NULL, (void*)signalConditionVariable, NULL);
    int result = checkMqttParamSet();
    // Wait for the signaling thread to finish
    pthread_join(thread, NULL);
    // Received mqtt signal proceed to mqtt init
    CU_ASSERT_EQUAL(result, 0);
    mqttCMRbus_Uninit();
}

void test_checkMqttParamSet_success()
{
    //Validation success for mqtt parameters, proceed to mqtt init	
    int result = checkMqttParamSet();
    CU_ASSERT_EQUAL(result, 1);
}

//Test case for convertToUpperCase
void test_convertToUppercase_passString_success()
{
    // Convert passed string to UpperCase
    char deviceId[12] = "device";
    convertToUppercase(deviceId);
    CU_ASSERT_STRING_EQUAL(deviceId, "DEVICE");
    //assign mixedstring to deviceId
    strcpy(deviceId, "deVice@123");
    convertToUppercase(deviceId);
    CU_ASSERT_STRING_EQUAL(deviceId, "DEVICE@123");
}

void test_convertToUppercase_passEmptyString_failure()
{
    // Do not convert if empty string
    char deviceId[] = "";
    convertToUppercase(deviceId);
    CU_ASSERT_STRING_EQUAL(deviceId, "");
}

// Test case for isRbusEnabled
void test_isRbusEnabled_success()
{
    // Checks MQTTCM RBUS mode active status is True
    bool result = isRbusEnabled();
    CU_ASSERT_TRUE(result);
}

// Test case for mqttCMRbusInit
void test_mqttCMRbusInit_success()
{
    // Checks whether mqttCMRbusInit is success
    int result = mqttCMRbusInit("componentName");
    CU_ASSERT_EQUAL(result, 1);  
}

// Test case for fetchMqttParamsFromDB
void test_fetchMqttParamsFromDB_success()
{
    // Check whether Mqtt params are fetched from DB
    fetchMqttParamsFromDB();
    char locationId[256];
    int locationIdResult = Get_Mqtt_LocationId(locationId);
    CU_ASSERT_STRING_EQUAL(locationId, "12345678927e9a892c670333");
    CU_ASSERT_EQUAL(locationIdResult, 0);

    char* clientId = Get_Mqtt_ClientId();
    CU_ASSERT_STRING_EQUAL(clientId, "123456789012");
    char broker[256];
    int brokerResult = Get_Mqtt_Broker(broker);
    CU_ASSERT_STRING_EQUAL(broker, "localhost");
    CU_ASSERT_EQUAL(brokerResult, 0);

    char Port[32];
    int portResult = Get_Mqtt_Port(Port);
    CU_ASSERT_STRING_EQUAL(Port, "123");
    CU_ASSERT_EQUAL(portResult, 0);
}

//Test case for getHostIPFromInterface
void test_getHostIPFromInterface_success()
{
    struct ifaddrs *ifaddr, *ifa;
    char *interface = NULL;
    char *ip = NULL;
    int result;
    if (getifaddrs(&ifaddr) == -1) {
        CU_FAIL("Failed to get network interfaces");
        return;
    }
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET) {
            // Extract the first valid interface
            interface = strdup(ifa->ifa_name);
            break;
        }
    }
    freeifaddrs(ifaddr);
    if (interface == NULL) {
        CU_FAIL("No valid network interface found");
        return;
    }
    // Check if able to get IP from HostInterface
    result = getHostIPFromInterface(interface, &ip);
    CU_ASSERT_EQUAL(result, 1);
    CU_ASSERT_PTR_NOT_NULL(ip);
    free(interface);
}

void test_getHostIPFromInterface_failure()
{
    char *interface = "wlppp";
    char *ip = NULL;
    int result = getHostIPFromInterface(interface, &ip);

    // Unable to get IP from HostInterface
    CU_ASSERT_EQUAL(result, 0);
    CU_ASSERT_PTR_NULL(ip);
}

//Test case for get_interface
void test_get_interface_success() 
{
    const char* devicePropsFile = "/tmp/device.properties";
    const char* content = "WEBCONFIG_INTERFACE=eth0\n";
    FILE* file = createFile(devicePropsFile);
    fputs(content, file);
    fclose(file);
    // Checks that the interface is fetched successfully
    char* interface = NULL;
    get_interface(&interface);
    CU_ASSERT_PTR_NOT_NULL(interface);
    CU_ASSERT_STRING_EQUAL(interface, "eth0");
    free(interface);
    if (remove(devicePropsFile) != 0) {
        CU_FAIL("Failed to remove device.properties file");
    }
}

void test_get_interface_failure() 
{
    // Unable to get interface
    char* interface = NULL;
    get_interface(&interface);
    CU_ASSERT_PTR_NULL(interface);
    free(interface);
}

//Test case for execute_mqtt_script
void test_executeMqttScript_success()
{
    char scriptName[] = "./script.sh";
    FILE* file = createFile(scriptName);
    fprintf(file, "#!/bin/sh\n");
    fprintf(file, "echo");  // Only 'echo' is needed in the script
    fclose(file);
    // Checks whether the script is executed successfully
    int result = execute_mqtt_script(scriptName);
    CU_ASSERT_EQUAL(result, 1);
    remove(scriptName);
}

void test_executeMqttScript_failure_emptystring()
{
    // Unable to execute the script
    char scriptName[] = "";  
    // Empty string to simulate failure
    int result = execute_mqtt_script(scriptName);
    CU_ASSERT_EQUAL(result, 0);	
}

//Test case for init_mqtt_timer
void test_init_mqtt_timer_success() 
{
    // Check if the max_count is set correctly and count is updated to 1
    mqtt_timer_t timer;
    int expected_max_count = 10;
    init_mqtt_timer(&timer, expected_max_count);
    CU_ASSERT_EQUAL(timer.max_count, expected_max_count);
    CU_ASSERT_EQUAL(timer.count, 1);
}

void test_init_mqtt_timer_failure() 
{
    // Check whether invalid max_count is set
    mqtt_timer_t timer;
    int expected_max_count = -1; // Invalid max_count value
    init_mqtt_timer(&timer, expected_max_count);
    CU_ASSERT_EQUAL(timer.max_count, expected_max_count);
    CU_ASSERT_EQUAL(timer.count, 1);
}   

// Test case for isReconnectNeeded
void test_isReconnectNeeded()
{
    // Check reconnect is needed
    int result = isReconnectNeeded();
    CU_ASSERT_EQUAL(result, 0);
}

// Test case for get_global_shutdown
void test_get_global_shutdown()
{
    int result = get_global_shutdown();
    CU_ASSERT_EQUAL(result, 0);
}

//Test case for valueChangeCheck 
void test_valueChangeCheck_success()
{
     char valueStored[] = "12345678b897eg89034";
     char valueChanged[] = "12345t675r6790000h67";
     int result = valueChangeCheck(valueStored, valueChanged);
     CU_ASSERT_EQUAL(result, 1);
}

void test_valueChangeCheck_failure()
{
     char valueStored[] = "12345678b897eg89034";
     char valueChanged[] = "12345678b897eg89034";
     int result = valueChangeCheck(valueStored, valueChanged);
     CU_ASSERT_EQUAL(result, 0);     
}

// Test case for get_global_mqtt_retry_cond
void test_get_global_mqtt_retry_cond()
{
    CU_ASSERT_PTR_EQUAL(&mqtt_retry_con, get_global_mqtt_retry_cond());
}

// Test case for get_global_mqtt_retry_mut
void test_get_global_mqtt_retry_mut()
{
    CU_ASSERT_PTR_EQUAL(&mqtt_retry_mut, get_global_mqtt_retry_mut());
}

//Test case for get_global_mqtt_cond
void test_get_global_mqtt_cond()
{
    CU_ASSERT_PTR_EQUAL(&mqtt_con, get_global_mqtt_cond());
}

// Test case for get_global_mqtt_mut
void test_get_global_mqtt_mut()
{
    CU_ASSERT_PTR_EQUAL(&mqtt_mut, get_global_mqtt_mut());
}

// Test case for registerRbusLogger 
void test_registerRbusLogger()
{
    int result = registerRbusLogger();
    CU_ASSERT_EQUAL(result, 1);
}
 
// Test case for rbus_log_handler
void test_rbus_log_handler()
{
    rbus_log_handler(0, "file1", 1, 0, "message1");
    rbus_log_handler(1, "file2", 2, 0, "message2");
    rbus_log_handler(2, "file3", 3, 0, "message3");
    rbus_log_handler(3, "file4", 4, 0, "message4");
    rbus_log_handler(4, "file5", 5, 0, "message5");
}

// Test case for get_global_rbus_handle
void test_get_global_rbus_handle()
{
    CU_ASSERT_EQUAL(rbus_handle, get_global_rbus_handle());
}

// Test case for mqtt_retry
void test_mqtt_retry()
{
	mqtt_timer_t mqtt_timer;
	pthread_t thread;
	pthread_create(&thread, NULL, (void*)signalConditionVariable, NULL);
	init_mqtt_timer(&mqtt_timer, MAX_MQTT_RETRY);
	int result = mqtt_retry(&mqtt_timer);
	// Wait for the signaling thread to finish
        pthread_join(thread, NULL);
	CU_ASSERT_EQUAL(result, 0);
} 
void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "tests", NULL, NULL );
    CU_add_test( *suite, "test validateForMqttInit_failure", test_validateForMqttInit_failure);
    CU_add_test( *suite, "test checkMqttParamSet_failure", test_checkMqttParamSet_failure);
    CU_add_test( *suite, "test validateForMqttInit_success", test_validateForMqttInit_success);
    CU_add_test( *suite, "test checkMqttParamSet_success", test_checkMqttParamSet_success);
    CU_add_test( *suite, "test writeToDBFile_success", test_writeToDBFile_success);
    CU_add_test( *suite, "test writeToDBFile_failure", test_writeToDBFile_failure);
    CU_add_test( *suite, "test get_from_file_success", test_get_from_file_success);
    CU_add_test( *suite, "test get_from_file_failure", test_get_from_file_failure);
    CU_add_test( *suite, "test convertToUppercase_passString_success", test_convertToUppercase_passString_success);
    CU_add_test( *suite, "test convertToUppercase_passEmptyString_failure", test_convertToUppercase_passEmptyString_failure);
    CU_add_test( *suite, "test isRbusEnabled_success", test_isRbusEnabled_success);
    CU_add_test( *suite, "test mqttCMRbusInit_success", test_mqttCMRbusInit_success);
    CU_add_test( *suite, "test fetchMqttParamsFromDB_success", test_fetchMqttParamsFromDB_success);
    CU_add_test( *suite, "test getHostIPFromInterface_success", test_getHostIPFromInterface_success);
    CU_add_test( *suite, "test getHostIPFromInterface_failure", test_getHostIPFromInterface_failure);
    CU_add_test( *suite, "test get_interface_success", test_get_interface_success);
    CU_add_test( *suite, "test get_interface_failure", test_get_interface_failure);
    CU_add_test( *suite, "test get_executeMqttScript_success", test_executeMqttScript_success);
    CU_add_test( *suite, "test get_executeMqttScript_failure_emptystring", test_executeMqttScript_failure_emptystring);
    CU_add_test( *suite, "test init_mqtt_timer_success", test_init_mqtt_timer_success);
    CU_add_test( *suite, "test init_mqtt_timer_failure", test_init_mqtt_timer_failure);
    CU_add_test( *suite, "test isReconnectNeeded", test_isReconnectNeeded);
    CU_add_test( *suite, "test get_global_shutdown", test_get_global_shutdown);
    CU_add_test( *suite, "test valueChangeCheck", test_valueChangeCheck_success);
    CU_add_test( *suite, "test valueChangeCheck", test_valueChangeCheck_failure);
    CU_add_test( *suite, "test get_global_mqtt_retry_cond", test_get_global_mqtt_retry_cond);
    CU_add_test( *suite, "test get_global_mqtt_retry_mut", test_get_global_mqtt_retry_mut);
    CU_add_test( *suite, "test get_global_mqtt_cond", test_get_global_mqtt_cond); 
    CU_add_test( *suite, "test get_global_mqtt_mut", test_get_global_mqtt_mut);
    CU_add_test( *suite, "test registerRbusLogger", test_registerRbusLogger);
    CU_add_test( *suite, "test rbus_log_handler", test_rbus_log_handler);
    CU_add_test( *suite, "test get_global_rbus_handle", test_get_global_rbus_handle);
    CU_add_test( *suite, "test mqtt_retry", test_mqtt_retry); 
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
