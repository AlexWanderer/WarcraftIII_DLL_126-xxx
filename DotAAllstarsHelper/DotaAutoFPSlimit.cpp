#include "Main.h"

BOOL FPS_LIMIT_ENABLED = FALSE;

ULARGE_INTEGER lastCPU, lastSysCPU, lastUserCPU;
int numProcessors;
unsigned int CPU_cores;
HANDLE Warcraft3_Process = NULL;
p_SetMaxFps _SetMaxFps = NULL;

int __stdcall SetMaxFps( int fps )
{
	_SetMaxFps( fps );
	return fps;
}

void InitThreadCpuUsage( ) {
	SYSTEM_INFO sysInfo;
	FILETIME ftime, fsys, fuser;


	numProcessors = std::thread::hardware_concurrency( );
	if ( numProcessors == 0 )
	{
		GetSystemInfo( &sysInfo );
		numProcessors = sysInfo.dwNumberOfProcessors;
	}

	GetSystemTimeAsFileTime( &ftime );
	memcpy( &lastCPU, &ftime, sizeof( FILETIME ) );


	GetProcessTimes( Warcraft3_Process, &ftime, &ftime, &fsys, &fuser );
	memcpy( &lastSysCPU, &fsys, sizeof( FILETIME ) );
	memcpy( &lastUserCPU, &fuser, sizeof( FILETIME ) );
}

double GetWar3CpuUsage( ) {
	FILETIME ftime, fsys, fuser;
	ULARGE_INTEGER now, sys, user;
	double percent;
	GetSystemTimeAsFileTime( &ftime );
	memcpy( &now, &ftime, sizeof( FILETIME ) );
	GetProcessTimes( Warcraft3_Process, &ftime, &ftime, &fsys, &fuser );
	memcpy( &sys, &fsys, sizeof( FILETIME ) );
	memcpy( &user, &fuser, sizeof( FILETIME ) );
	percent = ( double )( ( sys.QuadPart - lastSysCPU.QuadPart ) +
		( user.QuadPart - lastUserCPU.QuadPart ) );
	percent /= ( now.QuadPart - lastCPU.QuadPart );
	lastCPU = now;
	lastUserCPU = user;
	lastSysCPU = sys;
	return percent * 100.0;
}


#define MAX_WAR3_FPS 80
#define MIN_WAR3_FPS 24
int CurrentFps = 64;
void UpdateFPS( )
{
	double currentcpuusage = GetWar3CpuUsage( );
	if ( currentcpuusage > 88.0 )
	{
		if ( CurrentFps > MIN_WAR3_FPS )
		{
			CurrentFps -= 4;
			if ( *InGame )
				_SetMaxFps( CurrentFps );
			sprintf_s( MyFpsString, 512, "%s%i.0 CPU:%.1f", "|nFPS: %.1f/", CurrentFps, currentcpuusage );
		}
	}
	else if ( currentcpuusage < 70.0 )
	{
		if ( CurrentFps < MAX_WAR3_FPS )
		{
			CurrentFps += 4;
			if ( *InGame )
				_SetMaxFps( CurrentFps );
			sprintf_s( MyFpsString, 512, "%s%i.0 CPU:%.1f", "|nFPS: %.1f/", CurrentFps, currentcpuusage );
		}
	}
}



int __stdcall EnableAutoFPSlimit( BOOL enable )
{
	FPS_LIMIT_ENABLED = enable;
	if ( !FPS_LIMIT_ENABLED )
		_SetMaxFps( 200 );
	return enable;
}

