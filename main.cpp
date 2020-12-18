#include "VietnameseTextNormalizer.h"
#include <iostream>
#include <fstream>
#include <locale>
#include <codecvt>
#include <list>

//#undef WIN32_NORMALIZER_TOOL
#ifndef WIN32_NORMALIZER_TOOL
extern "C"
{
#include <Python.h>
}
#endif



/************************************************************************/
/* Main                                                                 */
/************************************************************************/
#ifdef WIN32_NORMALIZER_TOOL
#ifndef QBT_VALIDATE_TOOL
#include <Windows.h>
#include <iostream>
#include <string>
namespace std
{

	/************************************************************************/
	/*                                                                      */
	/************************************************************************/
	void			GoToXY(int x, int y)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
		HANDLE hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		COORD Cursor_an_Pos = { (short)x, (short)y };
		SetConsoleCursorPosition(hConsoleOutput, Cursor_an_Pos);
#else
		printf("%c[%d;%df", 0x1B, y, x);
#endif
	}
	void			SetTitle(const std::wstring& title)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
		SetConsoleTitleW(title.c_str());
#endif
	}
	void			ClearScreen(void)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
		CONSOLE_SCREEN_BUFFER_INFO  csbiInfo;
		HANDLE  hConsoleOut;
		COORD   Home = { 0, 0 };
		DWORD   dummy;
		hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(hConsoleOut, &csbiInfo);
		FillConsoleOutputCharacter(hConsoleOut, ' ', csbiInfo.dwSize.X * csbiInfo.dwSize.Y, Home, &dummy);
		csbiInfo.dwCursorPosition.X = 0;
		csbiInfo.dwCursorPosition.Y = 0;
		SetConsoleCursorPosition(hConsoleOut, csbiInfo.dwCursorPosition);
		GoToXY(0, 0);
		for (int i = 0; i < 50; i++) printf("                                                                                 ");
		GoToXY(0, 0);
#else
		system("clear");
#endif
	}
	void			HideCursor(void)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
		HANDLE hOut;
		CONSOLE_CURSOR_INFO ConCurInf;
		hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		ConCurInf.dwSize = 10;
		ConCurInf.bVisible = FALSE;
		SetConsoleCursorInfo(hOut, &ConCurInf);
#endif
	}
	std::wstring	GetLower(const std::wstring& wstr);
	void			Show(const std::wstring& title, const wchar_t* format, ...)
	{
		if (format)
		{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
			wchar_t 	buffer[10250] = { 0 };
			va_list args;
			va_start(args, format);
			_vsnwprintf_s(buffer, 10240, format, args);
			va_end(args);
			int supportFlag = 0;

			if (GetLower(title).find(L"lỗi") != std::wstring::npos) supportFlag = MB_ICONERROR;
			else if (GetLower(title).find(L"cảnh báo") != std::wstring::npos) supportFlag = MB_ICONWARNING;
			::MessageBoxW(0, buffer, title.c_str(), MB_OK | supportFlag);
#else
			wchar_t 	buffer[10250] = { 0 };
			va_list args;
			va_start(args, format);
			vswprintf(buffer, 10240, format, args);
			va_end(args);
			SetTextColor(CONSOLE_COLOR_BOLD);
			printf("%s\n", GetString(title).c_str());
			SetTextColor(CONSOLE_COLOR_DISABLE_BOLD);
			printf("%s\n", GetString(buffer).c_str());
#endif
		}
	}
	void			SetTextColor(int color)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
		HANDLE hConsoleOutput;
		hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO screen_buffer_info;
		GetConsoleScreenBufferInfo(hConsoleOutput, &screen_buffer_info);
		WORD wAttributes = screen_buffer_info.wAttributes;
		DWORD dcolor = color;
		dcolor &= 0x000f;
		wAttributes &= 0xfff0;
		wAttributes |= dcolor;
		SetConsoleTextAttribute(hConsoleOutput, wAttributes);
#else
#ifndef ON_REMOTE_LINUX
		std::cout << "\033[" << color << "m";
#endif
#endif
	}
	void			SetBackgroundColor(int color)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
		HANDLE hConsoleOutput;
		hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO screen_buffer_info;
		GetConsoleScreenBufferInfo(hConsoleOutput, &screen_buffer_info);
		WORD wAttributes = screen_buffer_info.wAttributes;
		DWORD dcolor = color;
		dcolor &= 0x000f;
		dcolor <<= 4;
		wAttributes &= 0xff0f;
		wAttributes |= dcolor;
		SetConsoleTextAttribute(hConsoleOutput, wAttributes);
#else
		std::cout << "\033[" << color << "m";
#endif
	}
	static void		guiWrite(int xsource, int ysource, int xdest, int ydest, const wchar_t* wstr/*=L"Quang"*/, int delay/*=15*/, int color/*=CONSOLE_COLOR_GREEN*/)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
		SetTextColor(color);
		if (xsource == xdest && ysource > ydest)
		{
			for (; wstr[0]; wstr++)
			{
				if (wstr[0] != 32)
				{
					for (int i = ysource; i >= ydest; i--)
					{
						GoToXY(xsource, i);
						wprintf(L"%c", wstr[0]);
						GoToXY(xsource, i + 1);
						wprintf(L" ");
						Sleep(delay);
					}
				}
				else
				{
					GoToXY(xsource, ydest);
					wprintf(L" ");
				}
				xsource++;
			}
		}
		if (ydest == ysource && xsource > xdest)
		{
			for (; wstr[0]; wstr++)
			{
				if (wstr[0] != 32)
				{
					for (int i = xsource; i >= xdest; i--)
					{
						GoToXY(i, ysource);
						wprintf(L"%c ", wstr[0]);
						Sleep(delay);
					}
				}
				else
				{
					GoToXY(xsource, ydest);
					wprintf(L" ");
				}
				xdest++;
			}
		}
#endif
	}
	void			Copyright(void)
	{
		ClearScreen();
		HideCursor();
		int x = 13;
		int y = 5;
		GoToXY(x, y + 1);  printf(" / _` | | | |/ _` | '_ \\ / _` | \\ \\/ / _ \\ '_ \\ / _` |");
		GoToXY(x, y + 2);  printf("| (_| | |_| | (_| | | | | (_| |  >  <  __/ | | | (_| |");
		GoToXY(x, y + 3);  printf(" \\__, |\\__,_|\\__,_|_| |_|\\__, | /_/\\_\\___|_| |_|\\__, |");
		GoToXY(x, y + 4);  printf("    | |                   __/ |                  __/ |");
		GoToXY(x, y + 5);  printf("    |_|                  |___/                  |___/");
		guiWrite(26, 15, 26, 13, L"Duoc viet boi Bui Tan Quang", 15, 2);
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
		Sleep(2000);
#endif
	}
	/************************************************************************/
	/* w-string                                                             */
	/************************************************************************/
	class				wcslesscmp
	{
	public:
		bool operator()(const std::wstring& lhs, const std::wstring& rhs)const
		{
			return lhs.compare(rhs) < 0;
		}
	};
	typedef				std::set<std::wstring, std::wcslesscmp>						wstringset;
	bool				operator !=(const std::wstring& a, const std::wstring& b)
	{
		return wcscmp(a.c_str(), b.c_str()) != 0;
	}
	bool				operator !=(const std::wstring& a, const wchar_t* b)
	{
		return wcscmp(a.c_str(), b) != 0;
	}

	wchar_t				GetBase(wchar_t wch)
	{
		switch (wch)
		{
		case L'A':case L'à':case L'À':case L'á':case L'Á':case L'ả':case L'Ả':case L'ã':case L'Ã':case L'ạ':case L'Ạ':return L'a';
		case L'E':case L'è':case L'È':case L'é':case L'É':case L'ẻ':case L'Ẻ':case L'ẽ':case L'Ẽ':case L'ẹ':case L'Ẹ':return L'e';
		case L'I':case L'ì':case L'Ì':case L'í':case L'Í':case L'ỉ':case L'Ỉ':case L'ĩ':case L'Ĩ':case L'ị':case L'Ị':return L'i';
		case L'O':case L'ò':case L'Ò':case L'ó':case L'Ó':case L'ỏ':case L'Ỏ':case L'õ':case L'Õ':case L'ọ':case L'Ọ':return L'o';
		case L'U':case L'ù':case L'Ù':case L'ú':case L'Ú':case L'ủ':case L'Ủ':case L'ũ':case L'Ũ':case L'ụ':case L'Ụ':return L'u';
		case L'Y':case L'ỳ':case L'Ỳ':case L'ý':case L'Ý':case L'ỷ':case L'Ỷ':case L'ỹ':case L'Ỹ':case L'ỵ':case L'Ỵ':return /*L'y'*/L'i';
		case L'Â':case L'ầ':case L'Ầ':case L'ấ':case L'Ấ':case L'ẩ':case L'Ẩ':case L'ẫ':case L'Ẫ':case L'ậ':case L'Ậ':return L'â';
		case L'Ê':case L'ề':case L'Ề':case L'ế':case L'Ế':case L'ể':case L'Ể':case L'ễ':case L'Ễ':case L'ệ':case L'Ệ':return L'ê';
		case L'Ô':case L'ồ':case L'Ồ':case L'ố':case L'Ố':case L'ổ':case L'Ổ':case L'ỗ':case L'Ỗ':case L'ộ':case L'Ộ':return L'ô';
		case L'Ă':case L'ằ':case L'Ằ':case L'ắ':case L'Ắ':case L'ẳ':case L'Ẳ':case L'ẵ':case L'Ẵ':case L'ặ':case L'Ặ':return L'ă';
		case L'Ơ':case L'ờ':case L'Ờ':case L'ớ':case L'Ớ':case L'ở':case L'Ở':case L'ỡ':case L'Ỡ':case L'ợ':case L'Ợ':return L'ơ';
		case L'Ư':case L'ừ':case L'Ừ':case L'ứ':case L'Ứ':case L'ử':case L'Ử':case L'ữ':case L'Ữ':case L'ự':case L'Ự':return L'ư';
		case L'Đ':return L'đ';
		case L'B':return L'b';
		case L'C':return L'c';
		case L'D':return L'd';
		case L'F':return L'f';
		case L'G':return L'g';
		case L'H':return L'h';
		case L'J':return L'j';
		case L'K':return L'k';
		case L'L':return L'l';
		case L'M':return L'm';
		case L'N':return L'n';
		case L'P':return L'p';
		case L'Q':return L'q';
		case L'R':return L'r';
		case L'S':return L's';
		case L'T':return L't';
		case L'V':return L'v';
		case L'W':return L'w';
		case L'X':return L'x';
		case L'Z':return L'z';
		}
		if (wch >= 'a' && wch <= 'z') return wch;
		if (wch >= 'A' && wch <= 'Z') return wch;
		if (wch >= '0' && wch <= '9') return wch;
		return L' ';
	}
	std::wstring		GetBase(const std::wstring& wstr)
	{
		std::wstring base;
		for (unsigned i = 0; i < wstr.size(); i++)
		{
			wchar_t baseChar = GetBase(wstr[i]);
			if (baseChar != L' ') base += baseChar;
		}
		return base;
	}
	wchar_t				GetLower(wchar_t wch)
	{
		switch (wch)
		{
		case 0xC0:/*AF*/
			return 0xE0/*af*/;
		case 0xC1:/*AS*/
			return 0xE1/*as*/;
		case 0x1EA2:/*AR*/
			return 0x1EA3/*ar*/;
		case 0xC3:/*AX*/
			return 0xE3/*ax*/;
		case 0x1EA0:/*AJ*/
			return 0x1EA1/*aj*/;
		case 0x102:/*AW*/
			return 0x103/*aw*/;
		case 0x1EB0:/*AWF*/
			return 0x1EB1/*awf*/;
		case 0x1EAE:/*AWS*/
			return 0x1EAF/*aws*/;
		case 0x1EB2:/*AWR*/
			return 0x1EB3/*awr*/;
		case 0x1EB4:/*AWX*/
			return 0x1EB5/*awx*/;
		case 0x1EB6:/*AWJ*/
			return 0x1EB7/*awj*/;
		case 0xC2:/*AA*/
			return 0xE2/*aa*/;
		case 0x1EA6:/*AAF*/
			return 0x1EA7/*aaf*/;
		case 0x1EA4:/*AAS*/
			return 0x1EA5/*aas*/;
		case 0x1EA8:/*AAR*/
			return 0x1EA9/*aar*/;
		case 0x1EAA:/*AAX*/
			return 0x1EAB/*aax*/;
		case 0x1EAC:/*AAJ*/
			return 0x1EAD/*aaj*/;
		case 0x110:/*DD*/
			return 0x111/*dd*/;
		case 0xC8:/*EF*/
			return 0xE8/*ef*/;
		case 0xC9:/*ES*/
			return 0xE9/*es*/;
		case 0x1EBA:/*ER*/
			return 0x1EBB/*er*/;
		case 0x1EBC:/*EX*/
			return 0x1EBD/*ex*/;
		case 0x1EB8:/*EJ*/
			return 0x1EB9/*ej*/;
		case 0xCA:/*EE*/
			return 0xEA/*ee*/;
		case 0x1EC0:/*EEF*/
			return 0x1EC1/*eef*/;
		case 0x1EBE:/*EES*/
			return 0x1EBF/*ees*/;
		case 0x1EC2:/*EER*/
			return 0x1EC3/*eer*/;
		case 0x1EC4:/*EEX*/
			return 0x1EC5/*eex*/;
		case 0x1EC6:/*EEJ*/
			return 0x1EC7/*eej*/;
		case 0xCC:/*IF*/
			return 0xEC/*if*/;
		case 0xCD:/*IS*/
			return 0xED/*is*/;
		case 0x1EC8:/*IR*/
			return 0x1EC9/*ir*/;
		case 0x128:/*IX*/
			return 0x129/*ix*/;
		case 0x1ECA:/*IJ*/
			return 0x1ECB/*ij*/;
		case 0xD2:/*OF*/
			return 0xF2/*of*/;
		case 0xD3:/*OS*/
			return 0xF3/*os*/;
		case 0x1ECE:/*OR*/
			return 0x1ECF/*or*/;
		case 0xD5:/*OX*/
			return 0xF5/*ox*/;
		case 0x1ECC:/*OJ*/
			return 0x1ECD/*oj*/;
		case 0xD4:/*OO*/
			return 0xF4/*oo*/;
		case 0x1ED2:/*OOF*/
			return 0x1ED3/*oof*/;
		case 0x1ED0:/*OOS*/
			return 0x1ED1/*oos*/;
		case 0x1ED4:/*OOR*/
			return 0x1ED5/*oor*/;
		case 0x1ED6:/*OOX*/
			return 0x1ED7/*oox*/;
		case 0x1ED8:/*OOJ*/
			return 0x1ED9/*ooj*/;
		case 0x1A0:/*OW*/
			return 0x1A1/*ow*/;
		case 0x1EDC:/*OWF*/
			return 0x1EDD/*owf*/;
		case 0x1EDA:/*OWS*/
			return 0x1EDB/*ows*/;
		case 0x1EDE:/*OWR*/
			return 0x1EDF/*owr*/;
		case 0x1EE0:/*OWX*/
			return 0x1EE1/*owx*/;
		case 0x1EE2:/*OWJ*/
			return 0x1EE3/*owj*/;
		case 0xD9:/*UF*/
			return 0xF9/*uf*/;
		case 0xDA:/*US*/
			return 0xFA/*us*/;
		case 0x1EE6:/*UR*/
			return 0x1EE7/*ur*/;
		case 0x168:/*UX*/
			return 0x169/*ux*/;
		case 0x1EE4:/*UJ*/
			return 0x1EE5/*uj*/;
		case 0x1AF:/*UW*/
			return 0x1B0/*uw*/;
		case 0x1EEA:/*UWF*/
			return 0x1EEB/*uwf*/;
		case 0x1EE8:/*UWS*/
			return 0x1EE9/*uws*/;
		case 0x1EEC:/*UWR*/
			return 0x1EED/*uwr*/;
		case 0x1EEE:/*UWX*/
			return 0x1EEF/*uwx*/;
		case 0x1EF0:/*UWJ*/
			return 0x1EF1/*uwj*/;
		case 0x1EF2:/*YF*/
			return 0x1EF3/*yf*/;
		case 0xDD:/*YS*/
			return 0xFD/*ys*/;
		case 0x1EF6:/*YR*/
			return 0x1EF7/*yr*/;
		case 0x1EF8:/*YX*/
			return 0x1EF9/*yx*/;
		case 0x1EF4:/*YJ*/
			return 0x1EF5/*yj*/;
		case 0xD0/*Kí tự đặc biệt Ð*/:
			return 0x111/*đ*/;
		}
		if (wch >= 'A' && wch <= 'Z') return wch + 'a' - 'A';
		return wch;
	}
	std::wstring		GetLower(const std::wstring& wstr)
	{
		std::wstring bufferLower;
		for (unsigned int i = 0; i < wstr.size(); i++)
		{
			bufferLower += GetLower(wstr[i]);
		}
		return bufferLower;
	}
	int					StartWith(const std::wstring& str, const std::wstring& subStr)
	{
		if (str.size() >= subStr.size() && subStr.size() > 0)
		{
			for (auto i = 0u; i < subStr.size(); i++)
			{
				if (str[i] != subStr[i]) return 0;
			}
			return 1;
		}
		return 0;
	}
	int					EndWith(const std::wstring& str, const std::wstring& subStr)
	{
		return (str.size() >= subStr.size() && subStr.size() > 0 && str.substr(str.size() - subStr.size()) == subStr);
	}
	std::wstring		GetWString(const std::string& str)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
		//	std::wstring buffer;
		//	int			bufferWideCharLength = str.size() + 10;
		//	wchar_t *	bufferWideChar = (wchar_t*)calloc(bufferWideCharLength + 100/*safe*/, sizeof(wchar_t));
		//	if (bufferWideChar)
		//	{
		//		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, bufferWideChar, bufferWideCharLength);
		//		buffer += bufferWideChar;
		//		free(bufferWideChar);
		//	}
		//	return buffer;

		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.from_bytes(str);
#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.from_bytes(str);
#endif
	}
	std::wstring		GetWString(const wchar_t* wstr, int length)
	{
		std::wstring buffer;
		if (wstr)
		{
			for (int ichar = 0; ichar < length && wstr[ichar]; ichar++) buffer += wstr[ichar];
		}
		return buffer;
	}
	std::wstring		GetWString(const char* str, int length)
	{
		std::string bufferA;
		//bufferA.reserve(length + 10);
		for (int iChar = 0; iChar < length; iChar++)
		{
			if (str[iChar]) bufferA += str[iChar];
		}
		return GetWString(bufferA);
	}
	std::wstring		GetWString(const char* str, long long int length)
	{
		std::string bufferA;
		//bufferA.reserve(length + 10);
		for (int iChar = 0; iChar < length; iChar++)
		{
			if (str[iChar]) bufferA += str[iChar];
		}
		return GetWString(bufferA);
	}
	std::string			GetString(const std::wstring& wstr)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
		std::string buffer;
		int		bufferUtf8Length = int(wstr.size() * 4 + 10);
		char* bufferUtf8 = (char*)calloc(bufferUtf8Length, sizeof(char));
		if (bufferUtf8)
		{
			WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), bufferUtf8, bufferUtf8Length, 0, 0);
			buffer += bufferUtf8;
			free(bufferUtf8);
		}
		return buffer;
#else
		std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
		return myconv.to_bytes(wstr);
#endif
	}
	/************************************************************************/
	/* file                                                                 */
	/************************************************************************/
	void				DeleteFile(const std::wstring& fileName)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) 
		::DeleteFileW(fileName.c_str());
#else
		remove(GetString(fileName).c_str());
#endif
	}
	void				ScanFile(const std::wstring& path, const std::wstring& extension, std::wstringset& fileSet)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
		std::wstring		wFolder = path;
		if (wFolder.size() && wFolder[wFolder.size() - 1] != L'\\') wFolder += L'\\';
		wFolder += extension;
		WIN32_FIND_DATAW		fd;
		HANDLE hFind = FindFirstFileW(wFolder.c_str(), &fd);
		if (hFind == INVALID_HANDLE_VALUE) return;
		do
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{//là thư mục
				if (wcscmp(fd.cFileName, L".") && wcscmp(fd.cFileName, L".."))
				{//không phải là thư mục đặc biệt
					std::wstring child = path;
					if (child.size() && child[child.size() - 1] != L'\\') child += L'\\';
					child += fd.cFileName;
					ScanFile(child, extension, fileSet);
				}
			}
			else
			{//là file
				std::wstring filePath = path;
				if (filePath.size() && filePath[filePath.size() - 1] != L'\\') filePath += L'\\';
				filePath += fd.cFileName;
				fileSet.insert(filePath);
			}
		} while (FindNextFileW(hFind, &fd));
		FindClose(hFind);
#else


		DIR* dir = opendir(GetString(path).c_str());
		class dirent* ent;
		class stat st;
		while ((ent = readdir(dir)) != NULL)
		{
			const std::string file_name = ent->d_name;
			const std::wstring full_file_name = path + L"/" + file_name;

			if (file_name[0] == '.')
			{
				/*do nothing*/
			}
			else if (stat(GetString(full_file_name).c_str(), &st) == -1)
			{
				/*do nothing*/
			}
			else
			{
				const bool is_directory = (st.st_mode & S_IFDIR) != 0;
				if (is_directory)
				{
					ScanFile(full_file_name, extension, fileSet);
				}
				else
				{
					fileSet.insert(full_file_name);
				}
			}
		}
		closedir(dir);
#endif
	}
	void				GetRealFileName(const std::wstring& filePath, std::wstring& rFilename)
	{
		rFilename.clear();
		for (auto i = 0u; i < filePath.size(); i++)
		{
			if (filePath[i] == L'\\' || filePath[i] == L'/') rFilename.clear();
			else rFilename += filePath[i];
		}
		auto pos = rFilename.find(L'.');
		if (pos != std::wstring::npos) rFilename.erase(pos, rFilename.size() - pos);
	}
	std::wstring		GetRealFileName(const std::wstring& filePath)
	{
		std::wstring rFilename;
		GetRealFileName(filePath, rFilename);
		return rFilename;
	}
	bool				ReadFile(const std::wstring& fileName, std::wstring& buffer)
	{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
		std::ifstream fileInputStream(fileName, std::ifstream::in | std::ifstream::binary);
#else
		std::ifstream fileInputStream(GetString(fileName), std::ifstream::in | std::ifstream::binary);
#endif
		if (fileInputStream.is_open())
		{
			fileInputStream.seekg(0, std::ios_base::end);
			long long fileSize = fileInputStream.tellg();
			fileInputStream.seekg(0, std::ios_base::beg);
			if (fileSize <= 4)
			{
				char* utf8data = new char[fileSize + 10];
				memset(utf8data, 0, fileSize + 10);
				fileInputStream.seekg(0, std::ios_base::beg);
				fileInputStream.read(utf8data, fileSize);
				buffer += GetWString(utf8data, fileSize);
				delete[] utf8data;
				fileInputStream.close();
				return true;
			}
			bool		flagValidateToRead = true;
			char		byteOrderMark1 = 0;
			char		byteOrderMark2 = 0;
			char		byteOrderMark3 = 0;
			char		byteOrderMark4 = 0;
			fileInputStream.read(&byteOrderMark1, 1);
			fileInputStream.read(&byteOrderMark2, 1);
			fileInputStream.read(&byteOrderMark3, 1);
			fileInputStream.read(&byteOrderMark4, 1);
			fileInputStream.seekg(0, std::ios_base::beg);
#pragma warning(push)
#pragma warning(disable : 4310)
			if (byteOrderMark1 == char(0xFE) && byteOrderMark2 == char(0xFF))
			{//Unicode Big Endian
				std::Show(std::wstring(L"Lỗi"), L"Error: Unsupported encode (Unicode Big Endian) with %ls \n", fileName.c_str());
				flagValidateToRead = false;
			}
			else if (byteOrderMark1 == (char)0x00 && byteOrderMark2 == (char)0x00 && byteOrderMark3 == (char)0xFE && byteOrderMark4 == (char)0xFF)
			{//UTF-32 Big Endian
				std::Show(std::wstring(L"Lỗi"), L"Error: Unsupported encode (UTF-32 Big Endian) with %ls \n", fileName.c_str());
				flagValidateToRead = false;
			}
			else if (byteOrderMark1 == (char)0xFF && byteOrderMark2 == (char)0xFE && byteOrderMark3 == (char)0x00 && byteOrderMark4 == (char)0x00)
			{//UTF-32 Little Endian
				std::Show(std::wstring(L"Lỗi"), L"Error: Unsupported encode (UTF-32 Little Endian) with %ls \n", fileName.c_str());
				flagValidateToRead = false;
			}
			else if ((byteOrderMark1 == (char)0x2B && byteOrderMark2 == (char)0x2F && byteOrderMark3 == (char)0x76))
			{//UTF-7
				std::Show(std::wstring(L"Lỗi"), L"Error: Unsupported encode (UTF-7) with %ls \n", fileName.c_str());
				flagValidateToRead = false;
			}
			else if (byteOrderMark1 == (char)0xFF && byteOrderMark2 == (char)0xFE)
			{//Unicode Little Endian
				wchar_t* unicodeData = new wchar_t[fileSize * 2 + 10];
				memset(unicodeData, 0, fileSize * 2 + 10);
				fileInputStream.seekg(2, std::ios_base::beg);
				fileInputStream.read((char*)unicodeData, fileSize);
				buffer += unicodeData;
				delete[] unicodeData;
			}
			else if (byteOrderMark1 == (char)0xEF && byteOrderMark2 == (char)0xBB && byteOrderMark3 == (char)0xBF)
			{//utf8
				char* utf8data = new char[fileSize + 10];
				memset(utf8data, 0, fileSize + 10);
				fileInputStream.seekg(3, std::ios_base::beg);
				fileInputStream.read(utf8data, fileSize);
				buffer += GetWString(utf8data, fileSize);
				delete[] utf8data;
			}
			else
			{//utf-8 without BOM
				char* utf8data = (char*)calloc(fileSize + 10, sizeof(char));
				if (utf8data)
				{
					fileInputStream.seekg(0, std::ios_base::beg);
					fileInputStream.read(utf8data, fileSize);
					buffer += GetWString(utf8data, fileSize);
					free(utf8data);
				}
				else
				{
					std::Show(std::wstring(L"Lỗi"), L"Can not malloc to open file %ls \n", fileName.c_str());
				}
			}
#pragma warning(pop)
			fileInputStream.close();
			return flagValidateToRead;
		}
		else
		{
			std::Show(std::wstring(L"Lỗi"), L"Error: Can not open file %ls \n", fileName.c_str());
			return false;
		}
	}
	std::wstring		ReadFile(const std::wstring& fileName)
	{
		std::wstring bufferContent;
		ReadFile(fileName, bufferContent);
		return bufferContent;
	}
	void				WriteFile(const std::wstring& fileName, const std::wstring& wstr, bool truncate)
	{
		for (auto i = 0u; i < fileName.size(); i++)
		{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
			if (fileName[i] == L'\\') ::CreateDirectoryW(fileName.substr(0, i).c_str(), 0);
#else
			int mkdir(const char* path, mode_t mode);
			if (fileName[i] == L'/') mkdir(GetString(fileName.substr(0, i)).c_str(), 0777);
#endif
		}
		if (truncate) DeleteFile(fileName);
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
		std::ofstream fileHandle(fileName, std::ios_base::out | std::ios_base::binary | (truncate ? std::ios_base::trunc : (std::ios_base::app | std::ios_base::ate)));
#else
		std::ofstream fileHandle(GetString(fileName), std::ios_base::out | std::ios_base::binary | (truncate ? std::ios_base::trunc : (std::ios_base::app | std::ios_base::ate)));
#endif
		if (fileHandle.is_open())
		{
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64) || defined(_MSC_VER)
			int buffUtf8Size = int(wstr.size() * 2 + 10);
			char* buffUtf8 = new char[buffUtf8Size];
			memset(buffUtf8, 0, buffUtf8Size);
			WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), buffUtf8, buffUtf8Size, 0, 0);
			fileHandle.write(buffUtf8, strlen(buffUtf8));
			fileHandle.close();
			delete[] buffUtf8;
#else
			std::string		bufferToWrite = GetString(wstr);
			int				writeSize = bufferToWrite.size();
			unsigned int	beforeWriteOffset = fileHandle.tellp();
			fileHandle.write(bufferToWrite.c_str(), writeSize);
			unsigned int	affterWriteOffset = fileHandle.tellp();
			unsigned int	writtenSize = affterWriteOffset - beforeWriteOffset;
			fileHandle.close();
			if (writtenSize != writeSize)
			{
				std::Show(std::wstring(L"Lỗi"), L"Error: write file %ls (utf8 size == %d bytes, write == %d bytes)\n", fileName.c_str(), writeSize, writtenSize);
				//return false;
			}
			//else return true;
#endif		
		}
		else
		{
			std::Show(std::wstring(L"Lỗi"), L"Error: Can not open to write file %ls \n", fileName.c_str());
		}
	}

}
void main(void)
{
	//	wchar_t testStr[] = L"Tôi làm việ ở ban công ngệ FPT, tôi là người viêt nam. hôm nay tôi ko thích ăn mì tôm. tôi làm đc 2 bài tập.";
	//	VietnameseTextNormalizer vntObject;
	//	vntObject.Input(testStr);
	//	vntObject.Normalize();
	//	vntObject.GenStandardText();
	//	::MessageBoxW(0, vntObject.standardText, L"Output", MB_OK);

	std::SetTitle(L"VietnameseTextNormalizer Build 2020.09.23");
	std::HideCursor();
	std::SetBackgroundColor(15);
	std::SetTextColor(2);
	std::ClearScreen();

#ifdef _DEBUG
	std::DeleteFile(L"log.txt");
#endif

	wprintf(L"Huong dan su dung :\n");
	wprintf(L"	Dat toan bo tap tin van ban  (*.txt) vao thu muc txt\n");
	wprintf(L"	Dat tool nay ngang hang voi thu muc txt va chay tool\n");
	wprintf(L"==>xem thu muc fix de lay ket qua\n\n");
	wprintf(L"...An phim bat ki de bat dau...\n");
#ifndef _DEBUG
	_getch();
#endif // !_DEBUG


	std::wstringset textFileSet;
	std::ScanFile(L"", L"*.*", textFileSet);
	int countTotalChange = 0;
	for (auto itxt = textFileSet.begin(); itxt != textFileSet.end(); itxt++)
	{
		if ((std::EndWith(std::GetLower(*itxt), L".txt")
			|| std::EndWith(std::GetLower(*itxt), L".ftext")
			|| std::EndWith(std::GetLower(*itxt), L".info")
			|| std::EndWith(std::GetLower(*itxt), L".xml"))
			&& std::GetRealFileName(*itxt) != L"info"
			&& std::GetRealFileName(*itxt) != L"log"
			&& (!StartWith(*itxt, L"fix\\")))
		{

			std::GoToXY(0, 15);
			wprintf(L"                                                                                         ");
			std::GoToXY(0, 15);
			wprintf(L"Read file %s..", itxt->c_str());
			std::wstring fileContent = std::ReadFile(*itxt);
			wprintf(L"fix..");

			VietnameseTextNormalizer vntObject;
			//vntObject.flagStandardTextForNLP = true;
			//vntObject.flagStandardTextForTTS = true;

			vntObject.Input(fileContent.c_str());
			vntObject.Normalize();
			vntObject.GenStandardText();
			wprintf(L"save..");
			if (vntObject.standardText && vntObject.standardTextChange > 0)
			{
				std::WriteFile(L"fix\\" + (*itxt), std::wstring(vntObject.standardText), true);
				countTotalChange += vntObject.standardTextChange;
#ifdef _DEBUG
				vntObject.logFile = fopen("log.txt", "a");
				fseek(vntObject.logFile, 0, SEEK_END);
				vntObject.Log("\n%ls :\n", std::GetRealFileName(*itxt).c_str());

				std::wstringset listedSet;
				for (auto textNode = vntObject.head; textNode != NULL; textNode = textNode->next)
				{
					if (textNode->originalText)
					{
						bool flagCurrentNodeIsChange = (textNode->textLength != textNode->originalTextLength);
						if (flagCurrentNodeIsChange == false)
						{
							for (int ichar = 0; ichar < textNode->textLength; ichar++)
							{
								if (textNode->text[ichar] != textNode->originalText[ichar])
								{
									flagCurrentNodeIsChange = true;
									ichar = textNode->textLength/*soft break*/;
								}
							}
						}
						if (flagCurrentNodeIsChange)
						{
							std::wstring currentList = std::GetWString(textNode->originalText, textNode->originalTextLength) + L" " + std::GetWString(textNode->text, textNode->textLength);
							if (listedSet.insert(currentList).second == true)
							{
								std::wstring originalSyllable = std::GetWString(textNode->originalText, textNode->originalTextLength);
								std::wstring newSyllable = std::GetWString(textNode->text, textNode->textLength);
								std::wstring originalBase = GetBase(originalSyllable);
								std::wstring newBase = GetBase(newSyllable);
								if (originalBase != newBase)
								{
									vntObject.Log("\t\t+Fix \"");
									vntObject.Log(textNode->originalText, textNode->originalTextLength);
									vntObject.Log("\" - %d character ----> \"", textNode->originalTextLength);
									vntObject.Log(textNode->text, textNode->textLength);
									vntObject.Log("\" - %d character\n", textNode->textLength);
								}
							}
						}
					}
				}



				vntObject.Log("\n");
				fclose(vntObject.logFile);
				vntObject.logFile = NULL;
#endif

			}
			wprintf(L"done!");
			std::GoToXY(0, 16);
			wprintf(L"Fix %s : %d change                    \r\n", itxt->c_str(), vntObject.standardTextChange);
			wprintf(L"Count total %d change                    \r\n", countTotalChange);
		}
	}
	wprintf(L"Done");
	_getch();
}
#endif
#else
#ifdef PY_MAJOR_VERSION
/************************************************************************/
/* Python wrapper                                                       */
/************************************************************************/
static PyObject* VietnameseTextNormalizerStandard(PyObject* self, PyObject* args)
{
	char				nullUtf8String[10] = { 0 };
	wchar_t				nullUnicodeString[10] = { 0 };
	char* utf8input = nullUtf8String;
	wchar_t* unicodeInput = nullUnicodeString;
	if (PyArg_ParseTuple(args, "s", &utf8input) && utf8input != NULL && utf8input != nullUtf8String)
	{
		std::string	utf8Result = utf8input;
		if (utf8input)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			std::wstring inputUcs2 = myconv.from_bytes(utf8input);
			size_t nChar = inputUcs2.size();
			qwchar* ucs2buffer = (qwchar*)qcalloc(nChar + 10/*safe*/, sizeof(qwchar));
			if (ucs2buffer)
			{
				for (size_t iChar = 0; iChar < nChar; iChar++)
				{
					ucs2buffer[iChar] = (qwchar)inputUcs2[iChar];
				}
				VietnameseTextNormalizer vntObject;
				vntObject.Input(ucs2buffer);
				vntObject.Normalize();
				vntObject.GenStandardText();
				if (vntObject.standardText && vntObject.standardTextChange > 0)
				{
					printf("Normalization : %d change(s) - Utf8 mode\n", vntObject.standardTextChange);
					std::wstring outputUcs2;
					outputUcs2.reserve(vntObject.standardTextLength + 10/*safe*/);
					for (int iChar = 0; iChar < vntObject.standardTextLength; iChar++)
					{
						outputUcs2 += (wchar_t)(vntObject.standardText[iChar]);
					}
					utf8Result = myconv.to_bytes(outputUcs2);
				}
				qfree(ucs2buffer);
			}
		}
		return Py_BuildValue("s", utf8Result.c_str());
	}
	else if (PyArg_ParseTuple(args, "u", &unicodeInput) && unicodeInput != NULL && unicodeInput != nullUnicodeString)
	{
		std::wstring		unicodeResult = unicodeInput;
		size_t				unicodeLength = unicodeResult.size();
		qwchar* ucs2buffer = (qwchar*)qcalloc(unicodeLength + 10/*safe*/, sizeof(qwchar));
		if (ucs2buffer)
		{
			for (size_t iChar = 0; iChar < unicodeLength; iChar++)
			{
				ucs2buffer[iChar] = (qwchar)unicodeInput[iChar];
			}
			VietnameseTextNormalizer vntObject;
			vntObject.Input(ucs2buffer);
			vntObject.Normalize();
			vntObject.GenStandardText();
			qfree(ucs2buffer);
			if (vntObject.standardText && vntObject.standardTextChange > 0)
			{
				printf("Normalization : %d change(s) - Ucs2 mode\n", vntObject.standardTextChange);
				unicodeResult.reserve(vntObject.standardTextLength + 10/*safe*/);
				for (int iChar = 0; iChar < vntObject.standardTextChange; iChar++)
				{
					unicodeResult += (wchar_t)(vntObject.standardText[iChar]);
				}
			}
		}
		return  Py_BuildValue("u", (Py_UNICODE*)(unicodeResult.c_str()));
	}
	PyObject* argsObject = NULL;
	PyArg_ParseTuple(args, "O", &argsObject);
	return argsObject;
}
static PyObject* VietnameseTextNormalizerForTTS(PyObject* self, PyObject* args)
{
	char				nullUtf8String[10] = { 0 };
	wchar_t				nullUnicodeString[10] = { 0 };
	char* utf8input = nullUtf8String;
	wchar_t* unicodeInput = nullUnicodeString;
	if (PyArg_ParseTuple(args, "s", &utf8input) && utf8input != NULL && utf8input != nullUtf8String)
	{
		std::string	utf8Result = utf8input;
		if (utf8input)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
			std::wstring inputUcs2 = myconv.from_bytes(utf8input);
			size_t nChar = inputUcs2.size();
			qwchar* ucs2buffer = (qwchar*)qcalloc(nChar + 10/*safe*/, sizeof(qwchar));
			if (ucs2buffer)
			{
				for (size_t iChar = 0; iChar < nChar; iChar++)
				{
					ucs2buffer[iChar] = (qwchar)inputUcs2[iChar];
				}
				VietnameseTextNormalizer vntObject;
				vntObject.flagStandardTextForNLP = true;
				vntObject.flagStandardTextForTTS = true;
				vntObject.Input(ucs2buffer);
				vntObject.Normalize();
				vntObject.GenStandardText();
				if (vntObject.standardText && vntObject.standardTextChange > 0)
				{
					printf("Normalization : %d change(s) - Utf8 mode\n", vntObject.standardTextChange);
					std::wstring outputUcs2;
					outputUcs2.reserve(vntObject.standardTextLength + 10/*safe*/);
					for (int iChar = 0; iChar < vntObject.standardTextLength; iChar++)
					{
						outputUcs2 += (wchar_t)(vntObject.standardText[iChar]);
					}
					utf8Result = myconv.to_bytes(outputUcs2);
				}
				qfree(ucs2buffer);
			}
		}
		return Py_BuildValue("s", utf8Result.c_str());
	}
	else if (PyArg_ParseTuple(args, "u", &unicodeInput) && unicodeInput != NULL && unicodeInput != nullUnicodeString)
	{
		std::wstring		unicodeResult = unicodeInput;
		size_t					unicodeLength = unicodeResult.size();
		qwchar* ucs2buffer = (qwchar*)qcalloc(unicodeLength + 10/*safe*/, sizeof(qwchar));
		if (ucs2buffer)
		{
			for (size_t iChar = 0; iChar < unicodeLength; iChar++)
			{
				ucs2buffer[iChar] = (qwchar)unicodeInput[iChar];
			}
			VietnameseTextNormalizer vntObject;
			vntObject.flagStandardTextForNLP = true;
			vntObject.flagStandardTextForTTS = true;
			vntObject.Input(ucs2buffer);
			vntObject.Normalize();
			vntObject.GenStandardText();
			qfree(ucs2buffer);
			if (vntObject.standardText && vntObject.standardTextChange > 0)
			{
				printf("Normalization : %d change(s) - Ucs2 mode\n", vntObject.standardTextChange);
				unicodeResult.reserve(vntObject.standardTextLength + 10/*safe*/);
				for (int iChar = 0; iChar < vntObject.standardTextChange; iChar++)
				{
					unicodeResult += (wchar_t)(vntObject.standardText[iChar]);
				}
			}
		}
		return  Py_BuildValue("u", (Py_UNICODE*)(unicodeResult.c_str()));
	}
	PyObject* argsObject = NULL;
	PyArg_ParseTuple(args, "O", &argsObject);
	return argsObject;
}
/************************************************************************/
/*                                                                      */
/************************************************************************/
static PyMethodDef	VietnameseTextNormalizerMethods[] = {
	{ "VietnameseTextNormalizer", VietnameseTextNormalizerStandard, METH_VARARGS, "OutputString VietnameseTextNormalizer(String)" },
	{ "Normalize", VietnameseTextNormalizerStandard, METH_VARARGS, "OutputString Normalize(String)" },
	{ "TTSNormalize", VietnameseTextNormalizerForTTS, METH_VARARGS, "OutputString TTSNormalize(String)" },
	{ NULL, NULL, 0, NULL }
};
#if (PY_MAJOR_VERSION == 3)
static struct PyModuleDef VietnameseTextNormalizerModule = {
	PyModuleDef_HEAD_INIT,
	"VietnameseTextNormalizer",   /* name of module */
	"Vietnamese Text Normalizer", /* module documentation, may be NULL */
	-1,       /* size of per-interpreter state of the module,
			  or -1 if the module keeps state in global variables. */
	VietnameseTextNormalizerMethods
};
PyMODINIT_FUNC PyInit_VietnameseTextNormalizer(void)
{
	char author[] = { (char)(9),(char)(66),(char)(-61),(char)(-71),(char)(105),(char)(32),(char)(84),(char)(-31),(char)(-70),(char)(-91),(char)(110),(char)(32),(char)(81),(char)(117),(char)(97),(char)(110),(char)(103),(char)(32),(char)(45),(char)(32),(char)(108),(char)(97),(char)(110),(char)(103),(char)(109),(char)(97),(char)(110),(char)(105),(char)(110),(char)(116),(char)(101),(char)(114),(char)(110),(char)(101),(char)(116),(char)(64),(char)(103),(char)(109),(char)(97),(char)(105),(char)(108),(char)(46),(char)(99),(char)(111),(char)(109),0 };
	printf("Init Vietnamese Text Normalizer version 1.5\n");
	printf("%s\n", author);
	return PyModule_Create(&VietnameseTextNormalizerModule);
}
PyMODINIT_FUNC PyInit_libVietnameseTextNormalizer(void)
{
	char author[] = { (char)(9),(char)(66),(char)(-61),(char)(-71),(char)(105),(char)(32),(char)(84),(char)(-31),(char)(-70),(char)(-91),(char)(110),(char)(32),(char)(81),(char)(117),(char)(97),(char)(110),(char)(103),(char)(32),(char)(45),(char)(32),(char)(108),(char)(97),(char)(110),(char)(103),(char)(109),(char)(97),(char)(110),(char)(105),(char)(110),(char)(116),(char)(101),(char)(114),(char)(110),(char)(101),(char)(116),(char)(64),(char)(103),(char)(109),(char)(97),(char)(105),(char)(108),(char)(46),(char)(99),(char)(111),(char)(109),0 };
	printf("Init Vietnamese Text Normalizer version 1.5\n");
	printf("%s\n", author);
	return PyModule_Create(&VietnameseTextNormalizerModule);
}
#else
PyMODINIT_FUNC initVietnameseTextNormalizer(void)
{
	char author[] = { (char)(9),(char)(66),(char)(-61),(char)(-71),(char)(105),(char)(32),(char)(84),(char)(-31),(char)(-70),(char)(-91),(char)(110),(char)(32),(char)(81),(char)(117),(char)(97),(char)(110),(char)(103),(char)(32),(char)(45),(char)(32),(char)(108),(char)(97),(char)(110),(char)(103),(char)(109),(char)(97),(char)(110),(char)(105),(char)(110),(char)(116),(char)(101),(char)(114),(char)(110),(char)(101),(char)(116),(char)(64),(char)(103),(char)(109),(char)(97),(char)(105),(char)(108),(char)(46),(char)(99),(char)(111),(char)(109),0 };
	printf("Init Vietnamese Text Normalization version 1.5\n");
	printf("%s\n", author);
	/* Create the module and add the functions */
	Py_InitModule("VietnameseTextNormalizer", VietnameseTextNormalizerMethods);
}
PyMODINIT_FUNC initlibVietnameseTextNormalizer(void)
{
	char author[] = { (char)(9),(char)(66),(char)(-61),(char)(-71),(char)(105),(char)(32),(char)(84),(char)(-31),(char)(-70),(char)(-91),(char)(110),(char)(32),(char)(81),(char)(117),(char)(97),(char)(110),(char)(103),(char)(32),(char)(45),(char)(32),(char)(108),(char)(97),(char)(110),(char)(103),(char)(109),(char)(97),(char)(110),(char)(105),(char)(110),(char)(116),(char)(101),(char)(114),(char)(110),(char)(101),(char)(116),(char)(64),(char)(103),(char)(109),(char)(97),(char)(105),(char)(108),(char)(46),(char)(99),(char)(111),(char)(109),0 };
	printf("Init Vietnamese Text Normalization version 1.0\n");
	printf("%s\n", author);
	/* Create the module and add the functions */
	Py_InitModule("VietnameseTextNormalizer", VietnameseTextNormalizerMethods);
}
#endif // (PY_MAJOR_VERSION == 3)
#endif
#endif




