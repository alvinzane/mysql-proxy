/* $%BEGINLICENSE%$
 Copyright (C) 2007-2008 MySQL AB, 2008 Sun Microsystems, Inc

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; version 2 of the License.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 $%ENDLICENSE%$ */
 

#ifndef _NETWORK_SOCKET_H_
#define _NETWORK_SOCKET_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "network-exports.h"

#ifdef HAVE_SYS_TIME_H
/**
 * event.h needs struct timeval and doesn't include sys/time.h itself
 */
#include <sys/time.h>
#endif

#include <sys/types.h>      /** u_char */
#ifndef _WIN32
#include <sys/socket.h>     /** struct sockaddr */

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>     /** struct sockaddr_in */
#endif
#include <netinet/tcp.h>

#ifdef HAVE_SYS_UN_H
#include <sys/un.h>         /** struct sockaddr_un */
#endif
/**
 * use closesocket() to close sockets to be compatible with win32
 */
#define closesocket(x) close(x)
#else
#include <winsock2.h>

#define socklen_t int
#endif
#include <glib.h>
#include <event.h>

#include "network-address.h"

typedef enum {
	NETWORK_SOCKET_SUCCESS,
	NETWORK_SOCKET_WAIT_FOR_EVENT,
	NETWORK_SOCKET_ERROR,
	NETWORK_SOCKET_ERROR_RETRY
} network_socket_retval_t;

/* a input or output stream */
typedef struct {
	GQueue *chunks;

	size_t len;    /* len in all chunks (w/o the offset) */
	size_t offset; /* offset in the first chunk */
} network_queue;

typedef struct network_mysqld_auth_challenge network_mysqld_auth_challenge;
typedef struct network_mysqld_auth_response network_mysqld_auth_response;

typedef struct {
	int fd;             /**< socket-fd */
	struct event event; /**< events for this fd */

	network_address *src; /**< getsockname() */
	network_address *dst; /**< getpeername() */

	guint32 packet_len; /**< the packet_len is a 24bit unsigned int */
	guint8  packet_id;  /**< id which increments for each packet in the stream */

	network_queue *recv_queue;
	network_queue *recv_queue_raw;
	network_queue *send_queue;

	off_t header_read;
	off_t to_read;
	
	/**
	 * data extracted from the handshake  
	 *
	 * all server-side only
	 */
	network_mysqld_auth_challenge *challenge;
	network_mysqld_auth_response  *response;

	gboolean is_authed;           /** did a client already authed this connection */

	/**
	 * store the default-db of the socket
	 *
	 * the client might have a different default-db than the server-side due to
	 * statement balancing
	 */	
	GString *default_db;     /** default-db of this side of the connection */
} network_socket;


NETWORK_API network_queue *network_queue_init(void);
NETWORK_API void network_queue_free(network_queue *queue);
NETWORK_API int network_queue_append(network_queue *queue, GString *chunk);
NETWORK_API GString *network_queue_pop_string(network_queue *queue, gsize steal_len, GString *dest);
NETWORK_API GString *network_queue_peek_string(network_queue *queue, gsize peek_len, GString *dest);

NETWORK_API network_socket *network_socket_init(void);
NETWORK_API void network_socket_free(network_socket *s);
NETWORK_API network_socket_retval_t network_socket_write(network_socket *con, int send_chunks);
NETWORK_API network_socket_retval_t network_socket_read(network_socket *con);
NETWORK_API network_socket_retval_t network_socket_set_non_blocking(network_socket *sock);
NETWORK_API network_socket_retval_t network_socket_connect(network_socket *con);
NETWORK_API network_socket_retval_t network_socket_connect_finish(network_socket *sock);
NETWORK_API network_socket_retval_t network_socket_bind(network_socket *con);
NETWORK_API network_socket *network_socket_accept(network_socket *srv);

#endif

