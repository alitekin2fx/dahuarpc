// ***************************************************************
//  dahuarpc_exp.cpp
//
//  Copyright (c) 2019, Ali Tekin
//
//  
//
//  History
//  04.09.2019 Ali Tekin, created
// ***************************************************************

#include "precomp.h"
#include "dahuarpc.h"

BOOL WINAPI dahuarpc_init()
{
	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return(FALSE);

	return(TRUE);
}

void WINAPI dahuarpc_cleanup()
{
	WSACleanup();
}

HDAHUARPC WINAPI dahuarpc_open(const char *addr, unsigned int port, int timeout)
{
	dahuarpc *rpc;

	rpc = new dahuarpc;
	if (!rpc->connect(addr, port, timeout))
	{
		delete rpc;
		return(NULL);
	}
	return((HDAHUARPC)rpc);
}

void WINAPI dahuarpc_close(HDAHUARPC handle)
{
	delete (dahuarpc*)handle;
}

BOOL WINAPI dahuarpc_login(HDAHUARPC handle, const char *username, const char *password)
{
	return(((dahuarpc*)handle)->login(username, password));
}

BOOL WINAPI dahuarpc_get_access_ctl_card_recs(HDAHUARPC handle, const char *start_time, 
	const char *end_time, dahuarpc_access_ctl_card_rec_proc rec_proc, void *context)
{
	BOOL result;
	dahuarpc *rpc;
	int32_t finder_id;

	rpc = (dahuarpc*)handle;
	if (rpc == NULL)
		return(FALSE);

	if (!rpc->record_finder_factory_create("AccessControlCardRec", &finder_id))
		return(FALSE);

	result = TRUE;
	if (!rpc->record_finder_start_find(finder_id, start_time, end_time))
	{
		result = FALSE;
	}
	else
	{
		int32_t rec_count;
		DWORD ticks_keep_alive, interval;

		interval = 0;
		ticks_keep_alive = GetTickCount();
		do
		{
			if (GetTickCount() - ticks_keep_alive > interval)
			{
				ticks_keep_alive = GetTickCount();
				if (!rpc->keep_alive(10))
				{
					result = FALSE;
					break;
				}

				interval = 5000;
			}

			if (!rpc->record_finder_do_find(finder_id, 10, rec_proc, context, &rec_count))
			{
				result = FALSE;
				return(FALSE);
			}
		} while(rec_count != 0);
	}

	if (!rpc->record_finder_destroy(finder_id))
		result = FALSE;

	return(result);
}
