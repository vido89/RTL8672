#include <stdio.h>

typedef struct {
	char name[20];
	int (*func)(int, char **);
} TVoIPBox;

extern int dbg_main(int argc, char *argv[]);
extern int fskgen_main(int argc, char *argv[]);
extern int led_main(int argc, char *argv[]);
extern int phonerecord_main(int argc, char *argv[]);
extern int reg_main(int argc, char *argv[]);
extern int ram_main(int argc, char *argv[]);
extern int slic_reset_main(int argc, char *argv[]);
extern int switchmii_main(int argc, char *argv[]);
extern int test_main(int argc, char *argv[]);
extern int vmwigen_main(int argc, char *argv[]);
extern int voicerecord_main(int argc, char *argv[]);
extern int rtcp_main(int argc, char *argv[]);
extern int iphone_test_main(int argc, char *argv[]);
extern int ring_test_main(int argc, char *argv[]);

TVoIPBox voip_box[] = {
#ifdef VOIPBOX_DBG
	{"dbg", dbg_main},
#endif
#ifdef VOIPBOX_FSKGEN
	{"fskgen", fskgen_main},
#endif
#ifdef VOIPBOX_LED
	{"led", led_main},
#endif
#ifdef VOIPBOX_PHONERECORD
	{"phonerecord", phonerecord_main},
#endif
#ifdef VOIPBOX_REG
	{"reg", reg_main},
#endif
#ifdef VOIPBOX_SLIC_RESET
	{"slic_reset", slic_reset_main},
#endif
#ifdef VOIPBOX_SWITCHMII
	{"switchmii", switchmii_main},
#endif
#ifdef VOIPBOX_TEST
	{"crash", test_main},
#endif
#ifdef VOIPBOX_VMWIGEN
	{"vmwigen", vmwigen_main},
#endif
#ifdef VOIPBOX_VOICERECORD
	{"voicerecord", voicerecord_main},
#endif
#ifdef VOIPBOX_RTCP_STATISTIC
	{"rtcp_statistic", rtcp_main},
#endif
#ifdef CONFIG_RTK_VOIP_DRIVERS_IP_PHONE
	{"iphone_test", iphone_test_main},
#endif
#ifdef VOIPBOX_RING_TEST
	{"ring_test", ring_test_main},
#endif
	{"", NULL}
};

int main(int argc, char *argv[])
{
	int i;

	for (i=0; voip_box[i].func; i++)
	{
		if (strcmp(argv[0], voip_box[i].name) == 0)
		{
			return voip_box[i].func(argc, argv);
		}
	}

	printf("voip box: cmd %s is not support\n", argv[0]);
	return 0;
}

