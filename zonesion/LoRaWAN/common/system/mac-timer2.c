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
//#include "board.h"
//#include "rtc-board.h"
#include "timer2.h"

#if 0
#define debug  printf
#else
#define debug(x,...) do{}while(0)
#endif
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

void TimerInit( TimerEvent_t *obj, void ( *callback )( void ) )
{
    obj->Timestamp = 0;
    obj->ReloadValue = 0;
    obj->IsRunning = false;
    obj->Callback = callback;
    obj->Next = NULL;
}
void TimerPrint(void)
{
  TimerEvent_t *cur = TimerListHead;
  debug("T: ");
  while (cur != NULL) {
    debug ("%p at %u, ", cur, cur->Timestamp);
    cur = cur->Next;
  }
  debug("\r\n");
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
    
    
    obj->Timestamp = clock_time()+obj->ReloadValue;
    obj->IsRunning = false;
    debug("set timer %p  exp: %u\r\n", obj, obj->Timestamp);
   
   
    TimerInsertTimer( obj, 0 );
    
    TimerPrint();
    
    BoardEnableIrq( );
}

static void TimerInsertTimer( TimerEvent_t *obj,  uint32_t a )
{
    uint32_t aggregatedTimestamp = 0;      // hold the sum of timestamps
    uint32_t aggregatedTimestampNext = 0;  // hold the sum of timestamps up to the next event

    TimerEvent_t* prev = TimerListHead;
    TimerEvent_t* cur = TimerListHead->Next;

    
    if (prev == NULL) {
      TimerListHead = obj;
      obj->Next = NULL;
      return;
    } else if (obj->Timestamp < prev->Timestamp){
      obj->Next = prev;
      TimerListHead = obj;
    } else{
      while (cur != NULL) {
        if (cur->Timestamp < obj->Timestamp) {
          prev = cur;
          cur = cur->Next;
        } else {
          obj->Next = cur;
          prev->Next = obj;
          return;
        }
      }
      prev->Next = obj;
      obj->Next = NULL;
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
      
       
      TimerEvent_t* elapsedTimer = TimerListHead;
        TimerListHead = TimerListHead->Next;
        
        debug("trige timer %p at: %u\r\n",elapsedTimer, elapsedTime);
        TimerPrint();
        elapsedTimer->IsRunning = false;
        elapsedTimer->Timestamp = 0;

        if( elapsedTimer->Callback != NULL )
        {
            elapsedTimer->Callback( );
        }
    }
    else
    {
        //TimerListHead->Timestamp -= elapsedTime;
    }


    // start the next TimerListHead if it exists
    if( TimerListHead != NULL )
    {
        if( TimerListHead->IsRunning != true )
        {
            TimerListHead->IsRunning = true;
            //TimerSetTimeout( TimerListHead );
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
    if (TimerListHead == obj) {
      debug("stop timer %p\r\n", obj);
      TimerListHead = obj->Next;
      TimerPrint();
      return;
    } else {
      cur = prev->Next;
      while (cur!=NULL) {
        if (cur == obj) {
          prev->Next = cur->Next;
          debug("stop timer %p\r\n", obj);
          TimerPrint();
          break;
        }
        prev = cur;
        cur = cur->Next;
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
    obj->Timestamp = clock_time()+value;
    obj->ReloadValue = value;
}

TimerTime_t TimerGetValue( void )
{
    return clock_time(); //RtcGetElapsedAlarmTime( );
}

TimerTime_t TimerGetCurrentTime( void )
{
    //return RtcGetTimerValue( );
  return clock_time();
}

TimerTime_t TimerGetElapsedTime( TimerTime_t savedTime )
{
    //return RtcComputeElapsedTime( savedTime );
  return clock_time() - savedTime;
}

TimerTime_t TimerGetFutureTime( TimerTime_t eventInFuture )
{
    //return RtcComputeFutureEventTime( eventInFuture );
  return clock_time()+eventInFuture;
}
//struct timer __t;
static void TimerSetTimeout( TimerEvent_t *obj )
{
    HasLoopedThroughMain = 0;
    //obj->Timestamp = RtcGetAdjustedTimeoutValue( obj->Timestamp );
    //RtcSetTimeout( obj->Timestamp );
    //timer_set(&__t, obj->Timestamp);
    obj->Timestamp = obj->Timestamp+clock_time();
}

void TimerLowPowerHandler( void )
{
    if( ( TimerListHead != NULL ) && ( TimerListHead->IsRunning == true ) )
    {
        if( HasLoopedThroughMain < 5 )
        {
            HasLoopedThroughMain++;
        }
        else
        {
            HasLoopedThroughMain = 0;
            if( GetBoardPowerSource( ) == /*BATTERY_POWER*/0 )
            {
                RtcEnterLowPowerStopMode( );
            }
        }
    }
}

void TimerProcess( void )
{
    //RtcProcess( );
  BoardDisableIrq( );
  TimerIrqHandler();
  BoardEnableIrq( );
}

