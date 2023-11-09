#ifndef __PQ_RPC_H__
#define __PQ_RPC_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _FILM_FWMode
{
	unsigned char fw_StatusFilmSequence;
	unsigned char fw_Status32Detected1;
	unsigned char fw_Status32Detected2;
	unsigned char fw_Status22Detected;
	unsigned char Fw_StatusFilmWeaveMode;
	unsigned char Fw_MixedFilmConfuse;
	unsigned char Fw_StatusMixedFilmDetected;
	unsigned char Fw_StatusFilmDetected;
	unsigned int Fw_TopLine1;
	unsigned int Fw_BotLine1;
	unsigned int Fw_TopLine2;
	unsigned int Fw_BotLine2;
	unsigned int Fw_TopLine3;
	unsigned int Fw_BotLine3;

}FILM_FWMode;

#define FILM22 1
#define FILM32 2
#define FILM3223 4
#define FILM64 8
#define FILM55 16
#define FILM2224 32
#define FILM32322 64
#define FILM87 128

void scalerVIP_film_by_GMV(void);

void scalerVIP_film_FWmode(void);
FILM_FWMode DETECTION_scalerVIP_film_FWmode(void);
void APPLY_scalerVIP_film_FWmode(FILM_FWMode DetInformation);
void	scalerVIP_film_FWmode_detection1(unsigned int FrameStaticPattern,unsigned int FramePairPattern,
		unsigned int PrevStaticPattern, unsigned int NextStaticPattern, unsigned char Strict,
		unsigned char *ExactMatch, unsigned char *Violate, unsigned int *film_index,unsigned int *count_still,
		unsigned char *p32Detected1, unsigned int *p32Sequence1,unsigned char film_Pair_en_auto,
		unsigned char film_Pair1_en,unsigned char film_HiFreq_flag,unsigned char film_Violate_strict_en);

void scalerVIP_film_FWmode_detection2(unsigned int NextStaticPattern, unsigned char Strict,
								unsigned int FrameStaticPattern,
								unsigned char PrevFieldMotion, unsigned char NextFieldMotion,
							    unsigned char *ExactMatch, unsigned char *Violate, unsigned int *film_index,
								unsigned char *p32Detected2, unsigned int *p32Sequence2,unsigned char film_Violate_strict_en);

void	scalerVIP_film_FWmode_detection22(	unsigned int NextStaticPattern, unsigned char Strict,
								unsigned char PrevFieldMotion, unsigned char NextFieldMotion,
							    unsigned char *ExactMatch, unsigned char *Violate, unsigned int *film_index, unsigned int *count_still22,
								unsigned char *p22Detected, unsigned int *p22Sequence,unsigned char film_Violate_strict_en);

void	scalerVIP_film_FWmode_detection22new(	unsigned int NextStaticPattern, unsigned char Strict,
								unsigned char PrevFieldMotion, unsigned char NextFieldMotion,
							    unsigned char *ExactMatch, unsigned char *Violate, unsigned int *film_index, unsigned int *count_still22,
								unsigned char *p22Detected, unsigned int *p22Sequence);

void scalerVIP_film_FWmode_film32_debounce(unsigned char *Detected_flag, unsigned int *Detected_cnt,unsigned char FieldNum);
void scalerVIP_film_FWmode_film22_debounce(unsigned char *Detected_flag, unsigned int *Detected_cnt,unsigned char FieldNum);


#ifdef __cplusplus
}
#endif

#endif /* __PQ_RPC_H__*/


