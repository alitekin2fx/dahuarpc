// ***************************************************************
//  dahuarpcrequest.cpp
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
#include "dahuarpcrequest.h"

dahuarpc_request::dahuarpc_request()
{
	json_obj = NULL;
}

dahuarpc_request::~dahuarpc_request()
{
	delete_objects();
}

void dahuarpc_request::delete_objects()
{
	json_object_put(json_obj);
	json_obj = NULL;
}

json_object* dahuarpc_request::get_json_obj()
{
	if (json_obj == NULL)
		json_obj = json_object_new_object();
	return(json_obj);
}

BOOL dahuarpc_request::send(SOCKET socket_handle, uint32_t seq_id, 
	uint32_t session_id, const void *data, int data_len, int timeout)
{
	BOOL res;
	int pack_len;
	void *pack_buf;

	pack_buf = create_packet(seq_id, session_id, data, data_len, &pack_len);
	if (pack_buf == NULL)
		return(FALSE);

	res = socket_send(socket_handle, pack_buf, pack_len, timeout);
	free(pack_buf);
	return(res);
}

void* dahuarpc_request::create_packet(uint32_t seq_id, uint32_t session_id, 
	const void *data, int data_len, int *pack_len)
{
	int json_len;
	void *pack_buf;
	const char *json_str;
	struct dahuarpc_header *header;

	json_str = json_object_to_json_string(json_obj);
	if (json_str == NULL)
		return(NULL);

	json_len = strlen(json_str);
	*pack_len = sizeof(struct dahuarpc_header) + json_len + data_len;
	pack_buf = malloc(*pack_len);
	if (pack_buf == NULL)
		return(NULL);

	header = (struct dahuarpc_header*)pack_buf;
	memset(header, 0, sizeof(struct dahuarpc_header));
	header->seq_id = seq_id;
	header->session_id = session_id;
	header->length = json_len + data_len;
	header->json_len = json_len;
	memcpy(header->magic, dahuarpc_magic, DAHUARPC_MAGIC_LEN);
	memcpy((char*)pack_buf + sizeof(struct dahuarpc_header), json_str, json_len);
	memcpy((char*)pack_buf + sizeof(struct dahuarpc_header) + json_len, data, data_len);
	return(pack_buf);
}
