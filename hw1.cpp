#pragma warning(disable:4996)
#include <Windows.h>
#include <crtdbg.h>
#include<FileAPI.h>
#include <memory>
#include <WinBase.h>
#include <conio.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include<iostream>
#include <locale.h>

using namespace std;

void print(_In_ const char* fmt, _In_ ...)
{
	char log_buffer[2048];
	va_list args;

	va_start(args, fmt);
	HRESULT hRes = StringCbVPrintfA(log_buffer, sizeof(log_buffer), fmt, args);
	if (S_OK != hRes)
	{
		fprintf(
			stderr,
			"%s, StringCbVPrintfA() failed. res = 0x%08x",
			__FUNCTION__,
			hRes
			);
		return;
	}

	OutputDebugStringA(log_buffer);
	fprintf(stdout, "%s \n", log_buffer);
}

bool is_file_existsW(_In_ const wchar_t* file_path)
{
	_ASSERTE(NULL != file_path);
	_ASSERTE(TRUE != IsBadStringPtrW(file_path, MAX_PATH));
	if ((NULL == file_path) || (TRUE == IsBadStringPtrW(file_path, MAX_PATH))) return false;

	WIN32_FILE_ATTRIBUTE_DATA info = { 0 };

	if (GetFileAttributesExW(file_path, GetFileExInfoStandard, &info) == 0)
		return false;
	else
		return true;
}
bool create_bob_txt()
{
	//char *locale = setlocale(LC_ALL, "en_US.UTF-8");
	// current directory 를 구한다.
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);
	if (0 == buflen)
	{
		print("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		return false;
	}

	buf = (PWSTR)malloc(sizeof(WCHAR)* buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		print("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		free(buf);
		return false;
	}

	// current dir \\ bob.txt 파일명 생성
	wchar_t file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob.txt",
		buf)))
	{
		print("err, can not create file name");
		free(buf);
		return false;
	}
	free(buf); buf = NULL;

	if (true == is_file_existsW(file_name))
	{
		::DeleteFileW(file_name);
	}

	// 파일 생성
	HANDLE file_handle = CreateFileW(
		file_name,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (file_handle == INVALID_HANDLE_VALUE)
	{
		print("err, CreateFile(path=%ws), gle=0x%08x", file_name, GetLastError());
		return false;
	}


	CloseHandle(file_handle);
	TCHAR * sfile = TEXT("E:\\visual\\ConsoleApplication2\\ConsoleApplication2\\bob.txt");
	DWORD dwBytesWritten;
	SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	HANDLE hFile = CreateFile(sfile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	TCHAR * swork = TEXT("동해물과 백두산이 마르고 닳도록 비오비 비오비 Best of the Best Hello world\n");
	size_t llen = _tcslen(swork);

	unsigned char smarker[3];
	smarker[0] = 0xEF;
	smarker[1] = 0xBB;
	smarker[2] = 0xBF;
	WriteFile(hFile, smarker, 3, &dwBytesWritten, NULL);

	char *utf8 = (char *)malloc(1000);
	int lBytesWritten = WideCharToMultiByte(CP_UTF8, 0, swork, -1, utf8, 1000, NULL, NULL);
	int err = GetLastError();
	WriteFile(hFile, utf8, lBytesWritten, &dwBytesWritten, NULL);
	free(utf8);
	SetEndOfFile(hFile);
	CloseHandle(hFile);

	return true;
}
bool delete_bob_txt()
{
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	buflen = GetCurrentDirectoryW(buflen, buf);
	if (0 == buflen)
	{
		print("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		return false;
	}
	buf = (PWSTR)malloc(sizeof(WCHAR)* buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		print("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());

		free(buf);
		return false;
	}
	wchar_t file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob.txt",
		buf)))
	{
		print("err, can not create file name");
		free(buf);
		return false;
	}
	free(buf); buf = NULL;
	DeleteFileW(file_name);
	return true;

}
int copy_bob_txt() {
	FILE *in, *out;
	char* buf;
	size_t len;
	const char* src = "bob.txt";
	const char* dst = "bob2.txt";

	if ((in = fopen(src, "rb")) == NULL) return 1;
	if ((out = fopen(dst, "wb")) == NULL) { fclose(in); return 2; }

	if ((buf = (char *)malloc(1024)) == NULL) { fclose(in); fclose(out); return 3; }

	while ((len = fread(buf, sizeof(char), sizeof(buf), in)) != NULL)
	if (fwrite(buf, sizeof(char), len, out) == 0) {
		fclose(in); fclose(out);
		free(buf);
		_unlink(dst); // 에러난 파일 지우고 종료
		return 4;
	}
	fclose(in); fclose(out);
	free(buf); // 메모리 할당 해제

	return 0;

}
/*[windows programming 과제]
+ 현재디렉토리\bob.txt 파일을 생성
+ 영문자/한글 조합으로 utf-8 인코딩 저장
+ 현재디렉토리\bob2.txt 복사
+ 현재디렉토리\bob2.txt 를 읽어서 화면(콘솔)에 출력 (한글 깨지면 x)
---- ReadFile() api 를 통해서 파일 읽기
---- Memory Mapped I/O 를 이용해서 파일 읽기
+ 현재디렉토리\bob.txt, 현재디렉토리\bob2.txt 삭제
*/


//void readFilewithReadFile() {
//   
//   HANDLE fHandle;
//   //TCHAR Strings[1025];
//   //char* Strings="";
//   char buf[1024] = { 0 };
//   DWORD result;
//   LARGE_INTEGER curPtr;
//   LARGE_INTEGER thisPtr;
//
//   
//   // read utf8 sring 
//   fHandle = CreateFile(_T("bob2.txt"), GENERIC_READ, 0, NULL, OPEN_ALWAYS, 0, NULL);
//   _ASSERTE(ReadFile(fHandle, buf, sizeof(buf), &result, NULL));
//   //_ASSERTE 실패할 때 !! 
//   
//   //SetConsoleOutputCP(65001);
//
//   char* a = &buf[3];
//   print("%s\n", a);
//   CloseHandle(fHandle);
//}
void read_bob_txt()
{
	wchar_t *buf = NULL;
	uint32_t buflen = 0;
	DWORD bytes_read = 0;
	HANDLE hFile;

	buflen = GetCurrentDirectoryW(buflen, buf);
	if (0 == buflen)
		print("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());

	buf = (PWSTR)malloc(sizeof(WCHAR)* buflen);
	if (0 == GetCurrentDirectoryW(buflen, buf))
	{
		print("err, GetCurrentDirectoryW() failed. gle = 0x%08x", GetLastError());
		free(buf);

	}

	wchar_t file_name[260];
	if (!SUCCEEDED(StringCbPrintfW(
		file_name,
		sizeof(file_name),
		L"%ws\\bob2.txt",
		buf)))
	{
		print("err, can not create file name");
		free(buf);
	}
	free(buf); buf = NULL;

	hFile = CreateFileW(file_name,               // file to open
		GENERIC_READ,          // open for reading
		FILE_SHARE_READ,       // share for reading
		NULL,                  // default security
		OPEN_EXISTING,         // existing file only
		FILE_ATTRIBUTE_NORMAL, // normal file
		NULL);                 // no attr. template

	if (hFile == INVALID_HANDLE_VALUE)
	{
		print("err, can not create file name");
		free(buf);
	}

	char buff[1024];

	memset(buff, 0x00, 1024);





	if (!ReadFile(hFile, buff, 1023, &bytes_read, NULL))
	{
		print("err, ReadFile() failed. gle = 0x%08x", GetLastError());
		CloseHandle(hFile);
		
	}


	wchar_t strUnicode[1024] = { 0, };

	int nLen = MultiByteToWideChar(CP_UTF8, 0, buff, strlen(buff), NULL, NULL);

	MultiByteToWideChar(CP_UTF8, 0, buff, strlen(buff), strUnicode, nLen);

	char strMultiByte[1024] = { 0, };

	nLen = WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, NULL, 0, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, strUnicode, -1, strMultiByte, nLen, NULL, NULL);

	printf("%s\n", strMultiByte + 1);


	CloseHandle(hFile);

	
}

/*
bool readFilewithReadFile()
{
	
	//wchar_t* cur = get_current_directory();
	//wchar_t file_path[1024];
	//StringCbPrintfW(file_path, sizeof(file_path), L"%ws\\bob2.txt", cur);
	
	wchar_t cur[1024] = { 0 };
	GetCurrentDirectoryW(sizeof(cur), cur);
	wchar_t path[1024];
	StringCbPrintfW(path, sizeof(path), L"%ws\\bob2.txt", cur);
	printf("path=%ws \n", path);

//	HANDLE file_handle = CreateFileW(L"bob2.txt", GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	HANDLE file_handle = CreateFileW(path, GENERIC_ALL, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	_ASSERTE(INVALID_HANDLE_VALUE == file_handle);

	DWORD bytes_written = 0;
	char buf[1024] = { 0 };
	ReadFile(file_handle, buf, sizeof(buf), &bytes_written, NULL);

	// mbs to wcs
	char* utf8_buf = &buf[3];

//	SetConsoleOutputCP(65001);
	printf("%s\n",  utf8_buf);

//	SetConsoleOutputCP(949);
//	printf("0x%02x, 0x%02x, 0x%02x, 0x%02x, %s\n", utf8_buf[0], utf8_buf[1], utf8_buf[2], utf8_buf[3], utf8_buf);

	//OutputDebugStringA(utf8_buf);


	CloseHandle(file_handle);
	return true;

}
*/
void readFilewithMemoryMappedIO() {




}
int main(int argc, _TCHAR* argv[])
{
	create_bob_txt();
	int code = copy_bob_txt();
	read_bob_txt();
	//   readFilewithMemoryMappedIO();

	//   delete_bob_txt();
	return 0;
}
// 2번째 파일 내용 화면에 출력 
//   - ReadFile()
//   - Memory Mapped I/O()
//두 파일 삭제