// ***************************************************************
//  dahuarpcresponse.cpp
//
//  Copyright (c) 2019, Ali Tekin
//
//  
//
//  History
//  08.09.2019 Ali Tekin, created
// ***************************************************************

#include "precomp.h"
#include "sockethelpers.h"
#include "dahuarpcresponse.h"

dahuarpc_response::dahuarpc_response()
{
	data = NULL;
	data_len = 0;
	json_obj = NULL;
}

dahuarpc_response::~dahuarpc_response()
{
	delete_objects();
}

void dahuarpc_response::delete_objects()
{
	free(data);
	data = NULL;
	data_len = 0;

	json_object_put(json_obj);
	json_obj = NULL;
}

BOOL dahuarpc_response::receive(SOCKET socket_handle, uint32_t seq_id, 
	uint32_t session_id, int timeout)
{
	char *json_str;
	struct dahuarpc_header header;

	delete_objects();
	if (!socket_receive(socket_handle, &header, sizeof(header), timeout))
		return(FALSE);

	if (memcmp(header.magic, dahuarpc_magic, DAHUARPC_MAGIC_LEN) != 0)
		return(FALSE);

	if (header.seq_id != seq_id)
		return(FALSE);

	if (session_id != 0 && header.session_id != session_id)
		return(FALSE);

	if (header.length < header.json_len)
		return(FALSE);

	json_str = (char*)malloc(header.json_len + 1);
	if (json_str == NULL)
		return(FALSE);

	if (!socket_receive(socket_handle, json_str, header.json_len, timeout))
	{
		free(json_str);
		return(FALSE);
	}

	json_str[header.json_len] = '\0';
	json_obj = json_tokener_parse(json_str);
	free(json_str);
	if (json_obj == NULL)
		return(FALSE);

	data_len = header.length - header.json_len;
	if (data_len != 0)
	{
		data = malloc(data_len);
		if (data == NULL)
			return(FALSE);

		if (!socket_receive(socket_handle, data, data_len, timeout))
			return(FALSE);
	}
	return(TRUE);
}
