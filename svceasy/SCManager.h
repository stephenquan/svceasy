#ifndef SCManager_H
#define SCManager_H

class CSCManager
{
public:
    CSCManager();
    ~CSCManager();

    HRESULT CreateService();
    HRESULT DeleteService();

    // OpenSCManager parameters

    BSTR MachineName() const { return m_MachineName; }
    HRESULT SetMachineName(LPCOLESTR value) { return SetValue(m_MachineName, value); }
    BSTR DatabaseName() const { return m_DatabaseName; }
    HRESULT SetDatabaseName(LPCOLESTR value) { return SetValue(m_DatabaseName, value); }

    // CreateService parameters

    BSTR ServiceName() const { return m_ServiceName; }
    HRESULT SetServiceName(LPCOLESTR value) { return SetValue(m_ServiceName, value);  }
    BSTR DisplayName() const { return m_DisplayName; }
    HRESULT SetDisplayName(LPCOLESTR value) { return SetValue(m_DisplayName, value); }
    DWORD ServiceType() const { return m_ServiceType; }
    HRESULT SetServiceType(DWORD value) { return SetValue(m_ServiceType, value); }
    DWORD StartType() const { return m_StartType; }
    HRESULT SetStartType(DWORD value) { return SetValue(m_StartType, value); }
    DWORD ErrorControl() const { return m_ErrorControl; }
    HRESULT SetErrorControl(DWORD value) { return SetValue(m_ErrorControl, value); }
    BSTR BinaryPathName() const { return m_BinaryPathName; }
    HRESULT SetBinaryPathName(LPCOLESTR value) { return SetValue(m_BinaryPathName, value); }
    BSTR LoadOrderGroup() const { return m_LoadOrderGroup; }
    HRESULT SetLoadOrderGroup(LPCOLESTR value) { return SetValue(m_LoadOrderGroup, value); }
    DWORD TagId() const { return m_TagId; }
    HRESULT SetTagId(DWORD value) { return SetValue(m_TagId, value); }
    BSTR Dependencies() const { return m_Dependencies; }
    HRESULT SetDependencies(LPCOLESTR value) { return SetValue(m_Dependencies, value); }
    BSTR ServiceStartName() const { return m_ServiceStartName; }
    HRESULT SetServiceStartName(LPCOLESTR value) { return SetValue(m_ServiceStartName, value); }
    BSTR Password() const { return m_Password; }
    HRESULT SetPassword(LPCOLESTR value) { return SetValue(m_Password, value); }
    BSTR ApplicationName() const { return m_ApplicationName; }
    HRESULT SetApplicationName(LPCOLESTR value) { return SetValue(m_ApplicationName, value); }
    BSTR CommandLine() const { return m_CommandLine; }
    HRESULT SetCommandLine(LPCOLESTR value) { return SetValue(m_CommandLine, value); }

    // ChangeServiceConfig2A function

    BSTR Description() const { return m_Description; }
    HRESULT SetDescription(LPCOLESTR value) { return SetValue(m_Description, value); }

    /// Misc

    int DebugMode() const { return m_DebugMode;  }
    HRESULT SetDebugModel(int value) { m_DebugMode = value; return S_OK; }

protected:
    // OpenSCManager parameters

    SC_HANDLE   m_hSCManager;
    BSTR        m_MachineName;
    BSTR        m_DatabaseName;
    DWORD       m_OpenSCManagerDesiredAccess;

    // CreateService parameters

    SC_HANDLE   m_hService;
    BSTR        m_ServiceName;
    BSTR        m_DisplayName;
    DWORD       m_ServiceType;
    DWORD       m_StartType;
    DWORD       m_ErrorControl;
    BSTR        m_BinaryPathName;
    BSTR        m_LoadOrderGroup;
    DWORD       m_TagId;
    BSTR        m_Dependencies;
    BSTR        m_ServiceStartName;
    BSTR        m_Password;

    // ChangeServiceConfig2A
    BSTR        m_Description;

    // Other parameters
    BSTR        m_ApplicationName;
    BSTR        m_CommandLine;

    // Debug  Model
    int         m_DebugMode;

protected:
    static HRESULT SetValue(BSTR& bstr, LPCWSTR value);
    static HRESULT SetValue(DWORD& dw, DWORD value) { dw = value; return S_OK; }

    void CloseAllHandles();

};


#endif
