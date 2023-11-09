#include "hdmi_edid.h"
#include "hdmi_debug.h"
#include "cea_861.h"
#include "hdmiInternal.h"

const VIC_TIMING_INFO vic_timing[] =
{   // vic, hactive, vactive, prog, hfreq, vfreq, htotal, vtotal, hSyncWidth, hBackPorch, vSyncWidth, vBackPorch, PolarityFlag
    { 1 , 640 , 480 , VIC_TB_PROGRESSIVE , 315 , 599 , 800 , 525 , 96 , 48 , 2 , 33 , VIC_SYNC_HN_VN } ,
    { 2 , 720 , 480 , VIC_TB_PROGRESSIVE , 315 , 599 , 858 , 525 , 62 , 60 , 6 , 30 , VIC_SYNC_HN_VN } ,
    { 3 , 720 , 480 , VIC_TB_PROGRESSIVE , 315 , 599 , 858 , 525 , 62 , 60 , 6 , 30 , VIC_SYNC_HN_VN } ,
    { 4 , 1280 , 720 , VIC_TB_PROGRESSIVE , 450 , 600 , 1650 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 5 , 1920 , 1080 , VIC_TB_INTERLACE , 338 , 600 , 2200 , 1125 , 44 , 148 , 5 , 15 , VIC_SYNC_HP_VP } ,
    { 6 , 1440 , 480 , VIC_TB_INTERLACE , 157 , 599 , 1716 , 525 , 124 , 114 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 7 , 1440 , 480 , VIC_TB_INTERLACE , 157 , 599 , 1716 , 525 , 124 , 114 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 8 , 1440 , 240 , VIC_TB_PROGRESSIVE , 157 , 601 , 1716 , 262 , 124 , 114 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 8 , 1440 , 240 , VIC_TB_PROGRESSIVE , 157 , 598 , 1716 , 263 , 124 , 114 , 3 , 15 , VIC_SYNC_HN_VN } ,    //TBD
    { 9 , 1440 , 240 , VIC_TB_PROGRESSIVE , 157 , 601 , 1716 , 262 , 124 , 114 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 9 , 1440 , 240 , VIC_TB_PROGRESSIVE , 157 , 598 , 1716 , 263 , 124 , 114 , 3 , 15 , VIC_SYNC_HN_VN } ,   //TBD
    { 10 , 2880 , 480 , VIC_TB_INTERLACE , 157 , 599 , 3432 , 525 , 248 , 228 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 11 , 2880 , 480 , VIC_TB_INTERLACE , 157 , 599 , 3432 , 525 , 248 , 228 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 12 , 2880 , 240 , VIC_TB_PROGRESSIVE , 157 , 601 , 3432 , 262 , 248 , 228 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 12 , 2880 , 240 , VIC_TB_PROGRESSIVE , 157 , 598 , 3432 , 263 , 248 , 228 , 3 , 15 , VIC_SYNC_HN_VN } ,   //TBD
    { 13 , 2880 , 240 , VIC_TB_PROGRESSIVE , 157 , 601 , 3432 , 262 , 248 , 228 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 13 , 2880 , 240 , VIC_TB_PROGRESSIVE , 157 , 598 , 3432 , 263 , 248 , 228 , 3 , 15 , VIC_SYNC_HN_VN } ,   //TBD
    { 14 , 1440 , 480 , VIC_TB_PROGRESSIVE , 315 , 599 , 1716 , 525 , 124 , 120 , 6 , 30 , VIC_SYNC_HN_VN } ,
    { 15 , 1440 , 480 , VIC_TB_PROGRESSIVE , 315 , 599 , 1716 , 525 , 124 , 120 , 6 , 30 , VIC_SYNC_HN_VN } ,
    { 16 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 675 , 600 , 2200 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 17 , 720 , 576 , VIC_TB_PROGRESSIVE , 313 , 500 , 864 , 625 , 64 , 68 , 5 , 39 , VIC_SYNC_HN_VN } ,
    { 18 , 720 , 576 , VIC_TB_PROGRESSIVE , 313 , 500 , 864 , 625 , 64 , 68 , 5 , 39 , VIC_SYNC_HN_VN } ,
    { 19 , 1280 , 720 , VIC_TB_PROGRESSIVE , 375 , 500 , 1980 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 20 , 1920 , 1080 , VIC_TB_INTERLACE , 281 , 500 , 2640 , 1125 , 44 , 148 , 5 , 15 , VIC_SYNC_HP_VP } ,
    { 21 , 1440 , 576 , VIC_TB_INTERLACE , 156 , 500 , 1728 , 625 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 22 , 1440 , 576 , VIC_TB_INTERLACE , 156 , 500 , 1728 , 625 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 23 , 1440 , 288 , VIC_TB_PROGRESSIVE , 156 , 501 , 1728 , 312 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 23 , 1440 , 288 , VIC_TB_PROGRESSIVE , 156 , 499 , 1728 , 313 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,   //TBD
    { 23 , 1440 , 288 , VIC_TB_PROGRESSIVE , 156 , 498 , 1728 , 314 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,   //TBD
    { 24 , 1440 , 288 , VIC_TB_PROGRESSIVE , 156 , 501 , 1728 , 312 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 24 , 1440 , 288 , VIC_TB_PROGRESSIVE , 156 , 499 , 1728 , 313 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,   //TBD
    { 24 , 1440 , 288 , VIC_TB_PROGRESSIVE , 156 , 498 , 1728 , 314 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,   //TBD
    { 25 , 2880 , 576 , VIC_TB_INTERLACE , 156 , 500 , 3456 , 625 , 252 , 276 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 26 , 2880 , 576 , VIC_TB_INTERLACE , 156 , 500 , 3456 , 625 , 252 , 276 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 27 , 2880 , 288 , VIC_TB_PROGRESSIVE , 156 , 501 , 3456 , 312 , 252 , 276 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 27 , 2880 , 288 , VIC_TB_PROGRESSIVE , 156 , 499 , 3456 , 313 , 252 , 276 , 3 , 19 , VIC_SYNC_HN_VN } ,   //TBD
    { 27 , 2880 , 288 , VIC_TB_PROGRESSIVE , 156 , 498 , 3456 , 314 , 252 , 276 , 3 , 19 , VIC_SYNC_HN_VN } ,   //TBD
    { 28 , 2880 , 288 , VIC_TB_PROGRESSIVE , 156 , 501 , 3456 , 312 , 252 , 276 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 28 , 2880 , 288 , VIC_TB_PROGRESSIVE , 156 , 499 , 3456 , 313 , 252 , 276 , 3 , 19 , VIC_SYNC_HN_VN } ,   //TBD
    { 28 , 2880 , 288 , VIC_TB_PROGRESSIVE , 156 , 498 , 3456 , 314 , 252 , 276 , 3 , 19 , VIC_SYNC_HN_VN } ,   //TBD
    { 29 , 1440 , 576 , VIC_TB_PROGRESSIVE , 313 , 500 , 1728 , 625 , 128 , 136 , 5 , 39 , VIC_SYNC_HN_VN } ,
    { 30 , 1440 , 576 , VIC_TB_PROGRESSIVE , 313 , 500 , 1728 , 625 , 128 , 136 , 5 , 39 , VIC_SYNC_HN_VN } ,
    { 31 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 563 , 500 , 2640 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 32 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 270 , 240 , 2750 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 33 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 281 , 250 , 2640 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 34 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 338 , 300 , 2200 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 35 , 2880 , 480 , VIC_TB_PROGRESSIVE , 315 , 599 , 3432 , 525 , 248 , 240 , 6 , 30 , VIC_SYNC_HN_VN } ,
    { 36 , 2880 , 480 , VIC_TB_PROGRESSIVE , 315 , 599 , 3432 , 525 , 248 , 240 , 6 , 30 , VIC_SYNC_HN_VN } ,
    { 37 , 2880 , 576 , VIC_TB_PROGRESSIVE , 313 , 500 , 3456 , 625 , 256 , 272 , 5 , 39 , VIC_SYNC_HN_VN } ,
    { 38 , 2880 , 576 , VIC_TB_PROGRESSIVE , 313 , 500 , 3456 , 625 , 256 , 272 , 5 , 39 , VIC_SYNC_HN_VN } ,
    { 39 , 1920 , 1080 , VIC_TB_INTERLACE , 313 , 500 , 2304 , 1250 , 168 , 184 , 5 , 57 , VIC_SYNC_HP_VN } ,
    { 40 , 1920 , 1080 , VIC_TB_INTERLACE , 563 , 1000 , 2640 , 1125 , 44 , 148 , 5 , 15 , VIC_SYNC_HP_VP } ,
    { 41 , 1280 , 720 , VIC_TB_PROGRESSIVE , 750 , 1000 , 1980 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 42 , 720 , 576 , VIC_TB_PROGRESSIVE , 625 , 1000 , 864 , 625 , 64 , 68 , 5 , 39 , VIC_SYNC_HN_VN } ,
    { 43 , 720 , 576 , VIC_TB_PROGRESSIVE , 625 , 1000 , 864 , 625 , 64 , 68 , 5 , 39 , VIC_SYNC_HN_VN } ,
    { 44 , 1440 , 576 , VIC_TB_INTERLACE , 313 , 1000 , 1728 , 625 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 45 , 1440 , 576 , VIC_TB_INTERLACE , 313 , 1000 , 1728 , 625 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 46 , 1920 , 1080 , VIC_TB_INTERLACE , 675 , 1200 , 2200 , 1125 , 44 , 148 , 5 , 15 , VIC_SYNC_HP_VP } ,
    { 47 , 1280 , 720 , VIC_TB_PROGRESSIVE , 900 , 1200 , 1650 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 48 , 720 , 480 , VIC_TB_PROGRESSIVE , 629 , 1199 , 858 , 525 , 62 , 60 , 6 , 30 , VIC_SYNC_HN_VN } ,
    { 49 , 720 , 480 , VIC_TB_PROGRESSIVE , 629 , 1199 , 858 , 525 , 62 , 60 , 6 , 30 , VIC_SYNC_HN_VN } ,
    { 50 , 1440 , 480 , VIC_TB_INTERLACE , 315 , 1199 , 1716 , 525 , 124 , 114 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 51 , 1440 , 480 , VIC_TB_INTERLACE , 315 , 1199 , 1716 , 525 , 124 , 114 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 52 , 720 , 576 , VIC_TB_PROGRESSIVE , 1250 , 2000 , 864 , 625 , 64 , 68 , 5 , 39 , VIC_SYNC_HN_VN } ,
    { 53 , 720 , 576 , VIC_TB_PROGRESSIVE , 1250 , 2000 , 864 , 625 , 64 , 68 , 5 , 39 , VIC_SYNC_HN_VN } ,
    { 54 , 1440 , 576 , VIC_TB_INTERLACE , 625 , 2000 , 1728 , 625 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 55 , 1440 , 576 , VIC_TB_INTERLACE , 625 , 2000 , 1728 , 625 , 126 , 138 , 3 , 19 , VIC_SYNC_HN_VN } ,
    { 56 , 720 , 480 , VIC_TB_PROGRESSIVE , 1259 , 2398 , 858 , 525 , 62 , 60 , 6 , 30 , VIC_SYNC_HN_VN } ,
    { 57 , 720 , 480 , VIC_TB_PROGRESSIVE , 1259 , 2398 , 858 , 525 , 62 , 60 , 6 , 30 , VIC_SYNC_HN_VN } ,
    { 58 , 1440 , 480 , VIC_TB_INTERLACE , 629 , 2398 , 1716 , 525 , 124 , 114 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 59 , 1440 , 480 , VIC_TB_INTERLACE , 629 , 2398 , 1716 , 525 , 124 , 114 , 3 , 15 , VIC_SYNC_HN_VN } ,
    { 60 , 1280 , 720 , VIC_TB_PROGRESSIVE , 180 , 240 , 3300 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 61 , 1280 , 720 , VIC_TB_PROGRESSIVE , 188 , 250 , 3960 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 62 , 1280 , 720 , VIC_TB_PROGRESSIVE , 225 , 300 , 3300 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 63 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 1350 , 1200 , 2200 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 64 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 1125 , 1000 , 2640 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 65 , 1280 , 720 , VIC_TB_PROGRESSIVE , 180 , 240 , 3300 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 66 , 1280 , 720 , VIC_TB_PROGRESSIVE , 188 , 250 , 3960 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 67 , 1280 , 720 , VIC_TB_PROGRESSIVE , 225 , 300 , 3300 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 68 , 1280 , 720 , VIC_TB_PROGRESSIVE , 375 , 500 , 1980 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 69 , 1280 , 720 , VIC_TB_PROGRESSIVE , 450 , 600 , 1650 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 70 , 1280 , 720 , VIC_TB_PROGRESSIVE , 750 , 1000 , 1980 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 71 , 1280 , 720 , VIC_TB_PROGRESSIVE , 900 , 1200 , 1650 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 72 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 270 , 240 , 2750 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 73 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 281 , 250 , 2640 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 74 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 338 , 300 , 2200 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 75 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 563 , 500 , 2640 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 76 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 675 , 600 , 2200 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 77 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 1125 , 1000 , 2640 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 78 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 1350 , 1200 , 2200 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 79 , 1680 , 720 , VIC_TB_PROGRESSIVE , 180 , 240 , 3300 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 80 , 1680 , 720 , VIC_TB_PROGRESSIVE , 188 , 250 , 3168 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 81 , 1680 , 720 , VIC_TB_PROGRESSIVE , 225 , 300 , 2640 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 82 , 1680 , 720 , VIC_TB_PROGRESSIVE , 375 , 500 , 2200 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 83 , 1680 , 720 , VIC_TB_PROGRESSIVE , 450 , 600 , 2200 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 84 , 1680 , 720 , VIC_TB_PROGRESSIVE , 825 , 1000 , 2000 , 825 , 40 , 220 , 5 , 95 , VIC_SYNC_HP_VP } ,
    { 85 , 1680 , 720 , VIC_TB_PROGRESSIVE , 990 , 1200 , 2000 , 825 , 40 , 220 , 5 , 95 , VIC_SYNC_HP_VP } ,
    { 86 , 2560 , 1080 , VIC_TB_PROGRESSIVE , 264 , 240 , 3750 , 1100 , 44 , 148 , 5 , 11 , VIC_SYNC_HP_VP } ,
    { 87 , 2560 , 1080 , VIC_TB_PROGRESSIVE , 281 , 250 , 3200 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 88 , 2560 , 1080 , VIC_TB_PROGRESSIVE , 338 , 300 , 3520 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 89 , 2560 , 1080 , VIC_TB_PROGRESSIVE , 563 , 500 , 3300 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 90 , 2560 , 1080 , VIC_TB_PROGRESSIVE , 660 , 600 , 3000 , 1100 , 44 , 148 , 5 , 11 , VIC_SYNC_HP_VP } ,
    { 91 , 2560 , 1080 , VIC_TB_PROGRESSIVE , 1250 , 1000 , 2970 , 1250 , 44 , 148 , 5 , 161 , VIC_SYNC_HP_VP } ,
    { 92 , 2560 , 1080 , VIC_TB_PROGRESSIVE , 1500 , 1200 , 3300 , 1250 , 44 , 148 , 5 , 161 , VIC_SYNC_HP_VP } ,
    { 93 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 540 , 240 , 5500 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 94 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 563 , 250 , 5280 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 95 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 675 , 300 , 4400 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 96 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 1125 , 500 , 5280 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 97 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 1350 , 600 , 4400 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 98 , 4096 , 2160 , VIC_TB_PROGRESSIVE , 540 , 240 , 5500 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 99 , 4096 , 2160 , VIC_TB_PROGRESSIVE , 563 , 250 , 5280 , 2250 , 88 , 128 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 100 , 4096 , 2160 , VIC_TB_PROGRESSIVE , 675 , 300 , 4400 , 2250 , 88 , 128 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 101 , 4096 , 2160 , VIC_TB_PROGRESSIVE , 1125 , 500 , 5280 , 2250 , 88 , 128 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 102 , 4096 , 2160 , VIC_TB_PROGRESSIVE , 1350 , 600 , 4400 , 2250 , 88 , 128 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 103 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 540 , 240 , 5500 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 104 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 563 , 250 , 5280 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 105 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 675 , 300 , 4400 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 106 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 1125 , 500 , 5280 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 107 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 1350 , 600 , 4400 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 108 , 1280 , 720 , VIC_TB_PROGRESSIVE , 360 , 480 , 2500 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 109 , 1280 , 720 , VIC_TB_PROGRESSIVE , 360 , 480 , 2500 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 110 , 1680 , 720 , VIC_TB_PROGRESSIVE , 360 , 480 , 2750 , 750 , 40 , 220 , 5 , 20 , VIC_SYNC_HP_VP } ,
    { 111 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 540 , 480 , 2750 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 112 , 1920 , 1080 , VIC_TB_PROGRESSIVE , 540 , 480 , 2750 , 1125 , 44 , 148 , 5 , 36 , VIC_SYNC_HP_VP } ,
    { 113 , 2560 , 1080 , VIC_TB_PROGRESSIVE , 528 , 480 , 3750 , 1100 , 44 , 148 , 5 , 11 , VIC_SYNC_HP_VP } ,
    { 114 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 1080 , 480 , 5500 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 115 , 4096 , 2160 , VIC_TB_PROGRESSIVE , 1080 , 480 , 5500 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 116 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 1080 , 480 , 5500 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 117 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 2250 , 1000 , 5280 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 118 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 2700 , 1200 , 4400 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 119 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 2250 , 1000 , 5280 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 120 , 3840 , 2160 , VIC_TB_PROGRESSIVE , 2700 , 1200 , 4400 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 121 , 5120 , 2160 , VIC_TB_PROGRESSIVE , 528 , 240 , 7500 , 2200 , 88 , 296 , 10 , 22 , VIC_SYNC_HP_VP } ,
    { 122 , 5120 , 2160 , VIC_TB_PROGRESSIVE , 550 , 250 , 7200 , 2200 , 88 , 296 , 10 , 22 , VIC_SYNC_HP_VP } ,
    { 123 , 5120 , 2160 , VIC_TB_PROGRESSIVE , 660 , 300 , 6000 , 2200 , 88 , 128 , 10 , 22 , VIC_SYNC_HP_VP } ,
    { 124 , 5120 , 2160 , VIC_TB_PROGRESSIVE , 1188 , 480 , 6250 , 2475 , 88 , 296 , 10 , 297 , VIC_SYNC_HP_VP } ,
    { 125 , 5120 , 2160 , VIC_TB_PROGRESSIVE , 1125 , 500 , 6600 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 126 , 5120 , 2160 , VIC_TB_PROGRESSIVE , 1350 , 600 , 5500 , 2250 , 88 , 128 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 127 , 5120 , 2160 , VIC_TB_PROGRESSIVE , 2250 , 1000 , 6600 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 193 , 5120 , 2160 , VIC_TB_PROGRESSIVE , 2700 , 1200 , 5500 , 2250 , 88 , 128 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 194 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 1080 , 240 , 11000 , 4500 , 176 , 592 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 195 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 1100 , 250 , 10800 , 4400 , 176 , 592 , 20 , 44 , VIC_SYNC_HP_VP } ,
    { 196 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 1320 , 300 , 9000 , 4400 , 176 , 592 , 20 , 44 , VIC_SYNC_HP_VP } ,
    { 197 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 2160 , 480 , 11000 , 4500 , 176 , 592 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 198 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 2200 , 500 , 10800 , 4400 , 176 , 592 , 20 , 44 , VIC_SYNC_HP_VP } ,
    { 199 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 2640 , 600 , 9000 , 4400 , 176 , 592 , 20 , 44 , VIC_SYNC_HP_VP } ,
    { 200 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 4500 , 1000 , 10560 , 4500 , 176 , 592 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 201 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 5400 , 1200 , 8800 , 4500 , 176 , 592 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 202 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 1080 , 240 , 11000 , 4500 , 176 , 592 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 203 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 1100 , 250 , 10800 , 4400 , 176 , 592 , 20 , 44 , VIC_SYNC_HP_VP } ,
    { 204 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 1320 , 300 , 9000 , 4400 , 176 , 592 , 20 , 44 , VIC_SYNC_HP_VP } ,
    { 205 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 2160 , 480 , 11000 , 4500 , 176 , 592 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 206 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 2200 , 500 , 10800 , 4400 , 176 , 592 , 20 , 44 , VIC_SYNC_HP_VP } ,
    { 207 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 2640 , 600 , 9000 , 4400 , 176 , 592 , 20 , 44 , VIC_SYNC_HP_VP } ,
    { 208 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 4500 , 1000 , 10560 , 4500 , 176 , 592 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 209 , 7680 , 4320 , VIC_TB_PROGRESSIVE , 5400 , 1200 , 8800 , 4500 , 176 , 592 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 210 , 10240 , 4320 , VIC_TB_PROGRESSIVE , 1188 , 240 , 12500 , 4950 , 176 , 592 , 20 , 594 , VIC_SYNC_HP_VP } ,
    { 211 , 10240 , 4320 , VIC_TB_PROGRESSIVE , 1100 , 250 , 13500 , 4400 , 176 , 592 , 20 , 44 , VIC_SYNC_HP_VP } ,
    { 212 , 10240 , 4320 , VIC_TB_PROGRESSIVE , 1350 , 300 , 11000 , 4500 , 176 , 296 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 213 , 10240 , 4320 , VIC_TB_PROGRESSIVE , 2376 , 480 , 12500 , 4950 , 176 , 592 , 20 , 594 , VIC_SYNC_HP_VP } ,
    { 214 , 10240 , 4320 , VIC_TB_PROGRESSIVE , 2200 , 500 , 13500 , 4400 , 176 , 592 , 20 , 44 , VIC_SYNC_HP_VP } ,
    { 215 , 10240 , 4320 , VIC_TB_PROGRESSIVE , 2700 , 600 , 11000 , 4500 , 176 , 296 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 216 , 10240 , 4320 , VIC_TB_PROGRESSIVE , 4500 , 1000 , 13200 , 4500 , 176 , 592 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 217 , 10240 , 4320 , VIC_TB_PROGRESSIVE , 5400 , 1200 , 11000 , 4500 , 176 , 296 , 20 , 144 , VIC_SYNC_HP_VP } ,
    { 218 , 4096 , 2160 , VIC_TB_PROGRESSIVE , 2250 , 1000 , 5280 , 2250 , 88 , 296 , 10 , 72 , VIC_SYNC_HP_VP } ,
    { 219 , 4096 , 2160 , VIC_TB_PROGRESSIVE , 2700 , 1200 , 4400 , 2250 , 88 , 128 , 10 , 72 , VIC_SYNC_HP_VP }
};


/*--------------------------------------------
 * Func : _get_vic_timing
 *
 * Desc : get timing information of a given VIC
 *
 * Para : vic : (I) VIC  
 *
 * Retn : NULL : no Video timing of the given VIC
 *        others : handle of the VIC timing info
 *-------------------------------------------*/
const VIC_TIMING_INFO* _get_vic_timing(unsigned char vic)
{
    int i, table_size =  sizeof(vic_timing)/sizeof(VIC_TIMING_INFO);
    for (i=0; i<table_size; i++)
    {
        if (vic_timing[i].vic == vic)
            return &vic_timing[i];
    }
    return NULL;
}


#define INPUT_SEARCH_ACTIVE_TOL   5
#define INPUT_SEARCH_FREQ_TOL   8

/*--------------------------------------------
 * Func : serch_vic_timing
 *
 * Desc : get timing information of a given VIC
 *
 * Para : vic : (I) VIC  
 *
 * Retn : NULL : no Video timing of the given VIC
 *        others : handle of the VIC timing info
 *-------------------------------------------*/
const VIC_TIMING_INFO* _search_vic_timing(unsigned short h_active, unsigned short v_active, unsigned short v_freq, unsigned short h_total)
{
    int i;
    for (i=0; i<sizeof(vic_timing)/sizeof(VIC_TIMING_INFO); i++)
    {
        if ((HDMI_ABS(vic_timing[i].hTotal, h_total) <=INPUT_SEARCH_ACTIVE_TOL) &&
            (HDMI_ABS(vic_timing[i].hActive, h_active) <=INPUT_SEARCH_ACTIVE_TOL) &&
            (HDMI_ABS(vic_timing[i].vActive, v_active) <=INPUT_SEARCH_ACTIVE_TOL) &&
            (HDMI_ABS(vic_timing[i].vFreq, v_freq) <=INPUT_SEARCH_FREQ_TOL))
        {
            HDMI_INFO("[_search_vic_timing] Search success. VIC:%d, i=%d. (Original: h_active:%d, v_active:%d, v_freq:%d, h_total:%d,\n",
                vic_timing[i].vic, i, h_active, v_active, v_freq, h_total);
            return &vic_timing[i];
        }
    }
    return NULL;
}

/*--------------------------------------------
 * Func : _hdmi_vic_to_vic
 *
 * Desc : convert hdmi vic to vic.
 *        hdmi vic is a VIC that defined by HDMI 
 *        forum for 4Kx2K resolution extension. 
 *        For detail information, please check HDMI spec 1.4b 
 *        section 8.2.3.1
 *
 * Para : hdmi_vic : (I) hdmi_vic  
 *
 * Retn : 0 : unknow hdmi_vic
 *        others : associated vic value
 *-------------------------------------------*/
unsigned char _hdmi_vic_to_vic(unsigned char hdmi_vic)
{
    // mapping hdmi vic to CTA-861-G VIC
    // The mapping is specified on HDMI 2.0a table 10-2
    
    switch(hdmi_vic)
    {
    case 1:  return 95;
    case 2:  return 94;
    case 3:  return 93;
    case 4:  return 98;
    default: return 0;
    }
}


/*--------------------------------------------
 * Func : _svd_to_vic
 *
 * Desc : convert svd (short Video) to vic.  
 *
 * Para : svd : (I) SVD  
 *
 * Retn : vic : vic number 
 *                   svd              vic
 *                   0~128         0~128     (not native video format)
 *                   129~192     1~64       (      native video format)
 *                   193~255     193~255  (not native video format)
 *-------------------------------------------*/
unsigned char _svd_to_vic(unsigned char svd)
{
    if(svd>=129 && svd<=192)
    {
        return (svd & 0x7F);
    }
    else
    {
        return svd;
    }
}

/*--------------------------------------------
 * Func : _vic_to_svd
 *
 * Desc : convert vic to svd(short Video Descriptor).
 *
 * Para : vic : (I) vic (129 ~ 192 not used)
 *                  (I) property : VIC_NOT_NATIVE => not native video format, VIC_NATIVE => native video format (only vic between 1 ~ 64 has this attribute)
 * Retn : svd : svd number , 0: vic=0 or illegal vic
 *                   vic              svd
 * not native:  0~128        0~128
 *                   129~192    0
 *                   193~255    193~255
 *
 *       native:  1~64         129~192
 *                   0,65~255   0
 *-------------------------------------------*/
unsigned char _vic_to_svd(unsigned char vic, VIC_PROPERTY property)
{
    if(property == VIC_NATIVE)
    {
        if(vic>=1 && vic<=64)
        {
            return (vic |0x80);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        if(((vic>=0)&&(vic<=128))||((vic>=193)&&(vic<=255)))
        {
            return vic;
        }
        else
        {
            return 0;
        }
    }
}


/*--------------------------------------------
 * Func : _parse_cea_ext_blk_info
 *
 * Desc : extrat cea ext block information of the 
 *        given EDID. the cea ext block info will
 *        find out each field of the given CEA 
 *        ext block. to ease data analysis and 
 *        modification      
 *
 * Para : edid : (I) EDID
 *        p_info : (O) CEA_EXT_BLOCK info output
 *
 * Retn : 0 : successed, -1 : failed
 *-------------------------------------------*/
int _parse_cea_ext_blk_info(unsigned char* p_edid, CEA_EXT_BLK_INFO* p_info)
{
    if (p_edid==NULL || p_info==NULL)
    {
        HDMI_EDID_DBG("Parse hdmi edid failed, invalid parameters\n");
        return -1;
    }

    memset(p_info, 0, sizeof(CEA_EXT_BLK_INFO));

    p_info->hdr = *((CEA_EXT_HDR*) p_edid);

    HDMI_EDID_DBG("_parse_cea_ext_blk_info : EDID (%p) = %02x %02x %02x %02x ...\n", p_edid, p_edid[0], p_edid[1], p_edid[2], p_edid[3]);
    HDMI_EDID_DBG("_parse_cea_ext_blk_info : Tag : %d, Rev=%d, dtd_ofst=%d", p_info->hdr.tag, p_info->hdr.revision, p_info->hdr.dtd_ofst);
    HDMI_EDID_DBG("_parse_cea_ext_blk_info : undescan : %d, audio=%d, YCbCr444=%d, YCbCr422=%d, n_dtd=%d\n",
                p_info->hdr.undescan, p_info->hdr.audio, p_info->hdr.YCbCr444, p_info->hdr.YCbCr422,
                p_info->hdr.n_dtd);

    if (p_info->hdr.tag != 0x2 || p_info->hdr.revision != 0x3)
    {
        HDMI_EDID_DBG("Parse hdmi edid failed, invalid parameters\n");
        return -1;
    }

    if (p_info->hdr.dtd_ofst >=4)
    {
        unsigned char cdb_size = p_info->hdr.dtd_ofst - 4;
        int i;
        CEA_DATA_BLOCK* p_cdb;

        if ((p_info->hdr.dtd_ofst + (sizeof(CEA_DTD_BLK) * p_info->hdr.n_dtd)) > 127 )
        {
            HDMI_EDID_DBG("Parse hdmi edid failed, check dtd size failed (dtd_ofst=%d, p_info->hdr.n_dtd=%d (%d))\n",
                    p_info->hdr.dtd_ofst, p_info->hdr.n_dtd, sizeof(CEA_DTD_BLK));
            return -1;
        }

        // cea data block collection
        p_cdb = (CEA_DATA_BLOCK*) &p_edid[4];

        while (cdb_size)
        {
            unsigned char cdb_len = CEA_DATA_BLOCK_SIZE(p_cdb);

            if (cdb_len > cdb_size)
            {
                HDMI_EDID_DBG("Parse hdmi edid failed, parse cea data block failed, incomplete cea data block\n");
                break;
            }

            HDMI_EDID_DBG("_parse_cea_ext_blk_info : CDB[%d] = %p\n", p_info->n_cdb, p_cdb);
            p_info->p_cdb[p_info->n_cdb++] = p_cdb;
            p_cdb = (CEA_DATA_BLOCK*)((unsigned char*) p_cdb + cdb_len);
            cdb_size -= cdb_len;
        }

        // record address of dtd blocks
        for (i=0; i< p_info->hdr.n_dtd; i++)
        {
            unsigned int edid_index = p_info->hdr.dtd_ofst + (i * sizeof(CEA_DTD_BLK));
            if(edid_index <(MAX_EDID_LENGTH -128))
                p_info->p_dtd[i] = (CEA_DTD_BLK*) &p_edid[edid_index];
            HDMI_EDID_DBG("_parse_cea_ext_blk_info : DTD[%d] = %p\n", i, p_info->p_dtd[i]);
        }
    }
    else if (p_info->hdr.dtd_ofst==0)
    {
        if (p_info->hdr.n_dtd !=0)
        {
            HDMI_EDID_DBG("Parse hdmi edid failed, invalid parameters, n_dtd shoulbe be 0 when dtd_ofst is 0\n");
            return -1;
        }
        // no CEC data block collections, no nothing
    }
    else if (p_info->hdr.dtd_ofst > 0)
    {
        HDMI_EDID_DBG("Parse hdmi edid failed, invalid parameters, n_dtd shoulbe be 0 when dtd_ofst is 0\n");
        return -1;
    }

    p_info->checksum = p_edid[127];
    return 0;
}



/*--------------------------------------------
 * Func : _dump_cea_ext_blk_info
 *
 * Desc : dump information of a given cea ext block  
 *
 * Para : p_info : (I) CEA_EXT_BLOCK info
 *
 * Retn : N/A
 *-------------------------------------------*/
void _dump_cea_ext_blk_info(CEA_EXT_BLK_INFO* p_info)
{
    int i;

    HDMI_EDID_DBG("_dump_cea_ext_blk_info : Tag=%d, Rev=%d, dtd_ofst=%d", p_info->hdr.tag, p_info->hdr.revision, p_info->hdr.dtd_ofst);

    HDMI_EDID_DBG("_dump_cea_ext_blk_info : undescan : %d, audio=%d, YCbCr444=%d, YCbCr422=%d, n_dtd=%d\n",
                p_info->hdr.undescan, p_info->hdr.audio, p_info->hdr.YCbCr444, p_info->hdr.YCbCr422,
                p_info->hdr.n_dtd);

    HDMI_EDID_DBG("_dump_cea_ext_blk_info : n_cdb=%d,\n", p_info->n_cdb);

    for (i=0; i<p_info->n_cdb; i++)
    {
        CEA_DATA_BLOCK* p_cdb = p_info->p_cdb[i];

        if (p_cdb)
            HDMI_EDID_DBG("_dump_cea_ext_blk_info : CDB[%d] : tag=%d, length=%d\n", i, p_cdb->tag, p_cdb->length);
    }

    for (i=0; i<p_info->hdr.n_dtd; i++)
    {
        CEA_DTD_BLK* p_dtd = p_info->p_dtd[i];

        if (p_dtd)
        {
            HDMI_EDID_DBG("_dump_cea_ext_blk_info : DTD[%d] : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
                i, p_dtd->data[0], p_dtd->data[1], p_dtd->data[2], p_dtd->data[3], p_dtd->data[4], p_dtd->data[5], p_dtd->data[6],
                p_dtd->data[7], p_dtd->data[8], p_dtd->data[9], p_dtd->data[10], p_dtd->data[11], p_dtd->data[12], p_dtd->data[13],
                p_dtd->data[14], p_dtd->data[15], p_dtd->data[16], p_dtd->data[17]);
        }
    }

    HDMI_EDID_DBG("_dump_cea_ext_blk_info : CheckSum : %02x\n", p_info->checksum);
}


/*--------------------------------------------
 * Func : _compute_checksum
 *
 * Desc : compute checksum value a given data  
 *
 * Para : p_data : (I) p_data
 *        len    : (I) len
 *
 * Retn : 8 bits sum of the input data
 *-------------------------------------------*/
unsigned char _compute_checksum(unsigned char* p_data, unsigned char len)
{
    unsigned char cs = 0;

    while(len--)
        cs += *p_data++;

    return cs;
}


/*--------------------------------------------
 * Func : _parse_vsdb_info
 *
 * Desc : extrat information of a given VSDB
 *
 * Para : edid : (I) EDID
 *        p_info : (O) CEA_EXT_BLOCK info output
 *
 * Retn : 0 : successed, -1 : failed
 *-------------------------------------------*/
int _parse_vsdb_info(CEA_DATA_BLOCK *p_vsdb, CEA_HDMI_VSDB_INFO* p_info)
{
    unsigned char* p_data = &p_vsdb->data[8];

    memset(p_info, 0, sizeof(CEA_HDMI_VSDB_INFO));

    p_info->p_hdr = (CEA_HDMI_VSDB_HEAD*) p_vsdb;

    if (p_info->p_hdr->tag !=0x3 ||
        p_vsdb->length < 5 ||
        p_info->p_hdr->oui[0]!=0x03 ||
        p_info->p_hdr->oui[1]!=0x0C ||
        p_info->p_hdr->oui[2]!=0x00)
    {
        HDMI_EDID_DBG("_parse_vsdb_info failed, check tag & oui failed\n");
        return -1;
    }

    if (p_vsdb->length < 8)
        return 0;

    if (p_info->p_hdr->latency_present)
    {
        p_info->p_latency= (CEA_HDMI_VSDB_LATENCY*) p_data;
        p_data += sizeof(CEA_HDMI_VSDB_LATENCY);
    }

    if (p_info->p_hdr->i_latency_present)
    {
        p_info->p_i_latency= (CEA_HDMI_VSDB_LATENCY*) p_data;
        p_data += sizeof(CEA_HDMI_VSDB_LATENCY);
    }

    if (p_info->p_hdr->hdmi_video_present)
    {
        p_info->p_video= (CEA_HDMI_VSDB_VIDEO_HDR*) p_data;
        p_data += sizeof(CEA_HDMI_VSDB_VIDEO_HDR);

        if (p_info->p_video->hdmi_vic_len)
        {
            p_info->p_hdmi_vic = p_data;
            p_data += p_info->p_video->hdmi_vic_len;
        }

        if (p_info->p_video->hdmi_3d_len)
        {
            p_info->p_hdmi_3d_data = p_data;
            p_data += p_info->p_video->hdmi_3d_len;
        }
    }

    if (p_data > p_vsdb->data + p_vsdb->length)
    {
        HDMI_EDID_DBG("_parse_vsdb_info failed, incomplete vsdb\n");
        return -1;
    }

    return 0;
}


/*--------------------------------------------
 * Func : _dump_vsdb_info
 *
 * Desc : dump inforamtion of a Vendor Specific 
 *        Data Block
 *
 * Para : p_info : (I) VSDB information
 *
 * Retn : N/A
 *-------------------------------------------*/
void _dump_vsdb_info(CEA_HDMI_VSDB_INFO* p_info)
{
    CEA_HDMI_VSDB_HEAD* p_hdr = p_info->p_hdr;
    HDMI_EDID_INFO("_dump_vsdb_info : tag=%02x, len=%d, oui=%02x%02x%02x, PA=%d.%d.%d.%d\n",
        p_hdr->tag, p_hdr->length,
        p_hdr->oui[0], p_hdr->oui[1], p_hdr->oui[2],
        p_hdr->A, p_hdr->B, p_hdr->C, p_hdr->D);

    if (p_info->p_hdmi_vic)
        HDMI_EDID_INFO("_dump_vsdb_info : hdmi_vic_len=%d\n", 
            p_info->p_video->hdmi_vic_len);

    if (p_info->p_hdmi_3d_data)
        HDMI_EDID_INFO("_dump_vsdb_info : hdmi_3d_len=%d\n", 
            p_info->p_video->hdmi_3d_len);
}


/*--------------------------------------------
 * Func : _parse_dtd_info
 *
 * Desc : extraction information of a given DTD
 *
 * Para : p_dtd  : (I) DTD data block
 *        p_info : (O) DTD information output
 *
 * Retn : 0 : success
 *-------------------------------------------*/
int _parse_dtd_info(CEA_DTD_BLK* p_dtd, CEA_DTD_INFO* p_info)
{
    unsigned char* data = p_dtd->data;

    p_info->pixel_clock = (data[1]<<8) + data[0];
    p_info->h_active = (DTD_H_ACT_H(data[4])<<8) + data[2];
    p_info->h_blank  = (DTD_H_BLANKING_H(data[4])<<8) + data[3];
    p_info->v_active = (DTD_V_ACT_H(data[7])<<8) + data[5];
    p_info->v_blank  = (DTD_V_BLANKING_H(data[7])<<8) + data[6];
    return 0;
}


/*--------------------------------------------
 * Func : _gen_hdmi_edid
 *
 * Desc : generate hdmi edid via cea ext block info
 *
 * Para : p_info : (I) CEC ext block information
 *        edid_out : (O) EDID output 
 *
 * Retn : 0 : success
 *-------------------------------------------*/
int _gen_hdmi_edid(CEA_EXT_BLK_INFO* p_info, unsigned char edid_out[128])
{
    unsigned char   tmp[128];
    CEA_EXT_HDR*    p_hdr = (CEA_EXT_HDR*) tmp;
    CEA_DATA_BLOCK* p_cdb = (CEA_DATA_BLOCK*)(tmp + sizeof(CEA_EXT_HDR));
    CEA_DTD_BLK*    p_dtd;
    int i;

    memset(tmp, 0, sizeof(tmp));

    //-------------------------------------
    // copy header
    //-------------------------------------
    *p_hdr = p_info->hdr;
    p_hdr->dtd_ofst = sizeof(CEA_EXT_HDR); // init dtd ofset
    p_hdr->n_dtd = 0;

    //-------------------------------------
    // copy cdb collections
    //-------------------------------------
    if (p_info->n_cdb)
    {
        for (i=0; i<p_info->n_cdb; i++)
        {
            HDMI_EDID_DBG("copy CDB[%d/%d]=%p, tag=%d, len=%d, dtd_ofst=%d\n",
                    i, p_info->n_cdb, p_info->p_cdb[i], p_info->p_cdb[i]->tag,
                    CEA_DATA_BLOCK_SIZE(p_info->p_cdb[i]), p_hdr->dtd_ofst );

            if (p_info->p_cdb[i])
            {
                unsigned char cdb_len = CEA_DATA_BLOCK_SIZE(p_info->p_cdb[i]);
                memcpy(p_cdb, p_info->p_cdb[i], cdb_len);
                p_hdr->dtd_ofst += cdb_len;     // update dtd offset
                p_cdb = (CEA_DATA_BLOCK*) (tmp + p_hdr->dtd_ofst);
            }
        }
    }

    //-------------------------------------
    // copy dtds
    //-------------------------------------
    p_dtd = (CEA_DTD_BLK*) (tmp + p_hdr->dtd_ofst);

    for (i=0; i<6; i++)
    {
        if (p_info->p_dtd[i])
        {
            *p_dtd++ = *p_info->p_dtd[i];  // copy dtd
            p_hdr->n_dtd++;
        }
    }

    //-------------------------------------
    // compute checksum
    //-------------------------------------
    tmp[127] = (0x100 - _compute_checksum(tmp, 127)) & 0xFF;

    memcpy(edid_out, tmp, 128);
    return 0;
}

