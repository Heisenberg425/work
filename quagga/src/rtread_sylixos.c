/*
 * Kernel routing table read by sysctl function.
 * Copyright (C) 1997, 98 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include <zebra.h>

#include "memory.h"
#include "log.h"
#include "vrf.h"

#include "zebra/zserv.h"
#include "zebra/rt.h"
#include "zebra/kernel_socket.h"

extern int routing_sock;

/* Kernel routing table read up by sysctl function. */
void
route_read (struct zebra_vrf *zvrf)
{
  caddr_t buf, end;
  size_t bufsiz;
  struct rt_msghdr *rtm;
  struct rt_msghdr_list rtml;

  if (zvrf->vrf_id != VRF_DEFAULT)
    return;

  /* Get buffer size. */
  memset(&rtml, 0, sizeof(rtml));
  if (ioctl(routing_sock, SIOCLSTRTM, &rtml, sizeof(rtml)) < 0)
    {
      zlog_warn ("get rtm list size fail: %s", safe_strerror (errno));
      return;
    }

  /* Allocate buffer. */
  buf = XMALLOC (MTYPE_TMP, rtml.rtml_tsize);
  if (buf == NULL)
    {
      zlog_warn ("get buf fail.");
      return;
    }
  rtml.rtml_buf = (struct rt_msghdr *)buf;
  bufsiz = rtml.rtml_tsize;
  /* Read routing table information by calling ioctl(SIOCLSTRTM). */
  if (ioctl(routing_sock, SIOCLSTRTM, &rtml, sizeof(rtml)) < 0)
    {
      zlog_warn ("get rtm list size fail: %s", safe_strerror (errno));
      return;
    }

  for (end = buf + bufsiz; buf < end; buf += rtm->rtm_msglen) 
    {
      rtm = (struct rt_msghdr *) buf;
      /* We must set RTF_DONE here, so rtm_read() doesn't ignore the message. */
      SET_FLAG (rtm->rtm_flags, RTF_DONE);
      rtm_read (rtm);
    }

  /* Free buffer. */
  XFREE (MTYPE_TMP, rtml.rtml_buf);

  return;
}
