#ifndef __DAHUARPC_RESPONSE_H__
#define __DAHUARPC_RESPONSE_H__
#include "json.h"
#include "dahuarpcdef.h"

class dahuarpc_response
{
	void *data;
	int data_len;
	json_object *json_obj;

public:
	dahuarpc_response();
	~dahuarpc_response();

	void* get_data() const;
	int get_data_len() const;
	json_object* get_json_obj() const;

	BOOL receive(SOCKET socket_handle, uint32_t seq_id, 
		uint32_t session_id, int timeout);

protected:
	void delete_objects();
};

__inline
void* dahuarpc_response::get_data() const
{
	return(data);
}

__inline
int dahuarpc_response::get_data_len() const
{
	return(data_len);
}

__inline
json_object* dahuarpc_response::get_json_obj() const
{
	return(json_obj);
}

#endif /* __DAHUARPC_RESPONSE_H__ */
