#
# $Id: net.sunset.sh,v 1.1.1.1 2003/08/18 05:39:35 kaohj Exp $
#
if [ -n "$UML_private_CTL" ]
then
    net_eth0="eth0=daemon,10:00:00:ab:cd:01,unix,$UML_private_CTL,$UML_private_DATA";
else
    net_eth0="eth0=mcast,10:00:00:ab:cd:01,239.192.0.2,40800"
fi

net="$net_eth0"

