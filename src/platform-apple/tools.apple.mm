/*
 * This file is part of `et engine`
 * Copyright 2009-2013 by Sergey Reznik
 * Please, do not modify content without approval.
 *
 */

#include <sys/time.h>
#include <sys/stat.h>
#include <et/core/datastorage.h>
#include <et/core/tools.h>
#include <et/core/filesystem.h>

#if (ET_PLATFORM_MAC)
#	include <AppKit/NSWorkspace.h>
#	include <AppKit/NSScreen.h>
#elif (ET_PLATFORM_IOS)
#	include <UIKit/UIApplication.h>
#	include <UIKit/UIScreen.h>
#endif

static uint64_t startTime = 0;
static bool startTimeInitialized = false;

char et::pathDelimiter = '/';
char et::invalidPathDelimiter = '\\';

uint64_t queryActualTime();

float et::queryContiniousTimeInSeconds()
{
	return static_cast<float>(queryContiniousTimeInMilliSeconds()) / 1000.0f;
}

uint64_t et::queryContiniousTimeInMilliSeconds()
{
	if (!startTimeInitialized)
	{
		startTime = queryActualTime();
		startTimeInitialized = true;
	};
	
	return queryActualTime() - startTime;
}

uint64_t et::queryCurrentTimeInMicroSeconds()
{
	timeval tv = { };
	gettimeofday(&tv, 0);
	return static_cast<uint64_t>(tv.tv_sec) * 1000000 + static_cast<uint64_t>(tv.tv_usec);
}

uint64_t queryActualTime()
{
	timeval tv = { };
	gettimeofday(&tv, 0);
	return static_cast<uint64_t>(tv.tv_sec) * 1000 + static_cast<uint64_t>(tv.tv_usec) / 1000;
}

std::string et::applicationPath()
{
    char result[256] = { };
    
    CFURLRef bUrl = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFURLGetFileSystemRepresentation(bUrl, true, reinterpret_cast<UInt8*>(result), 256);
    CFRelease(bUrl);
    
    size_t i = 0;
    while ((i < 256) && result[++i]) { };
    result[i] = '/';
    
    return std::string(result);
}

std::string et::applicationPackagePath()
{
    return et::applicationPath();
}

std::string et::applicationDataFolder()
{
    char result[256] = { };
    
	CFURLRef bUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
    CFURLGetFileSystemRepresentation(bUrl, true, reinterpret_cast<UInt8*>(result), 256);
    CFRelease(bUrl);
    
    size_t i = 0;
    while ((i < 256) && result[++i]) { };
    result[i] = '/';
    
	return std::string(result);
}

bool et::fileExists(const std::string& name)
{
    NSString* fileName = [[NSString alloc] initWithUTF8String:name.c_str()];
	
	BOOL isDir = NO;
    BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:fileName isDirectory:&isDir];
#if (!ET_OBJC_ARC_ENABLED)
    [fileName release];
#endif
    return exists && !isDir;
}

bool et::folderExists(const std::string& name)
{
    NSString* fileName = [[NSString alloc] initWithUTF8String:name.c_str()];
	
	BOOL isDir = NO;
	BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:fileName isDirectory:&isDir];
#if (!ET_OBJC_ARC_ENABLED)
    [fileName release];
#endif
	return exists && isDir;
}

std::string et::libraryBaseFolder()
{
    @autoreleasepool
    {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES);
        NSString* folder = [[paths objectAtIndex:0] stringByAppendingString:@"/"];
        return std::string([folder cStringUsingEncoding:NSUTF8StringEncoding]);
    }
}

std::string et::temporaryBaseFolder()
{
    @autoreleasepool
    {
        return std::string([NSTemporaryDirectory() cStringUsingEncoding:NSUTF8StringEncoding]);
    }
}

std::string et::documentsBaseFolder()
{
    @autoreleasepool 
    {
        NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString* folder = [[paths objectAtIndex:0] stringByAppendingString:@"/"];
        return std::string([folder cStringUsingEncoding:NSUTF8StringEncoding]);
    }
}

bool et::createDirectory(const std::string& name, bool intermediates)
{
	NSError* error = nil;
	NSString* path = [NSString stringWithUTF8String:name.c_str()];
	
	BOOL result = [[NSFileManager defaultManager] createDirectoryAtPath:path
		withIntermediateDirectories:(intermediates ? YES : NO) attributes:nil error:&error];
	
	if (error)
		NSLog(@"Unable to create directory at %@, error: %@", path, error);
	
	return result;
}

bool et::removeDirectory(const std::string& name)
{
	NSError* error = nil;
	
	NSString* path = [NSString stringWithUTF8String:name.c_str()];
	BOOL result = [[NSFileManager defaultManager] removeItemAtPath:path error:&error];
	
	if (error)
		NSLog(@"Unable to delete directory at %@, error: %@", path, error);
	
	return result;
}

bool et::removeFile(const std::string& name)
{
	NSError* error = nil;
	
	NSString* path = [NSString stringWithUTF8String:name.c_str()];
	BOOL result = [[NSFileManager defaultManager] removeItemAtPath:path error:&error];
	
	if (error)
		NSLog(@"Unable to delete file at %@, error: %@", path, error);
	
	return result;
}

void et::getFolderContent(const std::string& folder, StringList& list)
{
	NSError* err = nil;
	NSString* path = [NSString stringWithCString:folder.c_str() encoding:NSUTF8StringEncoding];
	NSArray* files = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:path error:&err];
	
	for (NSString* file in files)
		list.push_back(folder + std::string([file cStringUsingEncoding:NSUTF8StringEncoding]));
}

void et::findFiles(const std::string& folder, const std::string& mask, bool /* recursive */, std::vector<std::string>& list)
{
	size_t maskLength = mask.length();
	size_t nameSearchCriteria = mask.find_last_of(".*");
	BOOL searchByName = nameSearchCriteria == maskLength - 1;
	BOOL searchByExt = mask.find_first_of("*.") == 0;
	
	NSError* err = nil;
	NSString* path = [NSString stringWithCString:folder.c_str() encoding:NSASCIIStringEncoding];
	NSArray* files = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:path error:&err];
	
	NSArray* filtered = files;
	
	if (searchByExt)
	{
		std::string extMask = mask.substr(2, mask.length() - 2);
		NSString* objcMask = [NSString stringWithCString:extMask.c_str() encoding:NSASCIIStringEncoding];
		filtered = [filtered filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"self ENDSWITH %@", objcMask]];
	}
	
	if (searchByName)
	{
		std::string nameMask = mask.substr(0, mask.length() - 3);
		NSString* objcMask = [NSString stringWithCString:nameMask.c_str() encoding:NSASCIIStringEncoding];
		objcMask = [objcMask stringByAppendingString:@"*"];
		filtered = [filtered filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"self LIKE %@", objcMask]];
	}
	
	for (NSString* file in filtered)
		list.push_back(folder + std::string([file cStringUsingEncoding:NSUTF8StringEncoding]));
}

void et::findSubfolders(const std::string& folder, bool recursive, std::vector<std::string>& list)
{
	NSString* path = [NSString stringWithCString:folder.c_str() encoding:NSASCIIStringEncoding];
	if (![path hasSuffix:@"/"])
		path = [path stringByAppendingString:@"/"];
	
	NSError* err = nil;
	NSArray* files = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:path error:&err];
	
	if (err != nil)
	{
		NSLog(@"Unable to findSubfolders: %@", err);
		return;
	}
	
	for (NSString* entry in files)
	{
		BOOL isDir = false;
		NSString* expandedPath = [path stringByAppendingFormat:@"%@/", entry];
 		if ([[NSFileManager defaultManager] fileExistsAtPath:expandedPath isDirectory:&isDir] && isDir)
		{
			std::string entryStr([expandedPath cStringUsingEncoding:NSASCIIStringEncoding]);
			list.push_back(entryStr);
			if (recursive)
				findSubfolders(entryStr, true, list);
		}
	}
}

void et::openUrl(const std::string& url)
{
	if (url.size() == 0) return;
	
	NSURL* aUrl = [NSURL URLWithString:[NSString stringWithUTF8String:url.c_str()]];
	
#if (ET_PLATFORM_IOS)
	[[UIApplication sharedApplication] openURL:aUrl];
#else
	[[NSWorkspace sharedWorkspace] openURL:aUrl];
#endif
}

std::string et::unicodeToUtf8(const std::wstring& w)
{
	NSString* s = [[NSString alloc] initWithBytes:w.c_str() length:w.length() * sizeof(wchar_t)
		encoding:NSUTF32LittleEndianStringEncoding];
	
#if (!ET_OBJC_ARC_ENABLED)
	[s autorelease];
#endif
	
	if (s == nil)
	{
		NSLog(@"Unable to convert wstring to NSString.");
		return std::string();
	}
	
	if (![s canBeConvertedToEncoding:NSUTF8StringEncoding])
	{
		NSLog(@"Unable to convert %@ to NSUTF8StringEncoding", s);
		return std::string();
	}
	
	NSUInteger actualLength = 0;
	
	[s getBytes:0 maxLength:0 usedLength:&actualLength
	   encoding:NSUTF8StringEncoding options:0 range:NSMakeRange(0, [s length]) remainingRange:0];
	
	BinaryDataStorage result(actualLength + sizeof(char), 0);
	
	[s getBytes:result.data() maxLength:result.dataSize() usedLength:0
	   encoding:NSUTF8StringEncoding options:0 range:NSMakeRange(0, [s length]) remainingRange:0];
	
	return std::string(reinterpret_cast<const char*>(result.data()));
}

std::wstring et::utf8ToUnicode(const std::string& mbcs)
{
	NSString* s = [NSString stringWithUTF8String:mbcs.c_str()];
	
	if (s == nil)
	{
		NSLog(@"Unable to convert UTF-8 `%s` to NSString.", mbcs.c_str());
		return std::wstring();
	}
	
	if (![s canBeConvertedToEncoding:NSUTF32LittleEndianStringEncoding])
	{
		NSLog(@"Unable to convert %@ to NSUTF32LittleEndianStringEncoding", s);
		return std::wstring();
	}
	
	NSUInteger actualLength = 0;
    
	[s getBytes:0 maxLength:0 usedLength:&actualLength
	   encoding:NSUTF32LittleEndianStringEncoding options:0 range:NSMakeRange(0, [s length]) remainingRange:0];
	
	BinaryDataStorage result(actualLength + sizeof(wchar_t), 0);
	
	[s getBytes:result.data() maxLength:result.dataSize() usedLength:0
	   encoding:NSUTF32LittleEndianStringEncoding options:0 range:NSMakeRange(0, [s length]) remainingRange:0];
	
	return std::wstring(reinterpret_cast<const wchar_t*>(result.data()));
}

std::string et::applicationIdentifierForCurrentProject()
{
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	return std::string(CFStringGetCStringPtr(CFBundleGetIdentifier(mainBundle), kCFStringEncodingMacRoman));
}

et::vec2i et::nativeScreenSize()
{
#if (ET_PLATFORM_IOS)

	CGSize size = [[UIScreen mainScreen] bounds].size;
	
	if (UIInterfaceOrientationIsLandscape([[UIApplication sharedApplication] statusBarOrientation]))
		return vec2i(static_cast<int>(size.height), static_cast<int>(size.width));
	else
		return vec2i(static_cast<int>(size.width), static_cast<int>(size.height));
	
#else

	NSSize size = [[NSScreen mainScreen] frame].size;
	return vec2i(static_cast<int>(size.width), static_cast<int>(size.height));

#endif
}

int64_t et::getFileDate(const std::string& path)
{
	struct stat s = { };
	stat(path.c_str(), &s);
	return s.st_mtimespec.tv_sec;
}