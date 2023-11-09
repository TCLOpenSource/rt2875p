#ifndef __RTK_MODULE_EVENT_MIRACAST_S_H__
#define __RTK_MODULE_EVENT_MIRACAST_S_H__
/*
######################################################################################
# MIRACAST EVENT STRING DEFINE
######################################################################################
*/
static struct event_event_s android_negotiation_event[]={
    {NEGOTIATION_CONNECT, "NEGOTIATION_CONNECT"},
    {NEGOTIATION_RTSP, "NEGOTIATION_RTSP"},
    {NEGOTIATION_HDCP, "NEGOTIATION_HDCP"},
    {NEGOTIATION_RTP, "NEGOTIATION_RTP"},
    {NEGOTIATION_DISCONNECT, "NEGOTIATION_DISCONNECT"},
    {EVENT_LIMIT_MAX,NULL},
};

static struct event_event_s android_rtppacket_event[]={
    {RTPPACKET_FIRSTPACKET, "RTPPACKET_FIRSTPACKET"},
    {RTPPACKET_LOST, "RTPPACKET_LOST"},
    {RTPPACKET_NEEDDECRYPT, "RTPPACKET_NEEDDECRYPT"},
    {EVENT_LIMIT_MAX,NULL},
};

static struct event_event_s android_decryption_event[]={
    {DECRYPTION_START, "DECRYPTION_START"},
    {DECRYPTION_STOP, "DECRYPTION_STOP"},
    {EVENT_LIMIT_MAX,NULL},
};

static struct event_event_s android_parser_event[]={
    {PARSER_AUDIO_TRACK_FOUND, "PARSER_AUDIO_TRACK_FOUND"},
    {PARSER_VIDEO_TRACK_FOUND, "PARSER_VIDEO_TRACK_FOUND"},
    {PARSER_AUDIO_FIRST_SAMPLE, "PARSER_AUDIO_FIRST_SAMPLE"},
    {PARSER_VIDEO_FIRST_FRAME, "PARSER_VIDEO_FIRST_FRAME"},
    {PARSER_AUDIO_BUF_FULL, "PARSER_AUDIO_BUF_FULL"},
    {PARSER_VIDEO_BUF_FULL, "PARSER_VIDEO_BUF_FULL"},
    {EVENT_LIMIT_MAX,NULL},
};

static struct event_event_s android_audio_event[]={
    {AUDIO_DECODE_FIRSTINPUT, "AUDIO_DECODE_FIRSTINPUT"},
    {AUDIO_DECODE_FIRSTOUTPUT, "AUDIO_DECODE_FIRSTOUTPUT"},
    {AUDIO_RENDER_OUTPUT, "AUDIO_RENDER_OUTPUT"},
    {EVENT_LIMIT_MAX,NULL},
};

static struct event_event_s android_video_event[]={
    {VIDEO_DECODE_FIRSTINPUT, "VIDEO_DECODE_FIRSTINPUT"},
    {VIDEO_DECODE_FIRSTOUTPUT, "VIDEO_DECODE_FIRSTOUTPUT"},
    {VIDEO_RENDER_OUTPUT, "VIDEO_RENDER_OUTPUT"},
    {EVENT_LIMIT_MAX,NULL},
};
/*
######################################################################################
# MIRACAST EVENT TYPE STRING DEFINE
######################################################################################
*/

static struct event_eventtype_s android_miracast_eventtype[]={
    {CAST_TYPE_NEGOTIATION, "CAST_TYPE_NEGOTIATION", android_negotiation_event},
    {CAST_TYPE_RTPPACKET, "CAST_TYPE_RTPPACKET", android_rtppacket_event},
    {CAST_TYPE_DECRYPTION, "CAST_TYPE_DECRYPTION", android_decryption_event},
    {CAST_TYPE_MEDIAPARSER, "CAST_TYPE_MEDIAPARSER", android_parser_event},
    {CAST_TYPE_AUDIO, "CAST_TYPE_AUDIO", android_audio_event},
    {CAST_TYPE_VIDEO, "CAST_TYPE_VIDEO", android_video_event},
    {EVENT_LIMIT_MAX,NULL,NULL},
};
#endif /* __RTK_MODULE_EVENT_MIRACAST_S_H__ */