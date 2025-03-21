#pragma once

#ifndef WIN32_LEAN_AND_MEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
	#define NOMINMAX
#endif

#include "Platform/GAPlatform.h"

#if IS_WIN32

	#include <direct.h>
	#include <windows.h>
	#include <VersionHelpers.h>
	#include <cerrno>
	#include <tchar.h>
	#include <sysinfoapi.h>

	#ifdef GA_SHARED_LIB 
		#include <comdef.h>
		#include <wbemidl.h>
		#pragma comment(lib, "wbemuuid.lib")
	#endif


	#pragma comment(lib, "Ws2_32.lib")
	#pragma comment(lib, "gdi32.lib")
	#pragma comment(lib, "crypt32.lib")

namespace gameanalytics
{
	class GAPlatformWin32 :
		public GAPlatform
	{
		public:

			std::string getOSVersion()			override;
			std::string getDeviceManufacturer() override;
			std::string getBuildPlatform()		override;
			std::string getPersistentPath()		override;
			std::string getDeviceModel()		override;

			void setupUncaughtExceptionHandler() override;

			std::string getConnectionType() override;

			virtual std::string getCpuModel() 			const override;
			virtual std::string getGpuModel() 			const override;
			virtual int 		getNumCpuCores() 		const override;
			virtual int64_t 	getTotalDeviceMemory() 	const override;

			virtual int64_t getAppMemoryUsage() const override;
			virtual int64_t getSysMemoryUsage() const override;

			virtual int64_t getBootTime() const override;

		private:

			static void signalHandler(int sig);

			static void (*old_state_ill)	(int);
			static void (*old_state_abrt)	(int);
			static void (*old_state_fpe)	(int);
			static void (*old_state_segv)	(int);
	};
}
#endif