#ifndef NTPTIME_H
#define NTPTIME_H

/* ====================================================================
 * The Vovida Software License, Version 1.0 
 * 
 * Copyright (c) 2000 Vovida Networks, Inc.  All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 
 * 3. The names "VOCAL", "Vovida Open Communication Application Library",
 *    and "Vovida Open Communication Application Library (VOCAL)" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact vocal@vovida.org.
 *
 * 4. Products derived from this software may not be called "VOCAL", nor
 *    may "VOCAL" appear in their name, without prior written
 *    permission of Vovida Networks, Inc.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.  IN NO EVENT SHALL VOVIDA
 * NETWORKS, INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT DAMAGES
 * IN EXCESS OF $1,000, NOR FOR ANY INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 * 
 * ====================================================================
 * 
 * This software consists of voluntary contributions made by Vovida
 * Networks, Inc. and many individuals on behalf of Vovida Networks,
 * Inc.  For more information on Vovida Networks, Inc., please see
 * <http://www.vovida.org/>.
 *
 */

//static const char* const NtpTime_hxx_Version =
//    "$Id: NtpTime.h,v 1.2 2007/09/28 14:02:46 eric Exp $";

#include "myTypes.h"
//#include <sys/types.h>
/* Kao
#include "vtypes.h"
*/

typedef struct stNtpTimeval
{
	unsigned long	tv_sec;		/* seconds */
	unsigned long	tv_usec;	/* and microseconds */
}NtpTimeval;

///
typedef struct stNtpTime
{
	uint32 seconds;
	uint32 fractional;
}NtpTime;

void Ntp_Init(void);
void Ntp_TimeInit(NtpTime* time, uint32 sec, uint32 frac);
void Ntp_TimeInitNull(NtpTime* time);
void Ntp_cpy(NtpTime* time1, const NtpTime* time2);

void NTP_addms(const NtpTime* rhs , const unsigned int delayMs, NtpTime* time_result);
void NTP_subms(const NtpTime* rhs , const unsigned int ms, NtpTime* time_result);
int NTP_sub(const NtpTime* rhs , const NtpTime* lhs);
bool NTP_isEqual(const NtpTime* rhs , const NtpTime* lhs);
bool NTP_isLarge(const NtpTime* rhs , const NtpTime* lhs);
bool NTP_isLess(const NtpTime* rhs , const NtpTime* lhs);
void Ntp_getTime(NtpTime* time_result);

void NTP_timetick (void);

#endif // NTPTIME_H
