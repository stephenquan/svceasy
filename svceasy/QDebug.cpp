#include "pch.h"
#include "QDebug.h"

QDebug& qDebug()
{
	static QDebug debug;
	return debug;
}

QDebug& QDebug::output(LPCSTR fmt, ...)
{
	if (!fmt) return outputNULL();
	char str[1024] = { };
	va_list args;
	va_start(args, fmt);
	vsprintf_s(str, fmt, args);
	va_end(args);
	return outputString(str);
}

QDebug& QDebug::output(LPCWSTR fmt, ...)
{
	if (!fmt) return outputNULL();
	WCHAR str[1024] = { };
	va_list args;
	va_start(args, fmt);
	vswprintf_s(str, fmt, args);
	va_end(args);
	return outputString(str);
}

QDebug& QDebug::outputString(LPCSTR str)
{
	if (!str) return outputNULL();
	OutputDebugStringA(str);
	std::cout << str;
	return *this;
}

QDebug& QDebug::outputString(LPCWSTR str)
{
	if (!str) return outputNULL();
	OutputDebugStringW(str);
	std::wcout << str;
	return *this;
}
