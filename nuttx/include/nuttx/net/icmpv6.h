/****************************************************************************
 * include/nuttx/net/icmpv6.h
 * Header file for the uIP ICMPv6 stack.
 *
 *   Copyright (C) 2007-2009, 2012, 2014 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * This logic was leveraged from uIP which also has a BSD-style license:
 *
 *   Author Adam Dunkels <adam@dunkels.com>
 *   Copyright (c) 2001-2003, Adam Dunkels.
 *   All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#ifndef __INCLUDE_NUTTX_NET_ICMPv6_H
#define __INCLUDE_NUTTX_NET_ICMPv6_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>

#include <nuttx/net/netconfig.h>
#include <nuttx/net/ip.h>
#include <nuttx/net/tcp.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* ICMPv6 definitions */

/* ICMPv6 Message Types */

#define ICMPv6_RESERVED                0    /* RFC 4443 */
#define ICMPv6_DEST_UNREACHABLE        1
#define ICMPv6_PACKET_TOO_BIG          2
#define ICMPv6_PACKET_TIME_EXCEEDED    3
#define ICMPv6_PACKET_PARAM_PROBLEM    4
#define ICMPv6_PRIVATE_ERR_MSG_1       100
#define ICMPv6_PRIVATE_ERR_MSG_2       101
#define ICMPv6_RESERVED_ERROR_MSG      127
#define ICMPv6_ECHO_REQUEST            128
#define ICMPv6_ECHO_REPLY              129
#define ICMPV6_MCAST_LISTEN_QUERY      130   /* RFC 2710 */
#define ICMPV6_MCAST_LISTEN_REPORT     131
#define ICMPV6_MCAST_LISTEN_DONE       132
#define ICMPV6_ROUTER_SOLICIT          133   /* RFC 4861 */
#define ICMPV6_ROUTER_ADVERTISE        134
#define ICMPv6_NEIGHBOR_SOLICIT        135
#define ICMPv6_NEIGHBOR_ADVERTISE      136
#define ICMPv6_REDIRECT                137
#define ICMPV6_ROUTER_RENUMBERING      138   /* Matt Crawford */
#define ICMPV6_NODE_INFO_QUERY         139   /* RFC 4620 */
#define ICMPV6_NODE_INFO_REPLY         140
#define ICMPV6_INV_NEIGHBOR_DISCOVERY  141   /* RFC 3122 */
#define ICMPV6_INV_NEIGHBOR_ADVERTISE  142
#define ICMPV6_HOME_AGENT_DISCOVERY    144   /* RFC 3775 */
#define ICMPV6_HOME_AGENT_REPLY        145
#define ICMPV6_MOBILE_PREFIX_SOLICIT   146
#define ICMPV6_MOBILE_PREFIX_ADVERTISE 147
#define ICMPv6_PRIVATE_INFO_MSG_1      200   /* RFC 4443 */
#define ICMPv6_PRIVATE_INFO_MSG_2      201
#define ICMPv6_RESERVED_INFO_MSG       255

#define ICMPv6_FLAG_S (1 << 6)

/* Header sizes */

#define ICMPv6_HDRLEN    4                             /* Size of ICMPv6 header */
#define IPICMPv6_HDRLEN  (ICMPv6_HDRLEN + IPv6_HDRLEN) /* Size of IPv6 + ICMPv6 header */

/* Option types */

#define ICMPv6_OPT_SRCLLADDR  1 /* Source Link-Layer Address */
#define ICMPv6_OPT_TGTLLADDR  2 /* Target Link-Layer Address */
#define ICMPv6_OPT_PREFIX     3 /* Prefix Information */
#define ICMPv6_OPT_REDIRECT   4 /* Redirected Header */
#define ICMPv6_OPT_MTU        5 /* MTU */

/* ICMPv6 Neighbor Advertisement message flags */

#define ICMPv6_FLAG_R    (1 << 7) /* Router flag */
#define ICMPv6_FLAG_S    (1 << 6) /* Solicited flag */
#define ICMPv6_FLAG_O    (1 << 5) /* Override flag */

/****************************************************************************
 * Public Type Definitions
 ****************************************************************************/

/* The ICMP and IP headers */

struct icmpv6_iphdr_s
{
  /* IPv6 Ip header */

  uint8_t  vtc;              /* Bits 0-3: version, bits 4-7: traffic class (MS) */
  uint8_t  tcf;              /* Bits 0-3: traffic class (LS), 4-bits: flow label (MS) */
  uint16_t flow;             /* 16-bit flow label (LS) */
  uint8_t  len[2];           /* 16-bit Payload length */
  uint8_t  proto;            /*  8-bit Next header (same as IPv4 protocol field) */
  uint8_t  ttl;              /*  8-bit Hop limit (like IPv4 TTL field) */
  net_ipv6addr_t srcipaddr;  /* 128-bit Source address */
  net_ipv6addr_t destipaddr; /* 128-bit Destination address */

  /* ICMPv6 header */

  uint8_t  type;             /* Defines the format of the ICMP message */
  uint8_t  code;             /* Further qualifies the ICMP messages */
  uint16_t chksum;           /* Checksum of ICMP header and data */

  /* Data following the ICMP header contains the data specific to the
   * message type indicated by the Type and Code fields.
   */
};

/* This the message format for the ICMPv6 Neighbor Solicitation message */

struct icmpv6_neighbor_solicit_s
{
  uint8_t  type;             /* Message Type: ICMPv6_NEIGHBOR_SOLICIT */
  uint8_t  code;             /* Further qualifies the ICMP messages */
  uint16_t chksum;           /* Checksum of ICMP header and data */
  uint8_t  flags[4];         /* See ICMPv6_FLAG_ definitions */
  net_ipv6addr_t tgtaddr;    /* 128-bit Target IPv6 address */
  uint8_t  opttype;          /* Option Type: ICMPv6_OPT_SRCLLADDR */
  uint8_t  optlen;           /* Option length: 8 octets */
#ifdef CONFIG_NET_ETHERNET
  uint8_t  srclladdr[6];     /* Options: Source link layer address */
#endif
};

/* This the message format for the ICMPv6 Neighbor Advertisement message */

struct icmpv6_neighbor_advertise_s
{
  uint8_t  type;             /* Message Type: ICMPv6_NEIGHBOR_ADVERTISE */
  uint8_t  code;             /* Further qualifies the ICMP messages */
  uint16_t chksum;           /* Checksum of ICMP header and data */
  uint8_t  flags[4];         /* See ICMPv6_FLAG_ definitions */
  net_ipv6addr_t tgtaddr;    /* Target IPv6 address */
  uint8_t  opttype;          /* Option Type: ICMPv6_OPT_TGTLLADDR */
  uint8_t  optlen;           /* Option length: 8 octets */
#ifdef CONFIG_NET_ETHERNET
  uint8_t  tgtlladdr[6];     /* Options: Target link layer address */
#endif
};

/* This the message format for the ICMPv6 Echo Request message */

struct icmpv6_echo_request_s
{
  uint8_t  type;             /* Message Type: ICMPv6_ECHO_REQUEST */
  uint8_t  code;             /* Further qualifies the ICMP messages */
  uint16_t chksum;           /* Checksum of ICMP header and data */
  uint16_t id;               /* Identifier */
  uint16_t seqno;            /* Sequence Number */
  uint8_t  data[1];          /* Data follows */
};

#define SIZEOF_ICMPV6_ECHO_REQUEST_S(n) \
  (sizeof(struct icmpv6_echo_request_s) - 1 + (n))

/* This the message format for the ICMPv6 Echo Reply message */

struct icmpv6_echo_reply_s
{
  uint8_t  type;             /* Message Type: ICMPv6_ECHO_REQUEST */
  uint8_t  code;             /* Further qualifies the ICMP messages */
  uint16_t chksum;           /* Checksum of ICMP header and data */
  uint16_t id;               /* Identifier */
  uint16_t seqno;            /* Sequence Number */
  uint8_t  data[1];          /* Data follows */
};

#define SIZEOF_ICMPV6_ECHO_REPLY_S(n) \
  (sizeof(struct icmpv6_echo_reply_s) - 1 + (n))

/* The structure holding the ICMP statistics that are gathered if
 * CONFIG_NET_STATISTICS is defined.
 */

#ifdef CONFIG_NET_STATISTICS
struct icmpv6_stats_s
{
  net_stats_t drop;       /* Number of dropped ICMP packets */
  net_stats_t recv;       /* Number of received ICMP packets */
  net_stats_t sent;       /* Number of sent ICMP packets */
  net_stats_t typeerr;    /* Number of ICMP packets with a wrong type */
};
#endif

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: imcp_ping
 *
 * Description:
 *   Send a ECHO request and wait for the ECHO response
 *
 * Parameters:
 *   addr  - The IP address of the peer to send the ICMP ECHO request to
 *           in network order.
 *   id    - The ID to use in the ICMP ECHO request.  This number should be
 *           unique; only ECHO responses with this matching ID will be
 *           processed (host order)
 *   seqno - The sequence number used in the ICMP ECHO request.  NOT used
 *           to match responses (host order)
 *   dsecs - Wait up to this many deci-seconds for the ECHO response to be
 *           returned (host order).
 *
 * Return:
 *   seqno of received ICMP ECHO with matching ID (may be different
 *   from the seqno argument (may be a delayed response from an earlier
 *   ping with the same ID). Or a negated errno on any failure.
 *
 * Assumptions:
 *   Called from the user level with interrupts enabled.
 *
 ****************************************************************************/

int icmpv6_ping(net_ipv6addr_t addr, uint16_t id, uint16_t seqno,
                uint16_t datalen, int dsecs);

#undef EXTERN
#ifdef __cplusplus
}
#endif
#endif /* __INCLUDE_NUTTX_NET_ICMPv6_H */
