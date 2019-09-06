#ifndef __DAHUARPC_REQUEST_H__
#define __DAHUARPC_REQUEST_H__
#include "json.h"
#include "dahuarpcdef.h"

class dahuarpc_request
{
	json_object *json_obj;

public:
	dahuarpc_request();
	~dahuarpc_request();

	void delete_objects();
	json_object* get_json_obj();

	BOOL send(SOCKET socket_handle, uint32_t seq_id, uint32_t session_id, 
		const void *data, int data_len, int timeout);

protected:
	void* create_packet(uint32_t seq_id, uint32_t session_id, 
		const void *data, int data_len, int *pack_len);
};

#endif /* __DAHUARPC_REQUEST_H__ */
