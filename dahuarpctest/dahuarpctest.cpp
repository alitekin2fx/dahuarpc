#include "precomp.h"

int rec_count;

BOOL __stdcall access_ctl_card_rec_proc(void *context, const char *recno, const char *cardno, 
	const char *cardtype, const char *password, const char *datetime, const char *status, 
	const char *method, const char *door, const char *readerid)
{
	if (_ftprintf((FILE*)context, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", recno, 
		cardno, cardtype, password, datetime, status, method, door, readerid) < 0)
	{
		perror("Could not write to file");
		return(FALSE);
	}

	rec_count++;
	return(TRUE);
}

int _tmain(int argc, _TCHAR* argv[])
{
	FILE *csvfile;
	HDAHUARPC handle;
	DWORD ticks_start;

	if (argc < 6)
	{
		printf("usage: dahuarpctest.exe <addr> <port> <username> <password> <csvfile>\n");
		return(0);
	}

	if (!dahuarpc_init())
	{
		perror("'dahuarpc_init' failed");
		return(FALSE);
	}

	handle = dahuarpc_open(argv[1], _ttol(argv[2]), 10000);
	if (handle == NULL)
	{
		perror("'dahuarpc_open' failed");
		dahuarpc_cleanup();
		return(FALSE);
	}

	if (!dahuarpc_login(handle, argv[3], argv[4]))
	{
		perror("'dahuarpc_login' failed");
		dahuarpc_close(handle);
		dahuarpc_cleanup();
		return(FALSE);
	}

	csvfile = _tfopen(argv[5], TEXT("w"));
	if (csvfile == NULL)
	{
		perror("Could not open file");
		dahuarpc_close(handle);
		dahuarpc_cleanup();
		return(FALSE);
	}

	if (_ftprintf(csvfile, "recno\tcardno\tcardtype\tpassword\tdatetime\tstatus"
		"\tmethod\tdoor\treaderid\n") < 0)
	{
		perror("Could not write to file");
		dahuarpc_close(handle);
		dahuarpc_cleanup();
		return(FALSE);
	}

	rec_count = 0;
	ticks_start = GetTickCount();
	if (!dahuarpc_get_access_ctl_card_recs(handle, NULL, NULL, 
		access_ctl_card_rec_proc, csvfile))
	{
		perror("'dahuarpc_get_access_ctl_card_recs' failed");
		dahuarpc_close(handle);
		dahuarpc_cleanup();
		return(FALSE);
	}

	printf("%d records saved in %d seconds.\n", rec_count, (GetTickCount() - ticks_start) / 1000);
	dahuarpc_close(handle);
	dahuarpc_cleanup();
	return(TRUE);
}

