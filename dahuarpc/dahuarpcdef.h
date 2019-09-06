#ifndef __DAHUARPCDEF_H__
#define __DAHUARPCDEF_H__

#define DAHUARPC_MAGIC_LEN	8

#pragma pack(push, 1)
struct dahuarpc_header
{
	uint8_t magic[DAHUARPC_MAGIC_LEN];
	uint32_t session_id;
	uint32_t seq_id;
	uint32_t length;
	uint32_t unknown1;
	uint32_t json_len;
	uint32_t unknown2;
};
#pragma pack(pop)

extern const uint8_t dahuarpc_magic[DAHUARPC_MAGIC_LEN];

#endif /* __DAHUARPCDEF_H__ */
