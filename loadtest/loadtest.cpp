// loadtest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS
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

#include "..\detours\src\detours.h"
#include "loadtest.h"

using namespace std;

BOOL
(*EnumPrintProcessorDatatypesW)(
    LPWSTR   pName,
    LPWSTR   pPrintProcessorName,
    DWORD     Level,
    LPBYTE    pDatatypes,
    DWORD     cbBuf,
    LPDWORD   pcbNeeded,
    LPDWORD   pcReturned
    );
typedef struct _DATATYPES_INFO_1W
{
    LPWSTR    pName;
} DATATYPES_INFO_1W, * PDATATYPES_INFO_1W, * LPDATATYPES_INFO_1W;

const wchar_t* tpwinprn = LR"(C:\windows\system32\spool\PRTPROCS\x64\TPWinPrn.dll)";
const wchar_t* pps[] = {
    tpwinprn,
    LR"(C:\Program Files\Seagull\Printer Drivers\Common\Seagull_XPMLServer.dll)",
    LR"(C:\windows\System32\ACTIVEDS.dll)",
    LR"(C:\windows\System32\adsldpc.dll)",
    LR"(C:\windows\System32\advapi32.dll)",
    LR"(C:\windows\System32\AltecLM.dll)",
    LR"(C:\windows\System32\APMon.dll)",
    LR"(C:\windows\System32\bcrypt.dll)",
    LR"(C:\windows\System32\bcryptPrimitives.dll)",
    LR"(C:\windows\System32\bsq16aL6.DLL)",
    LR"(C:\windows\System32\cabinet.dll)",
    LR"(C:\windows\System32\cfgmgr32.dll)",
    LR"(C:\windows\System32\clbcatq.dll)",
    LR"(C:\windows\System32\clusapi.dll)",
    LR"(C:\windows\System32\CNAP3SMD.DLL)",
    LR"(C:\windows\System32\CNAS0MOK.DLL)",
    LR"(C:\windows\System32\CNAS0MPK.DLL)",
    LR"(C:\windows\System32\CNMLMDF.DLL)",
    LR"(C:\windows\System32\CNMLMDG.DLL)",
    LR"(C:\windows\System32\CNMLMEH.DLL)",
    LR"(C:\windows\System32\CNMLMEI.DLL)",
    LR"(C:\windows\System32\CNMLMFF.DLL)",
    LR"(C:\windows\System32\CNMLMFH.DLL)",
    LR"(C:\windows\System32\CNMLMFT.DLL)",
    LR"(C:\windows\System32\cnwilm64.dll)",
    LR"(C:\windows\System32\ColorAdapterClient.dll)",
    LR"(C:\windows\System32\combase.dll)",
    LR"(C:\windows\System32\CONCRT140.dll)",
    LR"(C:\windows\System32\cpprest140_2_9.dll)",
    LR"(C:\windows\System32\CRYPT32.dll)",
    LR"(C:\windows\System32\CRYPTBASE.dll)",
    LR"(C:\windows\System32\cryptsp.dll)",
    LR"(C:\windows\System32\cscapi.dll)",
    LR"(C:\windows\System32\dbgcore.DLL)",
    LR"(C:\windows\System32\dbghelp.dll)",
    LR"(C:\windows\System32\deviceassociation.dll)",
    LR"(C:\windows\System32\DEVOBJ.dll)",
    LR"(C:\windows\System32\DEVRTL.dll)",
    LR"(C:\windows\System32\dkadtpinpa.DLL)",
    LR"(C:\windows\System32\DKADTPLANG.DLL)",
    LR"(C:\windows\System32\DL6RFLAI-1.DLL)",
    LR"(C:\windows\System32\DLHLSZIL.DLL)",
    LR"(C:\windows\System32\DLXBUZIL.DLL)",
    LR"(C:\windows\System32\DLXROZIL.DLL)",
    LR"(C:\windows\System32\DNSAPI.dll)",
    LR"(C:\windows\System32\DPAPI.DLL)",
    LR"(C:\windows\System32\drvstore.dll)",
    LR"(C:\windows\System32\DSROLE.dll)",
    LR"(C:\windows\System32\DUO_128MON.DLL)",
    LR"(C:\windows\System32\DUO_450MON.DLL)",
    LR"(C:\windows\System32\DUO_D1MON.DLL)",
    LR"(C:\windows\System32\E_PLMAPW.DLL)",
    LR"(C:\windows\System32\E_PLMCAJ.DLL)",
    LR"(C:\windows\System32\E_PLMCHJ.DLL)",
    LR"(C:\windows\System32\E_PLMCKJ.DLL)",
    LR"(C:\windows\System32\E_PLMCLJ.DLL)",
    LR"(C:\windows\System32\E_PLMCMJ.DLL)",
    LR"(C:\windows\System32\E_PLMCPJ.DLL)",
    LR"(C:\windows\System32\E_PLMCQJ.DLL)",
    LR"(C:\windows\System32\E_PLMCRJ.DLL)",
    LR"(C:\windows\System32\E_PLMCTJ.DLL)",
    LR"(C:\windows\System32\E_YLMBUBE.DLL)",
    LR"(C:\windows\System32\E_YLMBWDE.DLL)",
    LR"(C:\windows\System32\EA5LMTMm30.DLL)",
    LR"(C:\windows\System32\EA5LMTMT88V.DLL)",
    LR"(C:\windows\System32\EA5LMTMT88VI.DLL)",
    LR"(C:\windows\System32\EBPMONB.DLL)",
    LR"(C:\windows\SYSTEM32\ESENT.dll)",
    LR"(C:\windows\System32\FirewallAPI.dll)",
    LR"(C:\windows\System32\fwbase.dll)",
    LR"(C:\windows\System32\fwpuclnt.dll)",
    LR"(C:\windows\System32\FX6KOLAI-1.DLL)",
    LR"(C:\windows\System32\GDI32.dll)",
    LR"(C:\windows\System32\gdi32full.dll)",
    LR"(C:\windows\SYSTEM32\gpapi.dll)",
    LR"(C:\windows\System32\HID.DLL)",
    LR"(C:\windows\System32\HP1100LM.DLL)",
    LR"(C:\windows\System32\hpinksts0C54LM.dll)",
    LR"(C:\windows\System32\hpinksts1853LM.dll)",
    LR"(C:\windows\System32\hpinksts2554LM.dll)",
    LR"(C:\windows\System32\hpinksts3454LM.dll)",
    LR"(C:\windows\System32\hpinksts612aLM.dll)",
    LR"(C:\windows\System32\hpinksts622aLM.dll)",
    LR"(C:\windows\System32\hpinksts632aLM.dll)",
    LR"(C:\windows\System32\hpinksts9311LM.dll)",
    LR"(C:\windows\System32\hpinkstsad2aLM.dll)",
    LR"(C:\windows\System32\hpinkstsbe2aLM.dll)",
    LR"(C:\windows\System32\hpinkstsbf2aLM.dll)",
    LR"(C:\windows\System32\hpinkstsc12aLM.dll)",
    LR"(C:\windows\System32\hpinkstsc42aLM.dll)",
    LR"(C:\windows\System32\hpinkstsC511LM.dll)",
    LR"(C:\windows\System32\hpinkstsc52aLM.dll)",
    LR"(C:\windows\System32\hpinkstsC611LM.dll)",
    LR"(C:\windows\System32\hpinkstsc62aLM.dll)",
    LR"(C:\windows\System32\hpinkstsCF11LM.dll)",
    LR"(C:\windows\System32\hpinkstsDC11LM.dll)",
    LR"(C:\windows\System32\hpinkstsE511LM.dll)",
    LR"(C:\windows\System32\hpinkstsE911LM.dll)",
    LR"(C:\windows\System32\hpinkstsf42aLM.dll)",
    LR"(C:\windows\System32\HPLTLM5.DLL)",
    LR"(C:\windows\System32\hpmlm225.dll)",
    LR"(C:\windows\System32\hptcpmib.dll)",
    LR"(C:\windows\System32\HpTcpMon.dll)",
    LR"(C:\windows\System32\HPTcpMUI.dll)",
    LR"(C:\windows\System32\hpz3l5mu.dll)",
    LR"(C:\windows\System32\hpz3lw71.dll)",
    LR"(C:\windows\System32\hpzjrd01.dll)",
    LR"(C:\windows\System32\HTTPAPI.dll)",
    LR"(C:\windows\System32\imagehlp.dll)",
    LR"(C:\windows\SYSTEM32\IPHLPAPI.DLL)",
    LR"(C:\windows\System32\kernel.appcore.dll)",
    LR"(C:\windows\System32\KERNEL32.DLL)",
    LR"(C:\windows\System32\KERNELBASE.dll)",
    LR"(C:\windows\System32\KMPJL64.DLL)",
    LR"(C:\windows\System32\KOAXPJ_L.DLL)",
    LR"(C:\windows\System32\KOAYJJ_L.DLL)",
    LR"(C:\windows\System32\KOAYSJ_L.DLL)",
    LR"(C:\windows\System32\KOAYTJ_L.DLL)",
    LR"(C:\windows\System32\KOAYTJAL.DLL)",
    LR"(C:\windows\System32\KOAZCJ_L.DLL)",
    LR"(C:\windows\System32\KODJOJ_L.dll)",
    LR"(C:\windows\System32\KODJVJDL.DLL)",
    LR"(C:\windows\System32\KOFXOJ1L.DLL)",
    LR"(C:\windows\System32\KOFYTJ1L.DLL)",
    LR"(C:\windows\System32\KOI951_mfp.dll)",
    LR"(C:\windows\System32\KXPLM64.DLL)",
    LR"(C:\windows\System32\ldaNLM64.dll)",
    LR"(C:\windows\System32\libcrypto-1_1-x64.dll)",
    LR"(C:\windows\System32\libssl-1_1-x64.dll)",
    LR"(C:\windows\System32\LM450MON.DLL)",
    LR"(C:\windows\System32\lmad4ninpa.DLL)",
    LR"(C:\windows\System32\LMAD4NLANG.DLL)",
    LR"(C:\windows\System32\lmad5ninpa.DLL)",
    LR"(C:\windows\System32\LMAD5NLANG.DLL)",
    LR"(C:\windows\System32\LMPC2_MON.DLL)",
    LR"(C:\windows\System32\localspl.dll)",
    LR"(C:\windows\System32\LOGONCLI.DLL)",
    LR"(C:\windows\System32\LP350MON.DLL)",
    LR"(C:\windows\System32\LW400MON.DLL)",
    LR"(C:\windows\System32\mgmtapi.dll)",
    LR"(C:\windows\System32\MPR.dll)",
    LR"(C:\windows\System32\MSASN1.dll)",
    LR"(C:\windows\System32\mscms.dll)",
    LR"(C:\windows\System32\msvcp_win.dll)",
    LR"(C:\windows\System32\msvcp110_win.dll)",
    LR"(C:\windows\System32\MSVCP140.dll)",
    LR"(C:\windows\System32\MSVCR120.dll)",
    LR"(C:\windows\System32\msvcrt.dll)",
    LR"(C:\windows\system32\mswsock.dll)",
    LR"(C:\Windows\System32\msxml6.dll)",
    LR"(C:\windows\System32\NETAPI32.dll)",
    LR"(C:\windows\System32\netutils.dll)",
    LR"(C:\windows\System32\NRBMONN.DLL)",
    LR"(C:\windows\System32\NSI.dll)",
    LR"(C:\windows\SYSTEM32\ntdll.dll)",
    LR"(C:\windows\SYSTEM32\ntmarta.dll)",
    LR"(C:\windows\System32\ntprint.dll)",
    LR"(C:\windows\System32\ole32.dll)",
    LR"(C:\windows\System32\OLEAUT32.dll)",
    LR"(C:\windows\System32\ONP8MON.DLL)",
    LR"(C:\windows\System32\policymanager.dll)",
    LR"(C:\windows\System32\powrprof.dll)",
    LR"(C:\windows\System32\PrintIsolationProxy.dll)",
    LR"(C:\windows\System32\profapi.dll)",
    LR"(C:\windows\System32\PROPSYS.dll)",
    LR"(C:\windows\System32\PSAPI.DLL)",
    LR"(C:\windows\System32\pt2500lm.dll)",
    LR"(C:\windows\System32\QL106NL.DLL)",
    LR"(C:\Windows\System32\rasadhlp.dll)",
    LR"(C:\windows\System32\rica5Llm.dll)",
    LR"(C:\windows\System32\rica67lm.dll)",
    LR"(C:\windows\System32\rica6Mlm.dll)",
    LR"(C:\windows\System32\rica6Plm.dll)",
    LR"(C:\windows\System32\rica8Blm.dll)",
    LR"(C:\windows\System32\RPCRT4.dll)",
    LR"(C:\windows\system32\rsaenh.dll)",
    LR"(C:\windows\System32\SAMCLI.DLL)",
    LR"(C:\windows\System32\SAMLIB.dll)",
    LR"(C:\windows\System32\sdb4mlm.dll)",
    LR"(C:\windows\System32\Seagull_V3_NetMonDispatcher.dll)",
    LR"(C:\windows\System32\sechost.dll)",
    LR"(C:\windows\System32\Secur32.dll)",
    LR"(C:\windows\System32\SETUPAPI.dll)",
    LR"(C:\windows\System32\sfc_os.dll)",
    LR"(C:\windows\System32\shcore.dll)",
    LR"(C:\windows\System32\SHELL32.dll)",
    LR"(C:\windows\System32\shlwapi.dll)",
    LR"(C:\windows\System32\SN0ELMON.dll)",
    LR"(C:\windows\System32\snmpapi.dll)",
    LR"(C:\windows\System32\SPFILEQ.dll)",
    LR"(C:\windows\System32\SPINF.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\CNMPDDF.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\CNMPDDG.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\CNMPDEH.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\CNMPDEI.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\CNMPDFF.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\CNMPDFH.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\CNMPDFT.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\cnwfdpSE.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\CnXP0PP.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\dellopd.ppr.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\DKADTP4C.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\HP1100PP.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\hpcpp101.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\hpcpp108.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\hpcpp117.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\hpcpp135.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\hpcpp140.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\hpcpp145.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\hpcpp240.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\hpipp194.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\hpzpp5in.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\hpzpp5mu.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\hpzppw71.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\KOAYJJ_P.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\KOAYSJ_P.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\KOAYTJ_P.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\KOAYTJAP.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\LMAD4N4C.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\LMAD5N4C.DLL)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\sdb4mpc.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\ssl1cpc.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\us015pc.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\winprint.dll)",
    LR"(C:\windows\system32\spool\PRTPROCS\x64\x5print.dll)",
    LR"(C:\windows\System32\SPOOLSS.DLL)",
    LR"(C:\windows\System32\spoolsv.exe)",
    LR"(C:\windows\System32\SR0ELMON.dll)",
    LR"(C:\windows\System32\srvcli.dll)",
    LR"(C:\windows\System32\SS0ELMON.dll)",
    LR"(C:\windows\System32\ssl1clm.dll)",
    LR"(C:\windows\System32\sspicli.dll)",
    LR"(C:\windows\System32\tcpmon.dll)",
    LR"(C:\windows\System32\thinmon.dll)",
    LR"(C:\windows\System32\TPCCMon.dll)",
    LR"(C:\windows\System32\TPSW32.dll)",
    LR"(C:\windows\SYSTEM32\ualapi.dll)",
    LR"(C:\windows\System32\ucrtbase.dll)",
    LR"(C:\windows\System32\us015lm.dll)",
    LR"(C:\windows\System32\usbmon.dll)",
    LR"(C:\windows\System32\USER32.dll)",
    LR"(C:\windows\System32\USERENV.dll)",
    LR"(C:\windows\System32\VCRUNTIME140.dll)",
    LR"(C:\windows\System32\VERSION.dll)",
    LR"(C:\windows\System32\webservices.dll)",
    LR"(C:\windows\System32\win32spl.dll)",
    LR"(C:\windows\System32\win32u.dll)",
    LR"(C:\windows\System32\windows.storage.dll)",
    LR"(C:\windows\System32\WINHTTP.dll)",
    LR"(C:\windows\System32\WININET.dll)",
    LR"(C:\windows\SYSTEM32\WINNSI.DLL)",
    LR"(C:\windows\system32\winspool.drv)",
    LR"(C:\windows\System32\WINSTA.dll)",
    LR"(C:\windows\System32\WINTRUST.dll)",
    LR"(C:\windows\System32\wkscli.dll)",
    LR"(C:\windows\System32\WLDAP32.dll)",
    LR"(C:\windows\System32\WS2_32.dll)",
    LR"(C:\windows\System32\wsdapi.dll)",
    LR"(C:\windows\System32\wsnmp32.dll)",
    LR"(C:\windows\System32\WSOCK32.dll)",
    LR"(C:\windows\System32\WTSAPI32.dll)",
    LR"(C:\windows\System32\x5lrs.dll)",
    LR"(C:\windows\System32\x5lrsl.dll)",
    LR"(C:\windows\System32\ZDesignerLM.dll)",
    LR"(C:\windows\System32\zdnNLM64.dll)",
    LR"(C:\windows\WinSxS\amd64_microsoft.windows.common-controls_6595b64144ccf1df_5.82.17763.2237_none_6d05ce8459fa1d8d\COMCTL32.dll)",
    LR"(C:\windows\WinSxS\amd64_microsoft.windows.gdiplus_6595b64144ccf1df_1.1.17763.2237_none_0f5b3863addeb476\gdiplus.dll)",
    tpwinprn,
    LR"(C:\Program Files\EFA\TPPrintDM.dll)",
    nullptr };

    std::wstring getCmdOption(int argc, wchar_t* argv[], const std::wstring& option, const std::wstring& _default)
    {
        std::wstring cmd;
        for (int i = 0; i < argc; ++i)
        {
            std::wstring arg = argv[i];
            if (0 == arg.find(option))
            {
                std::size_t found = arg.find_first_of(option);
                cmd = arg.substr(found + option.size());
                return cmd;
            }
        }
        return _default;
    }

int wmain(int argc, wchar_t* argv[])
{
    std::wstring mode = getCmdOption(argc, argv, L"--mode=", L"detours");
    std::wstring folder = getCmdOption(argc, argv, L"--dir=", L".");
    std::wstring filter = getCmdOption(argc, argv, L"--filter=", L".*\\.dll");
    if (mode == L"loopdir")
    {
        using directory_iterator = filesystem::directory_iterator;
        for (const auto& dirEntry : directory_iterator(folder))
            if (regex_match(dirEntry.path().c_str(), wregex(filter, regex_constants::icase)))
                CheckDLL(dirEntry.path().c_str());
            else
                wcout << L"skipping: " << dirEntry.path().c_str() << endl;
    }
    else if (mode == L"spooldlls")
    {
        auto ppsListIndex = pps;
        bool skipfirst = getCmdOption(argc, argv, L"--skipfirst", L"no") != L"no";
        while (*ppsListIndex != nullptr)
        {
            if (skipfirst)
            {
                ppsListIndex++;
                skipfirst = false;
            }
            if (wcscmp(*ppsListIndex, tpwinprn) == 0)
            {
                std::wcout << ppsListIndex << L" arriving ...\n";
            }
            CheckDLL(*ppsListIndex);
            ppsListIndex++;
        }
    }
    return 0;
}

void CheckDLL(const wchar_t* i)
{
    static int loadedDLL = 0;
    static int loadedDLLTryies = 0;
    loadedDLLTryies++;
    std::wcout << loadedDLLTryies << L" checking: " << i << endl;

    auto hdl = LoadLibrary(i);
    if (hdl != NULL)
    {
        loadedDLL++;

        DWORD result = 0;

        std::vector<DWORD> indexes;

        //while (result != FLS_OUT_OF_INDEXES)
        //{
        //    result = FlsAlloc(NULL);
        //    indexes.push_back( result );
        //}

        //std::wcout << loadedDLL << L" Out of slots at attempt " << indexes.size() << endl;

        //for ( auto i : indexes )
        //{
        //    ::FlsFree( i );
        //}

        EnumPrintProcessorDatatypesW = (BOOL(__cdecl*)(
            LPWSTR   pName,
            LPWSTR   pPrintProcessorName,
            DWORD     Level,
            LPBYTE    pDatatypes,
            DWORD     cbBuf,
            LPDWORD   pcbNeeded,
            LPDWORD   pcReturned
            )) GetProcAddress(hdl, "EnumPrintProcessorDatatypesW");
        if (EnumPrintProcessorDatatypesW != nullptr)
        {
            DWORD dwNeeded;
            DWORD dwCount;
            BOOL rtn = EnumPrintProcessorDatatypesW(0,
                (LPTSTR)(LPCTSTR)i,
                1,
                0,
                0,
                &dwNeeded,
                &dwCount);
            auto err = GetLastError();
            std::wcout << loadedDLL << " " << i << L"\n" << L"rtn: " << rtn << L", Error: " << err << L"\n" << std::flush;
            PDATATYPES_INFO_1W buf = (PDATATYPES_INFO_1W)malloc(dwNeeded);
            if (buf)
            {
                SetLastError(0);
                rtn = EnumPrintProcessorDatatypesW(0,
                    (LPTSTR)(LPCTSTR)i,
                    1,
                    (LPBYTE)buf,
                    dwNeeded,
                    &dwNeeded,
                    &dwCount);
                err = GetLastError();
                std::wcout << loadedDLL << " " << L"rtn: " << rtn << L", Error: " << err << L"\n" << std::flush;
                if (rtn)
                {
                    for (DWORD i = 0; i < dwCount; i++)
                    {
                        std::wcout << loadedDLL << " " << buf[i].pName << L"\n" << std::flush;
                    }
                }
            }
        }
        else
        {
            std::wcout << loadedDLL << " " << i << L" kein print processor\n" << std::flush;
        }
    }
    else
    {
        std::wcout << loadedDLL << " " << i << L" LoadLibrary failed: " << GetLastError();
        std::wcout << "\n" << std::flush;
    }
}
