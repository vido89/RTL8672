/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001,2002,2003,2004,2005  Aymeric MOIZARD jack@atosc.org
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdlib.h>
#include <stdio.h>

#include "osip_port.h"
#include "osip_message.h"
#include "osip_parser.h"
#include "parser.h"

/* adds the via header to message.              */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_set_via (osip_message_t * sip, const char *hvalue)
{
  osip_via_t *via;
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return 0;

  i = osip_via_init (&via);
  if (i != 0)
    return -1;
  i = osip_via_parse (via, hvalue);
  if (i != 0)
    {
      osip_via_free (via);
      return -1;
    }
  sip->message_property = 2;
  osip_list_add (sip->vias, via, -1);
  return 0;
}

/* adds the via header to message in the first position. (to be used by proxy) */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_append_via (osip_message_t * sip, const char *hvalue)
{
  osip_via_t *via;
  int i;

  i = osip_via_init (&via);
  if (i != 0)
    return -1;
  i = osip_via_parse (via, hvalue);
  if (i != 0)
    {
      osip_via_free (via);
      return -1;
    }
  sip->message_property = 2;
  osip_list_add (sip->vias, via, 0);
  return 0;
}

/* returns the via header.                         */
/* INPUT : int pos | pos of via header.            */
/* INPUT : osip_message_t *sip | sip message.               */
/* OUTPUT: osip_via_t *via | structure to save results. */
/* returns null on error. */
int
osip_message_get_via (const osip_message_t * sip, int pos, osip_via_t ** dest)
{
  *dest = NULL;
  if (sip == NULL)
    return -1;
  if (osip_list_size (sip->vias) <= pos)
    return -1;
  *dest = (osip_via_t *) osip_list_get (sip->vias, pos);

  return pos;
}


int
osip_via_init (osip_via_t ** via)
{
  *via = (osip_via_t *) osip_malloc (sizeof (osip_via_t));
  if (*via == NULL)
    return -1;

  memset(*via, 0, sizeof(osip_via_t));

  (*via)->via_params = (osip_list_t *) osip_malloc (sizeof (osip_list_t));
  if ((*via)->via_params == NULL)
    {
      osip_free (*via);
      *via = NULL;
      return -1;
    }
  osip_list_init ((*via)->via_params);

  return 0;
}

void
osip_via_free (osip_via_t * via)
{
  if (via == NULL)
    return;
  osip_free (via->version);
  osip_free (via->protocol);
  osip_free (via->host);
  osip_free (via->port);
  osip_free (via->comment);
  osip_generic_param_freelist (via->via_params);

  osip_free (via);
}

int
osip_via_parse (osip_via_t * via, const char *hvalue)
{
  const char *version;
  const char *protocol;
  const char *host;
  const char *ipv6host;
  const char *port;
  const char *via_params;
  const char *comment;

  version = strchr (hvalue, '/');
  if (version == NULL)
    return -1;

  protocol = strchr (version + 1, '/');
  if (protocol == NULL)
    return -1;

  /* set the version */
  if (protocol - version < 2)
    return -1;
  via->version = (char *) osip_malloc (protocol - version);
  if (via->version == NULL)
    return -1;
  osip_strncpy (via->version, version + 1, protocol - version - 1);
  osip_clrspace (via->version);

  /* Here: we avoid matching an additionnal space */
  host = strchr (protocol + 1, ' ');
  if (host == NULL)
    return -1;			/* fixed in 0.8.4 */
  if (host == protocol + 1)	/* there are extra SPACE characters */
    {
      while (0 == strncmp (host, " ", 1))
	{
	  host++;
	  if (strlen (host) == 1)
	    return -1;		/* via is malformed */
	}
      /* here, we match the real space located after the protocol name */
      host = strchr (host + 1, ' ');
      if (host == NULL)
	return -1;		/* fixed in 0.8.4 */
    }

  /* set the protocol */
  if (host - protocol < 2)
    return -1;
  via->protocol = (char *) osip_malloc (host - protocol);
  if (via->protocol == NULL)
    return -1;
  osip_strncpy (via->protocol, protocol + 1, host - protocol - 1);
  osip_clrspace (via->protocol);

  /* comments in Via are not allowed any more in the latest draft (09) */
  comment = strchr (host, '(');

  if (comment != NULL)
    {
      char *end_comment;

      end_comment = strchr (host, ')');
      if (end_comment == NULL)
	return -1;		/* if '(' exist ')' MUST exist */
      if (end_comment - comment < 2)
	return -1;
      via->comment = (char *) osip_malloc (end_comment - comment);
      if (via->comment == NULL)
	return -1;
      osip_strncpy (via->comment, comment + 1, end_comment - comment - 1);
      comment--;
    }
  else
    comment = host + strlen (host);

  via_params = strchr (host, ';');

  if ((via_params != NULL) && (via_params < comment))
    /* via params exist */
    {
      char *tmp;

      if (comment - via_params + 1 < 2)
	return -1;
      tmp = (char *) osip_malloc (comment - via_params + 1);
      if (tmp == NULL)
	return -1;
      osip_strncpy (tmp, via_params, comment - via_params);
      __osip_generic_param_parseall (via->via_params, tmp);
      osip_free (tmp);
    }

  if (via_params == NULL)
    via_params = comment;

  /* add ipv6 support (0.8.4) */
  /* Via: SIP/2.0/UDP [mlke::zeezf:ezfz:zef:zefzf]:port;.... */
  ipv6host = strchr (host, '[');
  if (ipv6host != NULL && ipv6host < via_params)
    {
      port = strchr (ipv6host, ']');
      if (port == NULL || port > via_params)
	return -1;

      if (port - ipv6host < 2)
	return -1;
      via->host = (char *) osip_malloc (port - ipv6host);
      if (via->host == NULL)
	return -1;
      osip_strncpy (via->host, ipv6host + 1, port - ipv6host - 1);
      osip_clrspace (via->host);

      port = strchr (port, ':');
    }
  else
    {
      port = strchr (host, ':');
      ipv6host = NULL;
    }

  if ((port != NULL) && (port < via_params))
    {
      if (via_params - port < 2)
	return -1;
      via->port = (char *) osip_malloc (via_params - port);
      if (via->port == NULL)
	return -1;
      osip_strncpy (via->port, port + 1, via_params - port - 1);
      osip_clrspace (via->port);
    }
  else
    port = via_params;

  /* host is already set in the case of ipv6 */
  if (ipv6host != NULL)
    return 0;

  if (port - host < 2)
    return -1;
  via->host = (char *) osip_malloc (port - host);
  if (via->host == NULL)
    return -1;
  osip_strncpy (via->host, host + 1, port - host - 1);
  osip_clrspace (via->host);

  return 0;
}


/* returns the via header as a string. */
/* INPUT : osip_via_t via* | via header.    */
/* returns null on error. */
int
osip_via_to_str (const osip_via_t * via, char **dest)
{
  char *buf;
  size_t len;
  size_t plen;
  char *tmp;

  *dest = NULL;
  if ((via == NULL) || (via->host == NULL)
      || (via->version == NULL) || (via->protocol == NULL))
    return -1;

  len = strlen (via->version) + 1 + strlen (via->protocol) + 1 + 3 + 2;	/* sip/xxx/xxx */
  len = len + strlen (via->host) + 3 + 1;
  if (via->port != NULL)
    len = len + strlen (via->port) + 2;

  buf = (char *) osip_malloc (len);
  if (buf == NULL)
    return -1;

  if (strchr (via->host, ':') != NULL)
    {
      if (via->port == NULL)
	sprintf (buf, "SIP/%s/%s [%s]", via->version, via->protocol,
		 via->host);
      else
	sprintf (buf, "SIP/%s/%s [%s]:%s", via->version, via->protocol,
		 via->host, via->port);
    }
  else
    {
      if (via->port == NULL)
	sprintf (buf, "SIP/%s/%s %s", via->version, via->protocol, via->host);
      else
	sprintf (buf, "SIP/%s/%s %s:%s", via->version, via->protocol,
		 via->host, via->port);
    }



  {
    int pos = 0;
    osip_generic_param_t *u_param;

    while (!osip_list_eol (via->via_params, pos))
      {
	u_param =
	  (osip_generic_param_t *) osip_list_get (via->via_params, pos);

	if (u_param->gvalue == NULL)
	  plen = strlen (u_param->gname) + 2;
	else
	  plen = strlen (u_param->gname) + strlen (u_param->gvalue) + 3;
	len = len + plen;
	buf = (char *) osip_realloc (buf, len);
	tmp = buf;
	tmp = tmp + strlen (tmp);
	if (u_param->gvalue == NULL)
	  sprintf (tmp, ";%s", u_param->gname);
	else
	  sprintf (tmp, ";%s=%s", u_param->gname, u_param->gvalue);
	pos++;
      }
  }

  if (via->comment != NULL)
    {
      len = len + strlen (via->comment) + 4;
      buf = (char *) osip_realloc (buf, len);
      tmp = buf;
      tmp = tmp + strlen (tmp);
      sprintf (tmp, " (%s)", via->comment);
    }
  *dest = buf;
  return 0;
}

void
via_set_version (osip_via_t * via, char *version)
{
  via->version = version;
}

char *
via_get_version (osip_via_t * via)
{
  if (via == NULL)
    return NULL;
  return via->version;
}

void
via_set_protocol (osip_via_t * via, char *protocol)
{
  via->protocol = protocol;
}

char *
via_get_protocol (osip_via_t * via)
{
  if (via == NULL)
    return NULL;
  return via->protocol;
}

void
via_set_host (osip_via_t * via, char *host)
{
  via->host = host;
}

char *
via_get_host (osip_via_t * via)
{
  if (via == NULL)
    return NULL;
  return via->host;
}

void
via_set_port (osip_via_t * via, char *port)
{
  via->port = port;
}

char *
via_get_port (osip_via_t * via)
{
  if (via == NULL)
    return NULL;
  return via->port;
}

void
via_set_comment (osip_via_t * via, char *comment)
{
  via->comment = comment;
}

char *
via_get_comment (osip_via_t * via)
{
  if (via == NULL)
    return NULL;
  return via->comment;
}

int
osip_via_clone (const osip_via_t * via, osip_via_t ** dest)
{
  int i;
  osip_via_t *vi;

  *dest = NULL;
  if (via == NULL)
    return -1;
  if (via->version == NULL)
    return -1;
  if (via->protocol == NULL)
    return -1;
  if (via->host == NULL)
    return -1;

  i = osip_via_init (&vi);
  if (i != 0)
    return -1;
  vi->version = osip_strdup (via->version);
  vi->protocol = osip_strdup (via->protocol);
  vi->host = osip_strdup (via->host);
  if (via->port != NULL)
    vi->port = osip_strdup (via->port);
  if (via->comment != NULL)
    vi->comment = osip_strdup (via->comment);

  {
    int pos = 0;
    osip_generic_param_t *u_param;
    osip_generic_param_t *dest_param;

    while (!osip_list_eol (via->via_params, pos))
      {
	u_param =
	  (osip_generic_param_t *) osip_list_get (via->via_params, pos);
	i = osip_generic_param_clone (u_param, &dest_param);
	if (i != 0)
	  {
	    osip_via_free (vi);
	    return -1;
	  }
	osip_list_add (vi->via_params, dest_param, -1);
	pos++;
      }
  }
  *dest = vi;
  return 0;
}

int
osip_via_match (osip_via_t * via1, osip_via_t * via2)
{
  /* Can I really compare it this way??
     There exist matching rules for via header, but this method
     should only be used to detect retransmissions so the result should
     be exactly equivalent. (This may not be true if the retransmission
     traverse a different set of proxy...  */
  char *_via1;
  char *_via2;
  int i;

  if (via1 == NULL || via2 == NULL)
    return -1;
  i = osip_via_to_str (via1, &_via1);
  if (i != 0)
    return -1;
  i = osip_via_to_str (via2, &_via2);
  if (i != 0)
    {
      osip_free (_via1);
      return -1;
    }

  i = strcmp (_via1, _via2);
  osip_free (_via1);
  osip_free (_via2);
  if (i != 0)
    return -1;
  return 0;
}
