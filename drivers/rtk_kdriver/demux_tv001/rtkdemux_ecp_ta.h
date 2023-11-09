#ifndef RTKDEMUX_TA
#define RTKDEMUX_TA
#include "rtkdemux.h"

typedef enum name
{
	DEV_ITEM_TOTAL_DROPPING_PIN,                //0
	DEV_ITEM_PINMAP,                            //1
}DEV_ITEM_ID;

typedef enum{
	CH_ITEM_STATE,                          //0
	CH_ITEM_TUNER_ID,                       //1
	CH_ITEM_TSPACKETSIZE,                   //2
	CH_ITEM_DEMUXTAGET_TABLE,               //3
	CH_ITEM_DEMUXTAGET_TABLECB,             //4
	CH_ITEM_ACTIVEPESTARGET,                //5
	CH_ITEM_PES_MAP,                        //6
	CH_ITEM_DEMUXIN,                        //7
	CH_ITEM_DEMUXOUT,                       //8
	CH_ITEM_NAVBUF,                         //9
	CH_ITEM_PININFO,                        //10
	CH_ITEM_PCRPID,                         //11
	CH_ITEM_DEMUXTARGET_INFO,               //12
	CH_ITEM_PCRPROCESSER,                   //13
	CH_ITEM_PCRTRACKING,                    //14
	CH_ITEM_AUDIO_AD_INFO,                  //15
	CH_ITEM_AUDIO_FORMATE_INFO,             //16
	CH_ITEM_AD_CHANNEL_TARGET,              //17
	CH_ITEM_TP_BUFFER_OFFSET,               //18
	CH_ITEM_TP_BUFFER,                      //19
	CH_ITEM_TP_SRC,                         //20
	CH_ITEM_DROP_FLAG,                      //21
	CH_ITEM_RINGFULL_CHECKTIMER,            //22
	CH_ITEM_START_STREAMING,                //23
	CH_ITEM_BS_DEC_BUFFER,                  //24
	CH_ITEM_BS_DEC_BUFFER_H,                //25
	CH_ITEM_IB_DEC_BUFFER,                  //26
	CH_ITEM_IB_DEC_BUFFER_H,                //27
	//CH_ITEM_VIDEO_DELIVER_SIZE,             //28
	//CH_ITEM_AUDIO_DELIVER_SIZE,             //29
}CH_ITEM_ID;

////////////////////////////////////////////////////////////////////////
int DEMUX_TA_ECP_INIT(demux_channel *pCh);
int DEMUX_TA_ECP_UNINIT(demux_channel *pCh);

int DEMUX_TA_ECP_inv_range(demux_channel *pCh, long pTPRelase, unsigned int len);
int DEMUX_TA_ECP_UpdatePininfo(demux_channel *pCh, unsigned int pin, DEMUX_STREAM_TYPE_T streamType, unsigned char bufferType, unsigned long buffer_Header, unsigned long buffer);
int DEMUX_TA_ECP_UpdatePESBufferinfo(demux_channel *pCh, unsigned int pin, unsigned int isSetup);
int DEMUX_TA_ECP_Parse(demux_channel *pCh, unsigned long demuxIn, unsigned long demuxOut);
int DEMUX_TA_ECP_DeliverData(demux_channel *pCh,unsigned long demuxOut, REFCLOCK *pRefClock);
int DEMUX_TA_ECP_DeliverPrivateInfo(demux_channel *pCh, int infoId, char* pInfo, unsigned int len, long ibBuf, long bsBuf);


#endif //RTKDEMUX_TA
