#include "../Shared/stdafx.h"
#include "../Shared/trace.h"
#include <TlHelp32.h>

typedef struct tagFINDING_CONHOST
{
	HWND  ConsoleWindow;
	DWORD CurrentThreadId;
	DWORD Result;
}
FINDING_CONHOST, *PFINDING_CONHOST;

BOOL CALLBACK FindThreadId(HWND hWnd, LPARAM lParam)
{
	PFINDING_CONHOST State = (PFINDING_CONHOST)lParam;

	if (hWnd == State->ConsoleWindow)
	{
		State->Result = State->CurrentThreadId;
		return FALSE;
	}

	return TRUE;
}

DWORD WINAPI FindConhostUIThreadId(HWND hWnd)
{
	HideConsoleTrace(L"FindConhostUIThreadId: hWnd=0x%1!p!", hWnd);

	if (!hWnd)
	{
		return 0;
	}

	HANDLE Snapshot = CreateToolhelp32Snapshot(
		TH32CS_SNAPTHREAD,
		0
	);

	if (Snapshot == INVALID_HANDLE_VALUE)
	{
		HideConsoleTraceLastError(L"CreateToolhelp32Snapshot");
		return 0;
	}

	FINDING_CONHOST State;
	State.ConsoleWindow = hWnd;
	State.Result = 0;

	THREADENTRY32 ThreadEntry;
	ThreadEntry.dwSize = sizeof(ThreadEntry);

	if (!Thread32First(Snapshot, &ThreadEntry))
	{
		HideConsoleTraceLastError(L"Thread32First");
		goto Cleanup;
	}

	do
	{
		State.CurrentThreadId = ThreadEntry.th32ThreadID;

		if (State.CurrentThreadId)
		{
			HideConsoleTrace(
				L"FindConhostUIThreadId: EnumThreadWindows ThreadId=%1!u!",
				State.CurrentThreadId
			);

			EnumThreadWindows(
				State.CurrentThreadId,
				FindThreadId,
				(LPARAM)&State
			);

			if (State.Result)
				break;
		}

	} while (Thread32Next(Snapshot, &ThreadEntry));

	HideConsoleTrace(L"FindConhostUIThreadId: Result=%1!u!", State.Result);

Cleanup:

	CloseHandle(Snapshot);
	return State.Result;
}