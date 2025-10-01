// oledbAPI.h
//
//////////////////////////////////////////////////////////////////////

#if !defined(__OLEDBAPI_H__)
#define __OLEDBAPI_H__

#include "SQLAPI.h"

// API header(s)
#include <oledb.h>

// API declarations
class SQLAPI_API oledbAPI : public IsaAPI
{
public:
	oledbAPI();

public:
	virtual void InitializeClient(const SAOptions *pOptions);
	virtual void UnInitializeClient(const SAPI* pSAPI, const SAOptions* pOptions);

	virtual int GetClientVersion() const;

	virtual ISAConnection *NewConnection(SAConnection *pConnection);

	virtual void ThreadInit();
	virtual void ThreadEnd();

protected:
	void ResetAPI();

public:
	static void CheckAndFreePropertySets(ULONG cPropertySets, DBPROPSET *rgPropertySets);
	BSTR SysAllocString(const SAString& s) const;
	void SysFreeString(BSTR bstr) const;
	static void CheckHRESULT(HRESULT hr);
	void Check(HRESULT hrOLEDB, IUnknown * pIUnknown, REFIID riid) const;
	void Check(const SAString &sCommandText, HRESULT hrOLEDB, IUnknown * pIUnknown, REFIID riid) const;
};

class SQLAPI_API oledbConnectionHandles : public saConnectionHandles
{
public:
	oledbConnectionHandles();

	IDBInitialize *pIDBInitialize;
	IDBDataSourceAdmin *pIDBDataSourceAdmin;

	IDBCreateCommand *pIDBCreateCommand;
	ITransactionLocal *pITransactionLocal;
};

class SQLAPI_API oledbCommandHandles : public saCommandHandles
{
public:
	oledbCommandHandles();

	ICommandText *pICommandText;
	IMultipleResults *pIMultipleResults;
	IRowset *pIRowset;
};

extern const GUID SA_DBPROPSET_DATASOURCEINFO;

#define SACON_OPTION_OLEDB_PROVIDER _TSA("OLEDBProvider")
#define SACON_OPTION_OLEDB_INIT _TSA("CoInitializeEx_COINIT")

#define SACMD_OPTION_OLEDB_COMMAND_PREPARE _TSA("ICommandPrepare")
#define SACMD_OPTION_OLEDB_COMMAND_EXECUTE_RIID _TSA("Execute_riid")
#define SAPAR_OPTION_OLEDB_USE_STREAM _TSA("UseStreamForLongOrLobParameters")

#endif // !defined(__OLEDBAPI_H__)
