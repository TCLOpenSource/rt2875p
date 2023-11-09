/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
#include "sony_windows_sleep.h"
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
int sony_windows_Sleep (unsigned long sleepTimeMs)
{
    BOOL qpcSupported = TRUE;
    LARGE_INTEGER qpcStartTime;
    LARGE_INTEGER qpcFrequency;

    if (sleepTimeMs == 0) {
        return 1;
    }

    if (!QueryPerformanceFrequency(&qpcFrequency)) {
        qpcSupported = FALSE;
    }

    /* Start time measurement using QueryPerformanceCounter. */
    if (!QueryPerformanceCounter(&qpcStartTime)) {
        qpcSupported = FALSE;
    }

    Sleep(sleepTimeMs);

    /* Double-check sleeping time using QueryPerformanceCounter. */
    if (qpcSupported) {
        LARGE_INTEGER qpcCurrentTime;
        unsigned long currentDelayMs = 0;
        unsigned long targetDelayMs = sleepTimeMs;

        do {
            if (!QueryPerformanceCounter(&qpcCurrentTime)) {
                return 0;
            }

            currentDelayMs = (unsigned long)(1000 * (qpcCurrentTime.QuadPart - qpcStartTime.QuadPart) / qpcFrequency.QuadPart);

            if (currentDelayMs >= targetDelayMs) {
                break;
            }
        } while(1);
    }

    return 1;
}

int sony_windows_ImproveTimeResolution (void)
{
    MMRESULT mmResult = 0;
    TIMECAPS timeCaps;

    /* Get multimedia timer resolution information. */
    mmResult = timeGetDevCaps(&timeCaps, sizeof(timeCaps));
    if (mmResult != TIMERR_NOERROR) {
        return 0;
    }

    /* Change resolution. */
    mmResult = timeBeginPeriod(timeCaps.wPeriodMin);
    if (mmResult != TIMERR_NOERROR) {
        return 0;
    }

    return 1;
}

int sony_windows_RestoreTimeResolution (void)
{
    MMRESULT mmResult = 0;
    TIMECAPS timeCaps;

    /* Get multimedia timer resolution information. */
    mmResult = timeGetDevCaps(&timeCaps, sizeof(timeCaps));
    if (mmResult != TIMERR_NOERROR) {
        return 0;
    }

    /* Change resolution. */
    mmResult = timeEndPeriod(timeCaps.wPeriodMin);
    if (mmResult != TIMERR_NOERROR) {
        return 0;
    }

    return 1;
}
