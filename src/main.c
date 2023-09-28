/*
 * Copyright 2023 Comcast Cable Communications Management, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "stdlib.h"
#include "signal.h"
#ifdef INCLUDE_BREAKPAD
#include "breakpad_wrapper.h"
#endif
#include "mqttcm_log.h"
#include "mqttcm_connect.h"
#include "mqttcm_privilege.h"

/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
#ifndef INCLUDE_BREAKPAD
static void sig_handler(int sig);
#endif

pthread_mutex_t mqttcm_mut= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  mqttcm_con= PTHREAD_COND_INITIALIZER;
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

int main()
{
	int ret = 0;
#ifdef INCLUDE_BREAKPAD
	breakpad_ExceptionHandler();
#else
	signal(SIGTERM, sig_handler);
	signal(SIGINT, sig_handler);
	signal(SIGUSR1, sig_handler);
	signal(SIGUSR2, sig_handler);
	signal(SIGSEGV, sig_handler);
	signal(SIGBUS, sig_handler);
	signal(SIGKILL, sig_handler);
	signal(SIGFPE, sig_handler);
	signal(SIGILL, sig_handler);
	signal(SIGQUIT, sig_handler);
	signal(SIGHUP, sig_handler);
	signal(SIGALRM, sig_handler);
#endif
	MqttCMInfo("********** Starting component: %s **********\n", MQTT_COMPONENT_NAME);
	mqttcm_drop_root_privilege();
	if(isRbusEnabled())
	{
		registerRbusLogger();
		MqttCMInfo("RBUS mode. mqttCMRbusInit\n");
		ret = mqttCMRbusInit(MQTT_COMPONENT_NAME);
		if(ret)
		{
			MqttCMInfo("Registering mqtt CM parameters\n");
			regMqttDataModel();
			subscribeToCurrentActiveInterfaceEvent();
			MqttCMInfo("Proceed to mqtt connection with default configuration\n");
			do
			{
				mqttCMConnectBroker();
				MqttCMInfo("Reconnectflag value is %d\n", isReconnectNeeded());

			}while(isReconnectNeeded());

			MqttCMInfo("mqttCMConnectBroker done\n");
		}
		else
		{
			MqttCMError("mqttCMRbusInit failed\n");
		}
	}
	else
	{
		MqttCMInfo("DBUS mode. MqttCM is not supported in Dbus\n");
	}

	MqttCMInfo("pthread_mutex_lock mqttcm_mut and wait.\n");
	pthread_mutex_lock(&mqttcm_mut);
	pthread_cond_wait(&mqttcm_con, &mqttcm_mut);
	MqttCMInfo("pthread_mutex_unlock mqttcm_mut\n");
	pthread_mutex_unlock (&mqttcm_mut);
	MqttCMInfo("Exiting mqttcm main thread!!\n");
	return 1;
}

const char *rdk_logger_module_fetch(void)
{
    return "LOG.RDK.MQTTCM";
}

/*----------------------------------------------------------------------------*/
/*                             Internal functions                             */
/*----------------------------------------------------------------------------*/
#ifndef INCLUDE_BREAKPAD
static void sig_handler(int sig)
{

	if ( sig == SIGINT ) 
	{
		signal(SIGINT, sig_handler); /* reset it to this function */
		MqttCMError("SIGINT received!\n");
		exit(0);
	}
	else if ( sig == SIGUSR1 ) 
	{
		signal(SIGUSR1, sig_handler); /* reset it to this function */
		MqttCMError("SIGUSR1 received!\n");
	}
	else if ( sig == SIGUSR2 ) 
	{
		MqttCMError("SIGUSR2 received!\n");
	}
	else if ( sig == SIGCHLD ) 
	{
		signal(SIGCHLD, sig_handler); /* reset it to this function */
		MqttCMError("SIGHLD received!\n");
	}
	else if ( sig == SIGPIPE ) 
	{
		signal(SIGPIPE, sig_handler); /* reset it to this function */
		MqttCMError("SIGPIPE received!\n");
	}
	else if ( sig == SIGALRM ) 
	{
		signal(SIGALRM, sig_handler); /* reset it to this function */
		MqttCMError("SIGALRM received!\n");
	}
	else 
	{
		MqttCMError("Signal %d received!\n", sig);
		exit(0);
	}
	
}
#endif
