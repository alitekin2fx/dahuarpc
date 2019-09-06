#ifndef __DAHUARPC_EXP_H__
#define __DAHUARPC_EXP_H__

DECLARE_HANDLE(HDAHUARPC);

typedef BOOL (__stdcall *dahuarpc_access_ctl_card_rec_proc)(void *context, const char *recno, 
	const char *cardno, const char *cardtype, const char *password, const char *datetime, 
	const char *status, const char *method, const char *door, const char *readerid);

#if defined(__cplusplus)
extern "C"{
#endif //__cplusplus

BOOL WINAPI dahuarpc_init();
void WINAPI dahuarpc_cleanup();

HDAHUARPC WINAPI dahuarpc_open(const char *addr, unsigned int port, int timeout);
void WINAPI dahuarpc_close(HDAHUARPC handle);

BOOL WINAPI dahuarpc_login(HDAHUARPC handle, const char *username, const char *password);
BOOL WINAPI dahuarpc_get_access_ctl_card_recs(HDAHUARPC handle, const char *start_time, 
	const char *end_time, dahuarpc_access_ctl_card_rec_proc rec_proc, void *context);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* __DAHUARPC_EXP_H__ */
