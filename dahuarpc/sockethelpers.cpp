// ***************************************************************
//  sockethelpers.cpp
//
//  Copyright (c) 2019, Ali Tekin
//
//  
//
//  History
//  05.09.2019 Ali Tekin, created
// ***************************************************************

#include "precomp.h"
#include "sockethelpers.h"

BOOL socket_set_non_blocking_mode(SOCKET socket_handle, BOOL non_blocking)
{
	unsigned long on;

	on = non_blocking ? 1 : 0;
	return(ioctlsocket(socket_handle, FIONBIO, &on) != SOCKET_ERROR);
}

BOOL socket_wait_for_read(SOCKET socket_handle, int timeout, BOOL *fd_isset)
{
	int retval;
	fd_set readfds;
	timeval timeval;

	FD_ZERO(&readfds);
	FD_SET(socket_handle, &readfds);

	timeval.tv_sec = timeout / 1000;
	timeval.tv_usec = timeout % 1000;
	retval = select(0, &readfds, NULL, NULL, &timeval);
	if (retval < 0)
		return(FALSE);

	*fd_isset = retval ? FD_ISSET(socket_handle, &readfds) : FALSE;
	return(TRUE);
}

BOOL socket_wait_for_write(SOCKET socket_handle, int timeout, BOOL *fd_isset)
{
	int retval;
	fd_set writefds;
	timeval timeval;

	FD_ZERO(&writefds);
	FD_SET(socket_handle, &writefds);

	timeval.tv_sec = timeout / 1000;
	timeval.tv_usec = timeout % 1000;
	retval = select(0, NULL, &writefds, NULL, &timeval);
	if (retval < 0)
		return(FALSE);

	*fd_isset = retval ? FD_ISSET(socket_handle, &writefds) : FALSE;
	return(TRUE);
}

BOOL socket_send(SOCKET socket_handle, const void *buffer, int length, int timeout)
{
	int pos;

	pos = 0;
	while(pos < length)
	{
		int res;
		BOOL fd_isset;

		if (!socket_wait_for_write(socket_handle, timeout, &fd_isset))
			return(FALSE);

		if (!fd_isset)
		{
			WSASetLastError(WSAETIMEDOUT);
			return(FALSE);
		}

		res = send(socket_handle, static_cast<const char*>(buffer) + pos, length - pos, 0);
		if (res <= 0)
			return(FALSE);

		pos += res;
	}
	return(TRUE);
}

BOOL socket_receive(SOCKET socket_handle, void *buffer, int length, int timeout)
{
	int pos;

	pos = 0;
	while(pos < length)
	{
		int res;
		BOOL fd_isset;

		if (!socket_wait_for_read(socket_handle, timeout, &fd_isset))
			return(FALSE);

		if (!fd_isset)
		{
			WSASetLastError(WSAETIMEDOUT);
			return(FALSE);
		}

		res = recv(socket_handle, static_cast<char*>(buffer) + pos, length - pos, 0);
		if (res <= 0)
			return(FALSE);

		pos += res;
	}
	return(TRUE);
}
