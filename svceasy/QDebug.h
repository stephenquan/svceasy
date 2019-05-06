#ifndef QDebug_H
#define QDebug_H

class QDebug
{
public:
	QDebug& operator << (BSTR str) { return outputString(str); }
	QDebug& operator << (DWORD value) { return output("%d", value); }
	QDebug& operator << (int value) { return output("%d", value); }
	QDebug& operator << (HRESULT hr) { return output("0x%x", hr); }
	QDebug& operator << (HANDLE handle) { return output("0x%p", handle); }
	QDebug& operator << (LPCSTR str) { return outputString(str); }
	QDebug& operator << (LPCWSTR str) { return outputString(str); }
	QDebug& outputNULL() { return output("NULL"); }
	QDebug& output(LPCSTR fmt, ...);
	QDebug& output(LPCWSTR fmt, ...);
	QDebug& outputString(LPCSTR str);
	QDebug& outputString(LPCWSTR str);
};

QDebug& qDebug();

#endif
