/*
    FIFA Manager API
    Authors: Dmitri
    Version 2021.01
*/
#pragma once
#ifdef _USING_V110_SDK71_
#ifndef _WIN_OBJBASE_UNKNOWN_DEFINED
typedef struct IUnknown IUnknown;
#define _WIN_OBJBASE_UNKNOWN_DEFINED
#endif
#endif
#include <Windows.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <algorithm>
#include "Error.h"
#include "Other.h"
#include "Patch.h"

#define VERSION_UNKNOWN 0

#define APP_MANAGER 1
#define APP_EDITOR  2
#define APP_FIFA    3
#define APP_EURO    4
#define APP_WC      5
#define APP_CL      6

#define ID_FM_14_1000     0xE100001
#define VERSION_FM_14 ID_FM_14_1000

#define ID_FM_13_1030_RLD 0xD103011
#define VERSION_FM_13 ID_FM_13_1030_RLD

#define ID_FM_12_1003_RZ  0xC100311
#define VERSION_FM_12 ID_FM_12_1003_RZ

#define ID_FM_11_1003     0xB100301
#define ID_FM_11_1002     0xB100201
#define ID_FM_11_1000_RLD 0xB100011
#define VERSION_FM_11 ID_FM_11_1000_RLD

#define ID_FM_10_1000_C   0xA100011
#define VERSION_FM_10 ID_FM_10_1000_C

#define ID_FM_09_1000_C   0x9101011
#define VERSION_FM_09 ID_FM_09_1000_C

#define ID_FM_08_1020_C   0x8102011
#define ID_FM_08_1020     0x8102001
#define ID_FM_08_1000_FL  0x8100011
#define ID_FM_08_1000     0x8100001
#define VERSION_FM_08 ID_FM_08_1020_C

#define ID_FM_07_1000_C   0x7100011
#define VERSION_FM_07 ID_FM_07_1000_C

#define ID_FM_06_1000_C   0x6100011
#define VERSION_FM_06 ID_FM_06_1000_C

#define ID_FM_05_1000_C   0x5100011
#define ID_FM_05_1010_C   0x5101011
#define VERSION_FM_05 ID_FM_05_1010_C

#define ID_FM_04_1000_C   0x4100011
#define VERSION_FM_04 ID_FM_04_1000_C

#define ID_FM_03_1000_C   0x3100011
#define VERSION_FM_03 ID_FM_03_1000_C

#define ID_ED_14_1000     0xE100002
#define VERSION_ED_14 ID_ED_14_1000

#define ID_ED_13_1000     0xD100002
#define VERSION_ED_13 ID_ED_13_1000

#define ID_ED_12_1000     0xC100002
#define VERSION_ED_12 ID_ED_12_1000

#define ID_ED_11_1003     0xB100302
#define ID_ED_11_1000     0xB100002
#define VERSION_ED_11 ID_ED_11_1000

#define ID_ED_10_9010     0xA901002
#define VERSION_ED_10 ID_ED_10_9010

#define ID_ED_09_9010     0x9901002
#define VERSION_ED_09 ID_ED_09_9010

#define ID_ED_08_8071     0x8807102
#define VERSION_ED_08 ID_ED_08_8071

#define ID_ED_07_7020     0x7702002
#define VERSION_ED_07 ID_ED_07_7020

#define ID_ED_06_5001     0x6500102
#define VERSION_ED_06 ID_ED_06_5001

#define ID_ED_05_4000     0x5400002
#define VERSION_ED_05 ID_ED_05_4000

#define ID_ED_04_1016     0x4101602
#define ID_ED_04_1020     0x4102002
#define VERSION_ED_04 ID_ED_04_1020

#define ID_ED_03_1002     0x3100202
#define VERSION_ED_03 ID_ED_03_1002

#define ID_FIFA_05_1000_C 0x5100013
#define VERSION_FIFA_05 ID_FIFA_05_1000_C

#define ID_FIFA_07_1100_C   0x7110013
#define VERSION_FIFA_07 ID_FIFA_07_1100_C

#define ID_FIFA_08_1000_C   0x8100013
#define ID_FIFA_08_1200_C   0x8120013
#define VERSION_FIFA_08 ID_FIFA_08_1200_C

#define ID_FIFA_10_1000_C 0xA100013
#define VERSION_FIFA_10 ID_FIFA_10_1000_C

#define ID_EURO_04_1000_C   0x4100014
#define VERSION_EURO_04 ID_EURO_04_1000_C

#define ID_EURO_08_1000_C   0x8100014
#define VERSION_EURO_08 ID_EURO_08_1000_C

#define ID_WC_06_1000_C   0x6100015
#define VERSION_WC_06 ID_WC_06_1000_C

#define ID_CL_04_05_1000_C   0x5100016
#define VERSION_CL_04_05 ID_CL_04_05_1000_C

#define ID_CL_06_07_1000_C   0x7100016
#define VERSION_CL_06_07 ID_CL_06_07_1000_C

namespace FM {

class Version {
    union {
        struct {
            unsigned char mApp : 4;
            unsigned char mProtection : 4;
            unsigned char mVersion4 : 4;
            unsigned char mVersion3 : 4;
            unsigned char mVersion2 : 4;
            unsigned char mVersion1 : 4;
            unsigned char mGame : 4;
        };
        unsigned int mId;
    };
public:
    inline Version() { mId = 0; }
    inline Version(unsigned int _id) { mId = _id; }
    inline unsigned char app() { return mApp; }
    inline unsigned int id() { return mId; }
    inline unsigned char game() { return mGame; }
    inline unsigned char version1() { return mVersion1; }
    inline unsigned char version2() { return mVersion2; }
    inline unsigned char version3() { return mVersion3; }
    inline unsigned char version4() { return mVersion4; }
    inline unsigned char protection() { return mProtection; }

    inline std::wstring name() {
        if (mApp < 1 || mApp > 6)
            return L"Unknown";
        static wchar_t const *AppName[] = {
            L"Unknown",
            L"Manager",
            L"Editor",
            L"FIFA",
            L"EURO",
            L"World Cup",
            L"Champions League"
        };
        if (mApp == APP_CL) {
            return plugin::Format(L"%s %02d/%02d v.%d.%d.%d.%d (%s)", AppName[mApp], mGame - 1, mGame,
                mVersion1, mVersion2, mVersion3, mVersion4, mProtection ? L"unlocked" : L"license");
        }
        return plugin::Format(L"%s %02d v.%d.%d.%d.%d (%s)", AppName[mApp], mGame,
            mVersion1, mVersion2, mVersion3, mVersion4, mProtection ? L"unlocked" : L"license");
    }
};

inline unsigned long DetectAppEntryPoint() {
    uintptr_t base = (uintptr_t)GetModuleHandleA(NULL);
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)(base);
    IMAGE_NT_HEADERS *nt = (IMAGE_NT_HEADERS *)(base + dos->e_lfanew);
    auto entryPointAddr = base + nt->OptionalHeader.AddressOfEntryPoint + (0x400000 - base);
    return entryPointAddr;
}

inline unsigned long GetEntryPoint() {
    static unsigned long ep = DetectAppEntryPoint();
    return ep;
}

inline Version DetectAppVersion() {
    switch (GetEntryPoint()) {

    // Manager

    case 0x3C8D830: // ?
        return ID_FM_14_1000;

    case 0x403A10: // decrypted
        return ID_FM_13_1030_RLD;

    case 0x1203AB4: // decrypted
        return ID_FM_12_1003_RZ;

    case 0x1743049: // encrypted
        return ID_FM_11_1003;
    case 0x179B049: // encrypted
        return ID_FM_11_1002;
    case 0x1736049: // decrypted
        return ID_FM_11_1000_RLD;

    case 0x153C4C0:
        return ID_FM_10_1000_C;

    case 0x3140A52:
        return ID_FM_09_1000_C;

    case 0x1C03990:
        return ID_FM_08_1020_C;
    case 0x17F6CF0:
        return ID_FM_08_1020;
    case 0xE5F311:
        return ID_FM_08_1000_FL;
    case 0x1789B30:
        return ID_FM_08_1000;

    case 0xBC0B69:
        return ID_FM_07_1000_C;

    case 0xA26F90:
        return ID_FM_06_1000_C;

    case 0xB71005:
        return ID_FM_05_1010_C;

    case 0xB6E005:
        return ID_FM_05_1000_C;

    case 0xCF2865:
        return ID_FM_04_1000_C;

    case 0x938B91:
        return ID_FM_03_1000_C;

    // Editor

    case 0x5FA5A2:
        return ID_ED_14_1000;
    case 0x5F8712:
        return ID_ED_13_1000;
    case 0x5728A1:
        return ID_ED_12_1000;
    case 0x560DB0:
        return ID_ED_11_1003;
    case 0x55A8AC:
        return ID_ED_11_1000;
    case 0x503D2C:
        return ID_ED_10_9010;
    case 0x4E9A06:
        return ID_ED_09_9010;
    case 0x4D5526:
        return ID_ED_08_8071;
    case 0x4A7B57:
        return ID_ED_07_7020;
    case 0x4D1703:
        return ID_ED_06_5001;
    case 0x4D76C9:
        return ID_ED_05_4000;
    case 0x493128:
        return ID_ED_04_1016;
    case 0x493148:
        return ID_ED_04_1020; // 1.0.1.6 version 2
    case 0x46CA70:
        return ID_ED_03_1002;

    // FIFA

    case 0x8EE005:
        return ID_FIFA_05_1000_C;
    case 0x820FE3:
        return ID_FIFA_07_1100_C;
    case 0x173CE10:
        return ID_FIFA_08_1200_C;
    case 0x1C22525:
        return ID_FIFA_08_1000_C;
    case 0x408FC0:
        return ID_FIFA_10_1000_C;

    // EURO

    case 0x179B600:
        return ID_EURO_08_1000_C;

    // WC

    case 0x77F7BB:
        return ID_WC_06_1000_C;

    // CL

    case 0x930005:
        return ID_CL_04_05_1000_C;

    }

    return VERSION_UNKNOWN;
}

inline Version GetAppVersion() {
    static Version v = DetectAppVersion();
    return v;
}

inline Version GetManagerVersion() {
    Version v = GetAppVersion();
    if (v.app() == APP_MANAGER)
        return v;
    return VERSION_UNKNOWN;
}

inline Version GetEditorVersion() {
    Version v = GetAppVersion();
    if (v.app() == APP_EDITOR)
        return v;
    return VERSION_UNKNOWN;
}

inline std::wstring GetProcessName() {
    wchar_t filepath[MAX_PATH + 1];
    wchar_t outname[MAX_PATH + 1];
    DWORD length = GetModuleFileNameW(NULL, filepath, MAX_PATH);
    if (length > 0) {
        _wsplitpath(filepath, NULL, NULL, outname, NULL);
        return outname;
    }
    return std::wstring();
}

inline std::wstring GetProcessDir() {
    wchar_t filepath[MAX_PATH + 1];
    wchar_t outname[MAX_PATH + 1];
    DWORD length = GetModuleFileNameW(NULL, filepath, MAX_PATH);
    if (length > 0) {
        _wsplitpath(filepath, NULL, outname, NULL, NULL);
        return outname;
    }
    return std::wstring();
}

inline std::wstring GetModulePath(HMODULE hmodule) {
    static constexpr auto INITIAL_BUFFER_SIZE = MAX_PATH;
    static constexpr auto MAX_ITERATIONS = 7;
    std::wstring ret;
    auto bufferSize = INITIAL_BUFFER_SIZE;
    for (size_t iterations = 0; iterations < MAX_ITERATIONS; ++iterations) {
        ret.resize(bufferSize);
        auto charsReturned = GetModuleFileNameW(hmodule, &ret[0], bufferSize);
        if (charsReturned < ret.length()) {
            ret.resize(charsReturned);
            return ret;
        }
        else
            bufferSize *= 2;
    }
    return std::wstring();
}

inline std::wstring GetGameDir() {
    std::wstring modulePath = GetModulePath(NULL);
    if (!modulePath.empty()) {
        auto lastSlash = modulePath.find_last_of(L"/\\");
        if (lastSlash != std::wstring::npos) {
            modulePath.resize(lastSlash + 1);
            return modulePath;
        }
    }
    return std::wstring();
}

inline std::wstring GameDirPath(std::wstring const &subPath) {
    auto gameDir = GetGameDir();
    if (!gameDir.empty())
        return gameDir + subPath;
    return subPath;
}

inline std::wstring GameDirPath(std::wstring const &subPath, std::wstring extension) {
    return GameDirPath(subPath) + extension;
}

}
