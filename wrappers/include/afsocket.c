// vim: syntax=c tabstop=4 softtabstop=0 noexpandtab laststatus=1 ruler

/**
 * wrappers/includes/afsocket.c
 *
 * Socket functions for wrappers.
 *
 * LICENSE:
 *
 * This file is part of UNetLab (Unified Networking Lab).
 *
 * UNetLab is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * UNetLab is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with UNetLab. If not, see <http://www.gnu.org/licenses/>.
 *
 * @author Andrea Dainese <andrea.dainese@gmail.com>
 * @copyright 2014-2015 Andrea Dainese
 * @license http://www.gnu.org/licenses/gpl.html
 * @link http://www.unetlab.com/
 * @version 20150715
 */

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <sys/un.h>
#include <unistd.h>

#include "log.h"
#include "params.h"

extern int device_id;
extern int tenant_id;

// AF_UNIX socket: listen
int afsocket_listen(char *server_socketfile, char *remote_socketfile, int *server_socket, int *remote_socket) {
    int rc = -1;
    struct sockaddr_un remote_addr;
    memset(&remote_addr, 0, sizeof(remote_addr));
    struct sockaddr_un server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    // Setting AF_UNIX remote (sending)  socket
    *remote_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (*remote_socket < 0) {
        rc = 1;
        UNLLog(LLERROR, "Error while setting remote AF_UNIX: %s (%i)\n",strerror(errno), rc);
        return rc;
    }
    remote_addr.sun_family = AF_UNIX;
    strncpy(remote_addr.sun_path, remote_socketfile, sizeof(remote_addr.sun_path) - 1);
    while (connect(*remote_socket, (struct sockaddr *)&remote_addr, sizeof(struct sockaddr_un)) < 0) {
        rc = 2;
        UNLLog(LLERROR, "Error while connecting local AF_UNIX: %s (%i)\n",strerror(errno), rc);
        return rc;
    }

    // Setting AF_UNIX local (receiving) socket
    *server_socket = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (*server_socket < 0) {
        rc = 3;
        UNLLog(LLERROR, "Error while setting local AF_UNIX: %s (%i)\n",strerror(errno), rc);
        return rc;
    }
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, server_socketfile, sizeof(server_addr.sun_path) -1 );
    if (bind(*server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un))) {
        rc = 4;
        UNLLog(LLERROR, "Error while binding local AF_UNIX: %s (%i)\n", strerror(errno), rc);
        return rc;
    }
    UNLLog(LLINFO, "Local (%i) and remote (%i) AF_UNIX are configured.\n", *server_socket, *remote_socket);
    return 0;
}

// AF_UNIX socket: receive
int afsocket_receive(void *c, int server_socket, int bytesToRead) {
    int length = 0;
    //memset(c, 0, sizeof(*c)); what's that for???

    if ((length = read(server_socket, c, bytesToRead)) <= 0) {
        // Read error
        UNLLog(LLERROR, "Failed to receive data from local AF_UNIX (s=%i, l=%i): %s (%i)\n", server_socket, length, strerror(errno), length);
        return length;
    }
    UNLLog(LLVERBOSE, "Received data from local AF_UNIX (s=%i, l=%i).\n", server_socket, length);
    return length;
}
