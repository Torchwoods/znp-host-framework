/**************************************************************************************************
 * Filename:       main.c
 * Description:    This file contains the main for the gnu platform.
 *
 *
 * Copyright (C) 2013 Texas Instruments Incorporated - http://www.ti.com/
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "rpc.h"
#include "cmdLine.h"

#include "dbgPrint.h"

//处理串口的ZNP数据
void *rpcTask(void *argument)
{
	while (1)
	{
		rpcProcess();
	}

	dbg_print(PRINT_LEVEL_WARNING, "rpcTask exited!\n");
}

void *appTask(void *argument)
{
	while (1)
	{
		appProcess(NULL);
	}
}

int main(int argc, char* argv[])
{
	char * selectedSerialPort;
	pthread_t rpcThread, appThread;

	dbg_print(PRINT_LEVEL_INFO, "%s -- %s %s\n", argv[0], __DATE__, __TIME__);

	// accept only 1
	if (argc < 2)
	{
		//usage(argv[0]);
		dbg_print(PRINT_LEVEL_INFO, "attempting to use /dev/ttyACM0\n\n");
		selectedSerialPort = "/dev/ttyACM0";
	}
	else
	{
		selectedSerialPort = argv[1];
	}

	int serialPortFd = rpcOpen(selectedSerialPort, 0);
	if (serialPortFd == -1)
	{
		dbg_print(PRINT_LEVEL_ERROR, "could not open serial port\n");
		exit(-1);
	}

	//init the rpc que client 初始化队列
	rpcInitMq();

	//init the application thread to register the callbacks
	appInit();

	//Start the Rx thread
	dbg_print(PRINT_LEVEL_INFO, "creating RPC thread\n");
	
	pthread_create(&rpcThread, NULL, rpcTask, (void *) &serialPortFd);

	//Start the example thread
	dbg_print(PRINT_LEVEL_INFO, "creating example thread\n");
	pthread_create(&appThread, NULL, appTask, NULL);

	while (1);

}
