#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <iostream>
#include <tchar.h> // Include tchar.h for _tcscmp and related functions

#pragma comment(lib, "psapi.lib")

BOOL InjectDLL(DWORD dwPID, const wchar_t* dllPath)
{
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);
    if (!hProcess)
    {
        std::cerr << "Failed to open process. Error: " << GetLastError() << std::endl;
        return FALSE;
    }

    LPVOID pRemotePath = VirtualAllocEx(hProcess, NULL, (wcslen(dllPath) + 1) * sizeof(wchar_t), MEM_COMMIT, PAGE_READWRITE);
    if (!pRemotePath)
    {
        std::cerr << "Failed to allocate memory in the remote process. Error: " << GetLastError() << std::endl;
        CloseHandle(hProcess);
        return FALSE;
    }

    if (!WriteProcessMemory(hProcess, pRemotePath, (LPVOID)dllPath, (wcslen(dllPath) + 1) * sizeof(wchar_t), NULL))
    {
        std::cerr << "Failed to write to the remote process memory. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW"),
        pRemotePath, 0, NULL);

    if (!hThread)
    {
        std::cerr << "Failed to create remote thread. Error: " << GetLastError() << std::endl;
        VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    WaitForSingleObject(hThread, INFINITE);

    VirtualFreeEx(hProcess, pRemotePath, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return TRUE;
}

BOOL IsFileOperationProcess(DWORD processID)
{
    TCHAR processName[MAX_PATH] = TEXT("<unknown>");

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

    if (NULL != hProcess)
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded))
        {
            GetModuleBaseName(hProcess, hMod, processName, sizeof(processName) / sizeof(TCHAR));
        }
    }

    CloseHandle(hProcess);

    return (_tcscmp(processName, TEXT("explorer.exe")) == 0);
}

int main()
{
    const wchar_t* dllPath = L"C:\\Users\\SAFC Lab\\source\\repos\\Det11Jul\\x64\\Debug\\Det11Jul.dll"; // Replace with the path to your DLL
    DWORD processIDs[1024], cbNeeded, cProcesses;

    if (!EnumProcesses(processIDs, sizeof(processIDs), &cbNeeded))
    {
        std::cerr << "Failed to enumerate processes. Error: " << GetLastError() << std::endl;
        return 1;
    }

    cProcesses = cbNeeded / sizeof(DWORD);

    for (unsigned int i = 0; i < cProcesses; i++)
    {
        if (processIDs[i] != 0 && IsFileOperationProcess(processIDs[i]))
        {
            if (!InjectDLL(processIDs[i], dllPath))
            {
                std::cerr << "Failed to inject DLL into process " << processIDs[i] << ". Error: " << GetLastError() << std::endl;
            }
        }
    }

    std::cout << "DLL injection completed." << std::endl;

    return 0;
}
