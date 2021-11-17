// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <Windows.h>
#include <winspool.h>

#include <filesystem>
#include <regex>
#include <array>

#include <detours.h>

using namespace std;
EXTERN_C IMAGE_DOS_HEADER __ImageBase;

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
    static HMODULE avoidUnloadingThisDLL = NULL;
    if (avoidUnloadingThisDLL == NULL)
    {
        char szModuleNameRaw[MAX_PATH];
        ::GetModuleFileNameA((HINSTANCE)&__ImageBase, szModuleNameRaw, MAX_PATH);
        ::GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN, szModuleNameRaw, &avoidUnloadingThisDLL);
    }

    DATATYPES_INFO_1* pInfo1 = (DATATYPES_INFO_1*)pDatatypes;
    wchar_t** pMyDatatypes = (wchar_t**)c_DataTypes;
    size_t              cbTotal = 0;
    LPBYTE              pEnd;
    *pcReturned = 0;
    pEnd = (LPBYTE)pInfo1 + cbBuf;
    while ( *pMyDatatypes )
    {
        cbTotal += wcslen(*pMyDatatypes) * sizeof(WCHAR) + sizeof(WCHAR) +
            sizeof(DATATYPES_INFO_1);

        pMyDatatypes++;
    }
    *pcbNeeded = (DWORD)cbTotal;
    if ( cbTotal <= cbBuf )
    {
        pMyDatatypes = (wchar_t**)c_DataTypes;
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
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
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

enum class slot_allocation_e : uint32_t
{
    small_alloc = 128,
    medium_alloc = 4096,
    override_alloc = static_cast<uint32_t>(medium_alloc),
};

using memory_ptr_t = void*;
using callback_fn_t = PFLS_CALLBACK_FUNCTION;

struct fiber_slot_t
{
    callback_fn_t callback;
    memory_ptr_t memory;
};

std::array<fiber_slot_t, static_cast<size_t>(slot_allocation_e::override_alloc)> g_fiber_slots;
uint32_t g_last_index = 0;

DWORD WINAPI override_FlsAlloc(_In_opt_ PFLS_CALLBACK_FUNCTION lpCallback)
{
    if ( const auto idx = base_FlsAlloc(lpCallback); idx != FLS_OUT_OF_INDEXES )
    {
        return idx;
    }

    uint32_t i;
    if ( g_last_index < g_fiber_slots.size() )
    {
        // fast path
        i = g_last_index++;
    }
    else
    {
        // slow path
        for ( i = 0; i < g_fiber_slots.size(); ++i )
        {
            if ( g_fiber_slots[i].callback == nullptr )
            {
                break;
            }
        }
    }

    if ( i == g_fiber_slots.size() )
    {
        SetLastError( STATUS_NO_MEMORY );
        return FLS_OUT_OF_INDEXES;
    }

    g_fiber_slots[i].callback = lpCallback != nullptr ? lpCallback : reinterpret_cast<decltype(fiber_slot_t::callback)>(~static_cast<size_t>(0));
    return static_cast<uint32_t>(slot_allocation_e::small_alloc) + i;
}

BOOL WINAPI override_FlsFree(_In_ DWORD dwFlsIndex)
{
    if ( base_FlsFree(dwFlsIndex) && ::GetLastError() != STATUS_INVALID_PARAMETER )
    {
        return TRUE;
    }

    dwFlsIndex -= static_cast<uint32_t>(slot_allocation_e::small_alloc);
    if ( dwFlsIndex < g_fiber_slots.size() )
    {
        if ( auto& slot = g_fiber_slots[dwFlsIndex]; 
            slot.callback != reinterpret_cast<decltype(fiber_slot_t::callback)>(~static_cast<size_t>(0)) && slot.callback != nullptr )
        {
            slot.callback( slot.memory );
            slot.callback = nullptr;
            slot.memory = nullptr;
            return TRUE;
        }
    }
    SetLastError( STATUS_INVALID_PARAMETER );
    return FALSE;
}

PVOID WINAPI override_FlsGetValue(_In_ DWORD dwFlsIndex)
{
    if ( dwFlsIndex < static_cast<uint32_t>(slot_allocation_e::small_alloc) )
    {
        return base_FlsGetValue(dwFlsIndex);
    }
    dwFlsIndex -= static_cast<uint32_t>(slot_allocation_e::small_alloc);
    if ( dwFlsIndex < g_fiber_slots.size() )
    {
        return g_fiber_slots[dwFlsIndex].memory;
    }

    SetLastError( STATUS_INVALID_PARAMETER );
    return nullptr;
}

BOOL WINAPI override_FlsSetValue(_In_ DWORD dwFlsIndex, _In_opt_ PVOID lpFlsData)
{
    if ( dwFlsIndex < static_cast<uint32_t>(slot_allocation_e::small_alloc) )
    {
        return base_FlsSetValue(dwFlsIndex, lpFlsData);
    }
    dwFlsIndex -= static_cast<uint32_t>(slot_allocation_e::small_alloc);
    if ( dwFlsIndex < g_fiber_slots.size() )
    {
        g_fiber_slots[dwFlsIndex].memory = lpFlsData;
        return TRUE;
    }
    SetLastError( STATUS_INVALID_PARAMETER );
    return FALSE;
}

slot_allocation_e determine_system_fls_slot_alloc_max()
{
    std::vector<uint32_t> slots;
    slots.reserve( static_cast<size_t>(slot_allocation_e::small_alloc) + 1 );
    while ( slots.emplace_back( ::FlsAlloc( nullptr ) ) != FLS_OUT_OF_INDEXES && slots.size() <= static_cast<size_t>(slot_allocation_e::small_alloc) )
    {
    }
    for ( const auto& idx : slots )
    {
        ::FlsFree( idx );
    }
    return slots.size() >= static_cast<size_t>(slot_allocation_e::small_alloc) ? slot_allocation_e::medium_alloc : slot_allocation_e::small_alloc;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    const auto slot_alloc = determine_system_fls_slot_alloc_max();
    if ( slot_alloc == slot_allocation_e::medium_alloc )
    {
        ::OutputDebugStringW( L"TPSpoolFlsHook: Omitting Fls* hooks, because OS supports enough slots already\n" );
        return TRUE;
    }
    ::OutputDebugStringW( L"TPSpoolFlsHook: Registering Fls* hooks, because OS does not support enough slots\n" );

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
        DetourDetach(&(PVOID&)base_FlsAlloc, override_FlsAlloc);
        DetourDetach(&(PVOID&)base_FlsGetValue, override_FlsGetValue);
        DetourDetach(&(PVOID&)base_FlsSetValue, override_FlsSetValue);
        DetourDetach(&(PVOID&)base_FlsFree, override_FlsFree);

        auto error = DetourTransactionCommit();
        break;
    }
    return TRUE;
}

