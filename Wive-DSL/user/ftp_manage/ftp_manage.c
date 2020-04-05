#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include "debug.h"

char bftpd_pid[42];
char bftpd_enable[42];
char bftpd_sel[42];

char *get_config_opt(FILE *fp_config, char *buf, const char *opt_name)
{
	char *ret;
	ret =NULL;
	if (fp_config == NULL)
	{
		DEBUG_PRINT("fp_config=NULL\n");
		return ret;
	}

	fseek(fp_config, 0, SEEK_SET);

	while ( fgets(buf, 40, fp_config) )
	{
		DEBUG_PRINT("fgets:%s\n", buf);
		if ( (strncmp(buf, opt_name, strlen(opt_name)) == 0) && (buf[strlen(opt_name)] == '=') )
		{
			ret = &(buf[strlen(opt_name)+1]);
			if ( (*ret) == 0x0a )
			{
				ret = NULL;
			}
			else
			{
				DEBUG_PRINT("get the opt: %s=%s\n", opt_name, ret);
			}
			break;
		}
	}

	return ret;
}

int main(int argc, char **argv)
{
	FILE *fp_config=NULL;
	FILE *fp_map=NULL;
	char *bftpd_pid_value;
	char *bftpd_enable_value;
	char *bftpd_sel_value;
	unsigned int usb_mask;
	unsigned int usb_sel;

	cdh_debug_init();
	fp_config = fopen("/var/run/bftpd/ftpdconfig", "r+");
	if (fp_config == NULL)
	{
		DEBUG_PRINT("Fail to open \"/var/run/bftpd/ftpdpassword\"!\n");
		cdh_debug_cleanup();

		// additional operation *****
		return -1;
	}
	DEBUG_PRINT("%s\n", "main...\n");
	// check the bftpd is exist?
	bftpd_pid_value = get_config_opt(fp_config, bftpd_pid, "bftpd_pid");
	DEBUG_PRINT("bftpd_pid_value=%s\n", bftpd_pid_value);

	// check the bftpd is enable?
	bftpd_enable_value = get_config_opt(fp_config, bftpd_enable, "ftp_enable");
	DEBUG_PRINT("bftpd_enable_value=%s\n", bftpd_enable_value);

	// get the usb selection?
	bftpd_sel_value = get_config_opt(fp_config, bftpd_sel, "ftp_root");
	DEBUG_PRINT("bftpd_sel_value=%s\n", bftpd_sel_value);

	if (sscanf(bftpd_sel_value, "%x", &usb_sel) < 0)
	{
		DEBUG_PRINT("Fail to convert string to hex integer\n");
		fclose(fp_config);
		fp_config=NULL;
		cdh_debug_cleanup();
		// additional operation *****
		return -1;
	}

	DEBUG_PRINT("usb_sel=%x\n", usb_sel);

	// check the usb1 is enable?
	// check the usb2 is enable?
	// check the usb... is enable?
	fp_map = fopen("/tmp/usb/mnt_map", "r");
	if (fp_map == NULL)
	{
	/*
		DEBUG_PRINT("Fail to open \"/tmp/usb/mnt_map\"!\n");
		fclose(fp_config);
		fp_config=NULL;
		cdh_debug_cleanup();
		return -1;
	*/
		usb_mask = 0;
	}
	else
	{
		fscanf(fp_map, "%x", &usb_mask);
		DEBUG_PRINT("%x", usb_mask);
		fclose(fp_map);
		fp_map = NULL;
	}

	if ( (usb_mask == 0) || (bftpd_enable_value == NULL) || ( ! strncmp(bftpd_enable_value, "0", 1) ) )
	{
		if ( (bftpd_pid_value) && (bftpd_pid_value[0] != 0) )
		{
			int ipid;
			long pos;
			if (sscanf(bftpd_pid_value, "%d", &ipid) < 0)
			{
				DEBUG_PRINT("Fail to convert ipid to integer\n");
			}
			else
			{
				char strtemp[64];
				DEBUG_PRINT("kill ... %d...\n", ipid);
				kill(ipid, SIGKILL);
				
				fseek(fp_config, 0, SEEK_SET);
				pos = ftell(fp_config);
				while ( fgets(strtemp, 62, fp_config) )
				{
					if ( ! strncmp(strtemp, "bftpd_pid=", 10) )
					{
	                    fseek(fp_config, pos, SEEK_SET);
						fprintf(fp_config, "bftpd_pid=\n\n");
						fflush(fp_config);
						printf("change bftpd_pid=\n");
						break;
					}
					
					pos = ftell(fp_config);
				}
			}
		}
	}
	else if ( ( usb_mask & (1 << usb_sel) )
	       && (bftpd_enable_value != NULL)
		   && ( strncmp(bftpd_enable_value, "0", 1) )
		    )
	{
		char command[64];

		// create the symbolic link
/*		if (usb_sel == 0)
		{
			sprintf(command, "/bin/ln -s -f /tmp/usb/sda /var/bftpd_root");
		}
		else if (usb_sel == 16)
		{
			sprintf(command, "/bin/ln -s -f /tmp/usb/sdb /var/bftpd_root");
		}
		else if (usb_sel < 16)
		{
			sprintf(command, "/bin/ln -s -f /mnt/usb1_%d /var/bftpd_root", usb_sel);
		}
		else if (usb_sel > 16)
		{
			sprintf(command, "/bin/ln -s -f /mnt/usb2_%d /var/bftpd_root", usb_sel-16);
		}
*/	
		sprintf(command, "/bin/ln -s -f /mnt /var/bftpd_root");

		system(command);

		// if nessary, start the bftpd
		if (bftpd_pid_value == NULL)
		{
			char *myopt[3] = {NULL, "-d", NULL};
			char *myprog = "/bin/bftpd";
			myopt[0] = myprog;
			// start the bftpd
			execv(myprog, myopt);
		}
	}
	else if ( (bftpd_enable_value != NULL)
		   && ( strncmp(bftpd_enable_value, "0", 1) )
		    )
	{
		char command[64];
		unsigned int i;

		// create the symbolic link
		usb_sel = 32;
		for (i=0; i<32; i++)
		{
			if ( usb_mask & (1 << i) )
			{
				usb_sel = i;
				break;
			}
		}

		if (usb_sel >= 32)
		{
			if (bftpd_pid_value)
			{
				int ipid;
				long pos;
				if (sscanf(bftpd_pid_value, "%d", &ipid) < 0)
				{
					DEBUG_PRINT("Fail to convert ipid to integer\n");
				}
				else
				{
					char strtemp[64];
					DEBUG_PRINT("kill %d...\n", ipid);
					kill(ipid, SIGKILL);
				
					fseek(fp_config, 0, SEEK_SET);
					pos = ftell(fp_config);
					while ( fgets(strtemp, 62, fp_config) )
					{
						if ( ! strncmp(strtemp, "bftpd_pid=", 10) )
						{
		                    fseek(fp_config, pos, SEEK_SET);
							fprintf(fp_config, "bftpd_pid=\n\n");
							fflush(fp_config);
							printf("change bftpd_pid=\n");
							break;
						}
						
						pos = ftell(fp_config);
					}
				}
			}
		}
		else
		{
/*
			if (usb_sel == 0)
			{
				sprintf(command, "/bin/ln -s -f /mnt/usb/sda /var/bftpd_root");
			}
			else if (usb_sel == 16)
			{
				sprintf(command, "/bin/ln -s -f /mnt/usb/sdb /var/bftpd_root");
			}
			else if (usb_sel < 16)
			{
				sprintf(command, "/bin/ln -s -f /mnt/usb1_%d /var/bftpd_root", usb_sel);
			}
			else if (usb_sel > 16)
			{
				sprintf(command, "/bin/ln -s -f /mnt/usb2_%d /var/bftpd_root", usb_sel-16);
			}
*/	
			sprintf(command, "/bin/ln -s -f /mnt /var/bftpd_root");
			
			system(command);

			// if nessary, start the bftpd
			if (bftpd_pid_value == NULL)
			{
				char *myopt[3] = {NULL, "-d", NULL};
				char *myprog = "/bin/bftpd";
				myopt[0] = myprog;
				// start the bftpd
				execv(myprog, myopt);

				// start the bftpd
				// system("/bin/bftpd -d");
			}
		}
	}

	// close the file 
	fclose(fp_config);
	fp_config=NULL;
	cdh_debug_cleanup();
	
	return 0;
}

