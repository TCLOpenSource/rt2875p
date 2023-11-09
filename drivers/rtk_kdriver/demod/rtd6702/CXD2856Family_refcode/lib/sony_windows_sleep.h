/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2017/11/30
  Modification ID : 253a4918e2da2cf28a9393596fa16f25024e504d
------------------------------------------------------------------------------*/
/**
 @file  sony_windows_sleep.h

 @brief Windows specific sleep function definition.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_WINDOWS_SLEEP_H
#define SONY_WINDOWS_SLEEP_H

/*------------------------------------------------------------------------------
 Function Prototypes
------------------------------------------------------------------------------*/
/**
 @brief High precision sleep function for Windows PC.
        This function ensures that sleeping time is not shorter than specified time.

 @param sleepTimeMs Sleep time in ms.

 @return 1 if successful. 0 if failed.
*/
int sony_windows_Sleep (unsigned long sleepTimeMs);

/**
 @brief This function improves timer accuracy by setting the timer resolution to the
        minimum supported by the Windows PC.  This function should be called at the
        start of the application.  If this function is used,
        ::sony_windows_RestoreTimeResolution should be called at the end of the
        application.

 @return 1 if successful. 0 if failed.
*/
int sony_windows_ImproveTimeResolution (void);

/**
 @brief This function restores the time resolution setting configured
        by ::sony_windows_ImproveTimeResolution.

 @return 1 if successful. 0 if failed.
*/
int sony_windows_RestoreTimeResolution (void);

#endif /* SONY_WINDOWS_SLEEP_H */
