#pragma once

inline void AppendStringFormat(std::string& str, const char* format, ...)
{
	va_list l;
	va_start(l, format);
	int len = _vscprintf(format, l);

	char buf[1024];
	char* p = buf;
	if (len + 1 > ARRAYSIZE(buf))
		p = (char*)malloc(len + 1);

	vsprintf_s(p, len + 1, format, l);
	str.append(p, len);

	if (p != buf)
		free(p);

	va_end(l);
}