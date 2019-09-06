#ifndef __DAHUARPC_H__
#define __DAHUARPC_H__
#include "json.h"
#include "dahuarpcexp.h"
#include "dahuarpcdef.h"

class dahuarpc
{
	uint32_t seq_id;
	uint32_t query_id;
	int socket_timeout;
	uint32_t session_id;
	SOCKET socket_handle;
	const char *client_type;

public:
	dahuarpc();
	~dahuarpc();

	BOOL connect(const char *addr, unsigned int port, int timeout);
	void disconnect();

	BOOL login(const char *username, const char *password);
	BOOL record_finder_get_caps();
	BOOL record_finder_list_method();
	BOOL record_finder_factory_create(const char *name, int32_t *finder_id);
	BOOL record_finder_destroy(int32_t finder_id);
	BOOL record_finder_start_find(int32_t finder_id, const char *start_time, const char *end_time);
	BOOL record_finder_do_find(int32_t finder_id, int32_t max_count, 
		dahuarpc_access_ctl_card_rec_proc rec_proc, void *context, 
		int32_t *rec_count);

	BOOL keep_alive(int32_t timeout);
	BOOL event_manager_factory_instance(int32_t *manager_id);
	BOOL event_manager_attach(int32_t manager_id, boolean *result);

protected:
	const char* dahua_md5_hash(char *hash_buffer, json_object *realm, 
		json_object *random, const char *username, 
		const char *password) const;
};

#endif /* __DAHUARPC_H__ */
