#ifndef XMLConfig_H
#define XMLConfig_H

class CXMLConfig
{
public:
	CXMLConfig();
	~CXMLConfig();

public:
	HRESULT create();
	void empty();
	void clearModified();
	void setModified();
	HRESULT createOrSelectRoot(LPCOLESTR rootTagName, IXMLDOMElement** ppRoot);
	HRESULT setValue(LPCOLESTR rootTagName, LPCOLESTR attr, LPCOLESTR value);
	HRESULT getValue(LPCOLESTR rootTagName, LPCOLESTR attr, LPWSTR value, size_t len, LPCOLESTR defaultValue = nullptr);
	HRESULT load(LPCOLESTR path);
	HRESULT save(LPCOLESTR path);

protected:
	IXMLDOMDocument* m_pDoc;
	bool m_modified;

};

#endif
