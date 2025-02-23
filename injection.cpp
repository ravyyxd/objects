#include <windows.h>
#include <tlhelp32.h>
#include <shlwapi.h>
#include <iostream>
#include <fstream>

using namespace std;

const string PROGRAM_TITLE = "x64 Injector"; // The title of the console window
const string DLL_NAME = "Module.dll"; // The name of the dll to inject

BOOL Inject(DWORD pID, const char* DLL_NAME)
{
    HANDLE Proc;
    HMODULE hLib;
    char buf[50] = { 0 };
    LPVOID RemoteString, LoadLibAddy;
    if (!pID)
        return false;
    Proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
    if (!Proc)
    {
        sprintf(buf, "OpenProcess() failed: %d", GetLastError());
        MessageBoxA(NULL, buf, "Loader", MB_OK);
        printf(buf);
        return false;
    }
    LoadLibAddy = (LPVOID)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
    // Allocate space in the process for our DLL
    RemoteString = (LPVOID)VirtualAllocEx(Proc, NULL, strlen(DLL_NAME), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    // Write the string name of our DLL in the memory allocated
    WriteProcessMemory(Proc, (LPVOID)RemoteString, DLL_NAME, strlen(DLL_NAME), NULL);
    // Load our <strong class="highlight">DLL</strong>
    CreateRemoteThread(Proc, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibAddy, (LPVOID)RemoteString, NULL, NULL);
    CloseHandle(Proc);
    return true;
}

DWORD GetTargetThreadIDFromProcName(const wchar_t* ProcName)
{
    PROCESSENTRY32W pe;
    HANDLE thSnapShot;
    BOOL retval;

    thSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (thSnapShot == INVALID_HANDLE_VALUE)
    {
        wcout << L"Error: Unable to create toolhelp snapshot!" << endl;
        return 0;
    }
    pe.dwSize = sizeof(PROCESSENTRY32W);
    retval = Process32FirstW(thSnapShot, &pe);

    while (retval)
    {
        if (wcsstr(pe.szExeFile, ProcName))
        {
            CloseHandle(thSnapShot);
            return pe.th32ProcessID;
        }
        retval = Process32NextW(thSnapShot, &pe);
    }

    CloseHandle(thSnapShot);
    return 0;
}


inline bool FileExists(const std::string& name) {
    ifstream f(name.c_str());
    if (f.good()) {
        f.close();
        return true;
    }
    else {
        f.close();
        return false;
    }
}

int main() {

    SetConsoleTitleA(PROGRAM_TITLE.c_str());

    cout << "Make sure ROBLOX is fully loaded, then press Enter to inject " + DLL_NAME + "...";
    cin.get();

    // Get running process RobloxPlayerBeta.exe
    DWORD pID = GetTargetThreadIDFromProcName(L"RobloxPlayerBeta.exe");

    // Get the dll's full path name
    char buf[MAX_PATH] = { 0 };
    GetFullPathNameA(DLL_NAME.c_str(), MAX_PATH, buf, NULL);

    // Inject the dll
    if (!FileExists(DLL_NAME) || !Inject(pID, buf)) {
        cout << "Injection failed. Please make sure " + DLL_NAME + " exists in the program's directory and try again." << endl;
        cin.get();
        return 1;
    }

    cout << "Successfully injected into RobloxPlayerBeta.exe! zex Injector" << endl;
    Sleep(3000);

    return 0;
}