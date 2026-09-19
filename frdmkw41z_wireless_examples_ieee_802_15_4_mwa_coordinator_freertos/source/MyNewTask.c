/*
 * MyNewTask.c
 *
 *  Created on: Sep 7, 2026
 *      Author: dbarraga
 */

#include "MyNewTask.h"
#include "SerialManager.h"

/* Interface used by mwa_coordinator.c; reused here only for debug traces */
extern uint8_t interfaceId;

osaEventId_t mMyEvents;
/* Global Variable to store our TimerID */
tmrTimerID_t myTimerID = gTmrInvalidTimerID_c;
/* Handler ID for task */
osaTaskId_t gMyTaskHandler_ID;
/* Local variable to store the current state of the LEDs */
static uint8_t ledsState = 0;
/* Last counter value (0-3) received from the coordinator's data indication */
static uint8_t receivedCounter = 0;

static void myTaskTimerCallback(void *param);

/* OSA Task Definition*/
OSA_TASK_DEFINE(My_Task, gMyTaskPriority_c, 1, gMyTaskStackSize_c, FALSE );

/* Main custom task */
void My_Task(osaTaskParam_t argument)
{
	osaEventFlags_t customEvent;
	myTimerID = TMR_AllocateTimer();
	while(1)
	{
		OSA_EventWait(mMyEvents, osaEventFlagsAll_c, FALSE, osaWaitForever_c,
		&customEvent);
		if( !gUseRtos_c && !customEvent)
		{
			break;
		}
		/* Depending on the received event */
		switch(customEvent){
		case gMyNewTaskEvent1_c:
		TMR_StartIntervalTimer(myTimerID, /*myTimerID*/
								1000, /* Timer's Timeout */
								myTaskTimerCallback, /* pointer to
			myTaskTimerCallback function */
								NULL
			);
			TurnOffLeds(); /* Ensure all LEDs are turned off */
			break;
		case gMyNewTaskEvent2_c: /* Event called from myTaskTimerCallback */
			if(!ledsState) {
				TurnOnLeds();
				ledsState = 1;
			}
			else {
				TurnOffLeds();
				ledsState = 0;
			}
			break;
		case gMyNewTaskEvent3_c: /* Event to stop the timer */
			ledsState = 0;
			TurnOffLeds();
			TMR_StopTimer(myTimerID);
			break;
		case gMyNewTaskEvent4_c: /* Event to show the received counter (0-3) on the LEDs */
		{
			/* Data typed on a UART terminal arrives as the ASCII digit ('0'-'3') */
			uint8_t counter = receivedCounter;
			if( counter >= '0' && counter <= '3' )
			{
				counter -= '0';
			}
//			Serial_Print(interfaceId, "[MyTask] gMyNewTaskEvent4_c received, counter=", gAllowToBlock_d);
//			Serial_PrintDec(interfaceId, counter);
//			Serial_Print(interfaceId, "\r\n", gAllowToBlock_d);
			TurnOffLeds();
			switch(counter){
			case 0:
				Led3On();
				break;
			case 1:
				Led2On();
				break;
			case 2:
				Led4On();
				break;
			case 3:
				Led4On();
				Led2On();
				break;
			default:
				break;
			}
			break;
		}
		default:
			break;
		}
	}
}

/* Function to init the task */
void MyTask_Init(void)
{
	mMyEvents = OSA_EventCreate(TRUE);
	/* The instance of the MAC is passed at task creaton */
	gMyTaskHandler_ID = OSA_TaskCreate(OSA_TASK(My_Task), NULL);
}

/* This is the function called by the Timer each time it expires */
static void myTaskTimerCallback(void *param)
{
	OSA_EventSet(mMyEvents, gMyNewTaskEvent2_c);
}

/* Public function to send an event to stop the timer */
void MyTaskTimer_Stop(void)
{
	OSA_EventSet(mMyEvents, gMyNewTaskEvent3_c);
}

/* Public function to send an event to start the timer */
void MyTaskTimer_Start(void)
{
	OSA_EventSet(mMyEvents, gMyNewTaskEvent1_c);
}

/* Public function to display the received counter (0-3) on the LEDs */
void MyTask_SetCounterLed(uint8_t counter)
{
	receivedCounter = counter;
	OSA_EventSet(mMyEvents, gMyNewTaskEvent4_c);
}

