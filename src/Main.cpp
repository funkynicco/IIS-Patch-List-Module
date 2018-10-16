#include "StdAfx.h"

class MyModule : public CHttpModule
{
	enum class DataType
	{
		XML,
		JSON
	};

public:
	inline REQUEST_NOTIFICATION_STATUS OnBeginRequest(IHttpContext* pHttpContext, IHttpEventProvider* pProvider) override
	{
		HRESULT hr;
		auto rawRequest = pHttpContext->GetRequest()->GetRawHttpRequest();

		DataType type = DataType::XML;

		PCWSTR pszPath = rawRequest->CookedUrl.pAbsPath;
		int cchPath = (int)rawRequest->CookedUrl.AbsPathLength;
		if (rawRequest->CookedUrl.pQueryString)
		{
			cchPath = int(rawRequest->CookedUrl.pQueryString - rawRequest->CookedUrl.pAbsPath);

			std::wstring queryString = rawRequest->CookedUrl.pQueryString;
			if (queryString.find(L"?xml") != std::wstring::npos)
				type = DataType::XML;
			else if (queryString.find(L"?json") != std::wstring::npos)
				type = DataType::JSON;
		}

		char url[512]; // url path without query string
		url[WideCharToMultiByte(CP_UTF8, 0, pszPath, cchPath, url, ARRAYSIZE(url), nullptr, nullptr)] = 0;
		if (strcmp(url, "/x-list") != 0)
			return RQ_NOTIFICATION_CONTINUE; // only process /x-list path

		auto response = pHttpContext->GetResponse();
		response->Clear();
		
		std::vector<FileInfo> files;
		if (FAILED(hr = GenerateFileList(files, pHttpContext->GetApplication()->GetApplicationPhysicalPath())))
		{
			response->SetStatus(500, "Could not generate file list.", 0, hr);
			return RQ_NOTIFICATION_FINISH_REQUEST;
		}

		LPCSTR contentType;
		std::string data;

		switch (type)
		{
		case DataType::XML:
			data = FileListToXml(files);
			contentType = "application/xml; charset=UTF-8";
			break;
		case DataType::JSON:
			data = FileListToJson(files);
			contentType = "application/json; charset=UTF-8";
			break;
		default:
			data = "<h3>Unknown data format!</h3>";
			contentType = "text/html; charset=UTF-8";
			break;
		}
		
		response->SetHeader(HttpHeaderContentType, contentType, (USHORT)strlen(contentType), TRUE);

		HTTP_DATA_CHUNK dataChunk = {};
		dataChunk.DataChunkType = HttpDataChunkFromMemory;
		dataChunk.FromMemory.pBuffer = (PVOID)data.c_str();
		dataChunk.FromMemory.BufferLength = (ULONG)data.length();

		DWORD cbSent;
		if (FAILED(hr = response->WriteEntityChunks(&dataChunk, 1, FALSE, TRUE, &cbSent)))
		{
			response->SetStatus(500, "Server error", 0, hr);
			return RQ_NOTIFICATION_FINISH_REQUEST;
		}

		return RQ_NOTIFICATION_FINISH_REQUEST;
	}


	inline virtual void Dispose() override
	{
		// do not call the underlying CHttpModule::Dispose() as it will 'delete this;'
	}
};

class MyModuleFactory : public IHttpModuleFactory
{
public:
	inline HRESULT GetHttpModule(CHttpModule** ppModule, IModuleAllocator* pAllocator) override
	{
		*ppModule = &m_module;
		return S_OK;
	}

	inline void Terminate() override
	{
		delete this;
	}

private:
	MyModule m_module;
};

extern "C"
{
	__declspec(dllexport) HRESULT WINAPI RegisterModule(DWORD dwServerVersion, IHttpModuleRegistrationInfo* pModuleInfo, IHttpServer* pGlobalInfo)
	{
		HRESULT hr;
		if (FAILED(hr = pModuleInfo->SetRequestNotifications(new MyModuleFactory, RQ_BEGIN_REQUEST, 0)))
			return hr;

		return S_OK;
	}
}