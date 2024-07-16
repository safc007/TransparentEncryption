#include <windows.h>
#include <detours.h>
#include <iostream>
#include <shlwapi.h> // For PathMatchSpec

#pragma comment(lib, "detours.lib")
#pragma comment(lib, "shlwapi.lib")

// Original function pointers
static BOOL(WINAPI* Real_CopyFileW)(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists) = CopyFileW;
static BOOL(WINAPI* Real_CopyFileExW)(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags) = CopyFileExW;
static BOOL(WINAPI* Real_MoveFileW)(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName) = MoveFileW;
static BOOL(WINAPI* Real_MoveFileExW)(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags) = MoveFileExW;

// Hooked functions
BOOL WINAPI Hooked_CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
{
    std::wcout << L"Hooked_CopyFileW called with source: " << lpExistingFileName << L", destination: " << lpNewFileName << std::endl;
    return Real_CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);
}

BOOL WINAPI Hooked_CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
    std::wcout << L"Hooked_CopyFileExW called with source: " << lpExistingFileName << L", destination: " << lpNewFileName << std::endl;
    return Real_CopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
}

BOOL WINAPI Hooked_MoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
    std::wcout << L"Hooked_MoveFileW called with source: " << lpExistingFileName << L", destination: " << lpNewFileName << std::endl;
    return Real_MoveFileW(lpExistingFileName, lpNewFileName);
}

BOOL WINAPI Hooked_MoveFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags)
{
    std::wcout << L"Hooked_MoveFileExW called with source: " << lpExistingFileName << L", destination: " << lpNewFileName << std::endl;
    return Real_MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);
}

void SetupHooks()
{
    // Initialize Detours
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    std::wcout << L"SetupHooks working" << std::endl;

    // Create hooks for file operations
    if (DetourAttach(&(PVOID&)Real_CopyFileW, Hooked_CopyFileW) != NO_ERROR) {
        std::cerr << "Failed to attach CopyFileW" << std::endl;
    }
    else {
        std::wcout << L"Attached Hooked_CopyFileW successfully" << std::endl;
    }

    if (DetourAttach(&(PVOID&)Real_CopyFileExW, Hooked_CopyFileExW) != NO_ERROR) {
        std::cerr << "Failed to attach CopyFileExW" << std::endl;
    }
    else {
        std::wcout << L"Attached Hooked_CopyFileExW successfully" << std::endl;
    }

    if (DetourAttach(&(PVOID&)Real_MoveFileW, Hooked_MoveFileW) != NO_ERROR) {
        std::cerr << "Failed to attach MoveFileW" << std::endl;
    }
    else {
        std::wcout << L"Attached Hooked_MoveFileW successfully" << std::endl;
    }

    if (DetourAttach(&(PVOID&)Real_MoveFileExW, Hooked_MoveFileExW) != NO_ERROR) {
        std::cerr << "Failed to attach MoveFileExW" << std::endl;
    }
    else {
        std::wcout << L"Attached Hooked_MoveFileExW successfully" << std::endl;
    }

    // Commit the transaction
    if (DetourTransactionCommit() == NO_ERROR)
    {
        std::cout << "Hooks enabled." << std::endl;
    }
    else
    {
        std::cerr << "Failed to enable hooks." << std::endl;
    }
}

void RemoveHooks()
{
    // Initialize Detours
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    // Remove hooks for file operations
    if (DetourDetach(&(PVOID&)Real_CopyFileW, Hooked_CopyFileW) != NO_ERROR) {
        std::cerr << "Failed to detach CopyFileW" << std::endl;
    }
    else {
        std::wcout << L"Detached Hooked_CopyFileW successfully" << std::endl;
    }

    if (DetourDetach(&(PVOID&)Real_CopyFileExW, Hooked_CopyFileExW) != NO_ERROR) {
        std::cerr << "Failed to detach CopyFileExW" << std::endl;
    }
    else {
        std::wcout << L"Detached Hooked_CopyFileExW successfully" << std::endl;
    }

    if (DetourDetach(&(PVOID&)Real_MoveFileW, Hooked_MoveFileW) != NO_ERROR) {
        std::cerr << "Failed to detach MoveFileW" << std::endl;
    }
    else {
        std::wcout << L"Detached Hooked_MoveFileW successfully" << std::endl;
    }

    if (DetourDetach(&(PVOID&)Real_MoveFileExW, Hooked_MoveFileExW) != NO_ERROR) {
        std::cerr << "Failed to detach MoveFileExW" << std::endl;
    }
    else {
        std::wcout << L"Detached Hooked_MoveFileExW successfully" << std::endl;
    }

    // Commit the transaction
    if (DetourTransactionCommit() == NO_ERROR)
    {
        std::cout << "Hooks disabled." << std::endl;
    }
    else
    {
        std::cerr << "Failed to disable hooks." << std::endl;
    }
}

int main()
{
    // Set up the hooks
    SetupHooks();

    // Keep the program running to maintain hooks
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    // Remove the hooks
    RemoveHooks();

    return 0;
}
