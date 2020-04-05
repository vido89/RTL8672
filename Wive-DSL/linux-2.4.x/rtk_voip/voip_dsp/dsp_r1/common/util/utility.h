// jason++ 2005/04/04
// common routines

#ifndef __UTILITY_H
#define __UTILITY_H

// assembly routine defined in utility.S or g7231_exc_lbcs.S 

/* The Len parameter in memset32/64s and memcpy32/64s is counted by 2-bytes unit*/
void 	memset32s(Word32 *Dst, int c, Word32 Len);
void 	memset64s(Word32 *Dst, int c, Word32 Len);
void 	memcpy16s(Word16 *Dst, Word16 *Src, Word32 Len);
void 	memcpy32s(Word32 *Dst, Word32 *Src, Word32 Len);
void 	memcpy64s(Word32 *Dst, Word32 *Src, Word32 Len);

Word32	L_mac_loop1s(Word32 Acc, Word16 *Buff, Word16 Len);
Word32	L_mac_loop2s(Word32 Acc, Word16 *Buff1, Word16 *Buff2, Word16 Len);
Word32	L_msu_loops(Word32 Acc, Word16 *pCoe, Word16 *pDat, Word16 Len);
Word32	L_msu_loop1s(Word32 Acc, Word16 *pCoe, Word16 *pDat, Word16 Len);
void	mult_r_loop1s(Word16 *DstTable, Word16 *SrcTable, Word16 Len);
void	mult_r_loop2s(Word16 *DstTable, Word16 *Src1Table, Word16 *Src2Table, Word16 Len);
void 	shr_loop1s(Word16 *Dst, Word16 Exp, Word16 Len);
void 	shr_loops(Word16 *Dst, Word16 *Src, Word16 Exp, Word16 Len);

Word32 L_shl2(Word32 Acc);

Word16	L_shl1_rounds(Word32 Acc);	// round(L_shl(Acc, 1)) 
Word16	L_shl2_rounds(Word32 Acc);	// round(L_shl(Acc, 2)) 
Word16	L_shl2_L_mac_round(Word32 Acc, Word16 var1, Word16 var2);
Word16	L_shl2_extract_hs(Word32 Acc);
Word32	L_deposit_h_L_shrs(Word16 Corr, Word16 Exp);
Word16	L_deposit_h_L_mac_rounds(Word16 val1, Word16 val2, Word16 val3);
Word16	L_deposit_h_L_msu_rounds(Word16 val1, Word16 val2, Word16 val3);
Word32	L_shr_L_adds(Word32 Acc1, Word16 Exp);

void	i_mult_loops(Word16 *Dst, Word16 *Src1, Word16 Src2, Word16 Len);
Word32	L_add_imult_loops(Word32 val1, Word16 *Tv, Word16 *Imr, Word16 Len);

void 	add_loops(Word16 *Dst, Word16 *Src, Word16 Len);
void 	sub_loops(Word16 *Dst, Word16 *Src, Word16 Len);
void 	sub_loop2s(Word16 *Dst, Word16 *Src, Word16 Len);
void 	add_loop2s(Word16 *Dst, Word16 *Src1, Word16 *Src2, Word16 Len);
void	add_add_loops(Word16 *pExc, Word16 *Dpnt, Word16 *ImpResps, Word16 Len);

void Init_Pre_Process2(void);
void Pre_Process2(uint32 sid, Word16 *signal, Word16 lg);
void Pre_Process2s(uint32 sid, Word16 *signal, Word16 lg);
void Copy(Word16 x[], Word16 y[], Word16 L);

#endif
