// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <string>
#include <regex>
#include <map>
#include <tuple>
#include <array>
#include <detours.h>
using namespace std;

//typedef DWORD(WINAPI * fn_fls_alloc)(_In_opt_ PFLS_CALLBACK_FUNCTION lpCallback);
//typedef PVOID (WINAPI * fn_fls_get_value)(_In_ DWORD dwFlsIndex);
//typedef BOOL (WINAPI * fn_fls_set_value)(_In_ DWORD dwFlsIndex, _In_opt_ PVOID lpFlsData);
//typedef BOOL(WINAPI * fn_fls_free)(_In_ DWORD dwFlsIndex);

using fn_fls_alloc = DWORD(WINAPI*)(_In_opt_ PFLS_CALLBACK_FUNCTION lpCallback);
using fn_fls_get_value = PVOID(WINAPI*)(_In_ DWORD dwFlsIndex);
using fn_fls_set_value = BOOL(WINAPI*)(_In_ DWORD dwFlsIndex, _In_opt_ PVOID lpFlsData);
using fn_fls_free = BOOL(WINAPI*)(_In_ DWORD dwFlsIndex);


static fn_fls_alloc base_FlsAlloc = (fn_fls_alloc)GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsAlloc");
static fn_fls_get_value base_FlsGetValue = (fn_fls_get_value)GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsGetValue");
static fn_fls_set_value base_FlsSetValue = (fn_fls_set_value)GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsSetValue");
static fn_fls_free base_FlsFree = (fn_fls_free)GetProcAddress(GetModuleHandleW(L"api-ms-win-core-fibers-l1-1-0.dll"), "FlsFree");

using memory_ptr_t = void*;
using fiber_t = size_t;
using callback_fn_t = PFLS_CALLBACK_FUNCTION;
std::map<uint32_t, tuple<callback_fn_t, memory_ptr_t>> fiberMem;
uint32_t _lastUsedFlsIndex = 0;

//std::map<tuple<fiber_t, uint32_t>, tuple<callback_fn_t, memory_ptr_t>> fiberMem;
//std::map<tuple<fiber_t, uint32_t>, tuple<callback_fn_t, memory_ptr_t>>* fiberMem;

struct fiber_slot_t
{
    callback_fn_t callback;
    memory_ptr_t memory;
};
std::array<fiber_slot_t, 4096> _fiberSlots;
uint32_t _lastUsedSlot = 0x80;

#define ALWAYS_OVERRIDE_FLS
// #define BLOG

#ifdef BLOG
char buffer[64 * 1024];
char* bindex = buffer;
#endif

DWORD WINAPI override_FlsAlloc(_In_opt_ PFLS_CALLBACK_FUNCTION lpCallback)
{
#ifdef BLOG
    strcat(bindex, __FUNCTION__"\n");
    bindex += strlen(bindex);
#endif
#ifndef ALWAYS_OVERRIDE_FLS
    const auto fix = base_FlsAlloc(lpCallback);

    if (fix == FLS_OUT_OF_INDEXES)
#endif
    {
        //fiberMem.insert({ {reinterpret_cast<size_t>(::GetCurrentFiber()), fiberMem.size()}, {lpCallback, nullptr} });
        //std::get<0>(fiberMem.at(_lastUsedFlsIndex)) = lpCallback;
        //_lastUsedFlsIndex++;
#ifdef BLOG
        strcat(bindex, __FUNCTION__"fibMem++\n");
        bindex += strlen(bindex);
#endif
        //return static_cast<uint32_t>(fiberMem.size()) << 7;
        //return _lastUsedFlsIndex - 1;

        auto& slot = _fiberSlots[_lastUsedSlot];
        slot.callback = lpCallback;
        return ++_lastUsedSlot - 1;
    }

#ifndef ALWAYS_OVERRIDE_FLS
    return fix;
#endif
}

PVOID WINAPI override_FlsGetValue(_In_ DWORD dwFlsIndex)
{
#ifdef BLOG
    strcat(bindex, __FUNCTION__);
    bindex += strlen(bindex);
    _itoa(dwFlsIndex, bindex, 10);
    bindex += strlen(bindex);
    strcat(bindex, "\n");
    bindex += strlen(bindex);
#endif
    if (dwFlsIndex < 0x80)
        return base_FlsGetValue(dwFlsIndex);
#ifndef ALWAYS_OVERRIDE_FLS
    if (dwFlsIndex > 0x80)
#endif
    {
        //return std::get<1>(fiberMem.at({reinterpret_cast<size_t>(::GetCurrentFiber()), (dwFlsIndex >> 7) - 1}));
        //return std::get<1>(fiberMem.at(dwFlsIndex));
        return _fiberSlots[dwFlsIndex].memory;
    }

#ifndef ALWAYS_OVERRIDE_FLS
    return base_FlsGetValue(dwFlsIndex);
#endif
}

BOOL WINAPI override_FlsSetValue(_In_ DWORD dwFlsIndex, _In_opt_ PVOID lpFlsData)
{
#ifdef BLOG
    strcat(bindex, __FUNCTION__);
    bindex += strlen(bindex);
    _itoa(dwFlsIndex, bindex, 10);
    bindex += strlen(bindex);
    strcat(bindex, "\n");
    bindex += strlen(bindex);
#endif
    if (dwFlsIndex < 0x80)
        return base_FlsSetValue(dwFlsIndex, lpFlsData);
#ifndef ALWAYS_OVERRIDE_FLS
    if (dwFlsIndex > 0x80)
#endif
    {
        //std::get<1>(fiberMem.at({reinterpret_cast<size_t>(::GetCurrentFiber()), (dwFlsIndex >> 7) - 1})) = lpFlsData;
        //std::get<1>(fiberMem.at(dwFlsIndex)) = lpFlsData;
        _fiberSlots[dwFlsIndex].memory = lpFlsData;
        return TRUE;
    }

#ifndef ALWAYS_OVERRIDE_FLS
    return base_FlsSetValue(dwFlsIndex, lpFlsData);
#endif
}

BOOL WINAPI override_FlsFree(_In_ DWORD dwFlsIndex)
{
#ifdef BLOG
    strcat(bindex, __FUNCTION__);
    bindex += strlen(bindex);
    _itoa(dwFlsIndex, bindex, 10);
    bindex += strlen(bindex);
    strcat(bindex, "\n");
    bindex += strlen(bindex);
#endif
    if (dwFlsIndex < 0x80)
        return base_FlsFree(dwFlsIndex);

#ifndef ALWAYS_OVERRIDE_FLS
    if (dwFlsIndex > 0x80)
#endif
    {
        //const std::remove_pointer_t<decltype(fiberMem)>::key_type index = {reinterpret_cast<size_t>(::GetCurrentFiber()), (dwFlsIndex >> 7) - 1};
        //const auto index = dwFlsIndex;
        //if ( auto callback = std::get<0>(fiberMem.at(index)); callback != nullptr )
        //{
        //    //callback( nullptr ); todo, what is that parameter?
        //}
        //fiberMem.erase(index);

        auto& slot = _fiberSlots[dwFlsIndex];
        if (auto callback = slot.callback; callback != nullptr)
        {
            //callback( nullptr ); todo, what is that parameter?
        }
        memset(&slot, 0, sizeof(decltype(slot)));
        return TRUE;
    }

#ifndef ALWAYS_OVERRIDE_FLS
    return base_FlsFree(dwFlsIndex);
#endif
}

//extern "C" __declspec(dllexport) VOID CALLBACK DetourFinishHelperProcess(
//    _In_ HWND,
//    _In_ HINSTANCE,
//    _In_ LPSTR,
//    _In_ INT
//);

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (DetourIsHelperProcess())
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

