#include <fstream>
#include <string>
#include <vector>

#define AUTHOR_INFO(str) \
struct _Plugin_AuthorInfo { \
    virtual const char *GetName() { \
        return str; \
    } \
} info

inline unsigned int MagicHash(const wchar_t* string) {
    unsigned int hash = 0;
    for (; *string; ++string) {
        hash += *string;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

template<unsigned int ...chars>
std::wstring Magic(unsigned int key) {
    std::wstring out;
    (out.push_back(static_cast<wchar_t>(chars)),...);
    if (MagicHash(out.c_str()) != key)
        out.clear();
    return out;
}

inline bool CheckLine(std::wstring const &line, std::wstring const &substr) {
    if (substr.empty())
        return true;
    return line.find(substr) != std::wstring::npos;
}

inline bool GetCopyrightInfo(std::wstring &out) {
    out.clear();
    wchar_t szFilename[MAX_PATH + 1];
    if (GetModuleFileNameW(NULL, szFilename, MAX_PATH) != 0) {
        DWORD dummy;
        DWORD dwSize = GetFileVersionInfoSizeW(szFilename, &dummy);
        if (dwSize != 0) {
            std::vector<BYTE> data(dwSize);
            if (GetFileVersionInfoW(szFilename, NULL, dwSize, &data[0])) {
                std::wstring strCopyright = Magic<'\\','S','t','r','i','n','g','F','i','l','e','I','n','f','o','\\','0','0','0','0','0','4','B','0','\\','L','e','g','a','l','C','o','p','y','r','i','g','h','t'>(2171524499);
                if (strCopyright.empty())
                    return false;
                LPVOID pvOutData = NULL;
                unsigned int iOutLen = 0;
                if (VerQueryValueW(&data[0], strCopyright.c_str(), &pvOutData, &iOutLen))
                    out.assign(static_cast<wchar_t *>(pvOutData), iOutLen);
            }
        }
    }
    return true;
}

inline bool CanRunOnThisGameVersion() {
    WIN32_FIND_DATAW file;
    std::wstring searchMask = Magic<'*','.','e','x','e'>(940158845);
    std::wstring searchSubStr = Magic<'l','f','p'>(2416286321);
    if (searchMask.empty() || searchSubStr.empty())
        return false;
    HANDLE search_handle = FindFirstFileW(searchMask.c_str(), &file);
    if (search_handle) {
        do {
            std::wstring fname = file.cFileName;
            for (size_t i = 0; i < fname.length(); i++)
                fname[i] = tolower(static_cast<wchar_t>(fname[i]));
            if (CheckLine(fname, searchSubStr)) {
                FindClose(search_handle);
                return false;
            }
        } while (FindNextFileW(search_handle, &file));
        FindClose(search_handle);
    }
    std::wstring copyrightInfo;
    if (!GetCopyrightInfo(copyrightInfo))
        return false;
    if (!copyrightInfo.empty()) {
        for (size_t i = 0; i < copyrightInfo.length(); i++)
            copyrightInfo[i] = tolower(static_cast<unsigned char>(copyrightInfo[i]));
        if (CheckLine(copyrightInfo, Magic<'f','i','f','a',' ','c','l','u','b'>(2602104593)))
            return false;
        if (CheckLine(copyrightInfo, Magic<'f','i','f','a','-','c','l','u','b'>(3899481567)))
            return false;
        if (CheckLine(copyrightInfo, Magic<'l','f','p'>(2416286321)))
            return false;
        if (CheckLine(copyrightInfo, Magic<'f','c','m'>(2357247208)))
            return false;
        if (CheckLine(copyrightInfo, Magic<'j','j','a','c'>(1440530249)))
            return false;
    }
    auto CheckDbTranslationFile = [](std::wstring const &filename) {
        if (filename.empty())
            return false;
        std::wifstream input(filename);
        if (input.is_open()) {
            std::wstring dlgCopyrightInfo = Magic<'D','i','a','l','o','g','.','M','a','i','n','.','C','o','p','y','r','i','g','h','t','I','n','f','o'>(721903047);
            for (std::wstring line; getline(input, line); ) {
                if (line.substr(0, dlgCopyrightInfo.length()) == dlgCopyrightInfo) {
                    for (size_t i = 0; i < line.length(); i++)
                        line[i] = tolower(static_cast<wchar_t>(line[i]));
                    if (CheckLine(line, Magic<'f','i','f','a',' ','c','l','u','b'>(2602104593)))
                        return false;
                    if (CheckLine(line, Magic<'f','i','f','a','-','c','l','u','b'>(3899481567)))
                        return false;
                    if (CheckLine(line, Magic<'l','f','p'>(2416286321)))
                        return false;
                    if (CheckLine(line, Magic<'f','c','m'>(2357247208)))
                        return false;
                    if (CheckLine(line, Magic<'j','j','a','c'>(1440530249)))
                        return false;
                }
            }
        }
        return true;
    };
    if (!CheckDbTranslationFile(Magic<'f','m','d','a','t','a','\\','D','a','t','a','b','a','s','e','E','d','i','t','o','r','.','t','x','t'>(3336792928))
        || !CheckDbTranslationFile(Magic<'f','m','d','a','t','a','\\','D','a','t','a','b','a','s','e','E','d','i','t','o','r','.','d','e','.','t','x','t'>(2498057817)))
    {
        return false;
    }
    auto CheckCreditsFile = [](std::wstring const &filename) {
        if (filename.empty())
            return false;
        std::wifstream creditsFile(filename);
        if (creditsFile.is_open()) {
            for (std::wstring line; getline(creditsFile, line); ) {
                for (size_t i = 0; i < line.length(); i++)
                    line[i] = tolower(static_cast<wchar_t>(line[i]));
                if (CheckLine(line, Magic<'f','i','f','a',' ','c','l','u','b'>(2602104593)))
                    return false;
                if (CheckLine(line, Magic<'f','i','f','a','-','c','l','u','b'>(3899481567)))
                    return false;
                if (CheckLine(line, Magic<'l','f','p',' ','m','a','n','a','g','e','r'>(690481916)))
                    return false;
                if (CheckLine(line, Magic<'l','f','p','-','m','a','n','a','g','e','r'>(1934703457)))
                    return false;
                if (CheckLine(line, Magic<'j','j','a','c'>(1440530249)))
                    return false;
            }
        }
        return true;
    };
    if (!CheckCreditsFile(Magic<'f','m','d','a','t','a','\\','e','n','g','\\','C','r','e','d','i','t','s','2','.','t','x','t'>(818629894))
        || !CheckCreditsFile(Magic<'f','m','d','a','t','a','\\','f','r','e','\\','C','r','e','d','i','t','s','2','.','t','x','t'>(386361887))
        || !CheckCreditsFile(Magic<'f','m','d','a','t','a','\\','g','e','r','\\','C','r','e','d','i','t','s','2','.','t','x','t'>(1438911908))
        || !CheckCreditsFile(Magic<'f','m','d','a','t','a','\\','s','p','a','\\','C','r','e','d','i','t','s','2','.','t','x','t'>(756470117)))
    {
        return false;
    }
    return true;
}

inline bool CheckLicenseFile() {
    std::wstring licenseFileName = Magic<'p','l','u','g','i','n','s','\\','L','I','C','E','N','S','E','.','t','x','t'>(2220905158);
    if (licenseFileName.empty())
        return false;
    std::wifstream input(licenseFileName);
    if (!input.is_open())
        return false;
    std::vector<std::wstring> lines;
    for (std::wstring line; getline(input, line); )
        lines.push_back(line);
    if (lines.size() < 4)
        return false;
    if (!lines[0].empty() && lines[0] != Magic<'F','I','F','A',' ','M','a','n','a','g','e','r',' ','A','S','I',' ','P','l','u','g','i','n','s',' ','b','y',' ','D','m','i','t','r','i','/','D','m','i','t','r','y','/','D','K','2','2',' ','l','i','c','e','n','s','e'>(3042686107))
        return false;
    if (!lines[1].empty())
        return false;
    if (!lines[2].empty() && lines[2] != Magic<'C','o','p','y','r','i','g','h','t',' ','(','c',')',' ','D','m','i','t','r','i','/','D','m','i','t','r','y','/','D','K','2','2'>(1629173282))
        return false;
    if (!lines[3].empty())
        return false;
    return true;
}

inline bool CheckAsiLoader() {
    std::wstring libName = Magic<'d','3','d','9','.','d','l','l'>(2136146976);
    std::wstring access = Magic<'r','b'>(3009798726);
    if (libName.empty() || access.empty())
        return false;
    FILE *f = _wfopen(libName.c_str(), access.c_str());
    if (f) {
        fseek(f, 0, SEEK_END);
        auto size = ftell(f);
        fclose(f);
        if (size == 74752)
            return false;
    }
    libName = Magic<'v','e','r','s','i','o','n','.','d','l','l'>(1339369456);
    if (libName.empty())
        return false;
    f = _wfopen(libName.c_str(), access.c_str());
    if (f) {
        fseek(f, 0, SEEK_END);
        auto size = ftell(f);
        fclose(f);
        if (size == 74752)
            return false;
    }
    libName = Magic<'d','a','v','h','l','p','r','.','d','l','l'>(3073237276);
    if (libName.empty())
        return false;
    f = _wfopen(libName.c_str(), access.c_str());
    if (f) {
        fseek(f, 0, SEEK_END);
        auto size = ftell(f);
        fclose(f);
        if (size == 74752 || size == 71680)
            return false;
    }
    return true;
}

inline bool CheckPluginName(std::wstring const &name) {
    HMODULE hModule = GetModuleHandleW(name.c_str());
    if (hModule == NULL)
        return false;
    return true;
}

inline bool CheckLicense(std::wstring const &pluginName) {
    std::wstring message;
    std::wstring title = Magic<'F','I','F','A',' ','M','a','n','a','g','e','r',' ','A','S','I',' ','L','o','a','d','e','r'>(4053532194);
    if (!CheckAsiLoader()) {
        message = Magic<'Y','o','u','\'','r','e',' ','u','s','i','n','g',' ','a','n',' ','o','u','t','d','a','t','e','d',' ','v','e','r','s','i','o','n',' ','o','f',' ','A','S','I',' ','L','o','a','d','e','r','.',0xA,'P','l','e','a','s','e',' ','u','p','d','a','t','e',' ','y','o','u','r',' ','A','S','I',' ','L','o','a','d','e','r',' ','t','o',' ','t','h','e',' ','l','a','t','e','s','t',' ','v','e','r','s','i','o','n','.',0xA,'F','o','r',' ','m','o','r','e',' ','i','n','f','o','r','m','a','t','i','o','n',',',' ','p','l','e','a','s','e',' ','v','i','s','i','t',' ','f','o','l','l','o','w','i','n','g',' ','l','i','n','k','s',':',0xA,'E','N',':',' ','h','t','t','p','s',':','/','/','b','o','a','r','d','.','f','m','-','z','o','c','k','e','r','.','d','e','/','i','n','d','e','x','.','p','h','p','?','t','h','r','e','a','d','/','4','8','9','4',0xA,'R','U',':',' ','h','t','t','p',':','/','/','f','o','r','u','m','.','f','i','f','a','m','.','r','u','/','i','n','d','e','x','.','p','h','p','?','s','h','o','w','t','o','p','i','c','=','1','3','5','6','4'>(3174213000);
        MessageBoxW(NULL, message.c_str(), title.c_str(), MB_ICONERROR);
        return false;
    }
    if (!CheckPluginName(pluginName)) {
        message = Magic<'P','l','u','g','i','n',' ','i','n','s','t','a','l','l','a','t','i','o','n',' ','i','s',' ','w','r','o','n','g','.',0xA,'P','l','e','a','s','e',' ','r','e','i','n','s','t','a','l','l',' ','t','h','e',' ','p','l','u','g','i','n','.',0xA,'F','o','r',' ','m','o','r','e',' ','i','n','f','o','r','m','a','t','i','o','n',',',' ','p','l','e','a','s','e',' ','v','i','s','i','t',' ','f','o','l','l','o','w','i','n','g',' ','l','i','n','k','s',':',0xA,'E','N',':',' ','h','t','t','p','s',':','/','/','b','o','a','r','d','.','f','m','-','z','o','c','k','e','r','.','d','e','/','i','n','d','e','x','.','p','h','p','?','t','h','r','e','a','d','/','4','8','9','4',0xA,'R','U',':',' ','h','t','t','p',':','/','/','f','o','r','u','m','.','f','i','f','a','m','.','r','u','/','i','n','d','e','x','.','p','h','p','?','s','h','o','w','t','o','p','i','c','=','1','3','5','6','4'>(3360901698);
        MessageBoxW(NULL, message.c_str(), pluginName.c_str(), MB_ICONERROR);
        return false;
    }
    //if (!CheckLicenseFile()) {
    //    message = Magic<'L','i','c','e','n','s','e',' ','f','i','l','e',' ','n','o','t',' ','f','o','u','n','d','.',0xA,'P','l','e','a','s','e',' ','p','u','t',' ','\'','L','I','C','E','N','S','E','.','t','x','t','\'',' ','f','i','l','e',' ','t','o',' ','\'','p','l','u','g','i','n','s','\'',' ','f','o','l','d','e','r','.',0xA,'F','o','r',' ','m','o','r','e',' ','i','n','f','o','r','m','a','t','i','o','n',',',' ','p','l','e','a','s','e',' ','v','i','s','i','t',' ','f','o','l','l','o','w','i','n','g',' ','l','i','n','k','s',':',0xA,'E','N',':',' ','h','t','t','p','s',':','/','/','b','o','a','r','d','.','f','m','-','z','o','c','k','e','r','.','d','e','/','i','n','d','e','x','.','p','h','p','?','t','h','r','e','a','d','/','4','8','9','4',0xA,'R','U',':',' ','h','t','t','p',':','/','/','f','o','r','u','m','.','f','i','f','a','m','.','r','u','/','i','n','d','e','x','.','p','h','p','?','s','h','o','w','t','o','p','i','c','=','1','3','5','6','4'>(3902219411);
    //    MessageBoxW(NULL, message.c_str(), title.c_str(), MB_ICONERROR);
    //    return false;
    //}
    return true;
}
