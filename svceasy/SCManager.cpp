#include "pch.h"
#include "SCManager.h"
#include "QDebug.h"

CSCManager::CSCManager() :
	m_hSCManager(NULL),
	m_MachineName(NULL),
	m_DatabaseName(NULL),
	m_hService(NULL),
	m_ServiceName(NULL),
	m_DisplayName(NULL),
	m_ServiceType(SERVICE_WIN32_OWN_PROCESS),
	m_StartType(0),
	m_ErrorControl(SERVICE_ERROR_NORMAL),
	m_BinaryPathName(NULL),
	m_LoadOrderGroup(NULL),
	m_TagId(0),
	m_Dependencies(NULL),
	m_ServiceStartName(NULL),
	m_Password(NULL),
	m_Description(NULL),
	m_ApplicationName(NULL),
	m_CommandLine(NULL),
	m_DebugMode(0)
{
	SetApplicationName(OLESTR("%COMSPEC%"));
	SetCommandLine(OLESTR("%COMSPEC% /c path.bat"));
}

CSCManager::~CSCManager()
{
	CloseAllHandles();
	SetMachineName(NULL);
	SetDatabaseName(NULL);
	SetServiceName(NULL);
	SetDisplayName(NULL);
	SetBinaryPathName(NULL);
	SetLoadOrderGroup(NULL);
	SetDependencies(NULL);
	SetServiceStartName(NULL);
	SetPassword(NULL);
	SetDescription(NULL);
	SetApplicationName(NULL);
	SetCommandLine(NULL);
}

HRESULT CSCManager::CreateService()
{
	HRESULT hr = S_OK;
	TCHAR szDebug[1024] = { };

	CloseAllHandles();

	m_hSCManager = ::OpenSCManager(MachineName(), DatabaseName(), SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
	if (m_DebugMode > 0) qDebug() << "OpenSCManager -> " << m_hSCManager << "\n";
	if (m_hSCManager == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		if (m_DebugMode > 0) qDebug() << "hr -> " << hr << "\n";
		return hr;
	}

	if (m_DebugMode > 0)
	{
		qDebug() << "ServiceName: " << ServiceName() << "\n";
		qDebug() << "DisplayName: " << DisplayName() << "\n";
		qDebug() << "ServiceType: " << (int)ServiceType() << SERVICE_WIN32_OWN_PROCESS << "\n";
		qDebug() << "StartType: " << (int)StartType() << " " << SERVICE_DEMAND_START << "\n";
		qDebug() << "ErrorControl: " << (int)ErrorControl() << SERVICE_ERROR_NORMAL << "\n";
		qDebug() << "BinaryPathName: " << BinaryPathName() << "\n";
		qDebug() << "LoadOrderGroup: " << LoadOrderGroup() << "\n";
		qDebug() << "Dependencies: " << Dependencies() << "\n";
		qDebug() << "ServiceStartName: " << ServiceStartName() << "\n";
		qDebug() << "Password: " << Password() << "\n";
		qDebug() << "Description: " << Description() << "\n";
	}

	m_hService = ::CreateService(
		m_hSCManager,
		ServiceName(),
		DisplayName(),
		SERVICE_ALL_ACCESS,
		ServiceType(),
		StartType(),
		ErrorControl(),
		BinaryPathName(),
		LoadOrderGroup(),
		NULL,
		Dependencies(),
		ServiceStartName(),
		Password());
	if (m_DebugMode > 0) qDebug() << "CreateService -> " << m_hService << "\n";
	if (m_hService == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		if (m_DebugMode > 0) qDebug() << "hr -> " << hr << "\n";
		return hr;
	}

	SERVICE_DESCRIPTIONW serviceDesc = { };
	serviceDesc.lpDescription = Description();
	
	BOOL ok = ChangeServiceConfig2W(
		m_hService,
		SERVICE_CONFIG_DESCRIPTION,
		&serviceDesc
	);
	if (m_DebugMode > 0) qDebug() << "ChangeServiceConfig2A -> " << m_hService << " " << ok << "\n";

	CloseAllHandles();
	return S_OK;
}

HRESULT CSCManager::DeleteService()
{
	HRESULT hr = S_OK;

	CloseAllHandles();

	m_hSCManager = ::OpenSCManager(MachineName(), DatabaseName(), SC_MANAGER_CONNECT);
	if (m_hSCManager == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	m_hService = ::OpenService(m_hSCManager, ServiceName(), SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
	if (m_hService == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	SERVICE_STATUS ServiceStatus = { };
	if (ControlService(m_hService, SERVICE_CONTROL_STOP, &ServiceStatus))
	{
		Sleep(1000);

		while (QueryServiceStatus(m_hService, &ServiceStatus))
		{
			if (ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING)
			{
				break;
			}
			else break;

			Sleep(1000);
		}
	}

	BOOL ok = ::DeleteService(m_hService);
	if (ok == FALSE)
	{
		return S_FALSE;
	}

	CloseAllHandles();
	
	return S_OK;
}

HRESULT CSCManager::SetValue(BSTR& bstr, LPCWSTR value)
{
	if (bstr != NULL)
	{
		SysFreeString(bstr);
		bstr = NULL;
	}

	if (value == NULL)
	{
		return S_OK;
	}

	bstr = SysAllocString(value);
	if (bstr == NULL)
	{
		return E_OUTOFMEMORY;
	}

	return S_OK;
}

void CSCManager::CloseAllHandles()
{
	if (m_hService != NULL)
	{
		CloseServiceHandle(m_hService);
		m_hService = NULL;
	}

	if (m_hSCManager != NULL)
	{
		CloseServiceHandle(m_hSCManager);
		m_hSCManager = NULL;
	}
}
