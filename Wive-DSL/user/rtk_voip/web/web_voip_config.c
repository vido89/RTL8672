#include <stdio.h>
#include "web_voip.h"

#if CONFIG_RTK_VOIP_PACKAGE_865X || CONFIG_RTK_VOIP_PACKAGE_867X
#define ERR_MSG(msg)	{ \
    websWrite(wp, T("<html>\n")); \
    websWrite(wp, T("<body><blockquote><h4>%s</h4>\n"), msg); \
    websWrite(wp, T("<form><input type=\"button\" onclick=\"history.go (-1)\" value=\"&nbsp;&nbsp;OK&nbsp;&nbsp\" name=\"OK\"></form></blockquote></body>")); \
    websWrite(wp, T("</html>\n")); \
}
#else
#include "apmib.h"
#include "apform.h"
#endif

#if CONFIG_RTK_VOIP_PACKAGE_865X
int asp_voip_ConfigGet(webs_t wp, int argc, char_t **argv)
#else
int asp_voip_ConfigGet(int ejid, webs_t wp, int argc, char_t **argv)
#endif
{
	voipCfgParam_t *pVoIPCfg;

	if (voip_flash_get(&pVoIPCfg) != 0)
		return -1;

	if (strcmp(argv[0], "mode_http")==0)
	{
		websWrite(wp, pVoIPCfg->auto_cfg_mode ? "checked" : "");
	}
	else if (strcmp(argv[0], "mode_disable")==0)
	{
		websWrite(wp, pVoIPCfg->auto_cfg_mode ? "" : "checked");
	}
	else if (strcmp(argv[0], "http_addr")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->auto_cfg_http_addr);
	}
	else if (strcmp(argv[0], "http_port")==0)
	{
		websWrite(wp, "%d", pVoIPCfg->auto_cfg_http_port);
	}
	else if (strcmp(argv[0], "file_path")==0)
	{
		websWrite(wp, "%s", pVoIPCfg->auto_cfg_file_path);
	}
	else if (strcmp(argv[0], "expire")==0)
	{
		websWrite(wp, "%d", pVoIPCfg->auto_cfg_expire);
	}
	else
	{
		return -1;
	}

	return 0;
}

void asp_voip_ConfigSet(webs_t wp, char_t *path, char_t *query)
{
	voipCfgParam_t *pVoIPCfg;

	if (voip_flash_get(&pVoIPCfg) != 0)
		return;

	if (strcmp(websGetVar(wp, T("reset"), T("")), "Reset") == 0)
	{
		voipCfgParam_t *pDefVoIPCfg;

		if (voip_flash_get_default(&pDefVoIPCfg) == 0)
		{
			memcpy(pVoIPCfg, pDefVoIPCfg, sizeof(*pVoIPCfg));
		}
		else
		{
			flash_voip_default(pVoIPCfg);
		}

		goto asp_voip_ConfigSet_done;
	}
	
	pVoIPCfg->auto_cfg_mode = atoi(websGetVar(wp, T("mode"), T("0")));
	strcpy(pVoIPCfg->auto_cfg_http_addr, websGetVar(wp, T("http_addr"), T("")));
	pVoIPCfg->auto_cfg_http_port = atoi(websGetVar(wp, T("http_port"), T("80")));
	strcpy(pVoIPCfg->auto_cfg_file_path, websGetVar(wp, T("file_path"), T("")));
	pVoIPCfg->auto_cfg_expire = atoi(websGetVar(wp, T("expire"), T("0")));

asp_voip_ConfigSet_done:

	system("killall autocfg.sh");
	system("killall sleep");
	system("/bin/autocfg.sh&");

	voip_flash_set(pVoIPCfg);
	web_restart_solar();
	websRedirect(wp, T("/voip_config.asp"));
}

void asp_voip_ConfigSave(webs_t wp, char_t *path, char_t *query)
{
	voipCfgAll_t cfg_all;
	voipCfgParam_t *pVoIPCfg;
	char *ptr, *buf;
	int buf_len;

	ptr = websGetVar(wp, T("save"), T(""));
	if (ptr[0] == 0)
		return;

	if (voip_flash_get(&pVoIPCfg) != 0)
		return;

	memset(&cfg_all, 0, sizeof(cfg_all));
	memcpy(&cfg_all.current_setting, pVoIPCfg, sizeof(voipCfgParam_t));
	cfg_all.mode |= VOIP_CURRENT_SETTING;
	if (flash_voip_export(&cfg_all, &buf, &buf_len, 1) != 0)
	{
		ERR_MSG("export failed\n");
		return;
	}

	websWrite(wp, "HTTP/1.0 200 OK\n");
	websWrite(wp, "Content-Type: application/octet-stream;\n");
	websWrite(wp, "Content-Disposition: attachment;filename=\"config_voip.dat\" \n");
	websWrite(wp, "Pragma: no-cache\n");
	websWrite(wp, "Cache-Control: no-cache\n");
	websWrite(wp, "\n");
	websWriteBlock(wp, buf, buf_len);
	free(buf);
}

#if CONFIG_RTK_VOIP_PACKAGE_865X || CONFIG_RTK_VOIP_PACKAGE_867X
void asp_voip_ConfigLoad(webs_t wp, char_t *path, char_t *query)
{
#define CTYPE_MULTIPART	"multipart/form-data"
#define BOUNDARY_KWD	"boundary="
	voipCfgParam_t *pVoIPCfg;
	struct stat statbuf;
	char *buf;
	char *p;
	char boundary[40];
	char *part, *end_part;
	int endOfHeader;
	int lenContent;
	voipCfgAll_t	*voipall;


	if (voip_flash_get(&pVoIPCfg) != 0)
	{
		return;
	}

	fstat(wp->post_data_fd, &statbuf);
	lseek(wp->post_data_fd, SEEK_SET, 0);
	fprintf(stderr, "config size=%d\n", statbuf.st_size);
	
	// ------------------------------------------
	// parse multipart form for file upload 
	// ------------------------------------------
	if (strncmp(wp->content_type, CTYPE_MULTIPART, strlen(CTYPE_MULTIPART)) != 0)
	{
		char err_msg[256];

		sprintf(err_msg, "import failed: not multipart form (content type = %s)\n", wp->content_type);
		ERR_MSG(err_msg);
		return;
	}

	p = strstr(wp->content_type, BOUNDARY_KWD);
	if (p == NULL)
	{
		ERR_MSG("import failed: boundary not found\n");
		return;
	}

	/* skip over the 'boundary=' part */
	p += strlen(BOUNDARY_KWD);
	snprintf(boundary, sizeof(boundary), "--%s", p);

	buf = (char *) malloc(statbuf.st_size + 1); // +1 for null terminated
	if (buf == NULL)
	{
		ERR_MSG("import failed (OOM)\n");
		return;
	}
	buf[statbuf.st_size + 1] = 0;

	read(wp->post_data_fd, buf, statbuf.st_size);

	part = strstr(buf, boundary);
	if (part == NULL)
	{
		ERR_MSG("import failed: part not found\n");
	free(buf);
		return;
	}

	part = strchr(part, '\n');
	if (part == NULL)
	{
		ERR_MSG("import failed: part is end part\n");
		free(buf);
		return;
}
	part++;

	// TODO: check Content-Disposition ?

	endOfHeader = 0;
	while (!endOfHeader)
	{
		part = strchr(part, '\n');
		if (part == NULL)
		{
			ERR_MSG("import failed: part is end part2\n");
			free(buf);
			return;
		}

		part++;
		if ((*part) == '\n') {
			part += 1;
			endOfHeader = 1;
		}
		else if (((*part) == '\r') && ((*(part+1)) == '\n')) {
			part += 2;
			endOfHeader = 1;
		}
	}

	if (!endOfHeader)
	{
		ERR_MSG("import failed: end header not found\n");
		free(buf);
		return;		
	}

	end_part = (char *) memmem(part, statbuf.st_size - ((int) part - (int) buf), 
		boundary, strlen(boundary));

	if (end_part == NULL)
	{
		ERR_MSG("import failed: end_part not found\n");
		free(buf);
		return;		
	}

	lenContent = (int) end_part - (int) part - 2; // -2 for \r\n

	// bug fixed, voip config can't upgrade.
	voipall = malloc(sizeof(voipCfgAll_t));
	if (!voipall)
	{
		ERR_MSG("out of memory !!\r\n");
		free(buf);
		return;
	}
	memset (voipall,0 ,sizeof(voipCfgAll_t));
	memcpy(&voipall->current_setting, pVoIPCfg , sizeof (voipCfgParam_t));
	voipall->mode = VOIP_CURRENT_SETTING;
	if (flash_voip_import(voipall, part, lenContent) != 0)
	{
		ERR_MSG("import failed\n");
		free(buf);
		return;
	}
	free(buf);

#if 0
	free(wp->post_file_name);
	close(wp->post_data_fd);
	wp->post_file_name = NULL;
	wp->post_data_fd = NULL;
#endif

	voip_flash_set(&voipall->current_setting);
	free(voipall);
	//voip_flash_set(pVoIPCfg);
	web_restart_solar();
	websRedirect(wp, T("/voip_config.asp"));
}

#else

void asp_voip_ConfigLoad(webs_t wp, char_t *path, char_t *query)
{
	voipCfgAll_t cfg_all;
	voipCfgParam_t *pVoIPCfg;

	if (websGetVar(wp, T("load"), T(""))[0] == 0)
		return;

	memset(&cfg_all, 0, sizeof(cfg_all));

	if (voip_flash_get(&pVoIPCfg) == 0)
	{
		memcpy(&cfg_all.current_setting, pVoIPCfg, sizeof(voipCfgParam_t));
		cfg_all.mode |= VOIP_CURRENT_SETTING;
	}
	
	if (voip_flash_get_default(&pVoIPCfg) == 0)
	{
		memcpy(&cfg_all.default_setting, pVoIPCfg, sizeof(voipCfgParam_t));
		cfg_all.mode |= VOIP_DEFAULT_SETTING;
	}
	
	if (flash_voip_import(&cfg_all, wp->postData, wp->lenPostData) != 0)
	{
		ERR_MSG("import failed\n");
		return;
	}

	voip_flash_write(&cfg_all);
	web_restart_solar();
	websRedirect(wp, T("/voip_config.asp"));
}

#endif

int web_voip_saveConfig()
{
	voipCfgAll_t cfg_all;
	voipCfgParam_t *pVoIPCfg;

	if (voip_flash_get(&pVoIPCfg) != 0)
		return -1;

	memset(&cfg_all, 0, sizeof(cfg_all));
	memcpy(&cfg_all.current_setting, pVoIPCfg, sizeof(voipCfgParam_t));
	cfg_all.mode |= VOIP_CURRENT_SETTING;
	return flash_voip_export_to_file(&cfg_all, VOIP_CONFIG_PATH, 1);
}

