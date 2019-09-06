#ifndef __SOCKETHELPERS_H__
#define __SOCKETHELPERS_H__

BOOL socket_set_non_blocking_mode(SOCKET socket_handle, BOOL non_blocking);

BOOL socket_wait_for_read(SOCKET socket_handle, int timeout, BOOL *fd_isset);
BOOL socket_wait_for_write(SOCKET socket_handle, int timeout, BOOL *fd_isset);

BOOL socket_send(SOCKET socket_handle, const void *buffer, int length, int timeout);
BOOL socket_receive(SOCKET socket_handle, void *buffer, int length, int timeout);

#endif /* __SOCKETHELPERS_H__ */
