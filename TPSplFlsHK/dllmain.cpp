// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <Windows.h>
#include <winspool.h>

#include <filesystem>
#include <regex>
#include <array>

#include <detours.h>

using namespace std;

constexpr const wchar_t* c_DataTypes[] = {
    L"RAW",
    L"RAW [FF appended]",
    L"RAW [FF auto]",
    L"NT EMF 1.003",
    L"TEXT",
    L"NT EMF 1.006",
    L"NT EMF 1.007",
    L"NT EMF 1.008",
    L"TP EMZ 1.0",
    L"TP EMF 1.0",
    L"TP EMZ 2.0",
    L"TP XPS 1.0",
    nullptr
};

#pragma comment(linker, "/export:EnumPrintProcessorDatatypesW=EnumPrintProcessorDatatypesW")
BOOL CALLBACK
EnumPrintProcessorDatatypesW(LPWSTR  /*pName*/,
    LPWSTR  /*pPrintProcessorName*/,
    DWORD   /*Level*/,
    LPBYTE  pDatatypes,
    DWORD   cbBuf,
    LPDWORD pcbNeeded,
    LPDWORD pcReturned)
{
    DATATYPES_INFO_1* pInfo1 = (DATATYPES_INFO_1*)pDatatypes;
    wchar_t** pMyDatatypes = (wchar_t**)c_DataTypes;
    size_t              cbTotal = 0;
    LPBYTE              pEnd;


    // Star assuming failed / no entries returned

    *pcReturned = 0;

    // Pick up pointer to end of buffer given

    pEnd = (LPBYTE)pInfo1 + cbBuf;

    // Add up the minimum buffer required

    while ( *pMyDatatypes )
    {
        cbTotal += wcslen(*pMyDatatypes) * sizeof(WCHAR) + sizeof(WCHAR) +
            sizeof(DATATYPES_INFO_1);

        pMyDatatypes++;
    }

    // Set the buffer length returned/required

    *pcbNeeded = (DWORD)cbTotal;

    // Fill in the array only if there is sufficient space to

    if ( cbTotal <= cbBuf )
    {
        // Pick up our list of supported data types
        pMyDatatypes = (wchar_t**)c_DataTypes;

        /**
            Fill in the given buffer.  We put the data names at the end of
            the buffer, working towards the front.  The structures are put
            at the front, working towards the end.
        **/

        while ( *pMyDatatypes )
        {
            pEnd -= wcslen(*pMyDatatypes) * sizeof(WCHAR) + sizeof(WCHAR);
            wcscpy((LPWSTR)pEnd, *pMyDatatypes);
            pInfo1->pName = (LPWSTR)pEnd;
            pInfo1++;
            (*pcReturned)++;

            pMyDatatypes++;
        }
    }
    else
    {
        // Caller didn't have large enough buffer, set error and return
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }

    // Return success

    return TRUE;
}


using fn_fls_alloc = DWORD(WINAPI*)(_In_opt_ PFLS_CALLBACK_FUNCTION lpCallback);
using fn_fls_free = BOOL(WINAPI*)(_In_ DWORD dwFlsIndex);
using fn_fls_get_value = PVOID(WINAPI*)(_In_ DWORD dwFlsIndex);
using fn_fls_set_value = BOOL(WINAPI*)(_In_ DWORD dwFlsIndex, _In_opt_ PVOID lpFlsData);

static fn_fls_alloc base_FlsAlloc = (fn_fls_alloc)GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsAlloc");
static fn_fls_free base_FlsFree = (fn_fls_free)GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsFree");
static fn_fls_get_value base_FlsGetValue = (fn_fls_get_value)GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsGetValue");
static fn_fls_set_value base_FlsSetValue = (fn_fls_set_value)GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsSetValue");

//struct conversion_t
//{
//    void* from;
//    void* to;
//};
//std::array c_Conversions = {
//    conversion_t{reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsAlloc")), nullptr},
//    conversion_t{reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsFree")), nullptr},
//    conversion_t{reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsGetValue")), nullptr},
//    conversion_t{reinterpret_cast<void*>(GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsSetValue")), nullptr},
//};

using memory_ptr_t = void*;
using callback_fn_t = PFLS_CALLBACK_FUNCTION;

struct fiber_slot_t
{
    callback_fn_t callback;
    memory_ptr_t memory;
};

std::array<fiber_slot_t, 4096> g_fiber_slots;
uint32_t g_biggest_known_slot = 0x7f;

DWORD WINAPI override_FlsAlloc(_In_opt_ PFLS_CALLBACK_FUNCTION lpCallback)
{
    if ( const auto fix = base_FlsAlloc(lpCallback); fix != FLS_OUT_OF_INDEXES )
    {
        g_biggest_known_slot = max( g_biggest_known_slot, fix );
        return fix;
    }

    uint32_t i;
    for ( i = 0; i < g_fiber_slots.size(); ++i )
    {
        if ( g_fiber_slots[i].callback == nullptr )
        {
            break;
        }
    }

    if ( i == g_fiber_slots.size() )
    {
        return STATUS_NO_MEMORY;
    }

    auto& slot = g_fiber_slots[i];
    slot.callback = lpCallback ? lpCallback : reinterpret_cast<decltype(slot.callback)>(~static_cast<size_t>(0));
    return (g_biggest_known_slot + 1) + i;
}

BOOL WINAPI override_FlsFree(_In_ DWORD dwFlsIndex)
{
    if ( dwFlsIndex <= g_biggest_known_slot )
    {
        return base_FlsFree(dwFlsIndex);
    }
    if ( (dwFlsIndex - g_biggest_known_slot) < g_fiber_slots.size() )
    {
        if ( auto& slot = g_fiber_slots[dwFlsIndex - g_biggest_known_slot]; 
            slot.callback != reinterpret_cast<decltype(fiber_slot_t::callback)>(~static_cast<size_t>(0)) && slot.callback != nullptr )
        {
            slot.callback( slot.memory );
            memset( &slot, 0, sizeof(decltype(slot)) );
            return TRUE;
        }
    }
    SetLastError( STATUS_INVALID_PARAMETER );
    return FALSE;
}

PVOID WINAPI override_FlsGetValue(_In_ DWORD dwFlsIndex)
{
    if ( dwFlsIndex <= g_biggest_known_slot )
    {
        return base_FlsGetValue(dwFlsIndex);
    }
    if ( (dwFlsIndex - g_biggest_known_slot) < g_fiber_slots.size() )
    {
        return g_fiber_slots[dwFlsIndex - g_biggest_known_slot].memory;
    }

    SetLastError( STATUS_INVALID_PARAMETER );
    return nullptr;
}

BOOL WINAPI override_FlsSetValue(_In_ DWORD dwFlsIndex, _In_opt_ PVOID lpFlsData)
{
    if ( dwFlsIndex <= g_biggest_known_slot )
    {
        return base_FlsSetValue(dwFlsIndex, lpFlsData);
    }
    if ( (dwFlsIndex - g_biggest_known_slot) < g_fiber_slots.size() )
    {
        g_fiber_slots[dwFlsIndex - g_biggest_known_slot].memory = lpFlsData;
        return TRUE;
    }
    SetLastError( STATUS_INVALID_PARAMETER );
    return FALSE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch ( ul_reason_for_call )
    {
    case DLL_PROCESS_ATTACH:
        if ( DetourIsHelperProcess() )
        {
            return TRUE;
        }
        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        DetourAttach(&(PVOID&)base_FlsAlloc, override_FlsAlloc);
        DetourAttach(&(PVOID&)base_FlsGetValue, override_FlsGetValue);
        DetourAttach(&(PVOID&)base_FlsSetValue, override_FlsSetValue);
        DetourAttach(&(PVOID&)base_FlsFree, override_FlsFree);

        DetourTransactionCommit();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        //DetourDetach(&(PVOID&)Real_Echo, Mine_Echo);
        DetourDetach(&(PVOID&)base_FlsAlloc, override_FlsAlloc);
        DetourDetach(&(PVOID&)base_FlsGetValue, override_FlsGetValue);
        DetourDetach(&(PVOID&)base_FlsSetValue, override_FlsSetValue);
        DetourDetach(&(PVOID&)base_FlsFree, override_FlsFree);

        auto error = DetourTransactionCommit();
        break;
    }
    return TRUE;
}

