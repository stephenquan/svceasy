// Survey123Service.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include "SCManager.h"
#include "FormatMessage.h"
#include "QDebug.h"
#include "XMLConfig.h"

class CServiceApp
{
public:
	typedef enum ACTIONTYPE
	{
		ActionTypeInstall,
		ActionTypeUninstall,
		ActionTypeService,
		ActionTypeTest
	} ACTIONTYPE;

public:
	CServiceApp(int argc, wchar_t* argv[]);

	HRESULT ParseArgs(int argc, wchar_t* argv[]);
	HRESULT Run();
	HRESULT ActionInstall();
	HRESULT ActionUninstall();
	HRESULT ActionService();
	HRESULT ActionTest();
	void ServiceStart(DWORD dwArgc, LPWSTR *lpszArgv);
	void ServiceStop();
	void ServicePause();
	void ServiceContinue();
	void ServiceShutdown();

protected:
	static CServiceApp* __instance;
	CSCManager m_SCManager;
	ACTIONTYPE m_actionType;
	SERVICE_STATUS m_status;
	SERVICE_STATUS_HANDLE m_statusHandle;
	HANDLE m_hThread;
	DWORD m_dwThreadId;
	bool m_Running;
	WCHAR m_szConfigPath[1024];

	void SetServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode = NO_ERROR, DWORD dwWaitHint = 0);

	static void WINAPI __ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv) { __instance->ServiceMain(dwArgc, lpszArgv); }
	static void WINAPI __ServiceCtrlHandler(DWORD dwCtrl) { __instance->ServiceCtrlHandler(dwCtrl); }
	void WINAPI ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv);
	void WINAPI ServiceCtrlHandler(DWORD dwCtrl);

	static DWORD WINAPI __ThreadProc(LPVOID lpParam) { return ((CServiceApp*)lpParam)->ThreadProc(); }
	DWORD WINAPI ThreadProc();

};

CServiceApp* CServiceApp::__instance = NULL;

CServiceApp::CServiceApp(int argc, wchar_t* argv[]) :
	m_actionType(ActionTypeService),
	m_status({}),
	m_statusHandle(NULL),
	m_hThread(NULL),
	m_dwThreadId(0),
	m_Running(false)
{
	__instance = this;

	wcscpy_s(m_szConfigPath, 1024, argv[0]);
	LPWSTR pDot = wcsrchr(m_szConfigPath, '.');
	if (pDot)
	{
		wcscpy_s(pDot, 5, L".cfg");
	}
	else
	{
		wcscat_s(m_szConfigPath, 1024, L".cfg");
	}

	ParseArgs(argc, argv);

	TCHAR szPath[1024] = { };
	GetModuleFileName(::GetModuleHandle(NULL), szPath, 1024);

	m_SCManager.SetStartType(SERVICE_DEMAND_START);
	m_SCManager.SetBinaryPathName(szPath);
	//m_SCManager.SetDependencies(L"");
	//m_SCManager.SetServiceStartName(L"NT AUTHORITY\\LocalService");

	m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	m_status.dwCurrentState = SERVICE_START_PENDING;
	m_status.dwControlsAccepted = 0;
	m_status.dwControlsAccepted |= SERVICE_ACCEPT_STOP;
	m_status.dwControlsAccepted |= SERVICE_ACCEPT_SHUTDOWN;
	m_status.dwControlsAccepted |= SERVICE_ACCEPT_PAUSE_CONTINUE;
	m_status.dwWin32ExitCode = NO_ERROR;
	m_status.dwServiceSpecificExitCode = 0;
	m_status.dwCheckPoint = 0;
	m_status.dwWaitHint = 0;
}

HRESULT CServiceApp::ParseArgs(int argc, wchar_t* argv[])
{
	m_actionType = ActionTypeService;

	for (int i = 1; i < argc; i++)
	{
		if ((*argv[1] == L'-') || (*argv[1] == '/'))
		{
			if (_wcsicmp(L"install", argv[i] + 1) == 0 || _wcsicmp(L"i", argv[i] + 1) == 0)
			{
				m_actionType = ActionTypeInstall;
				continue;
			}

			if (_wcsicmp(L"uninstall", argv[i] + 1) == 0 || _wcsicmp(L"u", argv[i]+ 1) == 0)
			{
				m_actionType = ActionTypeUninstall;
				continue;
			}

			if (_wcsicmp(L"test", argv[i] + 1) == 0 || _wcsicmp(L"t", argv[i] + 1) == 0)
			{
				m_actionType = ActionTypeTest;
				continue;
			}
		}
	}

	return S_OK;
}

HRESULT CServiceApp::Run()
{
	HRESULT hr = S_OK;

	CXMLConfig config;
	TCHAR szServiceName[1024] = { };
	TCHAR szDisplayName[1024] = { };
	TCHAR szDescription[1024] = { };
	WCHAR szApplicationName[1024] = { };
	WCHAR szCommandLine[1024] = { };
	config.load(m_szConfigPath);
	config.getValue(OLESTR("Service"), OLESTR("serviceName"), szServiceName, 1024, OLESTR("svceasy"));
	config.getValue(OLESTR("Service"), OLESTR("displayName"), szDisplayName, 1024, OLESTR("svceasy"));
	config.getValue(OLESTR("Service"), OLESTR("description"), szDescription, 1024, OLESTR("svceasy"));
	config.getValue(OLESTR("Service"), OLESTR("applicationName"), szApplicationName, 1024, OLESTR("%COMSPEC%"));
	config.getValue(OLESTR("Service"), OLESTR("commandLine"), szCommandLine, 1024, OLESTR("%COMSPEC%"));
	config.save(m_szConfigPath);

	m_SCManager.SetServiceName(szServiceName);
	m_SCManager.SetDisplayName(szDisplayName);
	m_SCManager.SetDescription(szDescription);
	m_SCManager.SetApplicationName(szApplicationName);
	m_SCManager.SetCommandLine(szCommandLine);

	switch (m_actionType)
	{
	case ActionTypeInstall:
		return ActionInstall();
	case ActionTypeUninstall:
		return ActionUninstall();
	case ActionTypeService:
		return ActionService();
	case ActionTypeTest:
		return ActionTest();
	}

	return S_OK;
}

HRESULT CServiceApp::ActionInstall()
{
	HRESULT hr = S_OK;
	qDebug() << "Installing " << (LPCWSTR) m_SCManager.ServiceName() << " ... ";
	hr = m_SCManager.CreateService();
	if (FAILED(hr))
	{
		qDebug() << (LPCWSTR)CFormatMessage(GetLastError()) << "\n";
		return hr;
	}
	qDebug() << "done.";
	return S_OK;
}

HRESULT CServiceApp::ActionUninstall()
{
	HRESULT hr = S_OK;
	qDebug() <<  "Uninstalling " << (LPCWSTR) m_SCManager.ServiceName() << " ... ";
	hr = m_SCManager.DeleteService();
	if (FAILED(hr))
	{
		qDebug() << (LPCWSTR)CFormatMessage(GetLastError()) << "\n";
		return hr;
	}
	qDebug() << "done.";
	return S_OK;
}

HRESULT CServiceApp::ActionService()
{
	SERVICE_TABLE_ENTRY serviceTable[] =
	{
		{ m_SCManager.ServiceName(), __ServiceMain },
		{ NULL, NULL }
	};

	::StartServiceCtrlDispatcher(serviceTable);
	return S_OK;
}

HRESULT CServiceApp::ActionTest()
{
	ServiceStart(0, NULL);
	ServiceShutdown();
	return S_OK;
}


void CServiceApp::SetServiceStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	if (m_actionType == ActionTypeTest) return;
	m_status.dwCurrentState = dwCurrentState;
	m_status.dwWin32ExitCode = dwWin32ExitCode;
	m_status.dwWaitHint = dwWaitHint;
	m_status.dwCheckPoint++;
	::SetServiceStatus(m_statusHandle, &m_status);
}

void WINAPI CServiceApp::ServiceMain(DWORD dwArgc, LPWSTR *lpszArgv)
{
	m_statusHandle = RegisterServiceCtrlHandler(m_SCManager.ServiceName(), __ServiceCtrlHandler);
	ServiceStart(dwArgc, lpszArgv);
}

void CServiceApp::ServiceStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
	SetServiceStatus(SERVICE_START_PENDING);

	m_Running = true;
	m_hThread = CreateThread(NULL, 0, __ThreadProc, this, 0, &m_dwThreadId);
	if (m_hThread == NULL)
	{
		m_Running = false;
		SetServiceStatus(SERVICE_STOPPED);
		return;
	}

	SetServiceStatus(SERVICE_RUNNING);
}

void CServiceApp::ServiceStop()
{
	SetServiceStatus(SERVICE_STOP_PENDING);
	m_Running = false;
	WaitForSingleObject(m_hThread, INFINITE);
	SetServiceStatus(SERVICE_STOPPED);
}

void CServiceApp::ServicePause()
{
	SetServiceStatus(SERVICE_PAUSE_PENDING);
	SuspendThread(m_hThread);
	SetServiceStatus(SERVICE_PAUSED);
}

void CServiceApp::ServiceContinue()
{
	SetServiceStatus(SERVICE_CONTINUE_PENDING);
	ResumeThread(m_hThread);
	SetServiceStatus(SERVICE_RUNNING);
}

void CServiceApp::ServiceShutdown()
{
	m_Running = false;
	WaitForSingleObject(m_hThread, INFINITE);
	SetServiceStatus(SERVICE_STOPPED);
}

DWORD WINAPI CServiceApp::ThreadProc()
{
	BOOL ok = TRUE;
	HANDLE hStdOutPipeRead = NULL;
	HANDLE hStdOutPipeWrite = NULL;

	SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
	ok = CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &sa, 0);
	if (ok == FALSE)
	{
		SetServiceStatus(SERVICE_STOPPED);
		return 0;
	}

	WCHAR szApplicationName[1024] = { };
	ExpandEnvironmentStringsW(m_SCManager.ApplicationName(), szApplicationName, 1024);
	WCHAR szCommandLine[1024] = { };
	ExpandEnvironmentStringsW(m_SCManager.CommandLine(), szCommandLine, 1024);

	STARTUPINFO si = { };
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdError = hStdOutPipeWrite;
	si.hStdOutput = hStdOutPipeWrite;
	PROCESS_INFORMATION pi = { };
	LPCWSTR lpApplicationName = szApplicationName;
	LPWSTR lpCommandLine = szCommandLine;
	LPSECURITY_ATTRIBUTES lpProcessAttributes = NULL;
	LPSECURITY_ATTRIBUTES lpThreadAttribute = NULL;
	BOOL bInheritHandles = TRUE;
	DWORD dwCreationFlags = 0;
	LPVOID lpEnvironment = NULL;
	LPCWSTR lpCurrentDirectory = NULL;
	ok = CreateProcess(
		lpApplicationName,
		lpCommandLine,
		lpProcessAttributes,
		lpThreadAttribute,
		bInheritHandles,
		dwCreationFlags,
		lpEnvironment,
		lpCurrentDirectory,
		&si,
		&pi);
	if (ok == FALSE) return -1;

	CloseHandle(hStdOutPipeWrite);

	while (true)
	{
		DWORD dwWait = WaitForSingleObject(pi.hProcess, 2000);
		if (dwWait == WAIT_OBJECT_0) break;
	}

	// Clean up and exit.
	CloseHandle(hStdOutPipeRead);
	DWORD dwExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);
	SetServiceStatus(SERVICE_STOPPED);
	return 0;
}

void WINAPI CServiceApp::ServiceCtrlHandler(DWORD dwCtrl)
{
	switch (dwCtrl)
	{
	case SERVICE_CONTROL_STOP: 
		ServiceStop();
		break;
	case SERVICE_CONTROL_PAUSE: 
		ServicePause();
		break;
	case SERVICE_CONTROL_CONTINUE: 
		ServiceContinue();
		break;
	case SERVICE_CONTROL_SHUTDOWN: 
		ServiceShutdown();
		break;
	case SERVICE_CONTROL_INTERROGATE:
		break;
	default:
		break;
	}
}

int wmain(int argc, wchar_t* argv[])
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	CServiceApp(argc, argv).Run();
	CoUninitialize();
	return 0;
}
