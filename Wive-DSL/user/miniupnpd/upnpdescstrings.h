/* $Id: upnpdescstrings.h,v 1.3 2008/05/23 09:06:59 adsmt Exp $ */
/* miniupnp project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006 Thomas Bernard
 * This software is subject to the coditions detailed in
 * the LICENCE file provided within the distribution */
#ifndef __UPNPDESCSTRINGS_H__
#define __UPNPDESCSTRINGS_H__

#include "config.h"

#define URL_BASE "http://192.168.1.1:49152"

/* strings used in the root device xml description */
#define ROOTDEV_FRIENDLYNAME		"Realtek ADSL Modem IGD"
#define ROOTDEV_MANUFACTURER		"Realtek Semiconductor Corp."
#define ROOTDEV_MANUFACTURERURL		"http://www.realtek.com.tw"
#define ROOTDEV_MODELNAME			"Realtek ADSL Router"
#define ROOTDEV_MODELNUMBER "1.4.0"
#define ROOTDEV_MODELDESCRIPTION	"Realtek ADSL Router/Modem IGD"
#define ROOTDEV_MODELURL			OS_URL
#define ROOTDEV_UPC		"000000000001"

#define WANDEV_FRIENDLYNAME			"WANDevice"
#define WANDEV_MANUFACTURER			ROOTDEV_MANUFACTURER
#define WANDEV_MANUFACTURERURL		ROOTDEV_MANUFACTURERURL
#define WANDEV_MODELNAME			ROOTDEV_MODELNAME
#define WANDEV_MODELDESCRIPTION		ROOTDEV_MODELDESCRIPTION
#define WANDEV_MODELNUMBER			UPNP_VERSION
#define WANDEV_MODELURL				ROOTDEV_MODELURL
#define WANDEV_UPC					"000000000001"
#define WANDEV_UDN	"uuid:22222222-0000-c0a8-0101-00064f123333"

#define WANCDEV_FRIENDLYNAME		"WANConnectionDevice"
#define WANCDEV_MANUFACTURER		ROOTDEV_MANUFACTURER
#define WANCDEV_MANUFACTURERURL		ROOTDEV_MANUFACTURERURL
#define WANCDEV_MODELNAME			ROOTDEV_MODELNAME
#define WANCDEV_MODELDESCRIPTION ROOTDEV_MODELDESCRIPTION
#define WANCDEV_MODELNUMBER			UPNP_VERSION
#define WANCDEV_MODELURL			ROOTDEV_MODELURL
#define WANCDEV_UPC					"000000000001"
#define WANCDEV_UDN	"uuid:33333333-0000-c0a8-0101-00064f123333"

#define LANDEV_FRIENDLYNAME			"LANDevice"
#define LANDEV_MANUFACTURER			ROOTDEV_MANUFACTURER
#define LANDEV_MANUFACTURERURL		ROOTDEV_MANUFACTURERURL
#define LANDEV_MODELNAME			ROOTDEV_MODELNAME
#define LANDEV_MODELDESCRIPTION		ROOTDEV_MODELDESCRIPTION
#define LANDEV_MODELNUMBER			UPNP_VERSION
#define LANDEV_MODELURL				ROOTDEV_MODELURL
#define LANDEV_UPC					"000000000001"
#define LANDEV_UDN	"uuid:44444444-0000-c0a8-0101-00064f123333"
#endif

