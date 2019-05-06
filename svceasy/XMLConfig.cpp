#include "pch.h"
#include "XMLConfig.h"

class CSimpleBSTR
{
public:
    CSimpleBSTR(LPCOLESTR str) : m_bstr(NULL) { Assign(str); }
    ~CSimpleBSTR() { Clear(); }

    void Clear()
    {
        if (!m_bstr)
        {
            return;
        }

        SysFreeString(m_bstr);
        m_bstr = NULL;
    }

    HRESULT Assign(LPCOLESTR str)
    {
        Clear();

        if (!str)
        {
            return S_OK;
        }

        m_bstr = SysAllocString(str);
        if (!m_bstr)
        {
            return E_OUTOFMEMORY;
        }
        return S_OK;
    }

    CSimpleBSTR& operator= (LPCOLESTR str) { Assign(str); return *this; }
    operator BSTR& () { return m_bstr; }
    operator BSTR () const { return  m_bstr; }

protected:
    BSTR m_bstr;
};

class CSimpleVariant : public VARIANT
{
public:
    CSimpleVariant(LPCOLESTR str = nullptr) { VariantInit(this); Assign(str); }
    ~CSimpleVariant() { Clear(); }

    HRESULT Clear()
    {
        return VariantClear(this);
    }

    HRESULT Assign(LPCOLESTR str)
    {
        HRESULT hr = S_OK;
        hr = Clear();
        if (FAILED(hr)) return hr;
        if (!str) return S_OK;
        V_BSTR(this) = SysAllocString(str);
        if (V_BSTR(this) == NULL) return E_OUTOFMEMORY;
        vt = VT_BSTR;
        return S_OK;
    }

    CSimpleVariant& operator= (LPCOLESTR str) { Assign(str); return *this; }
    operator VARIANT& () { return *this; }
    operator VARIANT () const { return *this; }
};

template <class T>
class CSimplePtr
{
public:
    CSimplePtr(T* ptr = nullptr) : m_ptr(nullptr) { Assign(ptr); }
    ~CSimplePtr() { Clear(); }

    void Clear()
    {
        if (!m_ptr) return;
        m_ptr->Release();
        m_ptr = nullptr;
    }

    HRESULT Assign(IUnknown* ptr)
    {
        Clear();

        if (!ptr) return S_OK;

        return ptr->QueryInterface(&m_ptr);
    }

    operator void*() { return m_ptr; }
    operator void*() const { return m_ptr; }
    operator T*& () { return m_ptr; }
    operator T* () const { return m_ptr; }
    T** operator & () { return &m_ptr; }
    T* operator -> () { return m_ptr; }
    T* operator -> () const { return m_ptr; }
    CSimplePtr& operator= (IUnknown* ptr) { Assign(ptr); return *this; }

protected:
    T* m_ptr;
};

CXMLConfig::CXMLConfig() :
    m_modified(false)
{

}

CXMLConfig::~CXMLConfig()
{

}

HRESULT CXMLConfig::create()
{
    HRESULT hr = S_OK;

    if (m_pDoc) return S_OK;

    hr = CoCreateInstance(CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&m_pDoc);
    if (FAILED(hr)) return hr;
    setModified();
    return hr;
}

HRESULT CXMLConfig::createOrSelectRoot(LPCOLESTR rootTagName, IXMLDOMElement** ppRoot)
{
    HRESULT hr = S_OK;

    if (!m_pDoc)
    {
        hr = create();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = m_pDoc->get_documentElement(ppRoot);
    if (FAILED(hr))
    {
        return hr;
    }

    if (*ppRoot != nullptr)
    {
        return S_OK;
    }

    hr = m_pDoc->createElement(CSimpleBSTR(rootTagName), ppRoot);
    if (FAILED(hr))
    {
        return hr;
    }

    hr = m_pDoc->putref_documentElement(*ppRoot);
    if (FAILED(hr))
    {
        return hr;
    }

    setModified();
    return S_OK;
}

HRESULT CXMLConfig::setValue(LPCOLESTR rootTagName, LPCOLESTR attr, LPCOLESTR value)
{
    HRESULT hr = S_OK;

    CSimplePtr<IXMLDOMElement> pElement;
    hr = createOrSelectRoot(CSimpleBSTR(rootTagName), &pElement);
    if (FAILED(hr)) return hr;

    hr = pElement->setAttribute(CSimpleBSTR(attr), CSimpleVariant(value));
    if (FAILED(hr)) return hr;

    setModified();
    return hr;
}

HRESULT CXMLConfig::getValue(LPCOLESTR rootTagName, LPCOLESTR attr, LPWSTR value, size_t len, LPCOLESTR defaultValue)
{
    HRESULT hr = S_OK;

    CSimplePtr<IXMLDOMElement> pElement;
    hr = createOrSelectRoot(CSimpleBSTR(rootTagName), &pElement);
    if (FAILED(hr)) return hr;

    CSimpleVariant varValue;
    hr = pElement->getAttribute(CSimpleBSTR(attr), &varValue);
    if (hr != S_OK)
    {
        hr = pElement->setAttribute(CSimpleBSTR(attr), CSimpleVariant(defaultValue));
        if (SUCCEEDED(hr))
        {
            wcsncpy_s(value, len, defaultValue, wcslen(defaultValue));
            setModified();
            return S_OK;
        }
        return hr;
    }

    *value = '\0';
    if (varValue.vt != VT_BSTR)
    {
        return E_FAIL;
    }

    wcsncpy_s(value, len, V_BSTR(&varValue), SysStringLen(V_BSTR(&varValue)));
    return hr;
}

void CXMLConfig::empty()
{
    if (!m_pDoc) return;

    m_pDoc->Release();
    m_pDoc = nullptr;
    clearModified();
}

void CXMLConfig::clearModified()
{
    m_modified = false;
}

void CXMLConfig::setModified()
{
    m_modified = true;
}

HRESULT CXMLConfig::load(LPCOLESTR name)
{
    HRESULT hr = S_OK;

    empty();
    hr = create();
    if (FAILED(hr)) return hr;

    VARIANT_BOOL ok = VARIANT_TRUE;
    hr = m_pDoc->load(CSimpleVariant(name), &ok);
    return hr;
}

HRESULT CXMLConfig::save(LPCOLESTR name)
{
    HRESULT hr = S_OK;
    if (!m_pDoc)
    {
        hr = create();
        if (FAILED(hr))
        {
            return hr;
        }
    }
    hr = m_pDoc->save(CSimpleVariant(name));
    clearModified();
    return hr;
}
