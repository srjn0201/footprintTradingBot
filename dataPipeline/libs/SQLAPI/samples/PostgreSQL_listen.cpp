#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <locale.h>

#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#endif

#include <SQLAPI.h>
#include <samisc.h>

#include "pgAPI.h"

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "");

	try
	{
		SAConnection con;
		con.Connect(_TSA("localhost@test"), _TSA("postgres"), _TSA("java"), SA_PostgreSQL_Client);

		SACommand cmd(&con, _TSA("LISTEN TBL2"));
		cmd.Execute();

		pgAPI* api = (pgAPI * )con.NativeAPI();
		pgConnectionHandles* handles = (pgConnectionHandles*)con.NativeHandles();

		api->PQconsumeInput(handles->conn);
		int nnotifies = 0;
		PGnotify* notify;
		while (nnotifies < 4)
		{
			api->PQconsumeInput(handles->conn);
			while ((notify = api->PQnotifies(handles->conn)) != NULL)
			{
				fprintf(stderr,
					"ASYNC NOTIFY of '%s' received from backend PID %d\n",
					notify->relname, notify->be_pid);
				api->PQfreemem(notify);
				nnotifies++;
				api->PQconsumeInput(handles->conn);
			}
		}
	}
	catch (SAException& x)
	{
		printf("ERROR:\n%s\n", x.ErrText().GetMultiByteChars());
	}

	return 0;
}