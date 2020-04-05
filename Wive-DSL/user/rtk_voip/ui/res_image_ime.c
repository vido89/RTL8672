#include "ui_config.h"
#include "res_image.h"

#ifdef LCD_COL_ORIENTED
const unsigned char Image_ime_indicator_PinYin[] = {
13, /* width */
13, /* height */
M_IMAGE_FLAGS_LIST( IMAGE_FLAGS_ORIENTED_COL | IAMGE_FLAGS_1BPP ), /* flags */
0xFF, 0x1F,
0x77, 0x17,
0x01, 0x10,
0xB7, 0x1F,
0xBF, 0x17,
0x77, 0x1B,
0x03, 0x1C,
0xB5, 0x1F,
0x7F, 0x1F,
0x77, 0x1F,
0x03, 0x10,
0x75, 0x1F,
0xFF, 0x1F,
};
#endif /* LCD_COL_ORIENTED */

#if defined( LCD_COL2_ORIENTED ) && ( defined( LANG_BIG5 ) || defined( LANG_GB2312 ) )
const unsigned char Image_ime_indicator_PinYin[] = {
13, /* width */
13, /* height */
M_IMAGE_FLAGS_LIST( IMAGE_FLAGS_ORIENTED_COL2 | IAMGE_FLAGS_1BPP ), /* flags */
0xFF, 0x77, 0x01, 0xB7, 0xBF, 0x77, 0x03, 0xB5, 0x7F, 0x77, 0x03, 0x75, 0xFF, 
0x1F, 0x17, 0x10, 0x1F, 0x17, 0x1B, 0x1C, 0x1F, 0x1F, 0x1F, 0x10, 0x1F, 0x1F,
};
#endif /* LCD_COL2_ORIENTED && ( LANG_BIG5 || LANG_GB2312 ) */

#if defined( LCD_COL2_ORIENTED ) && defined( LANG_BIG5 )
const unsigned char Image_ime_indicator_Phonetic[] = {
13, /* width */
13, /* height */
M_IMAGE_FLAGS_LIST( IMAGE_FLAGS_ORIENTED_COL2 | IAMGE_FLAGS_1BPP ), /* flags */
0xFF, 0xFF, 0xFF, 0xF3, 0xF7, 0xF7, 0xF7, 0xF7, 0xF7, 0x07, 0xFF, 0xFF, 0xFF, 
0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1B, 0x1B, 0x19, 0x1E, 0x1F, 0x1F, 0x1F, 
};
#endif /* LCD_COL2_ORIENTED && LANG_BIG5 */

#if defined( LCD_COL2_ORIENTED ) && defined( LANG_GB2312 )
const unsigned char Image_ime_indicator_WuBiHua[] = {
13, /* width */
13, /* height */
M_IMAGE_FLAGS_LIST( IMAGE_FLAGS_ORIENTED_COL2 | IAMGE_FLAGS_1BPP ), /* flags */
0xFF, 0xEF, 0xA7, 0x59, 0x53, 0x5B, 0x1B, 0xA7, 0xA9, 0xA3, 0xAB, 0xEB, 0xFF, 
0x1F, 0x1D, 0x1D, 0x1D, 0x1D, 0x1D, 0x18, 0x16, 0x16, 0x16, 0x16, 0x12, 0x1F, 
};
#endif /* LCD_COL2_ORIENTED && LANG_GB2312 */

#if defined( LCD_COL2_ORIENTED ) && defined( LANG_GB2312 )
const unsigned char Image_ime_indicator_WuBiHua_root1[] = {
8, /* width */
16, /* height */
M_IMAGE_FLAGS_LIST( IMAGE_FLAGS_ORIENTED_COL2 | IAMGE_FLAGS_1BPP ), /* flags */
0x00, 0x80, 0x80, 0x80, 0x80, 0xC0, 0x80, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};
#endif /* LCD_COL2_ORIENTED && LANG_GB2312 */

#if defined( LCD_COL2_ORIENTED ) && defined( LANG_GB2312 )
const unsigned char Image_ime_indicator_WuBiHua_root2[] = {
8, /* width */
16, /* height */
M_IMAGE_FLAGS_LIST( IMAGE_FLAGS_ORIENTED_COL2 | IAMGE_FLAGS_1BPP ), /* flags */
0x00, 0x00, 0x0C, 0xF8, 0x10, 0x00, 0x00, 0x00, 
0x00, 0x00, 0x10, 0x3F, 0x00, 0x00, 0x00, 0x00, 
};
#endif /* LCD_COL2_ORIENTED && LANG_GB2312 */

#if defined( LCD_COL2_ORIENTED ) && defined( LANG_GB2312 )
const unsigned char Image_ime_indicator_WuBiHua_root3[] = {
8, /* width */
16, /* height */
M_IMAGE_FLAGS_LIST( IMAGE_FLAGS_ORIENTED_COL2 | IAMGE_FLAGS_1BPP ), /* flags */
0x00, 0x00, 0x00, 0x00, 0x08, 0xFC, 0x00, 0x00, 
0x00, 0x00, 0x20, 0x10, 0x0E, 0x01, 0x00, 0x00, 
};
#endif /* LCD_COL2_ORIENTED && LANG_GB2312 */

#if defined( LCD_COL2_ORIENTED ) && defined( LANG_GB2312 )
const unsigned char Image_ime_indicator_WuBiHua_root4[] = {
8, /* width */
16, /* height */
M_IMAGE_FLAGS_LIST( IMAGE_FLAGS_ORIENTED_COL2 | IAMGE_FLAGS_1BPP ), /* flags */
0x00, 0x08, 0x30, 0xE0, 0xC0, 0x80, 0x00, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 
};
#endif /* LCD_COL2_ORIENTED && LANG_GB2312 */

#if defined( LCD_COL2_ORIENTED ) && defined( LANG_GB2312 )
const unsigned char Image_ime_indicator_WuBiHua_root5[] = {
8, /* width */
16, /* height */
M_IMAGE_FLAGS_LIST( IMAGE_FLAGS_ORIENTED_COL2 | IAMGE_FLAGS_1BPP ), /* flags */
0x00, 0x20, 0x20, 0x20, 0x20, 0xE0, 0xE0, 0x00,
0x00, 0x00, 0x00, 0x04, 0x02, 0x01, 0x00, 0x00, 
};
#endif /* LCD_COL2_ORIENTED && LANG_GB2312 */

#if defined( LCD_COL2_ORIENTED ) && defined( LANG_GB2312 )
const unsigned char Image_ime_indicator_WuBiHua_root6[] = {
8, /* width */
16, /* height */
M_IMAGE_FLAGS_LIST( IMAGE_FLAGS_ORIENTED_COL2 | IAMGE_FLAGS_1BPP ), /* flags */
0x00, 0x08, 0x0C, 0x04, 0xC4, 0x2C, 0x38, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00, 
};
#endif /* LCD_COL2_ORIENTED && LANG_GB2312 */

#ifdef LCD_COL2_ORIENTED
const unsigned char Image_ime_indicator_English[] = {
13, /* width */
13, /* height */
M_IMAGE_FLAGS_LIST( IMAGE_FLAGS_ORIENTED_COL2 | IAMGE_FLAGS_1BPP ), /* flags */
//0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xA3, 0xBD, 0xA3, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 
//0x1F, 0x1F, 0x1F, 0x1C, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1C, 0x1F, 0x1F, 0x1F, 
0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0x47, 0x7B, 0x47, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF, 
0x1F, 0x1F, 0x1F, 0x19, 0x1E, 0x1F, 0x1F, 0x1F, 0x1E, 0x19, 0x1F, 0x1F, 0x1F, 
};
#endif /* LCD_COL2_ORIENTED */


