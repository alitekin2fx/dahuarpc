// ***************************************************************
//  dahuarpc.cpp
//
//  Copyright (c) 2019, Ali Tekin
//
//  
//
//  History
//  05.09.2019 Ali Tekin, created
// ***************************************************************

#include "precomp.h"
#include "md5.h"
#include "strptime.h"
#include "sockethelpers.h"
#include "dahuarpcrequest.h"
#include "dahuarpcresponse.h"
#include "dahuarpc.h"

const uint8_t dahuarpc_magic[DAHUARPC_MAGIC_LEN] =
{
	0x20, 0x00, 0x00, 0x00, 0x44, 0x48, 0x49, 0x50
};

dahuarpc::dahuarpc()
{
	seq_id = 0;
	query_id = 0;
	session_id = 0;
	client_type = "Dahua3.0-RPC";
	socket_handle = INVALID_SOCKET;
}

dahuarpc::~dahuarpc()
{
	disconnect();
}

BOOL dahuarpc::connect(const char *addr, unsigned int port, int timeout)
{
	SOCKET handle;
	struct sockaddr_in serv_addr;

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(addr);
	if (serv_addr.sin_addr.s_addr == INADDR_NONE)
	{
		struct hostent *host;

		host = gethostbyname(addr);
		if (host == NULL)
			return(FALSE);

		serv_addr.sin_addr.s_addr = ((struct in_addr*)host->h_addr)->s_addr;
	}

	serv_addr.sin_port = htons(port);
	handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (handle == INVALID_SOCKET)
		return(FALSE);

	if (!socket_set_non_blocking_mode(handle, TRUE))
	{
		closesocket(handle);
		return(FALSE);
	}

	if (::connect(handle, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
	{
		BOOL fd_isset;

		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			closesocket(handle);
			return(FALSE);
		}

		if (!socket_wait_for_write(handle, timeout, &fd_isset))
		{
			closesocket(handle);
			return(FALSE);
		}

		if (!fd_isset)
		{
			WSASetLastError(WSAETIMEDOUT);
			closesocket(handle);
			return(FALSE);
		}
	}

	socket_timeout = timeout;
	socket_handle = handle;
	return(TRUE);
}

void dahuarpc::disconnect()
{
	if (socket_handle == INVALID_SOCKET)
		return;

	closesocket(socket_handle);
	socket_handle = INVALID_SOCKET;
}

BOOL dahuarpc::login(const char *username, const char *password)
{
	dahuarpc_request request;
	dahuarpc_response response;
	char hash_buffer[16 * 2 + 1];
	json_object *realm, *random, *params;

	params = json_object_new_object();
	json_object_object_add(request.get_json_obj(), "id", json_object_new_int(++query_id));
	json_object_object_add(request.get_json_obj(), "method", json_object_new_string("global.login"));
	json_object_object_add(params, "clientType", json_object_new_string(client_type));
	json_object_object_add(params, "ipAddr", json_object_new_string("(null)"));
	json_object_object_add(params, "loginType", json_object_new_string("Direct"));
	json_object_object_add(params, "password", json_object_new_string(""));
	json_object_object_add(params, "userName", json_object_new_string(username));
	json_object_object_add(request.get_json_obj(), "params", params);
	json_object_object_add(request.get_json_obj(), "session", json_object_new_int(0));
	if (!request.send(socket_handle, ++seq_id, session_id, NULL, 0, socket_timeout) || 
		!response.receive(socket_handle, seq_id, session_id, socket_timeout))
		return(FALSE);

	params = json_object_object_get(response.get_json_obj(), "params");
	if (params == NULL)
		return(FALSE);

	realm = json_object_object_get(params, "realm");
	random = json_object_object_get(params, "random");
	session_id = json_object_get_int(json_object_object_get(response.get_json_obj(), "session"));

	request.delete_objects();
	params = json_object_new_object();
	json_object_object_add(request.get_json_obj(), "id", json_object_new_int(++query_id));
	json_object_object_add(request.get_json_obj(), "method", json_object_new_string("global.login"));
	json_object_object_add(request.get_json_obj(), "session", json_object_new_int(session_id));
	json_object_object_add(params, "userName", json_object_new_string(username));
	json_object_object_add(params, "password", json_object_new_string(
		dahua_md5_hash(hash_buffer, realm, random, username, password)));
	json_object_object_add(params, "clientType", json_object_new_string(client_type));
	json_object_object_add(params, "ipAddr", json_object_new_string("(null)"));
	json_object_object_add(params, "loginType", json_object_new_string("Direct"));
	json_object_object_add(params, "authorityType", json_object_new_string("Default"));
	json_object_object_add(request.get_json_obj(), "params", params);
	if (!request.send(socket_handle, ++seq_id, session_id, NULL, 0, socket_timeout) || 
		!response.receive(socket_handle, seq_id, session_id, socket_timeout))
		return(FALSE);

	return(json_object_get_boolean(json_object_object_get(response.get_json_obj(), "result")));
}

BOOL dahuarpc::record_finder_get_caps()
{
	dahuarpc_request request;
	dahuarpc_response response;

	json_object_object_add(request.get_json_obj(), "id", json_object_new_int(++query_id));
	json_object_object_add(request.get_json_obj(), "method", json_object_new_string("RecordFinder.getCaps"));
	json_object_object_add(request.get_json_obj(), "session", json_object_new_int(session_id));
	if (!request.send(socket_handle, ++seq_id, session_id, NULL, 0, socket_timeout) || 
		!response.receive(socket_handle, seq_id, session_id, socket_timeout))
		return(FALSE);

	printf("MaxPageSize = %d\n", json_object_get_int(json_object_object_get(
		json_object_object_get(json_object_object_get(response.get_json_obj(), 
		"params"), "caps"), "MaxPageSize")));
	return(TRUE);
}

BOOL dahuarpc::record_finder_list_method()
{
	dahuarpc_request request;
	dahuarpc_response response;
	json_object *method_array;

	json_object_object_add(request.get_json_obj(), "id", json_object_new_int(++query_id));
	json_object_object_add(request.get_json_obj(), "method", json_object_new_string("RecordFinder.listMethod"));
	json_object_object_add(request.get_json_obj(), "session", json_object_new_int(session_id));
	if (!request.send(socket_handle, ++seq_id, session_id, NULL, 0, socket_timeout) || 
		!response.receive(socket_handle, seq_id, session_id, socket_timeout))
		return(FALSE);

	method_array = json_object_object_get(json_object_object_get(
		response.get_json_obj(), "params"), "method");
	if (method_array == NULL)
		return(FALSE);

	for(int index = 0; index < json_object_array_length(method_array); index++)
	{
		json_object *method;

		method = json_object_array_get_idx(method_array, index);
		printf("Method = %s\n", json_object_get_string(method));
	}
	return(TRUE);
}

BOOL dahuarpc::record_finder_factory_create(const char *name, int32_t *finder_id)
{
	json_object *params;
	dahuarpc_request request;
	dahuarpc_response response;

	params = json_object_new_object();
	json_object_object_add(request.get_json_obj(), "id", json_object_new_int(++query_id));
	json_object_object_add(request.get_json_obj(), "method", json_object_new_string("RecordFinder.factory.create"));
	json_object_object_add(params, "name", json_object_new_string(name));
	json_object_object_add(request.get_json_obj(), "params", params);
	json_object_object_add(request.get_json_obj(), "session", json_object_new_int(session_id));
	if (!request.send(socket_handle, ++seq_id, session_id, NULL, 0, socket_timeout) || 
		!response.receive(socket_handle, seq_id, session_id, socket_timeout))
		return(FALSE);

	*finder_id = json_object_get_int(json_object_object_get(response.get_json_obj(), "result"));
	return(TRUE);
}

BOOL dahuarpc::record_finder_destroy(int32_t finder_id)
{
	dahuarpc_request request;
	dahuarpc_response response;

	json_object_object_add(request.get_json_obj(), "id", json_object_new_int(++query_id));
	json_object_object_add(request.get_json_obj(), "method", json_object_new_string("RecordFinder.destroy"));
	json_object_object_add(request.get_json_obj(), "object", json_object_new_int(finder_id));
	json_object_object_add(request.get_json_obj(), "session", json_object_new_int(session_id));
	return(request.send(socket_handle, ++seq_id, session_id, NULL, 0, socket_timeout) && 
		response.receive(socket_handle, seq_id, session_id, socket_timeout));
}

BOOL dahuarpc::record_finder_start_find(int32_t finder_id, const char *start_time, 
	const char *end_time)
{
	dahuarpc_request request;
	dahuarpc_response response;
	json_object *condition, *params;

	if (start_time == NULL || end_time == NULL)
	{
		condition = json_object_new_string("null");
	}
	else
	{
		struct tm tm;
		json_object *createtime_array;

		condition = json_object_new_object();
		createtime_array = json_object_new_array();

		memset(&tm, 0, sizeof(tm));
		strptime(start_time, "%Y-%m-%dT%H:%M:%S", &tm);
		json_object_array_add(createtime_array, json_object_new_int64(_mkgmtime(&tm)));

		memset(&tm, 0, sizeof(tm));
		strptime(start_time, "%Y-%m-%dT%H:%M:%S", &tm);
		json_object_array_add(createtime_array, json_object_new_int64(_mkgmtime(&tm)));

		json_object_object_add(condition, "CreateTime", createtime_array);
	}

	params = json_object_new_object();
	json_object_object_add(request.get_json_obj(), "id", json_object_new_int(++query_id));
	json_object_object_add(request.get_json_obj(), "method", json_object_new_string("RecordFinder.startFind"));
	json_object_object_add(request.get_json_obj(), "object", json_object_new_int(finder_id));
	json_object_object_add(params, "condition", condition);
	json_object_object_add(request.get_json_obj(), "params", params);
	json_object_object_add(request.get_json_obj(), "session", json_object_new_int(session_id));
	return(request.send(socket_handle, ++seq_id, session_id, NULL, 0, socket_timeout) && 
		response.receive(socket_handle, seq_id, session_id, socket_timeout));
}

BOOL dahuarpc::record_finder_do_find(int32_t finder_id, int32_t max_count, 
	dahuarpc_access_ctl_card_rec_proc rec_proc, void *context, 
	int32_t *rec_count)
{
	dahuarpc_request request;
	dahuarpc_response response;
	json_object *params, *record_array;

	params = json_object_new_object();
	json_object_object_add(request.get_json_obj(), "id", json_object_new_int(++query_id));
	json_object_object_add(request.get_json_obj(), "method", json_object_new_string("RecordFinder.doFind"));
	json_object_object_add(request.get_json_obj(), "object", json_object_new_int(finder_id));
	json_object_object_add(params, "count", json_object_new_int(max_count));
	json_object_object_add(request.get_json_obj(), "params", params);
	json_object_object_add(request.get_json_obj(), "session", json_object_new_int(session_id));
	if (!request.send(socket_handle, ++seq_id, session_id, NULL, 0, socket_timeout) || 
		!response.receive(socket_handle, seq_id, session_id, socket_timeout))
		return(FALSE);

	record_array = json_object_object_get(json_object_object_get(response.get_json_obj(), "params"), "records");
	if (record_array == NULL)
		return(FALSE);

	for(int index = 0; index < json_object_array_length(record_array); index++)
	{
		time_t date_time;
		json_object *record;
		char str_date_time[32 + 1];

		record = json_object_array_get_idx(record_array, index);
		date_time = json_object_get_int64(json_object_object_get(record, "DateTime"));
		strftime(str_date_time, sizeof(str_date_time) - 1, "%Y-%m-%dT%H:%M:%S", gmtime(&date_time));

		if (!rec_proc(context, json_object_get_string(json_object_object_get(record, "RecNo")), 
			json_object_get_string(json_object_object_get(record, "CardNo")), 
			json_object_get_string(json_object_object_get(record, "CardType")), 
			json_object_get_string(json_object_object_get(record, "Password")), 
			str_date_time, json_object_get_string(json_object_object_get(record, "Status")), 
			json_object_get_string(json_object_object_get(record, "Method")), 
			json_object_get_string(json_object_object_get(record, "Door")), 
			json_object_get_string(json_object_object_get(record, "ReaderID"))))
			return(FALSE);
	}

	*rec_count = json_object_get_int(json_object_object_get(json_object_object_get(
		response.get_json_obj(), "params"), "found"));
	return(TRUE);
}

BOOL dahuarpc::keep_alive(int32_t timeout)
{
	json_object *params;
	dahuarpc_request request;
	dahuarpc_response response;

	params = json_object_new_object();
	json_object_object_add(request.get_json_obj(), "id", json_object_new_int(++query_id));
	json_object_object_add(request.get_json_obj(), "method", json_object_new_string("global.keepAlive"));
	json_object_object_add(params, "timeout", json_object_new_int(timeout));
	json_object_object_add(request.get_json_obj(), "params", params);
	json_object_object_add(request.get_json_obj(), "session", json_object_new_int(session_id));
	return(request.send(socket_handle, ++seq_id, session_id, NULL, 0, socket_timeout) && 
		response.receive(socket_handle, seq_id, session_id, socket_timeout));
}

BOOL dahuarpc::event_manager_factory_instance(int32_t *manager_id)
{
	dahuarpc_request request;
	dahuarpc_response response;

	json_object_object_add(request.get_json_obj(), "id", json_object_new_int(++query_id));
	json_object_object_add(request.get_json_obj(), "method", json_object_new_string("eventManager.factory.instance"));
	json_object_object_add(request.get_json_obj(), "session", json_object_new_int(session_id));
	if (!request.send(socket_handle, ++seq_id, session_id, NULL, 0, socket_timeout) || 
		!response.receive(socket_handle, seq_id, session_id, socket_timeout))
		return(FALSE);

	*manager_id = json_object_get_int(json_object_object_get(response.get_json_obj(), "result"));
	return(TRUE);
}

BOOL dahuarpc::event_manager_attach(int32_t manager_id, boolean *result)
{
	dahuarpc_request request;
	dahuarpc_response response;
	json_object *params, *codes_array;

	params = json_object_new_object();
	codes_array = json_object_new_array();
	json_object_object_add(request.get_json_obj(), "id", json_object_new_int(++query_id));
	json_object_object_add(request.get_json_obj(), "method", json_object_new_string("eventManager.attach"));
	json_object_object_add(request.get_json_obj(), "object", json_object_new_int(manager_id));
	json_object_array_add(codes_array, json_object_new_string("All"));
	json_object_object_add(params, "codes", codes_array);
	json_object_object_add(request.get_json_obj(), "params", params);
	json_object_object_add(request.get_json_obj(), "session", json_object_new_int(session_id));
	if (!request.send(socket_handle, ++seq_id, session_id, NULL, 0, socket_timeout) || 
		!response.receive(socket_handle, seq_id, session_id, socket_timeout))
		return(FALSE);

	*result = json_object_get_boolean(json_object_object_get(response.get_json_obj(), "result"));
	return(TRUE);
}

const char* dahuarpc::dahua_md5_hash(char *hash_buffer, json_object *realm, 
	json_object *random, const char *username, const char *password) const
{
	int i;
	MD5_CTX ctx;
	unsigned char digest[16];

	/* PWDDB_HASH */
	MD5_Init(&ctx);
	MD5_Update(&ctx, username, strlen(username));
	MD5_Update(&ctx, ":", 1);
	MD5_Update(&ctx, json_object_get_string(realm), 
		json_object_get_string_len(realm));
	MD5_Update(&ctx, ":", 1);
	MD5_Update(&ctx, password, strlen(password));
	MD5_Final(digest, &ctx);
	for(i = 0; i < sizeof(digest); i++)
		sprintf(hash_buffer + i * 2, "%02X", digest[i]);

	/* RANDOM_HASH */
	MD5_Init(&ctx);
	MD5_Update(&ctx, username, strlen(username));
	MD5_Update(&ctx, ":", 1);
	MD5_Update(&ctx, json_object_get_string(random), 
		json_object_get_string_len(random));
	MD5_Update(&ctx, ":", 1);
	MD5_Update(&ctx, hash_buffer, strlen(hash_buffer));
	MD5_Final(digest, &ctx);
	for(i = 0; i < sizeof(digest); i++)
		sprintf(hash_buffer + i * 2, "%02X", digest[i]);

	return(hash_buffer);
}
