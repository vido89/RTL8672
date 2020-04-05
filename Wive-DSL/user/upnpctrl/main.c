#include <stdio.h>
#include <net/if.h>
#include <netdb.h>
#include <string.h>
#include <rtk/sysconfig.h>
#include <rtk/utility.h>
#include <config/autoconf.h>
#include <signal.h>

char g_IpFlag[5];
char g_extInterfaceName[10];
char g_intInterfaceName[10];

#ifdef CONFIG_USER_UPNPD
// 2008-12-12
// Magician: UPnP Daemon is obsoleted, use MiniUPnP Daemon instead.
#endif

#ifdef CONFIG_USER_MINIUPNPD
int main(int argc, char **argv)
{
	FILE *fp, *fp2;
	char ext_ifname[6];
	unsigned char upnpdEnable, upnpItf;
	unsigned char value[6];
	unsigned char ipaddr[16];
	struct in_addr inAddr;
	char temp_ext_IP[16], temp_ext_IP2[16];
	int int_pid;

	if(argc != 4)
	{
		printf("Usage: upnpctrl <up|down> <external ifname> <internal ifname>\n");
		printf("Example: upnpctrl up ppp0 br0\n");
		printf("Example: upnpctrl down ppp0 br0\n");
		exit(0);
	}

	// Save the IpFlag and interface names for later uses
	strcpy(g_IpFlag, argv[1]);
	strcpy(g_extInterfaceName, argv[2]);
	strcpy(g_intInterfaceName, argv[3]);
	//printf("g_IpFlag=%s g_extInterfaceName=%s g_intInterfaceName=%s\n", g_IpFlag, g_extInterfaceName, g_intInterfaceName);

	if(getInAddr(g_extInterfaceName, IP_ADDR, (void *)&inAddr) == 1)
	{
		strncpy(temp_ext_IP, inet_ntoa(inAddr), 16);
		temp_ext_IP[15] = '\0';
		//printf("%s interface IP is %s\n", g_extInterfaceName, temp_ext_IP);
	}
	else
		strcpy(temp_ext_IP, "255.255.255.255");   // Assign fake WAN IP if that is not available.

	if(mib_get(MIB_UPNP_DAEMON, (void *)&upnpdEnable) != 0)
	{
		if( upnpdEnable != 1 ) // MINIUPNP Daemon is not enabled.
			exit(0);
	}
	else
		printf("upnpctrl: Get MIB_UPNP_DAEMON failed!\n");

	if(mib_get(MIB_ADSL_LAN_IP, (void *)value) != 0)
	{
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
		ipaddr[15] = '\0';
	}

	if(strcmp(g_IpFlag, "up") == 0)
	{
		// Mason Yu
		if((fp=fopen("/var/run/linuxigd.pid", "r")) == NULL)
		{
			if(!mib_get(MIB_UPNP_EXT_ITF, (void *)&upnpItf))
			{
				printf("upnpctrl(up): Get MIB_UPNP_EXT_ITF failed!\n");
				return 0;
			}

			if( upnpItf != 0xff )
			{
				if (PPP_INDEX(upnpItf) != 0x0f)
					snprintf(ext_ifname, 6, "ppp%u", PPP_INDEX(upnpItf));	// PPP interface
				else
					snprintf(ext_ifname, 5, "vc%u", VC_INDEX(upnpItf));

				if(strcmp(ext_ifname, g_extInterfaceName) == 0)
				{
					printf("upnpctrl(up): Start MINIUPNPD on %s interface\n", g_extInterfaceName);

					// We should add multicast route and INPUT relue for Netfilter.
					// route add -net 239.0.0.0 netmask 255.0.0.0 br0
					va_cmd("/bin/route", 6, 1, "add", "-net", "239.0.0.0", "netmask", "255.0.0.0", "br0");
					// iptables -I INPUT 1 -p TCP --dport 49152 -j ACCEPT
					va_cmd("/bin/iptables", 9, 1, "-I", "INPUT", "1", "-p", "TCP", "--dport", "49152", "-j", "ACCEPT");

					// iptables -t nat -N MINIUPNPD
					va_cmd("/bin/iptables", 4, 1, "-t", "nat", "-N", "MINIUPNPD");
					// iptables -t nat -A PREROUTING -d 192.168.8.4 -i vc0 -j MINIUPNPD
					va_cmd("/bin/iptables", 10, 1, "-t", "nat", "-A", "PREROUTING", "-d", temp_ext_IP, "-i", g_extInterfaceName, "-j", "MINIUPNPD");

					// iptables -t filter -N MINIUPNPD
					va_cmd("/bin/iptables", 4, 1, "-t", "filter", "-N", "MINIUPNPD");
					// iptables -t filter -I FORWARD 1 -i vc0 -o ! vc0 -j MINIUPNPD
					va_cmd("/bin/iptables", 12, 1, "-t", "filter", "-I", "FORWARD", "1", "-i", g_extInterfaceName, "-o", "!", g_extInterfaceName, "-j", "MINIUPNPD");

					// Start miniupnpd
					// miniupnpd -i vc0 -a 192.168.1.1 -p 49152
					va_cmd("/bin/miniupnpd", 6, 0, "-i", g_extInterfaceName, "-a", ipaddr, "-p", "49152");

					fp2=fopen("/var/upnp_ext_ip.txt", "w");
					fprintf(fp2, "%s\n", temp_ext_IP);
					fclose(fp2);

	#ifdef CONFIG_WIFI_SIMPLE_CONFIG
					int_pid = read_pid("/var/run/mini_upnpd.pid"); //cathy
					if (int_pid > 0)
					{
						kill(int_pid, 9);
						unlink("/var/run/mini_upnpd.pid");
						va_cmd("/bin/mini_upnpd", 5, 0, "-wsc", "/tmp/wscd_config", "-igd", "/tmp/igd_config", "&");
					}
					else
						va_cmd("/bin/mini_upnpd", 3, 0, "-igd", "/tmp/igd_config", "&");
	#endif
					// The end of miniupnpd starting.
				}
				else
					printf("upnpctrl(up): The %s interface is not binded for MINIUPNPD.\n", g_extInterfaceName);
			}
			else
			{
				printf("upnpctrl(up): MINIUPNP binded WAN interface not set(error)!\n");
				return;
			}
		}
		else
		{
			fclose(fp);
			printf("upnpctrl(up): MINIUPNP is start already!\n");
		}
	}
	else if(strcmp(g_IpFlag, "down") == 0)
	{
		if((fp=fopen("/var/run/linuxigd.pid", "r")) != NULL)
		{
			if(!mib_get(MIB_UPNP_EXT_ITF, (void *)&upnpItf))
			{
				printf("upnpctrl(down): Get MIB_UPNP_EXT_ITF failed!\n");
				return 0;
			}

			if( upnpItf != 0xff )
			{
				if(PPP_INDEX(upnpItf) != 0x0f)
					snprintf(ext_ifname, 6, "ppp%u", PPP_INDEX(upnpItf));	// PPP interface
				else
					snprintf(ext_ifname, 5, "vc%u", VC_INDEX(upnpItf));

				if( strcmp(ext_ifname, g_extInterfaceName) == 0 )
				{
					// Mason Yu
					fp2=fopen("/var/upnp_ext_ip.txt", "r");
					fscanf(fp2, "%s\n", temp_ext_IP2);
					fclose(fp2);
					//printf("upnpctrl(down): Old external IP=%s New external IP=%s \n", temp_ext_IP2, temp_ext_IP);

					fscanf(fp, "%d", &int_pid);  // Get miniupnpd pid.
					fclose(fp);

					if (int_pid > 0)
					{
						// Magician: we should delete all rules of miniupnp for instant effect.
						// route del -net 239.0.0.0 netmask 255.0.0.0 br0
						va_cmd("/bin/route", 6, 1, "del", "-net", "239.0.0.0", "netmask", "255.0.0.0", "br0");
						// iptables -D INPUT -p TCP --dport 49152 -j ACCEPT
						va_cmd("/bin/iptables", 8, 1, "-D", "INPUT", "-p", "TCP", "--dport", "49152", "-j", "ACCEPT");

						// iptables -t nat -D PREROUTING -d 192.168.8.4 -i vc0 -j MINIUPNPD
						va_cmd("/bin/iptables", 10, 1, "-t", "nat", "-D", "PREROUTING", "-d", temp_ext_IP2, "-i", g_extInterfaceName, "-j", "MINIUPNPD");
						// iptables -t nat -N MINIUPNPD
						va_cmd("/bin/iptables", 4, 1, "-t", "nat", "-X", "MINIUPNPD");

						// iptables -t filter -I FORWARD 1 -i vc0 -o ! vc0 -j MINIUPNPD
						va_cmd("/bin/iptables", 11, 1, "-t", "filter", "-D", "FORWARD", "-i", g_extInterfaceName, "-o", "!", g_extInterfaceName, "-j", "MINIUPNPD");
						// iptables -t filter -N MINIUPNPD
						va_cmd("/bin/iptables", 4, 1, "-t", "filter", "-X", "MINIUPNPD");

						kill(int_pid, 9);
						unlink("/var/run/linuxigd.pid");
						printf("upnpctrl(down): MINIUPNPD is down\n");
					}
					else
						printf("upnpctrl(down): MINIUPNPD shutdown failed!");
					// modification for instant effect end.
				}
				else
					printf("upnpctrl(down): The %s interface is not binded for MINIUPNPD.\n", g_extInterfaceName);
			}
			else
				printf("upnpctrl(down): MINIUPNPD binded WAN interface is not set(error)!\n");
		}
		else
			printf("upnpctrl(down): MINIUPNPD did not start!\n");
	}
	else
		printf("Invalid argument: \"%s\"\n", g_IpFlag);

	return 1;
}
#endif  // CONFIG_USER_MINIUPNPD

