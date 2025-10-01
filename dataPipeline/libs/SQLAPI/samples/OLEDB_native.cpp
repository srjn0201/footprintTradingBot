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

#include "oledbAPI.h"

void fetchTable1(SACommand &cmd)
{
	cmd.setCommandText(_TSA("select * from Table1"));
	cmd.Execute();
	while (cmd.FetchNext())
	{
		for (int i = 1; i <= cmd.FieldCount(); ++i)
		{
			printf(i > 1 ? ", %s = %s" : "%s = %s",
				cmd.Field(i).Name().GetMultiByteChars(),
				cmd.Field(i).asString().GetMultiByteChars());
		}
		printf("\n");
	}
	printf("\n");
}

void printCatalog(SAConnection& con)
{
	SACommand cmd(&con);
	cmd.Open();

	oledbAPI* pAPI = (oledbAPI*)con.NativeAPI();
	oledbConnectionHandles* pConnectionHandles = (oledbConnectionHandles * )con.NativeHandles();
	oledbCommandHandles* pCommandHandles = (oledbCommandHandles * )cmd.NativeHandles();

	IDBSchemaRowset* pIDBSchemaRowset;
	HRESULT hr = pConnectionHandles->pIDBCreateCommand->QueryInterface(IID_IDBSchemaRowset, (void**)&pIDBSchemaRowset);
	pAPI->Check(hr, pConnectionHandles->pIDBCreateCommand, IID_IDBCreateCommand);
	
	IRowset* pIRowset;
	hr = pIDBSchemaRowset->GetRowset(NULL, DBSCHEMA_TABLES,
		0, NULL, IID_IRowset, 0, NULL, (IUnknown**)&pIRowset);
	
	pAPI->Check(hr, pIDBSchemaRowset, IID_IDBSchemaRowset);
	pCommandHandles->pIRowset = pIRowset;

	printf("-----------------------------------------------------------\n");
	while (cmd.FetchNext())
	{
		for (int i = 1; i <= cmd.FieldCount(); ++i)
		{
			SAField& f = cmd.Field(i);
			printf("%s = %s\n", f.Name().GetMultiByteChars(), f.asString().GetMultiByteChars());
		}
		printf("\n");
	}
	printf("-----------------------------------------------------------\n");
}

void printCatalogSQL(SAConnection& con)
{
	SACommand cmd(&con);
	cmd.setCommandText(_TSA("select * from MSysObjects where Type=1 and Flags=0"));
	cmd.Execute();
	while (cmd.FetchNext())
	{
		for (int i = 1; i <= cmd.FieldCount(); ++i)
		{
			printf(i > 1 ? ", %s = %s" : "%s = %s",
				cmd.Field(i).Name().GetMultiByteChars(),
				cmd.Field(i).asString().GetMultiByteChars());
		}
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char** argv)
{
	setlocale(LC_ALL, "");

	SAConnection con;

	try
	{
		// The connection string should contain the provider name before '!' but
		// aslo the option "OLEDBProvider" can be used. And the rest like with MSSQL:
		// <data source>@<initial catalog>;<provider init string>
		con.Connect(_TSA("Microsoft.ACE.OLEDB.12.0!C:\\src\\sample_database.accdb@"), _TSA("Admin"), _TSA(""), SA_OLEDB_Client);
		//con.Connect(_TSA("MSOLEDBSQL19!bedlam-m,1433@test;Encrypt=Optional"), _TSA("sa"), _TSA("Java_1970"), SA_OLEDB_Client);

		long lClientVersion = con.ClientVersion();
		printf("ClientVersion = %d.%d\n", HIWORD(lClientVersion), LOWORD(lClientVersion));
		long lServerVersion = con.ServerVersion();
		printf("Server Version = %d.%d %s\n\n", HIWORD(lServerVersion), LOWORD(lServerVersion),
			con.ServerVersionString().GetMultiByteChars());

		printCatalog(con);

		SACommand cmd(&con);
		cmd.setCommandText(_TSA("update Table1 set big_num=:1 where ID=:2"));
		// Set as 8-byte integer doesn't work even the reading returns VT_I8 type
		//cmd.Param(1).setAsInt64() = 111111111991ll;
		cmd.Param(1).setAsNumeric() = _TSA("111111111991");
		cmd.Param(2).setAsInt32() = 1;
		cmd.Execute();

		fetchTable1(cmd);

		cmd.setCommandText(_TSA("update Table1 set big_num=:1 where ID=:2"));
		cmd.Param(1).setAsNumeric() = _TSA("111111111999");
		cmd.Param(2).setAsInt32() = 1;
		cmd.Execute();

		fetchTable1(cmd);

		printCatalogSQL(con);
	}
	catch (SAException& x)
	{
		printf("ERROR:\n%s\n", x.ErrText().GetMultiByteChars());
	}

	return 0;
}