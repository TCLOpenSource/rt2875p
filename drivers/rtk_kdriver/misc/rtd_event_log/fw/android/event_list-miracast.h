#ifndef __RTK_MODULE_EVENT_MIRACAST_H__
#define __RTK_MODULE_EVENT_MIRACAST_H__
/*
######################################################################################
# MIRACAST EVENT TYPE DEFINE
######################################################################################
*/
typedef enum
{
    CAST_TYPE_NEGOTIATION = 0,
    CAST_TYPE_RTPPACKET,
    CAST_TYPE_DECRYPTION,
    CAST_TYPE_MEDIAPARSER,
    CAST_TYPE_AUDIO,
    CAST_TYPE_VIDEO,
    MIRACAST_TYPE_ID_MAX
}MIRACAST_TYPE_ID;

/*
######################################################################################
# MIRACAST EVENT DEFINE
######################################################################################
*/

typedef enum  {
    NEGOTIATION_CONNECT = 0,
    NEGOTIATION_RTSP,
    NEGOTIATION_HDCP,
    NEGOTIATION_RTP,
    NEGOTIATION_DISCONNECT,
    NEGOTIATION_EVENT_ID_MAX
}NEGOTIATION_EVENT_ID;

typedef enum  {
    RTPPACKET_FIRSTPACKET = 0,
    RTPPACKET_LOST,
    RTPPACKET_NEEDDECRYPT,
    RTPPACKET_EVENT_ID_MAX
}RTPPACKET_EVENT_ID;

typedef enum  {
    DECRYPTION_START = 0,
    DECRYPTION_STOP,
    DECRYPTION_EVENT_ID_MAX
}DECRYPTION_EVENT_ID;

typedef enum  {
    PARSER_AUDIO_TRACK_FOUND = 0,
    PARSER_VIDEO_TRACK_FOUND,
    PARSER_AUDIO_FIRST_SAMPLE,
    PARSER_VIDEO_FIRST_FRAME,
    PARSER_AUDIO_BUF_FULL,
    PARSER_VIDEO_BUF_FULL,
    MEDIAPARSER_EVENT_ID_MAX
}MEDIAPARSER_EVENT_ID;

typedef enum  {
    AUDIO_DECODE_FIRSTINPUT = 0,
    AUDIO_DECODE_FIRSTOUTPUT,
    AUDIO_RENDER_OUTPUT,
    AUDIO_EVENT_ID_MAX
}AUDIO_EVENT_ID;

typedef enum  {
    VIDEO_DECODE_FIRSTINPUT = 0,
    VIDEO_DECODE_FIRSTOUTPUT,
    VIDEO_RENDER_OUTPUT,
    VIDEO_EVENT_ID_MAX
}VIDEO_EVENT_ID;

#endif /* __RTK_MODULE_EVENT_MIRACAST_H__ */