#include "AntiTamperLib.h"
#include <Windows.h>
#include <iostream>
#include <string>
#include <bcrypt.h>
#include <vector>
#include <TlHelp32.h>
#include <cstring>
#include <stdio.h>
#include <winternl.h>
#include <Psapi.h>
#include <intrin.h>
#include <shlwapi.h>
#include <time.h>
#include <algorithm>
#include <signal.h>
#include "../xorstr.hpp"
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "Shlwapi.lib")

typedef NTSTATUS(NTAPI* pNtQueryInformationProcess)(
    HANDLE ProcessHandle,
    PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength,
    PULONG ReturnLength
    );

volatile bool g_integrityCheckFailed = false;
volatile DWORD g_checkInterval = 500;
HANDLE g_watchdogThread = NULL;
CRITICAL_SECTION g_criticalSection;
std::vector<uint8_t*> g_protectedRegions;
std::vector<size_t> g_protectedRegionSizes;
std::vector<uint32_t> g_protectedRegionChecksums;

extern "C" DWORD GetEntryPoint();

void AddProtectedRegion(void* address, size_t size) {
    EnterCriticalSection(&g_criticalSection);
    uint32_t checksum = 0;
    uint8_t* data = (uint8_t*)address;

    for (size_t i = 0; i < size; i++) {
        checksum = _rotl(checksum, 1) ^ data[i];
    }

    g_protectedRegions.push_back((uint8_t*)address);
    g_protectedRegionSizes.push_back(size);
    g_protectedRegionChecksums.push_back(checksum);
    LeaveCriticalSection(&g_criticalSection);
}

uint32_t CalculateChecksum(uint8_t* data, size_t size) {
    uint32_t checksum = 0;
    for (size_t i = 0; i < size; i++) {
        checksum = _rotl(checksum, 1) ^ data[i];
    }
    return checksum;
}

DWORD WINAPI MemoryWatchdogThread(LPVOID lpParam) {
    while (true) {
        Sleep(g_checkInterval + (rand() % 250));

        EnterCriticalSection(&g_criticalSection);
        for (size_t i = 0; i < g_protectedRegions.size(); i++) {
            uint32_t currentChecksum = CalculateChecksum(g_protectedRegions[i], g_protectedRegionSizes[i]);
            if (currentChecksum != g_protectedRegionChecksums[i]) {
                g_integrityCheckFailed = true;
                LeaveCriticalSection(&g_criticalSection);

                srand((unsigned int)time(NULL));
                switch (rand() % 5) {
                case 0: ExitProcess(0xDEAD); break;
                case 1: *(volatile int*)0 = 0; break;
                case 2: TerminateProcess(GetCurrentProcess(), 0xBAD1); break;
                case 3: RaiseException(0xC0000005, 0, 0, NULL); break;
                case 4: TerminateProcess(GetCurrentProcess(), 0xBAAD); break;
                }
                return 0;
            }
        }
        LeaveCriticalSection(&g_criticalSection);
    }
    return 0;
}

bool AdvancedAntiDebug() {
    PPEB pPeb = nullptr;

#if defined(_M_X64) || defined(__x86_64__)
    pPeb = (PPEB)__readgsqword(0x60);
#else
    pPeb = (PPEB)__readfsdword(0x30);
#endif

    if (pPeb->BeingDebugged) {
        return true;
    }

    DWORD NtGlobalFlags = *(DWORD*)((BYTE*)pPeb + 0x68);
    if (NtGlobalFlags & 0x70) {
        return true;
    }

    CONTEXT ctx = {};
    ctx.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    if (GetThreadContext(GetCurrentThread(), &ctx)) {
        if (ctx.Dr0 != 0 || ctx.Dr1 != 0 || ctx.Dr2 != 0 || ctx.Dr3 != 0) {
            return true;
        }
    }

    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    OutputDebugStringA("Anti-Debug Check");

    QueryPerformanceCounter(&end);
    double elapsed = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
    if (elapsed > 0.01) {
        return true;
    }

    HMODULE hNtdll = GetModuleHandleA("ntdll.dll");
    if (hNtdll) {
        pNtQueryInformationProcess NtQueryInformationProcess = (pNtQueryInformationProcess)GetProcAddress(
            hNtdll, "NtQueryInformationProcess");
        if (NtQueryInformationProcess) {
            DWORD dwProcessDebugPort = 0;
            NTSTATUS status = NtQueryInformationProcess(
                GetCurrentProcess(),
                ProcessDebugPort,
                &dwProcessDebugPort,
                sizeof(DWORD),
                NULL);
            if (NT_SUCCESS(status) && dwProcessDebugPort != 0) {
                return true;
            }
        }
    }

    return false;
}

void ScanForMaliciousProcesses() {
    const wchar_t* blacklistedProcesses[] = {
        L"x64dbg.exe", L"x32dbg.exe", L"ollydbg.exe", L"ida.exe", L"ida64.exe",
        L"idaq.exe", L"idaq64.exe", L"idaw.exe", L"idaw64.exe", L"dbg.exe",
        L"windbg.exe", L"reshacker.exe", L"importrec.exe", L"dumper.exe",
        L"protection_id.exe", L"scylla.exe", L"cheatengine-x86_64.exe",
        L"cheatengine.exe", L"cheat engine.exe", L"processhacker.exe",
        L"procexp.exe", L"procexp64.exe", L"processhacker2.exe",
        L"process hacker.exe", L"process hacker 2.exe", L"pestudio.exe",
        L"pestudio64.exe", L"debugger.exe", L"crack.exe", L"keygen.exe",
        L"dumper.exe", L"dnspy.exe", L"de4dot.exe", L"ilspy.exe",
        L"fiddler.exe", L"wireshark.exe", L"ghidra.exe", L"hxd.exe",
        L"cffexplorer.exe", L"lordpe.exe", L"petools.exe", L"megadumper.exe",
        L"frida-server.exe", L"frida-helper-32.exe", L"frida-helper-64.exe"
    };

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32 = { 0 };
        pe32.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(hSnapshot, &pe32)) {
            do {
                for (int i = 0; i < sizeof(blacklistedProcesses) / sizeof(blacklistedProcesses[0]); i++) {
                    if (_wcsicmp(pe32.szExeFile, blacklistedProcesses[i]) == 0) {
                        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                        if (hProcess) {
                            TerminateProcess(hProcess, 0);
                            CloseHandle(hProcess);
                        }
                        g_integrityCheckFailed = true;
                        break;
                    }
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
}

void DetectAndTerminateOnWindow() {
    const wchar_t* blacklistedWindows[] = {
        L"x64dbg", L"x32dbg", L"OllyDbg", L"IDA", L"IDA Pro", L"Immunity Debugger",
        L"WinDbg", L"Process Hacker", L"Cheat Engine", L"Ghidra", L"Binary Ninja",
        L"Hex Workshop", L"HxD", L"PE Explorer", L"CFF Explorer", L"Resource Hacker",
        L"Process Explorer", L"TCPView", L"Wireshark", L"API Monitor", L"RegMon",
        L"FileMon", L"procmon", L"Process Monitor", L"pestudio", L"PEiD",
        L"Detection", L"Debug", L"Debugger", L"Disassembler", L"Fiddler",
        L"Http Debugger", L"HTTP Analyzer", L"Crack", L"KeyGen", L"Serial",
        L"dnSpy", L"ILSpy", L"JustDecompile", L"dotPeek"
    };

    std::vector<std::wstring> foundWindows;

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        if (!IsWindowVisible(hwnd)) {
            return TRUE;
        }

        wchar_t windowTitle[256];
        GetWindowTextW(hwnd, windowTitle, sizeof(windowTitle) / sizeof(wchar_t));

        if (wcslen(windowTitle) > 0) {
            std::vector<std::wstring>* windows = reinterpret_cast<std::vector<std::wstring>*>(lParam);
            windows->push_back(windowTitle);
        }

        return TRUE;
        }, reinterpret_cast<LPARAM>(&foundWindows));

    for (const auto& window : foundWindows) {
        for (int i = 0; i < sizeof(blacklistedWindows) / sizeof(blacklistedWindows[0]); i++) {
            if (window.find(blacklistedWindows[i]) != std::wstring::npos) {
                g_integrityCheckFailed = true;
                break;
            }
        }
        if (g_integrityCheckFailed) break;
    }
}

bool DetectVirtualMachine() {
    bool isVM = false;

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\Disk\\Enum", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char buffer[1024] = { 0 };
        DWORD bufferSize = sizeof(buffer);
        DWORD type;

        if (RegQueryValueExA(hKey, "0", NULL, &type, (LPBYTE)buffer, &bufferSize) == ERROR_SUCCESS) {
            std::string value(buffer);
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);

            if (value.find("vmware") != std::string::npos ||
                value.find("vbox") != std::string::npos ||
                value.find("qemu") != std::string::npos ||
                value.find("virtual") != std::string::npos) {
                isVM = true;
            }
        }
        RegCloseKey(hKey);
    }

    const wchar_t* vmProcesses[] = {
        L"vmtoolsd.exe", L"vmusrvc.exe", L"vboxtray.exe", L"vboxservice.exe",
        L"vmwaretray.exe", L"vmwareuser.exe", L"VGAuthService.exe"
    };

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32 = { 0 };
        pe32.dwSize = sizeof(PROCESSENTRY32W);

        if (Process32FirstW(hSnap, &pe32)) {
            do {
                for (int i = 0; i < sizeof(vmProcesses) / sizeof(vmProcesses[0]); i++) {
                    if (_wcsicmp(pe32.szExeFile, vmProcesses[i]) == 0) {
                        isVM = true;
                        break;
                    }
                }
                if (isVM) break;
            } while (Process32NextW(hSnap, &pe32));
        }
        CloseHandle(hSnap);
    }

    return isVM;
}

bool DetectAPIHooks() {
    bool hooksDetected = false;

    struct FunctionToCheck {
        const char* dllName;
        const char* funcName;
    };

    const FunctionToCheck importantFunctions[] = {
        {"kernel32.dll", "CreateFileW"},
        {"kernel32.dll", "ReadFile"},
        {"kernel32.dll", "WriteFile"},
        {"kernel32.dll", "LoadLibraryA"},
        {"kernel32.dll", "LoadLibraryW"},
        {"kernel32.dll", "GetProcAddress"},
        {"kernel32.dll", "VirtualProtect"},
        {"kernel32.dll", "VirtualAlloc"},
        {"ntdll.dll", "NtOpenFile"},
        {"ntdll.dll", "NtCreateFile"},
        {"ntdll.dll", "NtReadFile"},
        {"ntdll.dll", "NtWriteFile"},
        {"ntdll.dll", "NtProtectVirtualMemory"},
        {"ntdll.dll", "NtMapViewOfSection"}
    };

    for (int i = 0; i < sizeof(importantFunctions) / sizeof(importantFunctions[0]); i++) {
        const FunctionToCheck& func = importantFunctions[i];

        HMODULE hModule = GetModuleHandleA(func.dllName);
        if (hModule) {
            FARPROC procAddr = GetProcAddress(hModule, func.funcName);
            if (procAddr) {
                unsigned char* funcBytes = (unsigned char*)procAddr;
                if (funcBytes[0] == 0xE9 || funcBytes[0] == 0xE8) {
                    hooksDetected = true;
                    break;
                }
                if (funcBytes[0] == 0xFF && (funcBytes[1] == 0x25 || funcBytes[1] == 0x15)) {
                    hooksDetected = true;
                    break;
                }
            }
        }
    }

    return hooksDetected;
}

void PolymorphicCodeSection(void* codeSection, size_t size) {
    DWORD oldProtect;
    if (VirtualProtect(codeSection, size, PAGE_EXECUTE_READWRITE, &oldProtect)) {
        VirtualProtect(codeSection, size, oldProtect, &oldProtect);
        FlushInstructionCache(GetCurrentProcess(), codeSection, size);
    }
}

void __declspec(noinline) ObfuscatedFunction() {
    __try {
        volatile int eax = 0;
        volatile int ebx = 0;

        eax = 0;
        if (eax == 0) {
        }
        else {
            ebx = 0x12345678;
            ebx += 0x87654321;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
    }
}

bool DetectTimingAttacks() {
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);

    QueryPerformanceCounter(&start);
    for (volatile int i = 0; i < 1000; i++) {
        int x = i * i + i / 2;
    }
    QueryPerformanceCounter(&end);

    double elapsed = (double)(end.QuadPart - start.QuadPart) * 1000000.0 / freq.QuadPart;

    static double baselineTime = 0;
    if (baselineTime == 0) {
        baselineTime = elapsed;
        return false;
    }

    return (elapsed > baselineTime * 10);
}

LONG WINAPI CustomExceptionHandler(EXCEPTION_POINTERS* ExceptionInfo) {
    if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT ||
        ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {
        g_integrityCheckFailed = true;
        ExitProcess(0);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void SetupSelfHealing() {
    AddVectoredExceptionHandler(1, CustomExceptionHandler);

    InitializeCriticalSection(&g_criticalSection);

    void* functionAddresses[] = {
        (void*)&AdvancedAntiDebug,
        (void*)&ScanForMaliciousProcesses,
        (void*)&DetectAndTerminateOnWindow,
        (void*)&DetectVirtualMachine,
        (void*)&DetectAPIHooks,
        (void*)&PolymorphicCodeSection,
        (void*)&ObfuscatedFunction,
        (void*)&DetectTimingAttacks,
        (void*)&SetupSelfHealing,
        (void*)&InitializeAntiTamper
    };

    for (int i = 0; i < sizeof(functionAddresses) / sizeof(functionAddresses[0]); i++) {
        MEMORY_BASIC_INFORMATION mbi;
        VirtualQuery(functionAddresses[i], &mbi, sizeof(mbi));
        AddProtectedRegion(functionAddresses[i], 1024);
    }

    g_watchdogThread = CreateThread(NULL, 0, MemoryWatchdogThread, NULL, 0, NULL);
    if (g_watchdogThread) {
        SetThreadPriority(g_watchdogThread, THREAD_PRIORITY_TIME_CRITICAL);
    }
}

bool PerformEnvironmentChecks() {
    char username[256] = { 0 };
    DWORD usernameSize = sizeof(username);
    GetUserNameA(username, &usernameSize);

    char computerName[256] = { 0 };
    DWORD computerNameSize = sizeof(computerName);
    GetComputerNameA(computerName, &computerNameSize);

    const char* suspiciousUsernames[] = {
        "admin", "sandbox", "malware", "virus", "analyze", "test", "lab"
    };

    const char* suspiciousComputerNames[] = {
        "sandbox", "virus", "mallab", "test", "vm-", "virtualbox", "vmware"
    };

    for (int i = 0; i < sizeof(suspiciousUsernames) / sizeof(suspiciousUsernames[0]); i++) {
        if (StrStrIA(username, suspiciousUsernames[i])) {
            return true;
        }
    }

    for (int i = 0; i < sizeof(suspiciousComputerNames) / sizeof(suspiciousComputerNames[0]); i++) {
        if (StrStrIA(computerName, suspiciousComputerNames[i])) {
            return true;
        }
    }

    for (int i = 0; i < 3; i++) {
        const char* envVarNames[] = { "COMPUTERNAME", "USERNAME", "PROCESSOR_IDENTIFIER" };
        char envValue[256] = { 0 };
        DWORD valueSize = GetEnvironmentVariableA(envVarNames[i], envValue, sizeof(envValue));

        if (valueSize > 0) {
            for (int j = 0; j < sizeof(suspiciousComputerNames) / sizeof(suspiciousComputerNames[0]); j++) {
                if (StrStrIA(envValue, suspiciousComputerNames[j])) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool DetectSandbox() {
    const wchar_t* suspiciousFiles[] = {
        L"C:\\WINDOWS\\system32\\drivers\\VBoxMouse.sys",
        L"C:\\WINDOWS\\system32\\drivers\\VBoxGuest.sys",
        L"C:\\WINDOWS\\system32\\drivers\\VBoxSF.sys",
        L"C:\\WINDOWS\\system32\\drivers\\VBoxVideo.sys",
        L"C:\\WINDOWS\\system32\\vboxdisp.dll",
        L"C:\\WINDOWS\\system32\\vboxhook.dll",
        L"C:\\WINDOWS\\system32\\vboxmrxnp.dll",
        L"C:\\WINDOWS\\system32\\vboxogl.dll",
        L"C:\\WINDOWS\\system32\\vboxoglarrayspu.dll",
        L"C:\\WINDOWS\\system32\\vboxservice.exe",
        L"C:\\WINDOWS\\system32\\vboxtray.exe",
        L"C:\\WINDOWS\\system32\\drivers\\vmhgfs.sys",
        L"C:\\WINDOWS\\system32\\drivers\\vmmouse.sys",
        L"C:\\WINDOWS\\system32\\drivers\\vmci.sys",
        L"C:\\Program Files\\VMware\\VMware Tools\\"
    };

    for (int i = 0; i < sizeof(suspiciousFiles) / sizeof(suspiciousFiles[0]); i++) {
        if (GetFileAttributesW(suspiciousFiles[i]) != INVALID_FILE_ATTRIBUTES) {
            return true;
        }
    }

    return false;
}

void EnhancedAntiDump() {
    DWORD oldProtect;
    HMODULE hModule = GetModuleHandleA(NULL);

    if (VirtualProtect(hModule, 4096, PAGE_READWRITE, &oldProtect)) {
        PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)hModule;
        PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((BYTE*)hModule + dosHeader->e_lfanew);

        DWORD entryPoint = ntHeaders->OptionalHeader.AddressOfEntryPoint;
        DWORD imageBase = ntHeaders->OptionalHeader.ImageBase;

        for (int i = sizeof(IMAGE_DOS_HEADER) / 2; i < dosHeader->e_lfanew; i++) {
            ((BYTE*)hModule)[i] ^= 0xFF;
        }

        ntHeaders->OptionalHeader.AddressOfEntryPoint = entryPoint;
        ntHeaders->OptionalHeader.ImageBase = imageBase;

        VirtualProtect(hModule, 4096, oldProtect, &oldProtect);
    }
}

extern "C" __declspec(dllexport) void InitializeAntiTamper() {
    srand((unsigned int)time(NULL) ^ GetCurrentProcessId());
    g_checkInterval = 400 + (rand() % 200);

    SetupSelfHealing();

    if (PerformEnvironmentChecks() || DetectSandbox() || DetectVirtualMachine()) {
        g_checkInterval = 100;
    }

    if (IsDebuggerPresent() || AdvancedAntiDebug() || DetectTimingAttacks()) {
        g_integrityCheckFailed = true;
        switch (rand() % 4) {
        case 0: ExitProcess(0); break;
        case 1: TerminateProcess(GetCurrentProcess(), 0xBAADF00D); break;
        case 2: *(volatile int*)1 = 0; break;
        case 3:
            DWORD oldProtect;
            void* entryPoint = (void*)GetEntryPoint;
            if (VirtualProtect(entryPoint, 32, PAGE_READWRITE, &oldProtect)) {
               memset(entryPoint, 0xC3, 32);
                VirtualProtect(entryPoint, 32, oldProtect, &oldProtect);
            }
            break;
        }
    }

    ScanForMaliciousProcesses();
    DetectAndTerminateOnWindow();

    if (DetectAPIHooks()) {
        
        g_integrityCheckFailed = true;
    }

    //EnhancedAntiDump();

    CreateThread(NULL, 0, [](LPVOID) -> DWORD {
        while (!g_integrityCheckFailed) {
            Sleep(g_checkInterval * 2);

            if (IsDebuggerPresent() || AdvancedAntiDebug() || DetectTimingAttacks()) {
                g_integrityCheckFailed = true;
            }

            ScanForMaliciousProcesses();
            DetectAndTerminateOnWindow();

            if (g_integrityCheckFailed) {
                Sleep(rand() % 5000);

                ExitProcess(rand());
            }
        }
        return 0;
        }, NULL, 0, NULL);

    void* functionAddresses[] = {
        (void*)&AdvancedAntiDebug,
        (void*)&DetectTimingAttacks,
        (void*)&DetectAPIHooks
    };

    for (int i = 0; i < sizeof(functionAddresses) / sizeof(functionAddresses[0]); i++) {
        MEMORY_BASIC_INFORMATION mbi;
        VirtualQuery(functionAddresses[i], &mbi, sizeof(mbi));
        PolymorphicCodeSection(functionAddresses[i], 512);
    }

    ObfuscatedFunction();

}

extern "C" DWORD GetEntryPoint() {
    return (DWORD)(ULONG_PTR)GetModuleHandleA(NULL);
}