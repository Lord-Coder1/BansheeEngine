#pragma once

#include "CmPrerequisitesUtil.h"

namespace CamelotFramework
{
	class CM_UTILITY_EXPORT FileSystem
	{
	public:
		static DataStreamPtr openFile(const WString& fullPath, bool readOnly = true);
		static DataStreamPtr createAndOpenFile(const WString& fullPath);

		static UINT64 getFileSize(const WString& fullPath);

		static void remove(const WString& fullPath, bool recursively = true);
		static void createDir(const WString& fullPath);

		static bool fileExists(const WString& fullPath);
		static bool dirExists(const WString& fullPath);

		static Vector<WString>::type getFiles(const WString& dirPath);

		static WString getWorkingDirectoryPath();
		static WString getParentDirectory(const WString& path);
	};
}