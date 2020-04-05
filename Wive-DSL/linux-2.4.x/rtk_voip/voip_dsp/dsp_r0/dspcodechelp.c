#include "codec_descriptor.h"

/*
 * Calculate number of frames in current packet
 */
uint32 G723GetNumberOfFramesInCurrentPacket( const unsigned char *pBuffer,
											 uint32 nSize, 
											 int TempLen,
											 int TempLenSID,
											 int *pbAppendixSID )
{
	/* 
	 * ITU G.723.1 (page 1) says, "It is possible to switch between
	 * the two rates at any 30ms frame boundary."
	 */
	uint32 nFrames = 0;
	
	while( nSize > 0 ) {
		if( pBuffer[ 0 ] & 0x02 ) {			// SID
			*pbAppendixSID = 1;
			break;
		} else if( pBuffer[ 0 ] & 0x01 )	// Voice: 5.3k
			TempLen = 20;
		else								// Voice: 6.4k
			TempLen = 24;
		
		nFrames ++;
		nSize -= TempLen;
		pBuffer += TempLen;
	}
	
	return nFrames;
}

uint32 GetNumberOfFramesInCurrentPacket( const unsigned char *pBuffer,
										 uint32 nSize, 
										 int TempLen,
										 int TempLenSID,
										 int *pbAppendixSID )
{
	uint32 nFrames = 0;

	while( nSize > 0 ) {
#ifdef SUPPORT_APPENDIX_SID
		if( nSize >= TempLen )
			nFrames ++;
		else if( nSize == TempLenSID ) {
			*pbAppendixSID = 1;	// NOTE: Appendix SID is not included in i!
			break;
		} else
			break; 		
#else
		nFrames ++;
#endif // SUPPORT_APPENDIX_SID
		nSize -= TempLen;
	}
	
	return nFrames;
}

/*
 * Get frame infomation of frame length
 */
uint32 G711GetFrameInfo_FrameLength( const unsigned char *pBuffer )
{
	return 80;
}

#ifdef CONFIG_RTK_VOIP_G729AB
uint32 G729GetFrameInfo_FrameLength( const unsigned char *pBuffer )
{
	return 10;
}
#endif

#ifdef CONFIG_RTK_VOIP_G726
uint32 G72616GetFrameInfo_FrameLength( const unsigned char *pBuffer )
{
	return 20;
}

uint32 G72624GetFrameInfo_FrameLength( const unsigned char *pBuffer )
{
	return 30;
}

uint32 G72632GetFrameInfo_FrameLength( const unsigned char *pBuffer )
{
	return 40;
}
		
uint32 G72640GetFrameInfo_FrameLength( const unsigned char *pBuffer )
{
	return 50;
}
#endif /* CONFIG_RTK_VOIP_G726 */

#ifdef CONFIG_RTK_VOIP_G7231
uint32 G723GetFrameInfo_FrameLength( const unsigned char *pBuffer )
{
	if( pBuffer[ 0 ] & 0x01 )	// RATE FLAG
		return 20;	// 1: 5.3k
	else
		return 24;	// 0: 6.3k		
}
#endif

#ifdef CONFIG_RTK_VOIP_GSMFR
uint32 GSMfrGetFrameInfo_FrameLength( const unsigned char *pBuffer )
{
	return 33;
}
#endif

#ifdef CONFIG_RTK_VOIP_T38
uint32 T38GetFrameInfo_FrameLength( const unsigned char *pBuffer )
{
	/* This is a dummy function */
	return 80;
}
#endif

/*
 * Get frame infomation of SID length
 */
uint32 G711GetFrameInfo_SidLength( uint32 nSize )
{
	// According to ITU G711 appendix II: SID length = M + 1 bytes (M >=0)
	return nSize - (nSize/80)*80;
}

#ifdef CONFIG_RTK_VOIP_G729AB
uint32 G729GetFrameInfo_SidLength( uint32 nSize )
{
	return 2;
}
#endif

#ifdef CONFIG_RTK_VOIP_G726
uint32 G72616GetFrameInfo_SidLength( uint32 nSize )
{
	return nSize - (nSize/20)*20;
}

uint32 G72624GetFrameInfo_SidLength( uint32 nSize )
{
	return nSize - (nSize/30)*30;
}

uint32 G72632GetFrameInfo_SidLength( uint32 nSize )
{
	return nSize - (nSize/40)*40;
}

uint32 G72640GetFrameInfo_SidLength( uint32 nSize )
{
	return nSize - (nSize/50)*50;	
}			
#endif /* CONFIG_RTK_VOIP_G726 */

#ifdef CONFIG_RTK_VOIP_G7231
uint32 G723GetFrameInfo_SidLength( uint32 nSize )
{
	return 4;
}
#endif

#ifdef CONFIG_RTK_VOIP_GSMFR
uint32 GSMfrGetFrameInfo_SidLength( uint32 nSize )
{
	/* GSM has no SID frame!! */
	return 5;	/* impossible value */
}
#endif

#ifdef CONFIG_RTK_VOIP_T38
uint32 T38GetFrameInfo_SidLength( uint32 nSize )
{
	/* This is a dummy function */
	return nSize;	/* impossible value */
}
#endif

