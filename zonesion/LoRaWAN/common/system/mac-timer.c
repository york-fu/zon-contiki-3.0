/*!
 * \file      timer.c
 *
 * \brief     Timer objects and scheduling management implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include "contiki.h"
#include "sys/clock.h"
#include "mac-timer.h"
#include "stm32f10x.h"


#define HW_TIMER_TYPE 2  // 0:systick, 1: rtc, 2: timer1

#if 0
#define debug  printf
#else
#define debug(x,...) do{}while(0)
#endif



PROCESS(loraMacTimer, "loraMacTimer");
/*!
 * This flag is used to loop through the main several times in order to be sure
 * that all pending events have been processed.
 */
volatile uint8_t HasLoopedThroughMain = 0;

/*!
 * Timers list head pointer
 */
static TimerEvent_t *TimerListHead = NULL;

/*!
 * \brief Adds or replace the head timer of the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param [IN]  obj Timer object to be become the new head
 * \param [IN]  remainingTime Remaining time of the previous head to be replaced
 */
static void TimerInsertNewHeadTimer( TimerEvent_t *obj, uint32_t remainingTime );

/*!
 * \brief Adds a timer to the list.
 *
 * \remark The list is automatically sorted. The list head always contains the
 *         next timer to expire.
 *
 * \param [IN]  obj Timer object to be added to the list
 * \param [IN]  remainingTime Remaining time of the running head after which the object may be added
 */
static void TimerInsertTimer( TimerEvent_t *obj, uint32_t remainingTime );

/*!
 * \brief Sets a timeout with the duration "timestamp"
 *
 * \param [IN] timestamp Delay duration
 */
static void TimerSetTimeout( TimerEvent_t *obj );

/*!
 * \brief Check if the Object to be added is not already in the list
 *
 * \param [IN] timestamp Delay duration
 * \retval true (the object is already in the list) or false
 */
static bool TimerExists( TimerEvent_t *obj );

/*!
 * \brief Read the timer value of the currently running timer
 *
 * \retval value current timer value
 */
TimerTime_t TimerGetValue( void );

void BoardDisableIrq(void);
void BoardEnableIrq(void);
void __timerPrint(void)
{
  TimerEvent_t *cur = TimerListHead;
  debug("T: ");
  while (cur != NULL) {
    debug ("%p at %u, ", cur, cur->Timestamp);
    cur = cur->Next;
  }
  debug("\r\n");
}

void TimerInit( TimerEvent_t *obj, void ( *callback )( void ) )
{
    obj->Timestamp = 0;
    obj->ReloadValue = 0;
    obj->IsRunning = false;
    obj->Callback = callback;
    obj->Next = NULL;
}

void TimerStart( TimerEvent_t *obj )
{
    uint32_t elapsedTime = 0;
    uint32_t remainingTime = 0;

    BoardDisableIrq( );

    if( ( obj == NULL ) || ( TimerExists( obj ) == true ) )
    {
        BoardEnableIrq( );
        return;
    }
    
    obj->Timestamp = obj->ReloadValue;
    obj->IsRunning = false;

    if( TimerListHead == NULL )
    {
        TimerInsertNewHeadTimer( obj, obj->Timestamp );
    }
    else
    {
        if( TimerListHead->IsRunning == true )
        {
            elapsedTime = TimerGetValue( );
            if( elapsedTime > TimerListHead->Timestamp )
            {
                elapsedTime = TimerListHead->Timestamp; // security but should never occur
            }
            remainingTime = TimerListHead->Timestamp - elapsedTime;
        }
        else
        {
            remainingTime = TimerListHead->Timestamp;
        }

        if( obj->Timestamp < remainingTime )
        {
            TimerInsertNewHeadTimer( obj, remainingTime );
        }
        else
        {
             TimerInsertTimer( obj, remainingTime );
        }
    }
    BoardEnableIrq( );
    debug("start t %p at %u rd %u\r\n", obj, TimerGetValue(), obj->ReloadValue);
    __timerPrint();
}

static void TimerInsertTimer( TimerEvent_t *obj, uint32_t remainingTime )
{
    uint32_t aggregatedTimestamp = 0;      // hold the sum of timestamps
    uint32_t aggregatedTimestampNext = 0;  // hold the sum of timestamps up to the next event

    TimerEvent_t* prev = TimerListHead;
    TimerEvent_t* cur = TimerListHead->Next;

    if( cur == NULL )
    { // obj comes just after the head
        obj->Timestamp -= remainingTime;
        prev->Next = obj;
        obj->Next = NULL;
    }
    else
    {
        aggregatedTimestamp = remainingTime;
        aggregatedTimestampNext = remainingTime + cur->Timestamp;

        while( prev != NULL )
        {
            if( aggregatedTimestampNext > obj->Timestamp )
            {
                obj->Timestamp -= aggregatedTimestamp;
                if( cur != NULL )
                {
                    cur->Timestamp -= obj->Timestamp;
                }
                prev->Next = obj;
                obj->Next = cur;
                break;
            }
            else
            {
                prev = cur;
                cur = cur->Next;
                if( cur == NULL )
                { // obj comes at the end of the list
                    aggregatedTimestamp = aggregatedTimestampNext;
                    obj->Timestamp -= aggregatedTimestamp;
                    prev->Next = obj;
                    obj->Next = NULL;
                    break;
                }
                else
                {
                    aggregatedTimestamp = aggregatedTimestampNext;
                    aggregatedTimestampNext = aggregatedTimestampNext + cur->Timestamp;
                }
            }
        }
    }
}

static void TimerInsertNewHeadTimer( TimerEvent_t *obj, uint32_t remainingTime )
{
    TimerEvent_t* cur = TimerListHead;

    if( cur != NULL )
    {
        cur->Timestamp = remainingTime - obj->Timestamp;
        cur->IsRunning = false;
    }

    obj->Next = cur;
    obj->IsRunning = true;
    TimerListHead = obj;
    TimerSetTimeout( TimerListHead );
}

void TimerIrqHandler( void )
{
    uint32_t elapsedTime = 0;

    // Early out when TimerListHead is null to prevent null pointer
    if ( TimerListHead == NULL )
    {
        return;
    }

    elapsedTime = TimerGetValue( );

    if( elapsedTime >= TimerListHead->Timestamp )
    {
        TimerListHead->Timestamp = 0;
    }
    else
    {
        TimerListHead->Timestamp -= elapsedTime;
    }

    TimerListHead->IsRunning = false;

    while( ( TimerListHead != NULL ) && ( TimerListHead->Timestamp == 0 ) )
    {
        TimerEvent_t* elapsedTimer = TimerListHead;
        TimerListHead = TimerListHead->Next;
        debug("Trige timer %p at %u, exp %u\r\n", elapsedTimer, elapsedTime, elapsedTimer->ReloadValue);
        if( elapsedTimer->Callback != NULL )
        {
            elapsedTimer->Callback( );
        }
    }

    // start the next TimerListHead if it exists
    if( TimerListHead != NULL )
    {
        if( TimerListHead->IsRunning != true )
        {
            TimerListHead->IsRunning = true;
            TimerSetTimeout( TimerListHead );
        }
    }
}

void TimerStop( TimerEvent_t *obj )
{
    BoardDisableIrq( );

    uint32_t elapsedTime = 0;
    uint32_t remainingTime = 0;

    TimerEvent_t* prev = TimerListHead;
    TimerEvent_t* cur = TimerListHead;

    // List is empty or the Obj to stop does not exist
    if( ( TimerListHead == NULL ) || ( obj == NULL ) )
    {
        BoardEnableIrq( );
        return;
    }

    if( TimerListHead == obj ) // Stop the Head
    {
        if( TimerListHead->IsRunning == true ) // The head is already running
        {
            elapsedTime = TimerGetValue( );
            if( elapsedTime > obj->Timestamp )
            {
                elapsedTime = obj->Timestamp;
            }

            remainingTime = obj->Timestamp - elapsedTime;

            TimerListHead->IsRunning = false;
            if( TimerListHead->Next != NULL )
            {
                TimerListHead = TimerListHead->Next;
                TimerListHead->Timestamp += remainingTime;
                TimerListHead->IsRunning = true;
                TimerSetTimeout( TimerListHead );
            }
            else
            {
                TimerListHead = NULL;
            }
        }
        else // Stop the head before it is started
        {
            if( TimerListHead->Next != NULL )
            {
                remainingTime = obj->Timestamp;
                TimerListHead = TimerListHead->Next;
                TimerListHead->Timestamp += remainingTime;
            }
            else
            {
                TimerListHead = NULL;
            }
        }
    }
    else // Stop an object within the list
    {
        remainingTime = obj->Timestamp;

        while( cur != NULL )
        {
            if( cur == obj )
            {
                if( cur->Next != NULL )
                {
                    cur = cur->Next;
                    prev->Next = cur;
                    cur->Timestamp += remainingTime;
                }
                else
                {
                    cur = NULL;
                    prev->Next = cur;
                }
                break;
            }
            else
            {
                prev = cur;
                cur = cur->Next;
            }
        }
    }
    BoardEnableIrq( );
}

static bool TimerExists( TimerEvent_t *obj )
{
    TimerEvent_t* cur = TimerListHead;

    while( cur != NULL )
    {
        if( cur == obj )
        {
            return true;
        }
        cur = cur->Next;
    }
    return false;
}

void TimerReset( TimerEvent_t *obj )
{
    TimerStop( obj );
    TimerStart( obj );
}

void TimerSetValue( TimerEvent_t *obj, uint32_t value )
{
    TimerStop( obj );
    obj->Timestamp = value;
    obj->ReloadValue = value;
}

unsigned int timer1Tick(void);
void timer1SetTimeout(unsigned int t);
void timer1Init(void);
void clock_delay_ms(unsigned int ms);

#if HW_TIMER_TYPE != 2
static unsigned int __save_tm;
#endif

static TimerTime_t TimerGetValue( void )
{
#if HW_TIMER_TYPE == 1
  return RtcGetTimerValue() - __save_tm; //RtcGetElapsedAlarmTime();
#elif HW_TIMER_TYPE == 2
  return timer1Tick();
#else
  return clock_time() - __save_tm; //RtcGetElapsedAlarmTime( );
#endif
}


TimerTime_t TimerGetCurrentTime( void )
{
#if HW_TIMER_TYPE == 1
  return RtcGetTimerValue();
#elif HW_TIMER_TYPE == 2
  return clock_time();
#else
    return clock_time(); 
#endif
}

TimerTime_t TimerGetElapsedTime( TimerTime_t savedTime )
{
  return TimerGetCurrentTime() - savedTime;//RtcComputeElapsedTime( savedTime );

}

TimerTime_t TimerGetFutureTime( TimerTime_t eventInFuture )
{
    //return RtcComputeFutureEventTime( eventInFuture );
  return TimerGetCurrentTime()+eventInFuture;
}

static void TimerSetTimeout( TimerEvent_t *obj )
{
    //HasLoopedThroughMain = 0;
    //obj->Timestamp = RtcGetAdjustedTimeoutValue( obj->Timestamp );
    //RtcSetTimeout( obj->Timestamp );

#if HW_TIMER_TYPE == 1
    __save_tm = TimerGetCurrentTime();
   RtcSetTimeout(tv+obj->Timestamp);
#elif HW_TIMER_TYPE == 2
   timer1SetTimeout(obj->Timestamp);
#else
    static struct etimer et;
    __save_tm = TimerGetCurrentTime();
    PROCESS_CONTEXT_BEGIN(&loraMacTimer);
    etimer_set(&et, obj->Timestamp / (1000/CLOCK_SECOND));
    PROCESS_CONTEXT_END(&loraMacTimer);
#endif
}

////////////////////////////////////////////////////////////////////////////////
static unsigned int __flag_c = 0;

#define irq_disable(ch) \
    NVIC->ICER[ch >> 0x05] = \
      (uint32_t)0x01 << (ch & (uint8_t)0x1F);
    
#define irq_enable(ch)  \
    NVIC->ISER[ch >> 0x05] = \
      (uint32_t)0x01 << (ch & (uint8_t)0x1F);
    
void BoardDisableIrq(void)
{
  
  if (__flag_c == 0) {
    __flag_c += 1;
    //asm volatile ("cpsid i");
    irq_disable(TIM1_UP_IRQn);
    
    irq_disable(EXTI9_5_IRQn);
    irq_disable(EXTI4_IRQn);
    irq_disable(EXTI3_IRQn);
    
  } else __flag_c += 1;
  
}
void BoardEnableIrq(void)
{
  
  if (__flag_c > 0) {
    __flag_c -= 1;
    if (__flag_c == 0) {
      //asm volatile ("cpsie i");
      irq_enable(TIM1_UP_IRQn);
      
      irq_enable(EXTI9_5_IRQn);
      irq_enable(EXTI4_IRQn);
      irq_enable(EXTI3_IRQn);
    }
  }
}
void DelayMs(int t)
{
    clock_delay_ms(t);
}

PROCESS_THREAD(loraMacTimer, ev, data)
{
  PROCESS_BEGIN();
#if HW_TIMER_TYPE == 1  
  RTC_Configuration();
#elif HW_TIMER_TYPE == 2  
  timer1Init();
#else
#endif
  while (1) {
#if HW_TIMER_TYPE == 0 
    PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER);
    TimerIrqHandler();
#else
    PROCESS_YIELD();
#endif
  }
  PROCESS_END();
}
