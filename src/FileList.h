#pragma once

struct FileInfo
{
	std::wstring VirtualPath;
	FILETIME LastWriteTime;
	LONGLONG Size;

	inline FileInfo(LPCWSTR virtualPath, WIN32_FIND_DATA* wfd)
	{
		VirtualPath = virtualPath;
		LastWriteTime = wfd->ftLastWriteTime;
		Size = (LONGLONG)wfd->nFileSizeHigh << 32 | wfd->nFileSizeLow;
	}
};

inline HRESULT GenerateFileList(std::vector<FileInfo>& list, LPCWSTR pszBasePath, LPCWSTR pszVirtualPath = L"")
{
	WCHAR path[MAX_PATH];
	WCHAR path2[MAX_PATH];
	PathCombine(path, pszBasePath, pszVirtualPath);
	PathCombine(path2, path, L"*");

	WIN32_FIND_DATA wfd;
	HANDLE hFind = FindFirstFileEx(path2, FindExInfoStandard, &wfd, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
	if (hFind == INVALID_HANDLE_VALUE)
		return HRESULT_FROM_WIN32(GetLastError());

	do
	{
		if (lstrcmpW(wfd.cFileName, L".") == 0 ||
			lstrcmpW(wfd.cFileName, L"..") == 0)
			continue;

		if (wfd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			continue;

		WCHAR vpath[MAX_PATH];
		PathCombine(vpath, pszVirtualPath, wfd.cFileName);

		if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			list.push_back(FileInfo(vpath, &wfd));
		else
			GenerateFileList(list, pszBasePath, vpath);

	} while (FindNextFile(hFind, &wfd));

	FindClose(hFind);
	return S_OK;
}

inline std::string FileListToXml(const std::vector<FileInfo>& files)
{
	std::string output;
	output.reserve(128 + files.size() * 96);

	output.append("<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n\r\n<Files>\r\n");

	for (const auto& it : files)
	{
		SYSTEMTIME st;
		FileTimeToSystemTime(&it.LastWriteTime, &st);

		WCHAR line[1024];
		swprintf_s(line,
			L"\t<File Path=\"%s\" Size=\"%lld\" LastModified=\"%04d-%02d-%02d %02d:%02d:%02d\" />\r\n",
			it.VirtualPath.c_str(),
			it.Size,
			st.wYear,
			st.wMonth,
			st.wDay,
			st.wHour,
			st.wMinute,
			st.wSecond);

		char s_line[1024];
		s_line[WideCharToMultiByte(CP_UTF8, 0, line, -1, s_line, ARRAYSIZE(s_line), nullptr, nullptr)] = 0;
		output.append(s_line);
	}

	output.append("</Files>\r\n");
	return output;
}

inline std::string FileListToJson(const std::vector<FileInfo>& files)
{
	std::string output;
	output.reserve(128 + files.size() * 96);

	output.append("[");

	int n = 0;

	for (const auto& it : files)
	{
		if (n++ != 0)
			output.append(",\r\n");

		SYSTEMTIME st;
		FileTimeToSystemTime(&it.LastWriteTime, &st);

		WCHAR line[1024];
		swprintf_s(line,
			L"\t{ \"path\": \"%s\", \"size\": %lld, \"lastModified\": \"%04d-%02d-%02d %02d:%02d:%02d\" }",
			it.VirtualPath.c_str(),
			it.Size,
			st.wYear,
			st.wMonth,
			st.wDay,
			st.wHour,
			st.wMinute,
			st.wSecond);

		char s_line[1024];
		s_line[WideCharToMultiByte(CP_UTF8, 0, line, -1, s_line, ARRAYSIZE(s_line), nullptr, nullptr)] = 0;

		char s_line2[1024];
		char* p2 = s_line2;
		for (char* p = s_line; *p; ++p)
		{
			switch (*p)
			{
			case '\\':
				*p2++ = '\\';
				*p2++ = '\\';
				break;
			default:
				*p2++ = *p;
				break;
			}
		}

		*p2 = 0;

		output.append(s_line2);
	}

	output.append("\r\n]\r\n");
	return output;
}