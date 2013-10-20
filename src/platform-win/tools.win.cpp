/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#include <Windows.h>
#include <Shlobj.h>
#include <et/core/tools.h>
#include <et/core/filesystem.h>
#include <et/core/containers.h>

static bool shouldInitializeTime = true;
static uint64_t performanceFrequency = 0;
static uint64_t initialCounter = 0;

void initTime()
{
	LARGE_INTEGER c = { };
	LARGE_INTEGER f = { };

	shouldInitializeTime = false;

	QueryPerformanceCounter(&c);
	QueryPerformanceFrequency(&f);

	initialCounter = c.QuadPart;
	performanceFrequency = f.QuadPart;
}

uint64_t et::queryContiniousTimeInMilliSeconds()
{
	if (shouldInitializeTime)
		initTime();

	LARGE_INTEGER c = { };
	QueryPerformanceCounter(&c);

	return 1000 * (c.QuadPart - initialCounter) / performanceFrequency;
}

float et::queryContiniousTimeInSeconds()
{
	return static_cast<float>(queryContiniousTimeInMilliSeconds()) / 1000.0f;
} 

char et::pathDelimiter = '\\';
char et::invalidPathDelimiter = '/';

std::string et::applicationPath()
{
	char ExePath[MAX_PATH] = { };
	GetModuleFileNameA(nullptr, ExePath, MAX_PATH);
	return getFilePath(normalizeFilePath(ExePath));
}

bool et::fileExists(const std::string& name)
{
	return GetFileAttributes(name.c_str()) != INVALID_FILE_ATTRIBUTES;
}

bool et::folderExists(const std::string& folder)
{
	auto fa = GetFileAttributes(folder.c_str());
	return (fa != INVALID_FILE_ATTRIBUTES) && ((fa & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
}

void et::findFiles(const std::string& folder, const std::string& mask, bool recursive, StringList& list)
{
	std::string normalizedFolder = addTrailingSlash(folder);
	std::string searchPath = normalizedFolder + mask;

	StringList folderList;
	if (recursive)
	{
		std::string foldersSearchPath = normalizedFolder + "*.*";
		WIN32_FIND_DATA folders = { };
		HANDLE folderSearch = FindFirstFile(foldersSearchPath.c_str(), &folders);
		if (folderSearch != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (strcmp(folders.cFileName, ".") && strcmp(folders.cFileName, "..") && 
					((folders.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
				{
					std::string folderName(folders.cFileName);
					folderList.push_back(normalizedFolder + folderName);
				}
			}
			while (FindNextFile(folderSearch, &folders));
			FindClose(folderSearch);
		}
	}

	WIN32_FIND_DATA data = { };
	HANDLE search = FindFirstFile(searchPath.c_str(), &data);

	if (search != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (strcmp(data.cFileName, ".") && strcmp(data.cFileName, ".."))
			{
				std::string fileName(data.cFileName);
				list.push_back(normalizedFolder + fileName);
			}
		}
		while (FindNextFile(search, &data));
		FindClose(search);
	}

	if (recursive)
	{
		ET_ITERATE(folderList, const std::string&, i, findFiles(i, mask, recursive, list))
	}
}

std::string et::applicationPackagePath()
{
	char buffer[MAX_PATH] = { };
	GetCurrentDirectory(MAX_PATH, buffer);
	return addTrailingSlash(std::string(buffer));
}

std::string et::applicationDataFolder()
{
	char buffer[MAX_PATH] = { };
	GetCurrentDirectory(MAX_PATH, buffer);
	return addTrailingSlash(std::string(buffer));
}

std::string et::documentsBaseFolder()
{
	wchar_t* path = nullptr;

	SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &path);
	std::string result = addTrailingSlash(unicodeToUtf8(path));

	CoTaskMemFree(path);

	return result;
}

std::string et::libraryBaseFolder()
{
	wchar_t* path = nullptr;

	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path);
	std::string result = addTrailingSlash(unicodeToUtf8(path));

	CoTaskMemFree(path);

	return result;
}

bool et::createDirectory(const std::string& name, bool recursive)
{
	if (recursive)
	{
		char delim[] = {pathDelimiter, 0};
		bool gotError = false;
		StringList components = split(name, std::string(delim));
		std::string path;
		for (auto& dir : components)
		{
			path += dir + "\\";
			if (!folderExists(path))
				gotError |= ::CreateDirectory(path.c_str(), nullptr) == 0;
		}

		return !gotError;
	}
	else
	{
		return ::CreateDirectory(name.c_str(), nullptr) == 0;
	}
}

bool et::removeFile(const std::string& path)
{
	return DeleteFile(path.c_str()) != 0;
}

void et::findSubfolders(const std::string& folder, bool recursive, StringList& list)
{
	std::string normalizedFolder = addTrailingSlash(folder);
	std::string foldersSearchPath = normalizedFolder + "*.*";
	StringList folderList;

	WIN32_FIND_DATA folders = { };
	HANDLE folderSearch = FindFirstFile(foldersSearchPath.c_str(), &folders);
	if (folderSearch != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (strcmp(folders.cFileName, ".") && strcmp(folders.cFileName, "..") && 
				((folders.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
			{
				std::string folderName(folders.cFileName);
				folderList.push_back(normalizedFolder + folderName + "\\");
			}
		}
		while (FindNextFile(folderSearch, &folders));
		FindClose(folderSearch);
	}

	if (recursive)
	{
		ET_ITERATE(folderList, const std::string&, i, findSubfolders(i, true, list))
	}

	list.insert(list.end(), folderList.begin(), folderList.end());
}
/*
std::string et::selectFile(const StringList& mask)
{
	size_t maskLen = 1;
	ET_ITERATE(mask, const std::string&, i, maskLen += i.size() + 1)

	char result[MAX_PATH] = { };
	char* filter = new char[maskLen];
	memset(filter, 0, maskLen);

	size_t offset = 0;
	ET_ITERATE(mask, const std::string&, i,
	{
		memcpy(filter + offset, i.data(), i.size());
		offset += i.size() + 1;
	})

	OPENFILENAME ofn = { };
	ofn.lStructSize = sizeof(ofn);
	ofn.hInstance = GetModuleHandle(0);
	ofn.lpstrFilter = filter;
	ofn.Flags = OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
	ofn.lpstrFile = result;
	ofn.nMaxFile = MAX_PATH;

	GetOpenFileName(&ofn);

	delete [] filter;
	return std::string(result);
}

std::string et::selectFile(const std::string& description, const std::string& ext)
{
	StringList m;
	m.push_back(description);
	m.push_back(ext);
	return selectFile(m);
}
*/

void et::openUrl(const std::string& url)
{
	ShellExecute(0, "open", url.c_str(), 0, 0, SW_SHOWNORMAL);
}

std::string et::unicodeToUtf8(const std::wstring& w)
{
	int mbcWidth = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, 0, 0, 0, 0);

	if (mbcWidth == 0)
		return std::string();

	DataStorage<char> result(mbcWidth + 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, result.data(), result.size(), 0, 0);

	return std::string(result.data());
}

std::wstring et::utf8ToUnicode(const std::string& mbcs)
{
	int uWidth = MultiByteToWideChar(CP_UTF8, 0, mbcs.c_str(), -1, 0, 0);
	if (uWidth == 0)
	{
		switch (GetLastError())
		{
		case ERROR_INSUFFICIENT_BUFFER:
			std::cout << "A supplied buffer size was not large enough, or it was incorrectly set to NULL." << std::endl;
			break;

		case ERROR_INVALID_FLAGS:
			std::cout << "The values supplied for flags were not valid." << std::endl;
			break;

		case ERROR_INVALID_PARAMETER:
			std::cout << "Any of the parameter values was invalid." << std::endl;
			break;

		case ERROR_NO_UNICODE_TRANSLATION:
			std::cout << "Invalid Unicode was found in a string" << std::endl;
			break;

		default:
			std::cout << "Failed to convert utf-8 to wchar_t" << std::endl;
		}

		return std::wstring();
	}

	DataStorage<wchar_t> result(uWidth + 1, 0);
	MultiByteToWideChar(CP_UTF8, 0, mbcs.c_str(), -1, result.data(), result.size());

	return std::wstring(result.data());
}

std::string et::applicationIdentifierForCurrentProject()
	{ return "com.et.app"; }

int64_t et::getFileDate(const std::string& path)
{
	WIN32_FIND_DATA findData = { };
	HANDLE search = FindFirstFile(path.c_str(), &findData);
	FindClose(search);

	return static_cast<int64_t>(findData.ftLastWriteTime.dwLowDateTime);
}