#ifndef __FSK__
#define __FSK__

/* FSK Message Format

+---------------+------------------+------------------+--------------------+------------------+----------+
|Message Type|Message Length|Parameter Type|Parameter Length|Parameter Data|Checksum|
+---------------+------------------+------------------+--------------------+------------------+----------+
*/

// Message Type
#define FSK_MSG_CALLSETUP			0x80		// Call Set-up
#define FSK_MSG_MWSETUP				0x82		// Message Waiting (VMWI)
#define FSK_MSG_ADVICECHARGE		0x86		// Advice of Charge
#define FSK_MSG_SMS					0x89		// Short Message Service

// Parameter Type
#define FSK_PARAM_DATEnTIME			0x01		// Date and Time
#define FSK_PARAM_CLI				0x02		// Calling Line Identify (CLI) 
#define FSK_PARAM_ABSCLI			0x04		// Reason for absence of CLI 
#define FSK_PARAM_MW				0x0b		// Message Waiting 
#define FSK_PARAM_CLI_NAME			0x07		// Calling Line Identify (CLI) Name.

#define CID_STATE0 0		//the seizure state
#define CID_STATE1 1		//the mark state
#define CID_STATE2 2		//the message state

void sendProSLICID (char mode, unsigned char msg_type, char *str, char *str2, char *cid_name);

void genSoftFskCID (uint32 chid, char mode, unsigned char msg_type, char *str, char *str2, char *cid_name);

extern char fsk_spec_areas[];
extern char fsk_spec_mode;

#define SOFT_FSK_CID_BUF_LENGTH 256		// the fsk caller id buffer.
#define FSK_CIR_BUF_MASK 255			// 0xff the mask of index

__attribute__((aligned(8))) typedef struct
{
	int16 cid_buf[SOFT_FSK_CID_BUF_LENGTH];		// caller id buffer voice data
	uint32 buf_put_index;				// the put index of caller id buffer
	uint32 buf_get_index;				// the get index of caller id buffer
	uint32 buf_length;				// the data length of caller id buffer
	uint32 state;					// the_caller id gen state
	uint32 ch_seizure_bit_cnt;			// the seizure bit conut
	uint32 ch_seizure_bit_gen_cnt;			// the gen seizure bit conut
	uint32 mark_sig_bit_cnt;			// the mark bit count
	uint32 mark_sig_bit_gen_cnt;			// the gen mark bit count
	int8 cid_byte_data[SOFT_FSK_CID_BUF_LENGTH];	// the caller id byte data
	uint32 cid_byte_data_index;			// the caller id byte data send index
	uint32 fskphase;				// the fsk phase shift
	uint32 fskcnt;					// the fsk gen - baud state
	uint32 mode;					// on-hook or off-hook , type-I or type-II.
	uint32 setup_flag;				// 1:setup ok
	uint32 final_zero_cnt;				// output zero voice at end of fsk cid.
}
TstVoipSoftFskCidGen;

#endif

