#include "Utils.h"

#include <cassert>
#include <fstream>
#include <stdexcept>

#include <windows.h>
#include <WinInet.h>


namespace utils
{

std::vector<uint8_t> readFile(const std::string& filename)
{
	std::ifstream f(filename, std::ios::in | std::ios::binary);
	if (!f)
		throw std::runtime_error(S() << "Cannot open file: \"" << filename << "\"");
	
	f.seekg(0, std::ios::end);
	const size_t size = static_cast<size_t>(f.tellg()); // it should fit into size_t
	f.seekg(0, std::ios::beg);

	std::vector<uint8_t> contents(size);
	f.read(reinterpret_cast<char*>(&contents[0]), size);

	return contents;
}

// https://learn.microsoft.com/en-us/visualstudio/debugger/how-to-set-a-thread-name-in-native-code?view=vs-2022
const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void setThreadName(const char* szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = -1;
	info.dwFlags = 0;
#pragma warning(push)
#pragma warning(disable: 6320 6322)
	__try {
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
#pragma warning(pop)
}

void setThreadName(const std::string& threadName)
{
	setThreadName(threadName.c_str());
}

void debugPrint(const std::string& text)
{
	OutputDebugStringA(text.c_str());
	OutputDebugStringA("\n");
}

int httpGet(const char* szHost, const char* szUri)
{
	HINTERNET hInternet = InternetOpenA("RedDotScanner", INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	HINTERNET hConnect = InternetConnectA(hInternet, szHost, 80, "", "", INTERNET_SERVICE_HTTP, 0, 0);
	HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", szUri, nullptr, nullptr, nullptr, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, 0);
	BOOL bSuccess = HttpSendRequestA(hRequest, nullptr, 0, nullptr, 0);
	assert(bSuccess);

	DWORD dwStatus = 0;
	DWORD dwStatusSize = sizeof(dwStatus);
	bSuccess = HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatus, &dwStatusSize, nullptr);
	assert(bSuccess);

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);

	return dwStatus;
}

} // namespace utils
