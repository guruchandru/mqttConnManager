#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <CUnit/Basic.h>
#include "../src/mqttcm_generic.h"

// Test function for Get_Mqtt_LocationId
void test_Get_Mqtt_LocationId(void)
{
    char *pString = NULL;
    pString = ( char * )malloc(50);
    int result = Get_Mqtt_LocationId(pString);

    CU_ASSERT_STRING_EQUAL(pString, "12345678927e9a892c670333");
    CU_ASSERT_EQUAL(result, 0);
    free(pString);
}

// Test function for Get_Mqtt_ClientId
void test_Get_Mqtt_ClientId(void)
{
    char *pString = Get_Mqtt_ClientId();
    CU_ASSERT_STRING_EQUAL(pString, "123456789012");
}

// Test function for Get_Mqtt_Broker
void test_Get_Mqtt_Broker(void)
{
    char *pString = NULL;
    pString = ( char * )malloc(50);
    int result = Get_Mqtt_Broker(pString);

    CU_ASSERT_STRING_EQUAL(pString, "localhost");
    CU_ASSERT_EQUAL(result, 0);
    free(pString);    
}

// Test function for Get_Mqtt_Port
void test_Get_Mqtt_Port(void)
{
    char *pString = NULL;
    pString = ( char * )malloc(50);
    int result = Get_Mqtt_Port(pString);

    CU_ASSERT_STRING_EQUAL(pString, "123");
    CU_ASSERT_EQUAL(result, 0);
    free(pString);    
}

// Test function for rbus_GetValueFromDB
void test_rbus_GetValueFromDB(void)
{
    char *pString1 = NULL; 
    char **pString2 = NULL; 
    
    int result = rbus_GetValueFromDB(pString1,pString2);

    CU_ASSERT_EQUAL(result, 0);
}

// Test function for rbus_StoreValueIntoDB
void test_rbus_StoreValueIntoDB(void)
{
    char *pString1 = NULL; 
    char *pString2 = NULL; 

    int result = rbus_StoreValueIntoDB(pString1,pString2);

    CU_ASSERT_EQUAL(result, 0);
    free(pString1);    
    free(pString2);     
}

void add_suites( CU_pSuite *suite )
{
    *suite = CU_add_suite( "tests", NULL, NULL );
    CU_add_test( *suite, "test Get_Mqtt_LocationId", test_Get_Mqtt_LocationId);
    CU_add_test( *suite, "test Get_Mqtt_ClientId", test_Get_Mqtt_ClientId);
    CU_add_test( *suite, "test Get_Mqtt_Broker", test_Get_Mqtt_Broker);
    CU_add_test( *suite, "test Get_Mqtt_Port", test_Get_Mqtt_Port);  
    CU_add_test( *suite, "test rbus_GetValueFromDB", test_rbus_GetValueFromDB);
    CU_add_test( *suite, "test rbus_StoreValueIntoDB", test_rbus_StoreValueIntoDB);    
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
