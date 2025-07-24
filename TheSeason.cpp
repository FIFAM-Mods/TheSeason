#include "plugin-std.h"
#include "Config.h"
#include "license_check/license_check.h"
#include "InternationalCupsFix.h"

AUTHOR_INFO("TheSeason.v2 ASI plugin for FIFA Manager by Dmitri and Purkas");

using namespace plugin;

#define PLUGIN_NAME "TheSeason.v2.3.1";
#define ENABLE_FM13_INTERNATIONAL_FIX true
unsigned int SEASON_START_YEAR = 2025;
unsigned int SEASON_START_MONTH = 7;
unsigned int SEASON_START_DAY = 1;
bool START_DAY_CUSTOM = false;
bool IS_ODD_YEAR = SEASON_START_YEAR % 2;
unsigned int MANAGER_ID = 14;
unsigned int SetWindowTitleAddr = 0;
bool FORCE_SEASON_START_DATE_RELOAD = false;
bool DEBUG = false;
bool SKIP_START_DATE_WARNING_MESSAGES = false;
bool DISABLE_ADDITIONAL_PATCHES = false;
unsigned int AGE_MIN = 10;
unsigned int AGE_MAX = 110;
unsigned int AGE_DEFAULT = 25;
unsigned int FOUNDATION_YEAR_DEFAULT = 2000;

//#define DUMP

class TheSeason {
public:
    struct StartDate {
        unsigned char day = 0;
        unsigned char month = 0;
        unsigned short year = 0;
    };

    static bool UCPPluginInstalled() {
        auto pluginPath = FM::GameDirPath(L"plugins\\UniversalConverterProject.Main.asi");
        auto attrs = GetFileAttributesW(pluginPath.c_str());
        return (attrs != INVALID_FILE_ATTRIBUTES) && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
    }

    template<bool Editor, unsigned int DateSet, unsigned int DateGetDay>
    StartDate GetStartDate(unsigned char forceWeekday) {
        StartDate date;
        date.year = SEASON_START_YEAR;
        if (!Editor && (START_DAY_CUSTOM || SEASON_START_MONTH != 7)) {
            date.day = SEASON_START_DAY;
            date.month = SEASON_START_MONTH;
        }
        else {
            unsigned int cjdate = 0;
            CallMethod<DateSet>(&cjdate, SEASON_START_YEAR, 7, 1);
            while ((cjdate % 7) != forceWeekday)
                cjdate++;
            date.day = CallMethodAndReturn<unsigned char, DateGetDay>(&cjdate);
            date.month = 7;
        }
        return date;
    }

    static void SetManagerId(unsigned int id) {
        MANAGER_ID = id;
    #ifdef DUMP
        AppendToFile(std::string("Editor " + std::to_string(MANAGER_ID) + "\n").c_str());
    #endif
    }

    template<typename T = int>
    static void MarkYear(unsigned int address, char const *description = "") {
        T year = patch::Get<T>(address);
        T newYear = year + SEASON_START_YEAR - 2000 - (MANAGER_ID - 1);
        patch::Set<T>(address, newYear);
    #ifdef DUMP
        std::string line = "    MarkYearWithValue";
        if (sizeof(T) != 4)
            line += std::string("<") + typeid(T).name() + ">";
        line += "(" + Format("0x%X", address) + ", " + std::to_string(year);
        if (description[0])
            line += std::string(", \"") + description + "\"";
        line += ");\n";
        AppendToFile(line.c_str());
    #endif
    }

    template<typename T = int>
    static void MarkYearWithValue(unsigned int address, T year, char const *description = "") {
        T newYear = year + SEASON_START_YEAR - 2000 - (MANAGER_ID - 1);
        patch::Set<T>(address, newYear);
    }

    template<typename T = int>
    static void SetYearFixedValue(unsigned int address, T newValue, char const *description = "") {
        patch::Set<T>(address, newValue);
    }

    static void AppendToFile(char const *what) {
        FILE *f = fopen("test.txt", "at");
        fputs(what, f);
        fclose(f);
    }

    static void METHOD SetEditorWindowTitle(void *wnd, DUMMY_ARG, wchar_t const *title) {
        std::wstring newTitle = title;
        std::wstring temp;
        temp.push_back(L'!');
        if (!EndsWith(newTitle, temp)) {
            temp.clear();
            temp.push_back(L' '); temp.push_back(L'-'); temp.push_back(L' '); temp.push_back(L' ');
            bool addPrefix = true;
            if (EndsWith(newTitle, temp)) {
                addPrefix = false;
                newTitle.pop_back();
            }
            else {
                std::wstring temp2;
                temp2.push_back(L' ');
                if (EndsWith(newTitle, temp2)) {
                    newTitle.pop_back();
                    addPrefix = true;
                }
            }
            if (addPrefix) {
                temp.clear();
                temp.push_back(L' '); temp.push_back(L'-'); temp.push_back(L' ');
                newTitle += temp;
            }
            temp.clear();
            temp.push_back(L'S'); temp.push_back(L'e'); temp.push_back(L'a'); temp.push_back(L's'); temp.push_back(L'o'); temp.push_back(L'n');
            temp.push_back(L' '); temp.push_back(L'%'); temp.push_back(L'd'); temp.push_back(L'/'); temp.push_back(L'%'); temp.push_back(L'd');
            newTitle += Format(temp, SEASON_START_YEAR, SEASON_START_YEAR + 1);
            temp.clear();
            temp.push_back(L' '); temp.push_back(L'('); temp.push_back(L'T'); temp.push_back(L'h'); temp.push_back(L'e'); temp.push_back(L'S');
            temp.push_back(L'e'); temp.push_back(L'a'); temp.push_back(L's'); temp.push_back(L'o'); temp.push_back(L'n'); temp.push_back(L'.');
            temp.push_back(L'v'); temp.push_back(L'2'); temp.push_back(L')');
            newTitle += temp;
        }
        if (SetWindowTitleAddr)
            CallMethodDynGlobal(SetWindowTitleAddr, wnd, newTitle.c_str());
    }

    static void PluginLoaded() {
        if (DEBUG) {
            Message(L"TheSeason plugin is loaded.\n"
                L"Current process: " + FM::GetProcessName() + L" (EntryPoint: " + Format(L"0x%X", FM::GetEntryPoint()) + L")\n"
                L"  plugin version: 2.1\n"
                L"  authors: Dmitri, Purkas\n"
                L"\n"
                L"Season start date: " + Format(L"%02d.%02d.%04d", SEASON_START_DAY, SEASON_START_MONTH, SEASON_START_YEAR));
        }
        if (SEASON_START_MONTH != 7 && !SKIP_START_DATE_WARNING_MESSAGES)
            Warning(L"You have changed starting month!\n"
                    L"This may cause crashes and errors in the game.");
    }

    static void PluginNotLoaded() {
        if (DEBUG) {
            Error(L"TheSeason plugin was not loaded!\n"
                  L"Current process: " + FM::GetProcessName() + L" (EntryPoint: " + Format(L"0x%X", FM::GetEntryPoint()) + L")\n"
                  L"Please check if you use the correct executable version.\n"
                  L"\n"
                  L"TheSeason plugin works with:\n"
                  //L"  FIFA Manager 13 1.0.3.0 RLD version\n"
                  //L"  Editor 13 1.0.0.0 (from FIFA Manager 13 ver. 1.0.3.0)\n"
                  //L"  Editor 14 1.0.0.0 (from FIFA Manager 14 ver. 1.0.0.0)\n"
                  //L"  FIFA Manager 11 1.0.0.3 version\n"
                  //L"  Editor 11 1.0.0.3 (from FIFA Manager 11 ver. 1.0.0.3)\n"
                  //L"  FIFA Manager 08 1.0.2.0 version\n"
                  //L"  Editor 08 8.0.7.1 (from FIFA Manager 08 ver. 1.0.2.0)\n"
                  //L"  FIFA Manager 07 1.0.0.0 version\n"
                  L"Read the _Installation_Readme.txt file for more details.");
        }
    }

    static void METHOD OnTCM05SetNewPlayerHistoryEntryStartDate(void *date1, DUMMY_ARG, void *date2) {
        CallMethod<0x4B0700>(date1, date2);
        CallMethod<0x4B0660>(date1, date1, 1);
    }

    TheSeason() {
        
        if (!CheckLicense(Magic<'T','h','e','S','e','a','s','o','n','.','a','s','i'>(1959387336)))
            return;

        auto v = FM::GetAppVersion();
        auto version = v.id();

        config_file settings(L"plugins\\season.ini");
        int testValue = settings["SEASON_START_YEAR"].asInt(-12345);
        if (testValue == -12345) {
            Warning(L"Settings file was not read!\n"
                    L"Please check if you have the following file:\n"
                    L"\n"
                    L"'plugins\\season.ini'\n"
                    L"\n"
                    L"If you don't have it, please reinstall the TheSeason plugin.\n"
                    L"Note that the plugin will be loaded with default settings in current session.");
        }
        else {
            SEASON_START_YEAR = settings["SEASON_START_YEAR"].asInt(SEASON_START_YEAR);
            SEASON_START_MONTH = settings["SEASON_START_MONTH"].asInt(SEASON_START_MONTH);
            int startDay = settings["SEASON_START_DAY"].asInt(-12345);
            if (startDay != -12345) {
                SEASON_START_DAY = startDay;
                START_DAY_CUSTOM = true;
            }
            bool forceReload = v.game() >= 3 && v.game() <= 11;
            FORCE_SEASON_START_DATE_RELOAD = settings["FORCE_SEASON_START_DATE_RELOAD"].asBool(forceReload);
            DEBUG = settings["DEBUG"].asBool(DEBUG);
            SKIP_START_DATE_WARNING_MESSAGES = settings["SKIP_START_DATE_WARNING_MESSAGES"].asBool(SKIP_START_DATE_WARNING_MESSAGES);
            DISABLE_ADDITIONAL_PATCHES = settings["DISABLE_ADDITIONAL_PATCHES"].asBool(DISABLE_ADDITIONAL_PATCHES);
            AGE_MIN = settings["AGE_MIN"].asInt(AGE_MIN);
            AGE_MAX = settings["AGE_MAX"].asInt(AGE_MAX);
            AGE_DEFAULT = settings["AGE_DEFAULT"].asInt(AGE_DEFAULT);
        }
        if (SEASON_START_YEAR >= 2010 && SEASON_START_YEAR < 2090)
            FOUNDATION_YEAR_DEFAULT = 2000;
        else if (SEASON_START_YEAR >= 1910 && SEASON_START_YEAR < 2010)
            FOUNDATION_YEAR_DEFAULT = 1900;
        else
            FOUNDATION_YEAR_DEFAULT = 1800;
        if (version == VERSION_FM_13) {

            PluginLoaded();

            patch::SetUChar(0xF5B11D + 1, SEASON_START_DAY);
            patch::SetUChar(0xF5B11F + 1, SEASON_START_MONTH);
            patch::SetUInt(0xF5B121 + 1, SEASON_START_YEAR);

            patch::SetUInt(0x53C51F + 4, SEASON_START_YEAR - AGE_MAX);
            patch::SetUInt(0x53C52B + 4, SEASON_START_YEAR - AGE_MIN);
            patch::SetUInt(0x15061F0 + 1, SEASON_START_YEAR - AGE_DEFAULT);

            patch::SetUInt(0x51CF64 + 4, SEASON_START_YEAR - AGE_MAX);
            patch::SetUInt(0x51CF11 + 1, SEASON_START_YEAR - AGE_DEFAULT);
            patch::SetUChar(0x51CF5D + 2, AGE_MIN);

            if (FORCE_SEASON_START_DATE_RELOAD) {
                void *game = patch::GetPointer(0x31264D0);
                if (!game) {
                    Warning("The start date was not initialized yet!\n"
                            "Start date reload is not needed.");
                }
                else {
                    patch::SetUInt(0x3126954, 0);
                    CallMethodDynGlobal(0xF5B0E0, game);
                }
            }

            if (ENABLE_FM13_INTERNATIONAL_FIX) {
                if (!UCPPluginInstalled())
                    PatchInternationalCups(v);
                else
                    Warning("Internationals fix is not applied - UCP Plugin is already installed");
            }

            if (!DISABLE_ADDITIONAL_PATCHES) {
                // 100.000.000 Transfer Fee limit
                patch::SetUInt(0x109F950, 900'000'000);
                patch::SetUInt(0x109F966, 900'000'000);
            }
        }
        /*else if (version == VERSION_FM_14) {

            PluginLoaded();

            patch::SetUChar(0xF2F3CD + 1, SEASON_START_DAY);
            patch::SetUChar(0xF2F3CF + 1, SEASON_START_MONTH);
            patch::SetUInt(0xF2F3D1 + 1, SEASON_START_YEAR);

            if (FORCE_SEASON_START_DATE_RELOAD) {
                void *game = patch::GetPointer(0x36E9918);
                if (!game) {
                    Warning("The start date was not initialized yet!\n"
                        "Start date reload is not needed.");
                }
                else {
                    patch::SetUInt(0x36E9D98, 0);
                    CallMethodDynGlobal(0xF2F390, game);
                }
            }
        }*/
        /*else if (version == VERSION_FM_12) {

            PluginLoaded();

            patch::SetUChar(0xE79342 + 1, SEASON_START_DAY);
            patch::SetUChar(0xE79344 + 1, SEASON_START_MONTH);
            patch::SetUInt(0xE79346 + 1, SEASON_START_YEAR);

            if (FORCE_SEASON_START_DATE_RELOAD) {
                void *game = patch::GetPointer(0x1650538);
                if (!game) {
                    Warning("The start date was not initialized yet!\n"
                        "Start date reload is not needed.");
                }
                else {
                    patch::SetUInt(0x16509A4, 0);
                    CallMethodDynGlobal(0xE79310, game);
                }
            }
        }*/
        else if (version == ID_FM_11_1003) {

            PluginLoaded();

            patch::SetUChar(0xDD8322 + 1, SEASON_START_DAY);
            patch::SetUChar(0xDD8324 + 1, SEASON_START_MONTH);
            patch::SetUInt(0xDD8326 + 1, SEASON_START_YEAR);

            unsigned int m = SEASON_START_YEAR % 4;
            unsigned int WCYear = 2022;
            if (m == 0)
                WCYear = SEASON_START_YEAR + 2;
            else if (m == 3)
                WCYear = SEASON_START_YEAR + 3;
            else if (m == 1)
                WCYear = SEASON_START_YEAR + 1;
            else if (m == 2)
                WCYear = SEASON_START_YEAR;

            patch::SetUInt(0xEA32ED + 1, WCYear);
            patch::SetUInt(0xEA4F6C + 1, WCYear - 1);


            patch::SetUInt(0x4DB5BF + 4, SEASON_START_YEAR - AGE_MAX);
            patch::SetUInt(0x4DB5CB + 4, SEASON_START_YEAR - AGE_MIN);
            patch::SetUInt(0xC44659 + 1, SEASON_START_YEAR - AGE_DEFAULT);

            patch::SetUInt(0x4BEB95 + 4, SEASON_START_YEAR - AGE_MAX);
            patch::SetUInt(0x4BEB49 + 1, SEASON_START_YEAR - AGE_DEFAULT);
            patch::SetUChar(0x4BEB8A + 2, AGE_MIN);

            if (FORCE_SEASON_START_DATE_RELOAD) {
                if (!patch::GetUInt(0x152DF80)) {
                    Warning("The start date was not initialized yet!\n"
                        "Start date reload is not needed.");
                }
                else {
                    patch::SetUInt(0x151F424, 0);
                    CallMethodDynGlobal(0xDD82F0, (void *)0x151F440);
                }
            }
        }
        else if (version == ID_FM_05_1010_C) {
            PluginLoaded();
            StartDate startDate = GetStartDate<false, 0x653B00, 0x653BB0>(6);
            patch::SetUChar(0x9A45D0 + 1, startDate.day);
            patch::SetUChar(0x9A45D2 + 1, startDate.month);
            patch::SetUInt(0x9A45D4 + 1, SEASON_START_YEAR);
            patch::SetUInt(0x4C56B7 + 4, SEASON_START_YEAR - AGE_MAX);
            patch::SetUInt(0x4C56BF + 4, SEASON_START_YEAR - AGE_MIN);
            patch::SetUInt(0x8F60FC + 1, SEASON_START_YEAR - AGE_DEFAULT);
            if (FORCE_SEASON_START_DATE_RELOAD) {
                Call<0x9A45D0>();
                CallMethod<0x6A41F0>(0xA8E638);
            }
        }
        else if (version == ID_FM_05_1000_C) {
            PluginLoaded();
            StartDate startDate = GetStartDate<false, 0x6536B0, 0x653760>(6);
            patch::SetUChar(0x9A3140 + 1, startDate.day);
            patch::SetUChar(0x9A3142 + 1, startDate.month);
            patch::SetUInt(0x9A3144 + 1, SEASON_START_YEAR);
            patch::SetUInt(0x4C52B7 + 4, SEASON_START_YEAR - AGE_MAX);
            patch::SetUInt(0x4C52BF + 4, SEASON_START_YEAR - AGE_MIN);
            patch::SetUInt(0x8F5ABC + 1, SEASON_START_YEAR - AGE_DEFAULT);
            if (FORCE_SEASON_START_DATE_RELOAD) {
                Call<0x9A3140>();
                CallMethod<0x6A3DA0>(0xA8C4C0);
            }
        }
        else if (version == ID_FM_04_1000_C) {
            PluginLoaded();
            StartDate startDate = GetStartDate<false, 0xA356F0, 0xA357A0>(6);
            patch::SetUChar(0xDAD070 + 1, startDate.day);
            patch::SetUChar(0xDAD072 + 1, startDate.month);
            patch::SetUInt(0xDAD074 + 1, SEASON_START_YEAR);
            patch::SetUInt(0x46500E + 1, SEASON_START_YEAR - AGE_MAX);
            patch::SetUInt(0x465009 + 1, SEASON_START_YEAR - AGE_MIN);
            patch::SetUInt(0xCD454C + 1, SEASON_START_YEAR - AGE_DEFAULT);
            if (FORCE_SEASON_START_DATE_RELOAD) {
                Call<0xDAD070>();
                CallMethod<0xA516A0>(0xEB1B78);
            }
        }
        else if (version == VERSION_ED_13) {

            PluginLoaded();

            SetManagerId(13);

            MarkYearWithValue(0x4BCE7C, 2012, "Game start date 1");
            MarkYearWithValue(0x4BCF2C, 2012, "Game start date 2 - for SOS scripts");
            MarkYearWithValue(0x422A65, 2012);
            MarkYearWithValue(0x422A87, 2012);
            MarkYearWithValue(0x422C6A, 2012);
            MarkYearWithValue(0x422C96, 2012);
            MarkYearWithValue(0x422CC4, 2012);
            MarkYearWithValue(0x422DE1, 2012, "Calendar start date");
            MarkYearWithValue(0x422FDA, 2012, "Calendar calculation date 1");
            MarkYearWithValue(0x42301B, 2012, "Calendar calculation date 2");
            MarkYearWithValue(0x423306, 2012, "Calendar calculation date 3");
            MarkYearWithValue(0x4234A3, 2012);
            MarkYearWithValue(0x4234CC, 2012);
            MarkYearWithValue(0x4236E6, 2012, "Calendar - season name - start");
            MarkYearWithValue(0x4236DA, 2013, "Calendar - season name - end");
            MarkYearWithValue(0x423978, 2012);
            MarkYearWithValue(0x4239BD, 2012);
            MarkYearWithValue(0x4242B5, 2012);
            MarkYearWithValue(0x424360, 2012);
            MarkYearWithValue(0x424512, 2012, "Calendar check - first season year");
            MarkYearWithValue(0x42452B, 2013, "Calendar check - second season year");
            MarkYearWithValue(0x4248C8, 2012);
            MarkYearWithValue(0x4248E8, 2012);
            MarkYearWithValue(0x429BE9, 2012, "Club foundation year - max year");
            MarkYearWithValue(0x437C49, 2012, "Sponsor contract - min year");
            MarkYearWithValue(0x437C5A, 2012, "Sponsor contract - default value");
            MarkYearWithValue(0x437F7F, 2012, "Sponsor contract calculation date 1");
            MarkYearWithValue(0x43877F, 2012, "Sponsor contract calculation date 2");
            MarkYearWithValue(0x46565A, 2012, "Manager joined club year - max");
            MarkYearWithValue(0x465665, 1913, "Manager joined club year - min");
            MarkYearWithValue(0x46DF67, 2012, "Adult/youth player min birthdate");
            MarkYearWithValue(0x46FDB2, 2012, "Buy-back clause - check");
            MarkYearWithValue(0x46FEB0, 2013, "Buy-back clause - default year");
            MarkYearWithValue(0x46FEDF, 2012, "Player contract - this year");
            MarkYearWithValue(0x46FF00, 2013, "Player contract - other years");
            MarkYearWithValue(0x46FF3E, 2013, "Player buy-back clause starting year");
            MarkYearWithValue(0x471048, 2012, "Player SOS - injury start");
            MarkYearWithValue(0x47105A, 2012, "Player SOS - injury end");
            MarkYearWithValue(0x471082, 2012, "Player SOS - loan start date");
            MarkYearWithValue(0x471094, 2013, "Player SOS - loan/future loan end date");
            MarkYearWithValue(0x4710AC, 2013, "Player SOS - future transfer start date");
            MarkYearWithValue(0x4710BE, 2014, "Player SOS - future transfer end date");
            MarkYearWithValue(0x4710D6, 2013, "Player SOS - future loan start date");
            MarkYearWithValue(0x4710E1, 2012, "Player SOS - ban until date");
            MarkYearWithValue(0x4710F4, 2013, "Player SOS - future join/leave date");
            MarkYearWithValue(0x470328, 2007, "Player SOS - start min date");
            MarkYearWithValue(0x470338, 2015, "Player SOS - start max date");
            MarkYearWithValue(0x470359, 2010, "Player SOS - end min date");
            MarkYearWithValue(0x470369, 2021, "Player SOS - end max date");
            MarkYearWithValue(0x4790DE, 2012, "Player history - joined club default value");
            MarkYearWithValue(0x4790EC, 2012, "Player history - left club default value");
            MarkYearWithValue(0x479D4E, 2012, "Player history - entry joined date");
            MarkYearWithValue(0x479D60, 2013, "Player history - entry left date");
            MarkYearWithValue(0x47A5A5, 2012, "Player list sorting - random contract check");
            MarkYearWithValue(0x47A5CE, 2012, "Player list sorting - random contract check");
            MarkYearWithValue(0x47AEC2, 2012, "Player list sorting - '?' symbol - 1");
            MarkYearWithValue(0x480AB8, 2012, "Player list sorting - '?' symbol - 2");
            MarkYearWithValue(0x47A5A5, 2018, "Player complete list sorting - 1 - random contract check");
            MarkYearWithValue(0x47A5CE, 2018, "Player complete list sorting - 1 - random contract check");
            MarkYearWithValue(0x481186, 2012, "Player complete list sorting - 2 - random contract check");
            MarkYearWithValue(0x4811AE, 2012, "Player complete list sorting - 2 - random contract check");
            MarkYearWithValue(0x48EE9F, 2012);
            MarkYearWithValue(0x48F468, 2022, "Select player dialog - contract end max year");
            MarkYearWithValue(0x48F46D, 2012, "Select player dialog - contract end min year");
            MarkYearWithValue(0x48F488, 2012, "Select player dialog - contract end default value 1");
            MarkYearWithValue(0x48F494, 2013, "Select player dialog - contract end default value 2");
            MarkYearWithValue(0x4A58FA, 2012);
            MarkYearWithValue(0x4F85F7, 2012, "DatabaseProblemTool - EndOfLoanIsDuration");
            MarkYearWithValue(0x4F8A1B, 2012, "DatabaseProblemTool - PlayerInvalidHistoryDate");
            MarkYearWithValue(0x505E63, 2012);
            MarkYearWithValue(0x521E8F, 2012, "Player - writing career entry to binary db");
            MarkYearWithValue(0x527D5B, 2013, "Player transfer - new career entry end date for new club");
            MarkYearWithValue(0x527D77, 2012, "Player transfer - new career entry end date for previous club");
            MarkYearWithValue(0x534493, 2012);
            MarkYearWithValue(0x53C726, 2012, "Process transfers hotfix - 1");
            MarkYearWithValue(0x53DA2C, 2012, "Process transfers hotfix - 2");
            MarkYearWithValue(0x559592, 2012, "Show contract players hotfix");
            MarkYearWithValue(0x57043F, 2012, "Staff generator");
            MarkYearWithValue(0x649905, 2012, "Season end date");
            MarkYearWithValue(0x4A0576, 2013);
            MarkYearWithValue(0x4F9514, 2013, "Calendar - ErrorFixturelist");
            MarkYearWithValue(0x528519, 2013);
            MarkYearWithValue(0x5339AB, 2013, "Player contract init");
            MarkYearWithValue(0x51BD71, 2015, "Future loan/reloan check");
            MarkYearWithValue(0x50BEA6, 2011, "Assitant - CheckStartOfContract");
            MarkYearWithValue(0x46CFCA, 1937, "Player birthdate - min");
            MarkYearWithValue(0x46CFDA, 2001, "Player birthdate - max");
            MarkYearWithValue(0x44EB6F, 1912, "Chairman birthdate - min");
            MarkYearWithValue(0x44EB7F, 2000, "Chairman birthdate - max");
            MarkYearWithValue(0x47786E, 1994, "Youth player birthdate - min");
            MarkYearWithValue(0x47789B, 2000, "Youth player birthdate - max");
            MarkYearWithValue(0x4783B8, 1940, "Player generator birthdate - min");
            MarkYearWithValue(0x4783C6, 1994, "Player generator youth birthdate - min");
            MarkYearWithValue(0x478417, 2000, "Player generator birthdate - max");
            MarkYearWithValue(0x5352A5, 1997, "CAC Player default init - 1");
            MarkYearWithValue(0x5352F4, 1997, "CAC Player default init - 2");
            MarkYearWithValue(0x489CAC, 1995, "CAC Player birthdate - max");
            MarkYearWithValue(0x489CB7, 1952, "CAC Player birthdate - min");
            MarkYearWithValue(0x4C3684, 212, "Winning years - check value 1");
            MarkYearWithValue<char>(0x4319AD, 13, "Winning years - short year version check value");
            MarkYearWithValue(0x4319C5, 313, "Winning years - check value 2");

            SetYearFixedValue<int>(0x437C3D + 1, SEASON_START_YEAR + 100, "Sponsor contract - max year");
            SetYearFixedValue<int>(0x4794E3 + 1, SEASON_START_YEAR - 50, "Player history - joined entry min year");
            SetYearFixedValue<int>(0x479515 + 1, SEASON_START_YEAR - 50, "Player history - left entry min year");
            SetYearFixedValue<int>(0x4795C7 + 1, SEASON_START_YEAR - 50, "Player joined club year - min year");
            //SetYearFixedValue<int>(0x504EDD + 1, (SEASON_START_YEAR - IS_ODD_YEAR), "International league matchdays check - year 1");
            //SetYearFixedValue<int>(0x504EED + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 2, "International league matchdays check - year 2");
            //SetYearFixedValue<int>(0x504F07 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 1, "International league matchdays check - year 3");
            //SetYearFixedValue<int>(0x504F17 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 3, "International league matchdays check - year 4");
            //SetYearFixedValue<int>(0x5055CB + 1, (SEASON_START_YEAR - IS_ODD_YEAR), "International round matchdays check - year 1");
            //SetYearFixedValue<int>(0x5055DB + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 2, "International round matchdays check - year 2");
            //SetYearFixedValue<int>(0x5055F1 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 1, "International round matchdays check - year 3");
            //SetYearFixedValue<int>(0x505601 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 3, "International round matchdays check - year 4");
            SetYearFixedValue<int>(0x494DF1 + 1, SEASON_START_YEAR - 100, "Coach birthdate - min");
            SetYearFixedValue<int>(0x494E01 + 1, SEASON_START_YEAR - 10, "Coach birthdate - max");

            //if (IS_ODD_YEAR) // not sure if we need this (red calendar for WC)
            //    patch::SetUChar(0x423075 + 2, 0);
            //else
            //    patch::SetUChar(0x423075 + 2, 1);
            patch::SetUChar(0x46DF63 + 2, 100); // person age
            if (!DISABLE_ADDITIONAL_PATCHES) {
                patch::SetUInt(0x429C26 + 1, 1'000'000'000); // initial capital
                patch::SetUInt(0x429C31 + 1, -1'000'000'000); // initial capital
                patch::SetUInt(0x429C62 + 1, 1'000'000); // fan base
                patch::SetUChar(0x465151 + 1, 20); // manager skill
            }
            if (patch::GetUInt(0x414160) == 0x4C2) // Editor title
                patch::SetUInt(0x414160, 0x0200EC81);
            SetWindowTitleAddr = patch::RedirectCall(0x4141D9, SetEditorWindowTitle);
        }
        else if (version == VERSION_ED_14) {
            PluginLoaded();

            SetManagerId(14);

            MarkYearWithValue(0x4BE38C, 2013, "Game start date 1");
            MarkYearWithValue(0x4BE43C, 2013, "Game start date 2 - for SOS scripts");
            MarkYearWithValue(0x422A65, 2013);
            MarkYearWithValue(0x422A87, 2013);
            MarkYearWithValue(0x422C6A, 2013);
            MarkYearWithValue(0x422C96, 2013);
            MarkYearWithValue(0x422CC4, 2013);
            MarkYearWithValue(0x422DE1, 2013, "Calendar start date");
            MarkYearWithValue(0x422FDA, 2013, "Calendar calculation date 1");
            MarkYearWithValue(0x42301B, 2013, "Calendar calculation date 2");
            MarkYearWithValue(0x4232FF, 2013, "Calendar calculation date 3");
            MarkYearWithValue(0x4234A3, 2013);
            MarkYearWithValue(0x4234CC, 2013);
            MarkYearWithValue(0x4236E6, 2013, "Calendar - season name - start");
            MarkYearWithValue(0x4236DA, 2014, "Calendar - season name - end");
            MarkYearWithValue(0x423978, 2013);
            MarkYearWithValue(0x4239BD, 2013);
            MarkYearWithValue(0x4242B5, 2013);
            MarkYearWithValue(0x424360, 2013);
            MarkYearWithValue(0x424512, 2013, "Calendar check - first season year");
            MarkYearWithValue(0x42452B, 2014, "Calendar check - second season year");
            MarkYearWithValue(0x4248C8, 2013);
            MarkYearWithValue(0x4248E8, 2013);
            MarkYearWithValue(0x429E39, 2013, "Club foundation year - max year");
            MarkYearWithValue(0x429FE6, 2013, "Sponsor contract - min year");
            MarkYearWithValue(0x429FF7, 2013, "Sponsor contract - default value");
            MarkYearWithValue(0x42A5CE, 2013, "Sponsor contract calculation date 1");
            MarkYearWithValue(0x42B841, 2013, "Sponsor contract calculation date 2");
            MarkYearWithValue(0x4658CA, 2013, "Manager joined club year - max");
            MarkYearWithValue(0x4658D5, 1914, "Manager joined club year - min");
            MarkYearWithValue(0x46E2E7, 2013, "Adult/youth player min birthdate");
            MarkYearWithValue(0x470202, 2013, "Buy-back clause - check");
            MarkYearWithValue(0x470300, 2014, "Buy-back clause - default year");
            MarkYearWithValue(0x47032F, 2013, "Player contract - this year");
            MarkYearWithValue(0x470350, 2014, "Player contract - other years");
            MarkYearWithValue(0x47038E, 2014, "Player buy-back clause starting year");
            MarkYearWithValue(0x471498, 2013, "Player SOS - injury start");
            MarkYearWithValue(0x4714AA, 2013, "Player SOS - injury end");
            MarkYearWithValue(0x4714D2, 2013, "Player SOS - loan start date");
            MarkYearWithValue(0x4714E4, 2014, "Player SOS - loan/future loan end date");
            MarkYearWithValue(0x4714FC, 2014, "Player SOS - future transfer start date");
            MarkYearWithValue(0x47150E, 2015, "Player SOS - future transfer end date");
            MarkYearWithValue(0x471526, 2014, "Player SOS - future loan start date");
            MarkYearWithValue(0x471531, 2013, "Player SOS - ban until date");
            MarkYearWithValue(0x471544, 2014, "Player SOS - future join/leave date");
            MarkYearWithValue(0x470778, 2008, "Player SOS - start min date");
            MarkYearWithValue(0x470788, 2016, "Player SOS - start max date");
            MarkYearWithValue(0x4707A9, 2011, "Player SOS - end min date");
            MarkYearWithValue(0x4707B9, 2022, "Player SOS - end max date");
            MarkYearWithValue(0x479A7E, 2013, "Player history - joined club default value");
            MarkYearWithValue(0x479A8C, 2013, "Player history - left club default value");
            MarkYearWithValue(0x47A6EE, 2013, "Player history - entry start date");
            MarkYearWithValue(0x47A700, 2014, "Player history - entry end date");
            MarkYearWithValue(0x47B0B5, 2013, "Player list sorting - random contract check");
            MarkYearWithValue(0x47B0DE, 2013, "Player list sorting - random contract check");
            MarkYearWithValue(0x47BA82, 2013, "Player list sorting - '?' symbol - 1");
            MarkYearWithValue(0x4819F8, 2013, "Player list sorting - '?' symbol - 2");
            MarkYearWithValue(0x47E1A3, 2013, "FM14 Player transfer - 1 - add loan SOS if player is loaned - career check date");
            MarkYearWithValue(0x47E20B, 2013, "FM14 Player transfer - 1 - add loan SOS if player is loaned - loan start date");
            MarkYearWithValue(0x47E21D, 2014, "FM14 Player transfer - 1 - add loan SOS if player is loaned - loan end date");
            MarkYearWithValue(0x48564A, 2013, "FM14 Player transfer - 2 - add loan SOS if player is loaned - career check date");
            MarkYearWithValue(0x4856AE, 2013, "FM14 Player transfer - 2 - add loan SOS if player is loaned - loan start date");
            MarkYearWithValue(0x4856C0, 2014, "FM14 Player transfer - 2 - add loan SOS if player is loaned - loan end date");
            MarkYearWithValue(0x494982, 2013, "FM14 Player transfer - 3 - add loan SOS if player is loaned - career check date");
            MarkYearWithValue(0x4949E2, 2013, "FM14 Player transfer - 3 - add loan SOS if player is loaned - loan start date");
            MarkYearWithValue(0x4949F4, 2014, "FM14 Player transfer - 3 - add loan SOS if player is loaned - loan end date");
            MarkYearWithValue(0x4820C6, 2013, "Player complete list sorting - random contract check");
            MarkYearWithValue(0x4820EE, 2013, "Player complete list sorting - random contract check");
            MarkYearWithValue(0x4902BF, 2013);
            MarkYearWithValue(0x490888, 2023, "Select player dialog - contract end max year");
            MarkYearWithValue(0x49088D, 2013, "Select player dialog - contract end min year");
            MarkYearWithValue(0x4908A8, 2013, "Select player dialog - contract end default value 1");
            MarkYearWithValue(0x4908B4, 2014, "Select player dialog - contract end default value 2");
            MarkYearWithValue(0x4A6E0A, 2013);
            MarkYearWithValue(0x4F9DA7, 2013, "DatabaseProblemTool - EndOfLoanIsDuration");
            MarkYearWithValue(0x4FA2BB, 2013, "DatabaseProblemTool - PlayerInvalidHistoryDate");
            MarkYearWithValue(0x507803, 2013);
            MarkYearWithValue(0x5238AF, 2013, "Player - writing career entry to binary db");
            MarkYearWithValue(0x529759, 2013, "Player transfer - new career entry end date for previous club");
            MarkYearWithValue(0x535F93, 2013);
            MarkYearWithValue(0x53E265, 2013, "Process transfers hotfix - 1");
            MarkYearWithValue(0x53F77C, 2013, "Process transfers hotfix - 2");
            MarkYearWithValue(0x55B2E2, 2013, "Show contract players hotfix");
            MarkYearWithValue(0x55BB5C, 2013);
            MarkYearWithValue(0x5722CF, 2013, "Staff generator");
            MarkYearWithValue(0x64B8A5, 2013, "Season end date");
            MarkYearWithValue(0x4A1A86, 2014);
            MarkYearWithValue(0x4FAEB4, 2014, "Calendar - ErrorFixturelist");
            MarkYearWithValue(0x52A019, 2014);
            MarkYearWithValue(0x5354AB, 2014, "Player contract init");
            MarkYearWithValue(0x51D711, 2016, "Future loan/reloan check");
            MarkYearWithValue(0x50D846, 2012, "Assitant - CheckStartOfContract");
            MarkYearWithValue(0x46D33A, 1938, "Player birthdate - min");
            MarkYearWithValue(0x46D34A, 2002, "Player birthdate - max");
            MarkYearWithValue(0x44ECBF, 1913, "Chairman birthdate - min");
            MarkYearWithValue(0x44ECCF, 2001, "Chairman birthdate - max");
            MarkYearWithValue(0x47802E, 1995, "Youth player age generator - min date");
            MarkYearWithValue(0x47805B, 2001, "Youth player age generator - max date");
            MarkYearWithValue(0x478C56, 1941, "Player generator birthdate - min");
            MarkYearWithValue(0x478C67, 1995, "Player generator youth birthdate - min");
            MarkYearWithValue(0x478CBC, 2001, "Player generator birthdate - max");
            MarkYearWithValue(0x536DA5, 1998, "CAC Player default init - 1");
            MarkYearWithValue(0x536DF4, 1998, "CAC Player default init - 2");
            MarkYearWithValue(0x48B06C, 1996, "CAC Player birthdate - max");
            MarkYearWithValue(0x48B077, 1953, "CAC Player birthdate - min");
            MarkYearWithValue(0x4C4BA4, 213, "Winning years - check value 1");
            MarkYearWithValue<char>(0x431AFD, 14, "Winning years - short year version check value");
            MarkYearWithValue(0x431B15, 314, "Winning years - check value 2");

            SetYearFixedValue<int>(0x429FDA + 1, SEASON_START_YEAR + 100, "Sponsor contract - max year");
            SetYearFixedValue<int>(0x479E83 + 1, SEASON_START_YEAR - 50, "Player history - joined entry min year");
            SetYearFixedValue<int>(0x479EB5 + 1, SEASON_START_YEAR - 50, "Player history - left entry min year");
            SetYearFixedValue<int>(0x479F67 + 1, SEASON_START_YEAR - 50, "Player joined club year - min year");
            //SetYearFixedValue<int>(0x50687D + 1, (SEASON_START_YEAR - IS_ODD_YEAR), "International league matchdays check - year 1");
            //SetYearFixedValue<int>(0x50688D + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 2, "International league matchdays check - year 2");
            //SetYearFixedValue<int>(0x5068A7 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 1, "International league matchdays check - year 3");
            //SetYearFixedValue<int>(0x5068B7 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 3, "International league matchdays check - year 4");
            //SetYearFixedValue<int>(0x506F6B + 1, (SEASON_START_YEAR - IS_ODD_YEAR), "International round matchdays check - year 1");
            //SetYearFixedValue<int>(0x506F7B + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 2, "International round matchdays check - year 2");
            //SetYearFixedValue<int>(0x506F91 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 1, "International round matchdays check - year 3");
            //SetYearFixedValue<int>(0x506FA1 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 3, "International round matchdays check - year 4");
            SetYearFixedValue<int>(0x496301 + 1, SEASON_START_YEAR - 100, "Coach birthdate - min");
            SetYearFixedValue<int>(0x496311 + 1, SEASON_START_YEAR - 10, "Coach birthdate - max");
            
            patch::SetUChar(0x46E2E3 + 2, 100); // person age
            if (!DISABLE_ADDITIONAL_PATCHES) {
                patch::SetUInt(0x429E76 + 1, 1'000'000'000); // initial capital
                patch::SetUInt(0x429E81 + 1, -1'000'000'000); // initial capital
                patch::SetUInt(0x429EB2 + 1, 1'000'000); // fan base
                patch::SetUChar(0x4653C1 + 1, 20); // manager skill
            }
            if (patch::GetUInt(0x414160) == 0x4C2) // Editor title
                patch::SetUInt(0x414160, 0x0200EC81);
            SetWindowTitleAddr = patch::RedirectCall(0x4141D9, SetEditorWindowTitle);
        }
        else if (version == VERSION_ED_12) {
            /*
            PluginLoaded();

            SetManagerId(12);

            MarkYear(0x49F8CD + 1, "Game start date 1");
            MarkYear(0x49F97D + 1, "Game start date 2 - for SOS scripts");
            //MarkYear(0x422A65);
            //MarkYear(0x422A87);
            //MarkYear(0x422C6A);
            //MarkYear(0x422C96);
            //MarkYear(0x422CC4);
            //MarkYear(0x422DE1, "Calendar start date");
            //MarkYear(0x422FDA, "Calendar calculation date 1");
            //MarkYear(0x42301B, "Calendar calculation date 2");
            //MarkYear(0x423306, "Calendar calculation date 3");
            //MarkYear(0x4234A3);
            //MarkYear(0x4234CC);
            //MarkYear(0x4236E6, "Calendar - season name - start");
            //MarkYear(0x4236DA, "Calendar - season name - end");
            //MarkYear(0x423978);
            //MarkYear(0x4239BD);
            //MarkYear(0x4242B5);
            //MarkYear(0x424360);
            //MarkYear(0x424512, "Calendar check - first season year");
            //MarkYear(0x42452B, "Calendar check - second season year");
            //MarkYear(0x4248C8);
            //MarkYear(0x4248E8);
            //MarkYear(0x429BE9, "Club foundation year - max year");
            //MarkYear(0x437C49, "Sponsor contract - min year");
            //MarkYear(0x437C5A, "Sponsor contract - default value");
            //MarkYear(0x437F7F, "Sponsor contract calculation date 1");
            //MarkYear(0x43877F, "Sponsor contract calculation date 2");
            //MarkYear(0x46565A, "Manager joined club year - max");
            //MarkYear(0x465665, "Manager joined club year - min");
            //MarkYear(0x46DF67, "Adult/youth player min birthdate");
            //MarkYear(0x46FDB2, "Buy-back clause - check");
            //MarkYear(0x46FEB0, "Buy-back clause - default year");
            //MarkYear(0x46FEDF, "Player contract - this year");
            //MarkYear(0x46FF00, "Player contract - other years");
            //MarkYear(0x46FF3E, "Player buy-back clause starting year");
            //MarkYear(0x471048, "Player SOS - injury start");
            //MarkYear(0x47105A, "Player SOS - injury end");
            //MarkYear(0x471082, "Player SOS - loan start date");
            //MarkYear(0x471094, "Player SOS - loan/future loan end date");
            //MarkYear(0x4710AC, "Player SOS - future transfer start date");
            //MarkYear(0x4710BE, "Player SOS - future transfer end date");
            //MarkYear(0x4710D6, "Player SOS - future loan start date");
            //MarkYear(0x4710E1, "Player SOS - ban until date");
            //MarkYear(0x4710F4, "Player SOS - future join/leave date");
            //MarkYear(0x470328, "Player SOS - start min date");
            //MarkYear(0x470338, "Player SOS - start max date");
            //MarkYear(0x470359, "Player SOS - end min date");
            //MarkYear(0x470369, "Player SOS - end max date");
            //MarkYear(0x4790DE, "Player history - joined club default value");
            //MarkYear(0x4790EC, "Player history - left club default value");
            //MarkYear(0x479D4E, "Player history - entry joined date");
            //MarkYear(0x479D60, "Player history - entry left date");
            //MarkYear(0x47A5A5, "Player list sorting - random contract check");
            //MarkYear(0x47A5CE, "Player list sorting - random contract check");
            //MarkYear(0x47AEC2, "Player list sorting - '?' symbol - 1");
            //MarkYear(0x480AB8, "Player list sorting - '?' symbol - 2");
            //MarkYear(0x47A5A5, "Player complete list sorting - 1 - random contract check");
            //MarkYear(0x47A5CE, "Player complete list sorting - 1 - random contract check");
            //MarkYear(0x481186, "Player complete list sorting - 2 - random contract check");
            //MarkYear(0x4811AE, "Player complete list sorting - 2 - random contract check");
            //MarkYear(0x48EE9F);
            //MarkYear(0x48F468, "Select player dialog - contract end max year");
            //MarkYear(0x48F46D, "Select player dialog - contract end min year");
            //MarkYear(0x48F488, "Select player dialog - contract end default value 1");
            //MarkYear(0x48F494, "Select player dialog - contract end default value 2");
            //MarkYear(0x4A58FA);
            //MarkYear(0x4F85F7, "DatabaseProblemTool - EndOfLoanIsDuration");
            //MarkYear(0x4F8A1B, "DatabaseProblemTool - PlayerInvalidHistoryDate");
            //MarkYear(0x505E63);
            //MarkYear(0x521E8F, "Player - writing career entry to binary db");
            //MarkYear(0x527D5B, "Player transfer - new career entry end date for new club");
            //MarkYear(0x527D77, "Player transfer - new career entry end date for previous club");
            //MarkYear(0x534493);
            //MarkYear(0x53C726, "Process transfers hotfix - 1");
            //MarkYear(0x53DA2C, "Process transfers hotfix - 2");
            //MarkYear(0x559592, "Show contract players hotfix");
            //MarkYear(0x57043F, "Staff generator");
            //MarkYear(0x649905, "Season end date");
            //MarkYear(0x4A0576);
            //MarkYear(0x4F9514, "Calendar - ErrorFixturelist");
            //MarkYear(0x528519);
            //MarkYear(0x5339AB, "Player contract init");
            //MarkYear(0x51BD71, "Future loan/reloan check");
            //MarkYear(0x50BEA6, "Assitant - CheckStartOfContract");
            //MarkYear(0x46CFCA, "Player birthdate - min");
            //MarkYear(0x46CFDA, "Player birthdate - max");
            //MarkYear(0x44EB6F, "Chairman birthdate - min");
            //MarkYear(0x44EB7F, "Chairman birthdate - max");
            //MarkYear(0x47786E, "Youth player birthdate - min");
            //MarkYear(0x47789B, "Youth player birthdate - max");
            //MarkYear(0x4783B8, "Player generator birthdate - min");
            //MarkYear(0x4783C6, "Player generator youth birthdate - min");
            //MarkYear(0x478417, "Player generator birthdate - max");
            //MarkYear(0x5352A5, "CAC Player default init - 1");
            //MarkYear(0x5352F4, "CAC Player default init - 2");
            //MarkYear(0x489CAC, "CAC Player birthdate - max");
            //MarkYear(0x489CB7, "CAC Player birthdate - min");
            //MarkYear(0x4C3684, "Winning years - check value 1");
            //MarkYear<char>(0x4319AD, "Winning years - short year version check value");
            //MarkYear(0x4319C5, "Winning years - check value 2");

            //SetYearFixedValue<int>(0x437C3D + 1, SEASON_START_YEAR + 100, "Sponsor contract - max year");
            //SetYearFixedValue<int>(0x4794E3 + 1, SEASON_START_YEAR - 50, "Player history - joined entry min year");
            //SetYearFixedValue<int>(0x479515 + 1, SEASON_START_YEAR - 50, "Player history - left entry min year");
            //SetYearFixedValue<int>(0x4795C7 + 1, SEASON_START_YEAR - 50, "Player joined club year - min year");
            //SetYearFixedValue<int>(0x504EDD + 1, (SEASON_START_YEAR - IS_ODD_YEAR), "International league matchdays check - year 1");
            //SetYearFixedValue<int>(0x504EED + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 2, "International league matchdays check - year 2");
            //SetYearFixedValue<int>(0x504F07 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 1, "International league matchdays check - year 3");
            //SetYearFixedValue<int>(0x504F17 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 3, "International league matchdays check - year 4");
            //SetYearFixedValue<int>(0x5055CB + 1, (SEASON_START_YEAR - IS_ODD_YEAR), "International round matchdays check - year 1");
            //SetYearFixedValue<int>(0x5055DB + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 2, "International round matchdays check - year 2");
            //SetYearFixedValue<int>(0x5055F1 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 1, "International round matchdays check - year 3");
            //SetYearFixedValue<int>(0x505601 + 1, (SEASON_START_YEAR - IS_ODD_YEAR) + 3, "International round matchdays check - year 4");
            //SetYearFixedValue<int>(0x494DF1 + 1, SEASON_START_YEAR - 100, "Coach birthdate - min");
            //SetYearFixedValue<int>(0x494E01 + 1, SEASON_START_YEAR - 10, "Coach birthdate - max");

            //patch::SetUChar(0x46DF63 + 2, 100); // person age
            //if (!DISABLE_ADDITIONAL_PATCHES) {
            //    patch::SetUInt(0x429C26 + 1, 1'000'000'000); // initial capital
            //    patch::SetUInt(0x429C31 + 1, -1'000'000'000); // initial capital
            //    patch::SetUInt(0x429C62 + 1, 1'000'000); // fan base
            //    patch::SetUChar(0x465151 + 1, 20); // manager skill
            //}
            SetWindowTitleAddr = patch::RedirectCall(0x410237, SetEditorWindowTitle);
            */
        }
        else if (version == ID_ED_11_1003) {

            PluginLoaded();

            SetManagerId(11);

            MarkYearWithValue(0x41CEB1, 2010);
            MarkYearWithValue(0x41D6B3, 2010);
            MarkYearWithValue(0x422D99, 2010);
            MarkYearWithValue(0x4567EA, 2010);
            MarkYearWithValue(0x45C04E, 2010);
            MarkYearWithValue(0x45E3BA, 2010);
            MarkYearWithValue(0x45E3CC, 2010);
            MarkYearWithValue(0x45E3F4, 2010);
            MarkYearWithValue(0x45E453, 2010);
            MarkYearWithValue(0x4639CF, 2010);
            MarkYearWithValue(0x4639DD, 2010);
            MarkYearWithValue(0x4645AD, 2010);
            MarkYearWithValue(0x4645C8, 2010);
            MarkYearWithValue(0x46A923, 2010);
            MarkYearWithValue(0x4781CD, 2010);
            MarkYearWithValue(0x4781E8, 2010);
            MarkYearWithValue(0x48D84B, 2010);
            MarkYearWithValue(0x49D2CE, 2010);
            MarkYearWithValue(0x4C5907, 2010);
            MarkYearWithValue(0x4C5C84, 2010);
            MarkYearWithValue(0x4D4F83, 2010);
            MarkYearWithValue(0x4D9F8B, 2010);
            MarkYearWithValue(0x4E410E, 2010);
            MarkYearWithValue(0x4E9E57, 2010);
            MarkYearWithValue(0x4EC9D5, 2010);
            MarkYearWithValue(0x4ECB51, 2010);
            MarkYearWithValue(0x4FD1A4, 2010);
            MarkYearWithValue(0x4FE559, 2010);
            MarkYearWithValue(0x504101, 2010);
            MarkYearWithValue(0x513BC2, 2010);
            MarkYearWithValue(0x52279C, 2010);
            MarkYearWithValue(0x429725, 2011);
            MarkYearWithValue(0x45E3FF, 2011);
            MarkYearWithValue(0x45E429, 2011);
            MarkYearWithValue(0x45E43B, 2011);
            MarkYearWithValue(0x45E462, 2011);
            MarkYearWithValue(0x4781F4, 2011);
            MarkYearWithValue(0x4C34C0, 2011);
            MarkYearWithValue(0x4EC9F4, 2011);
            MarkYearWithValue(0x45E411, 2012);
            MarkYearWithValue(0x45D769, 2013);
            MarkYearWithValue(0x45D7A2, 2019);
            MarkYearWithValue(0x4E6D38, 2020);
            MarkYearWithValue(0x4FEDF6, 2009);
            MarkYearWithValue(0x5146E8, 2009);
            MarkYearWithValue(0x514722, 2009);
            MarkYearWithValue(0x45D792, 2008);
            MarkYearWithValue(0x45D759, 2005);
            MarkYearWithValue(0x45B504, 1935);
            MarkYearWithValue(0x45B514, 1999);
            MarkYearWithValue(0x44351A, 1998);
            MarkYearWithValue(0x4626F6, 1998);
            MarkYearWithValue(0x462CE4, 1998);
            MarkYearWithValue(0x4FCFD6, 1995);
            MarkYearWithValue(0x4FD07E, 1995);
            MarkYearWithValue(0x4626C9, 1992);
            MarkYearWithValue(0x462CC9, 1992);
            MarkYearWithValue(0x4E4630, 1992);
            MarkYearWithValue(0x41E489, 2010);
            MarkYearWithValue(0x464C96, 2010);
            MarkYearWithValue(0x464CB8, 2010);
            MarkYearWithValue(0x46A900, 2010);
            MarkYearWithValue(0x4A3328, 2010);
            MarkYearWithValue(0x41D6AD, 2011);
            MarkYearWithValue(0x41E482, 2011);
            MarkYearWithValue<short>(0x4653B5, 2010);
            MarkYearWithValue<short>(0x4659D3, 2010);
            MarkYearWithValue<short>(0x46A2F0, 2010);
            MarkYearWithValue(0x4D8842, 2009);
            MarkYearWithValue(0x477C3D, 2010);
            MarkYearWithValue<short>(0x4C5D01, 2010);
            MarkYearWithValue<short>(0x4EABA5, 2010);
            MarkYearWithValue<short>(0x4EC293, 2010);
            MarkYearWithValue<short>(0x4EC987, 2010);
            MarkYearWithValue<short>(0x4ECB60, 2010);
            MarkYearWithValue<short>(0x4EAB9C, 2011);
            MarkYearWithValue<short>(0x4ECB69, 2011);
            MarkYearWithValue<char>(0x42970D, 11);


            SetYearFixedValue<int>(0x4781C7 + 1, SEASON_START_YEAR + 100, "Sponsor contract - max year");
            SetYearFixedValue<int>(0x463DC3 + 1, SEASON_START_YEAR - 50);
            SetYearFixedValue<int>(0x463DF3 + 1, SEASON_START_YEAR - 50);
            SetYearFixedValue<int>(0x463EA1 + 1, SEASON_START_YEAR - 50);
            SetYearFixedValue<int>(0x47DA64 + 1, SEASON_START_YEAR - 100, "Coach birthdate - min");
            SetYearFixedValue<int>(0x47DA74 + 1, SEASON_START_YEAR - 10, "Coach birthdate - max");

            patch::SetUChar(0x45C057 + 2, 100); // person age
            if (!DISABLE_ADDITIONAL_PATCHES) {
                patch::SetUInt(0x422DD6 + 1, 1'000'000'000); // initial capital
                patch::SetUInt(0x422DE1 + 1, -1'000'000'000); // initial capital
                patch::SetUInt(0x422E12 + 1, 1'000'000); // fan base
                patch::SetUChar(0x455D61 + 1, 20); // manager skill
            }
            SetWindowTitleAddr = patch::RedirectCall(0x4105B7, SetEditorWindowTitle);
        }
        else if (version == ID_FM_08_1020_C || version == ID_FM_08_1020) {

            PluginLoaded();
            patch::SetUChar(0xEE44BD + 1, SEASON_START_DAY);
            patch::SetUChar(0xEE44BF + 1, SEASON_START_MONTH);
            patch::SetUInt(0xEE44C1 + 1, SEASON_START_YEAR);
            
            patch::SetUInt(0x4BF15B + 4, SEASON_START_YEAR - AGE_MAX);
            patch::SetUInt(0x4BF163 + 4, SEASON_START_YEAR - AGE_MIN);
            patch::SetUInt(0xDFFAFF + 1, SEASON_START_YEAR - AGE_DEFAULT);

            patch::SetUInt(0x4AB742 + 4, SEASON_START_YEAR - AGE_MAX);
            patch::SetUInt(0x4AB6F4 + 1, SEASON_START_YEAR - AGE_DEFAULT);
            patch::SetUChar(0x4AB735 + 2, AGE_MIN);

            if (FORCE_SEASON_START_DATE_RELOAD) {
                Call<0xEE44BA>();
                void *game = CallAndReturn<void *, 0xA1DA2A>();
                CallMethod<0xA1DBEC>(game);
            }
        }
        else if (version == ID_FM_07_1000_C) {

            PluginLoaded();
            StartDate startDate = GetStartDate<false, 0xAD49FA, 0xAD4B19>(6);
            patch::SetUChar(0xC12D68 + 1, startDate.day);
            patch::SetUChar(0xC12D6A + 1, startDate.month);
            patch::SetUInt(0xC12D6C + 1, SEASON_START_YEAR);
            
            patch::SetUInt(0x4B364B + 4, SEASON_START_YEAR - AGE_MAX);
            patch::SetUInt(0x4B3653 + 4, SEASON_START_YEAR - AGE_MIN);
            patch::SetUInt(0xB6369E + 1, SEASON_START_YEAR - AGE_DEFAULT);

            patch::SetUInt(0x4A0B63 + 4, SEASON_START_YEAR - AGE_MAX);
            patch::SetUInt(0x4A0B19 + 1, SEASON_START_YEAR - AGE_DEFAULT);
            patch::SetUChar(0x4A0B5B + 2, AGE_MIN);

            if (FORCE_SEASON_START_DATE_RELOAD) {
                Call<0xC12D65>();
                void *game = CallAndReturn<void *, 0x81398B>();
                CallMethod<0x813AD4>(game);
            }
        }
        else if (version == ID_ED_08_8071) {
            PluginLoaded();
            SetManagerId(8);
            patch::SetUInt(0x48C8EA + 1, SEASON_START_YEAR);

            MarkYearWithValue(0x430594, 1907, "Player age 1");
            MarkYearWithValue(0x440AF8, 1932, "Player age 1");
            MarkYearWithValue(0x440ED4, 1932, "Unknown");
            MarkYearWithValue(0x447460, 1935, "Player generator");
            MarkYearWithValue(0x4522C9, 1947, "Player pool change");
            MarkYearWithValue(0x446F19, 1989, "Regen age");
            MarkYearWithValue(0x44746E, 1989, "Player generator");
            MarkYearWithValue(0x440ECD, 1989, "Unknown");
            MarkYearWithValue(0x4522BE, 1990, "Player pool change");
            MarkYearWithValue(0x4499F5, 1991, "Player list");
            MarkYearWithValue(0x49FFBD, 1992, "Unknown constructor");
            MarkYearWithValue(0x4A003C, 1992, "Unknown constructor");
            MarkYearWithValue(0x4305A4, 1995, "Player age 2");
            MarkYearWithValue(0x440B08, 1995, "Player age 2");
            MarkYearWithValue(0x446F46, 1995, "Regen age");
            MarkYearWithValue(0x447489, 1995, "Player generator");
            MarkYearWithValue(0x442A15, 2002, "Player contract 2");
            MarkYearWithValue(0x442A4E, 2005, "Player contract 1");
            MarkYearWithValue(0x48BDF3, 2006, "Assistant - Contracts check");
            MarkYearWithValue(0x4A1FF3, 2006, "Assistant - Process transfers");
            MarkYearWithValue<short>(0x49E0A7, 2006, "Manager");
            MarkYearWithValue(0x4623DE, 2007, "Start date 1");
            MarkYearWithValue(0x413233, 2007, "Calendar start");
            MarkYearWithValue(0x4158AD, 2007, "Club foundation year");
            MarkYearWithValue(0x413693, 2007, "Calendar 1");
            MarkYearWithValue(0x413FF9, 2007, "Calendar check 1");
            MarkYearWithValue(0x448C6D, 2007, "Player history");
            MarkYearWithValue(0x4491D3, 2007, "Player list 1");
            MarkYearWithValue(0x4491F2, 2007, "Player list 2");
            MarkYearWithValue<short>(0x4493DF, 2007, "Unknown");
            MarkYearWithValue<short>(0x44C8E0, 2007, "Unknown");
            MarkYearWithValue(0x44CA77, 2007, "Player list 1");
            MarkYearWithValue(0x44CA96, 2007, "Player list 2");
            MarkYearWithValue(0x47EE2A, 2007, "Problem tool history");
            MarkYearWithValue<short>(0x47EEA9, 2007, "Problem tool history");
            MarkYearWithValue(0x49AB0E, 2007, "Unknown");
            MarkYearWithValue(0x49D45F, 2007, "Unknown");
            MarkYearWithValue(0x49DFD3, 2007, "Manager 1");
            MarkYearWithValue(0x49E001, 2007, "Manager 2");
            MarkYearWithValue(0x49E015, 2007, "Manager 3");
            MarkYearWithValue(0x49F992, 2007, "Unknown");
            MarkYearWithValue<short>(0x49F9A1, 2007, "Unknown");
            MarkYearWithValue(0x4A0764, 2007, "Unknown");
            MarkYearWithValue(0x4A18C8, 2007, "Process transfer");
            MarkYearWithValue(0x4B46DC, 2007, "Fix contract");
            MarkYearWithValue(0x41368D, 2008, "Calendar 2");
            MarkYearWithValue(0x413FF2, 2008, "Calendar check 2");
            MarkYearWithValue(0x41B362, 2008, "Club wins");
            MarkYearWithValue<char>(0x41B34A, 8, "Club wins");
            MarkYearWithValue(0x47F1E2, 2008, "Problem tool fixture list");
            MarkYearWithValue(0x49DFED, 2008, "Manager");
            MarkYearWithValue<short>(0x49E09E, 2008, "Manager");
            MarkYearWithValue<short>(0x49F9AA, 2008, "Unknown");
            MarkYearWithValue(0x47F1DC, 2009, "Problem tool fixture list");
            MarkYearWithValue(0x442A25, 2009, "Player contract");
            MarkYearWithValue(0x442A5E, 2012, "Player contract");
            MarkYearWithValue(0x49ACD6, 2015, "clb");
            MarkYearWithValue(0x442A5E, 2024, "clb");

            SetYearFixedValue<int>(0x442B0D + 1, SEASON_START_YEAR - 50);
            SetYearFixedValue<int>(0x448393 + 1, SEASON_START_YEAR - 50);
            SetYearFixedValue<int>(0x4483C3 + 1, SEASON_START_YEAR - 50);
            SetYearFixedValue<int>(0x448471 + 1, SEASON_START_YEAR - 50);
            SetYearFixedValue<int>(0x47EECE + 1, SEASON_START_YEAR - 50);
            SetYearFixedValue<int>(0x48BDE7 + 2, SEASON_START_YEAR - 50);
            SetYearFixedValue<int>(0x49AC9C + 1, SEASON_START_YEAR - 50);
            SetYearFixedValue<int>(0x49E028 + 1, SEASON_START_YEAR - 50);
            SetYearFixedValue<int>(0x4C4376 + 6, SEASON_START_YEAR - 50);

            if (!DISABLE_ADDITIONAL_PATCHES) {
                patch::SetUInt(0x4158EA + 1, 1'000'000'000); // initial capital
                patch::SetUInt(0x4158F5 + 1, -1'000'000'000); // initial capital
                patch::SetUInt(0x415926 + 1, 1'000'000); // fan base
            }

            SetWindowTitleAddr = patch::RedirectCall(0x40D1D3, SetEditorWindowTitle);
        }
        else if (version == ID_ED_07_7020) {
            PluginLoaded();
            SetManagerId(7);
            StartDate startDate = GetStartDate<true, 0x47D0B0, 0x47D0D0>(0);
            patch::SetUChar(0x47C84B + 1, startDate.day);
            patch::SetUChar(0x47C84D + 1, startDate.month);
            patch::SetUInt(0x47C84F + 1, SEASON_START_YEAR);
            SetYearFixedValue<short>(0x42C118 + 3, SEASON_START_YEAR - 100, "Birthdate Federation");
            SetYearFixedValue<short>(0x42C118 + 1, SEASON_START_YEAR - 10, "Birthdate Federation");
            SetYearFixedValue<short>(0x405915 + 1, SEASON_START_YEAR, "Club foundation");
            SetYearFixedValue<short>(0x4607EE + 7, FOUNDATION_YEAR_DEFAULT, "Club foundation default");
            SetYearFixedValue<int>(0x407FB8 + 1, SEASON_START_YEAR + 1, "Club history check 1");
            SetYearFixedValue<char>(0x407F9F + 2, SEASON_START_YEAR - 2000, "Club history check 2");
            SetYearFixedValue<short>(0x413B5C + 3, SEASON_START_YEAR - 100, "Birthdate Players and Staff");
            SetYearFixedValue<short>(0x413B5C + 1, SEASON_START_YEAR - 10, "Birthdate Players and Staff");
            SetYearFixedValue<short>(0x434FC4 + 3, SEASON_START_YEAR - 100, "Birthdate Country Managers");
            SetYearFixedValue<short>(0x434FC4 + 1, SEASON_START_YEAR - 10, "Birthdate Country Managers");
            SetYearFixedValue<int>(0x489EB3 + 1, SEASON_START_YEAR - 20, "Birthdate default");
            SetYearFixedValue<short>(0x41A1D7 + 3, SEASON_START_YEAR - 50, "Player join date");
            SetYearFixedValue<short>(0x487091 + 7, SEASON_START_YEAR - 50, "Player join date - club load");
            SetYearFixedValue<short>(0x419583 + 3, SEASON_START_YEAR - 50, "Player history 1");
            SetYearFixedValue<short>(0x419583 + 1, SEASON_START_YEAR + 1, "Player history 1");
            SetYearFixedValue<short>(0x4195C2 + 3, SEASON_START_YEAR - 50, "Player history 2");
            SetYearFixedValue<short>(0x4195C2 + 1, SEASON_START_YEAR + 5, "Player history 2");
            SetYearFixedValue<short>(0x41A344 + 1, SEASON_START_YEAR + 1, "Player history default");
            SetYearFixedValue<int>(0x47BF96 + 1, SEASON_START_YEAR - 1, "Contracts check");
            SetYearFixedValue<int>(0x417991 + 1, SEASON_START_YEAR + 10, "Player contract");
            SetYearFixedValue<int>(0x4870CA + 1, SEASON_START_YEAR + 10, "Player contract - club load");
            SetYearFixedValue<int>(0x416438 + 1, SEASON_START_YEAR - 2, "Player start career init 1");
            SetYearFixedValue<int>(0x41642D + 1, SEASON_START_YEAR + 2, "Player start career init 1");
            SetYearFixedValue<int>(0x416444 + 1, SEASON_START_YEAR, "Player start career init 1");
            SetYearFixedValue<int>(0x41648F + 1, SEASON_START_YEAR - 2, "Player start career init 2");
            SetYearFixedValue<int>(0x416484 + 1, SEASON_START_YEAR + 5, "Player start career init 2");
            SetYearFixedValue<int>(0x41649B + 1, SEASON_START_YEAR, "Player start career init 2");
            SetYearFixedValue<int>(0x4164AE + 1, SEASON_START_YEAR - 50, "Player start career init 3");
            SetYearFixedValue<int>(0x41649B + 1, SEASON_START_YEAR, "Player start career init 3");
            SetYearFixedValue<int>(0x41668B + 1, SEASON_START_YEAR, "Player start career create 1");
            SetYearFixedValue<int>(0x41669D + 1, SEASON_START_YEAR, "Player start career create 1");
            SetYearFixedValue<short>(0x416707 + 2, SEASON_START_YEAR, "Player start career create 2");
            SetYearFixedValue<int>(0x416711 + 1, SEASON_START_YEAR + 1, "Player start career create 2");
            SetYearFixedValue<int>(0x416723 + 1, SEASON_START_YEAR + 1, "Player start career create 2");
            SetYearFixedValue<short>(0x41676B + 2, SEASON_START_YEAR - 5, "Player start career create 3");
            SetYearFixedValue<int>(0x416775 + 1, SEASON_START_YEAR, "Player start career create 3");
            SetYearFixedValue<short>(0x4167E4 + 2, SEASON_START_YEAR, "Player start career create 4");
            SetYearFixedValue<int>(0x4167EE + 1, SEASON_START_YEAR, "Player start career create 4");
            SetYearFixedValue<int>(0x416800 + 1, SEASON_START_YEAR + 1, "Player start career create 4");
            SetYearFixedValue<int>(0x417A3C + 1, SEASON_START_YEAR, "Contract process 1");
            SetYearFixedValue<int>(0x417A5A + 1, SEASON_START_YEAR + 1, "Contract process 1");
            SetYearFixedValue<int>(0x417AA3 + 1, SEASON_START_YEAR, "Contract process 2");
            SetYearFixedValue<int>(0x417AC1 + 1, SEASON_START_YEAR, "Contract process 2");
            SetYearFixedValue<int>(0x417B0A + 1, SEASON_START_YEAR, "Contract process 3");
            SetYearFixedValue<int>(0x417B53 + 1, SEASON_START_YEAR, "Contract process 4");
            SetYearFixedValue<int>(0x417B71 + 1, SEASON_START_YEAR + 1, "Contract process 4");
            SetYearFixedValue<int>(0x489E72 + 1, SEASON_START_YEAR, "Player start career default");
            SetYearFixedValue<int>(0x489E89 + 1, SEASON_START_YEAR + 1, "Player start career default");
            SetYearFixedValue<short>(0x489F31 + 7, SEASON_START_YEAR, "Player join date default");
            SetYearFixedValue<short>(0x489F28 + 7, SEASON_START_YEAR + 1, "Player contract default");
            SetYearFixedValue<int>(0x41857F + 1, SEASON_START_YEAR - 100, "Player generator init");
            SetYearFixedValue<int>(0x41857A + 1, SEASON_START_YEAR - 10, "Player generator init");
            SetYearFixedValue<int>(0x417FF4 + 1, SEASON_START_YEAR - 100, "Player generator update");
            SetYearFixedValue<int>(0x417FEF + 1, SEASON_START_YEAR - 10, "Player generator update");
            SetYearFixedValue<int>(0x417F53 + 3, SEASON_START_YEAR - 29, "Player generator generate");
            SetYearFixedValue<int>(0x42358B + 1, SEASON_START_YEAR - 18, "Playerpool init");
            SetYearFixedValue<int>(0x423586 + 1, SEASON_START_YEAR - 10, "Playerpool init");
            SetYearFixedValue<int>(0x48C41F + 1, SEASON_START_YEAR, "Playerpool random");
            SetYearFixedValue<int>(0x417FE1 + 1, SEASON_START_YEAR + 2, "Unknown generator");
            SetYearFixedValue<int>(0x48BD29 + 1, SEASON_START_YEAR - 15, "Unknown 1991 1");
            SetYearFixedValue<int>(0x48BD8B + 1, SEASON_START_YEAR - 15, "Unknown 1991 2");
            SetYearFixedValue<int>(0x42851E + 1, SEASON_START_YEAR, "Calendar 1");
            SetYearFixedValue<int>(0x428517 + 2, SEASON_START_YEAR + 1, "Calendar 1");
            SetYearFixedValue<int>(0x4289B7 + 2, SEASON_START_YEAR, "Calendar 2");
            SetYearFixedValue<int>(0x4289B0 + 2, SEASON_START_YEAR + 1, "Calendar 2");
            SetYearFixedValue<int>(0x46EEF3 + 1, SEASON_START_YEAR + 1, "Calendar 3");
            SetYearFixedValue<int>(0x46EEEC + 2, SEASON_START_YEAR + 2, "Calendar 3");
            SetYearFixedValue<int>(0x418B4D + 1, SEASON_START_YEAR, "Unknown player generator");
            SetYearFixedValue<int>(0x428D31 + 1, SEASON_START_YEAR, "Unknown calendar");
            SetYearFixedValue<int>(0x46F86E + 1, SEASON_START_YEAR, "Problem tool 1");
            SetYearFixedValue<short>(0x46F8D8 + 7, SEASON_START_YEAR, "Problem tool 1");
            SetYearFixedValue<int>(0x486EA7 + 1, SEASON_START_YEAR, "Database check");
            SetYearFixedValue<short>(0x48B784 + 7, SEASON_START_YEAR, "Unknown 1");
            SetYearFixedValue<short>(0x48B78D + 7, SEASON_START_YEAR + 1, "Unknown 1");
            SetYearFixedValue<int>(0x48B7A0 + 1, SEASON_START_YEAR, "Unknown 1");
            SetYearFixedValue<int>(0x497A36 + 1, SEASON_START_YEAR, "Loan check");
            if (!DISABLE_ADDITIONAL_PATCHES)
                patch::SetUInt(0x40592A + 1, 5200);
        }
        else if (version == ID_ED_05_4000) {
            PluginLoaded();
            SetManagerId(5);
            StartDate startDate = GetStartDate<true, 0x4B0510, 0x4B0540>(6);
            patch::SetUChar(0x4A4E3B + 1, startDate.day);
            patch::SetUChar(0x4A4E3D + 1, startDate.month);
            patch::SetUInt(0x4A4E46 + 1, SEASON_START_YEAR);
            SetYearFixedValue<short>(0x442F28 + 3, SEASON_START_YEAR - 100, "Birthdate Federation");
            SetYearFixedValue<short>(0x442F28 + 1, SEASON_START_YEAR - 10, "Birthdate Federation");
            SetYearFixedValue<short>(0x421BE5 + 1, SEASON_START_YEAR, "Club foundation");
            SetYearFixedValue<short>(0x476E18 + 7, FOUNDATION_YEAR_DEFAULT, "Club foundation default");
            SetYearFixedValue<short>(0x42F942 + 3, SEASON_START_YEAR - 100, "Birthdate 1");
            SetYearFixedValue<short>(0x42F942 + 1, SEASON_START_YEAR - 10, "Birthdate 1");
            SetYearFixedValue<short>(0x44C104 + 3, SEASON_START_YEAR - 100, "Birthdate 2");
            SetYearFixedValue<short>(0x44C104 + 1, SEASON_START_YEAR - 10, "Birthdate 2");
            SetYearFixedValue<int>(0x4AB792 + 1, SEASON_START_YEAR - 20, "Birthdate default");
            SetYearFixedValue<short>(0x4346B7 + 3, SEASON_START_YEAR - 50, "Player join date");
            SetYearFixedValue<short>(0x4AABC8 + 7, SEASON_START_YEAR - 50, "Player join date - club load");
            SetYearFixedValue<short>(0x433C13 + 3, SEASON_START_YEAR - 50, "Player history 1");
            SetYearFixedValue<short>(0x433C13 + 1, SEASON_START_YEAR + 1, "Player history 1");
            SetYearFixedValue<short>(0x433C52 + 3, SEASON_START_YEAR - 50, "Player history 2");
            SetYearFixedValue<short>(0x433C52 + 1, SEASON_START_YEAR + 1, "Player history 2");
            SetYearFixedValue<int>(0x432EF2 + 1, SEASON_START_YEAR + 10, "Player contract");
            SetYearFixedValue<int>(0x4AABFE + 1, SEASON_START_YEAR + 10, "Player contract - club load");
            SetYearFixedValue<int>(0x4AABBA + 1, SEASON_START_YEAR, "Player - club load unknown");
            SetYearFixedValue<short>(0x43200F + 3, SEASON_START_YEAR - 4, "Player start career 2");
            SetYearFixedValue<short>(0x43200F + 1, SEASON_START_YEAR, "Player start career 2");
            SetYearFixedValue<short>(0x43204E + 3, SEASON_START_YEAR - 4, "Player start career 2");
            SetYearFixedValue<short>(0x43204E + 1, SEASON_START_YEAR + 4, "Player start career 2");
            SetYearFixedValue<int>(0x432F6E + 1, SEASON_START_YEAR, "Player start career default");
            SetYearFixedValue<int>(0x432F9C + 1, SEASON_START_YEAR, "Player start career default");
            SetYearFixedValue<int>(0x432FBA + 1, SEASON_START_YEAR + 1, "Player start career default");
            SetYearFixedValue<int>(0x4AB7F2 + 1, SEASON_START_YEAR, "Player unknown 1");
            SetYearFixedValue<int>(0x4AB810 + 1, SEASON_START_YEAR + 1, "Player unknown 2");
            patch::Nop(0x4AB88C, 1); // player default join date
            patch::SetUChar(0x4ACE67 + 1, 0); // player history default start date
            patch::RedirectCall(0x4ACE62, OnTCM05SetNewPlayerHistoryEntryStartDate);
            if (!DISABLE_ADDITIONAL_PATCHES)
                patch::SetUInt(0x421BFA + 1, 10200);
        }
        else if (version == ID_ED_04_1016) {
            PluginLoaded();
            SetManagerId(4);
            StartDate startDate = GetStartDate<true, 0x478B20, 0x478B40>(6);
            patch::SetUChar(0x47008B + 1, startDate.day);
            patch::SetUChar(0x47008D + 1, startDate.month);
            patch::SetUInt(0x470096 + 1, SEASON_START_YEAR);
            SetYearFixedValue<short>(0x431E98 + 3, SEASON_START_YEAR - 100, "Birthdate Federation");
            SetYearFixedValue<short>(0x431E98 + 1, SEASON_START_YEAR - 10, "Birthdate Federation");
            SetYearFixedValue<short>(0x415065 + 1, SEASON_START_YEAR, "Club foundation");
            SetYearFixedValue<short>(0x456510 + 7, FOUNDATION_YEAR_DEFAULT, "Club foundation default");
            SetYearFixedValue<short>(0x4213DC + 3, SEASON_START_YEAR - 100, "Birthdate 1");
            SetYearFixedValue<short>(0x4213DC + 1, SEASON_START_YEAR - 10, "Birthdate 1");
            SetYearFixedValue<short>(0x43A67C + 3, SEASON_START_YEAR - 100, "Birthdate 2");
            SetYearFixedValue<short>(0x43A67C + 1, SEASON_START_YEAR - 10, "Birthdate 2");
            SetYearFixedValue<int>(0x475642 + 1, SEASON_START_YEAR - 20, "Birthdate default");
            SetYearFixedValue<short>(0x4251FD + 3, SEASON_START_YEAR - 50, "Player join date");
            SetYearFixedValue<int>(0x474F26 + 1, SEASON_START_YEAR - 50, "Player join date - club load");
            SetYearFixedValue<int>(0x425222 + 1, SEASON_START_YEAR + 10, "Player contract");
            SetYearFixedValue<int>(0x474F6D + 1, SEASON_START_YEAR + 10, "Player contract - club load");
            SetYearFixedValue<short>(0x42428F + 3, SEASON_START_YEAR - 4, "Player start career 2");
            SetYearFixedValue<short>(0x42428F + 1, SEASON_START_YEAR, "Player start career 2");
            SetYearFixedValue<short>(0x4242CE + 3, SEASON_START_YEAR - 4, "Player start career 2");
            SetYearFixedValue<short>(0x4242CE + 1, SEASON_START_YEAR + 4, "Player start career 2");
            SetYearFixedValue<int>(0x425327 + 1, SEASON_START_YEAR, "Player start career default");
            SetYearFixedValue<int>(0x425356 + 1, SEASON_START_YEAR, "Player start career default");
            SetYearFixedValue<int>(0x425374 + 1, SEASON_START_YEAR + 1, "Player start career default");
            SetYearFixedValue<int>(0x4756D2 + 1, SEASON_START_YEAR, "Player unknown 1");
            SetYearFixedValue<int>(0x47571A + 1, SEASON_START_YEAR + 1, "Player unknown 2");
            patch::Nop(0x475792, 1); // player default join date
            if (!DISABLE_ADDITIONAL_PATCHES)
                patch::SetUInt(0x41507A + 1, 5200);
        }
        else if (version == ID_FIFA_07_1100_C) {
            PluginLoaded();
            patch::SetUInt(0x493F47 + 1, SEASON_START_YEAR); // GetVirtualAttribBaseAge()
            patch::SetUInt(0x494155 + 1, SEASON_START_YEAR); // Database::Today()
            patch::SetUInt(0x4A87A1 + 1, SEASON_START_YEAR); // TournamentCmn::GetTournamentManager()
            patch::SetUInt(0x4A89F9 + 1, SEASON_START_YEAR);
        }
        else if (version == ID_FIFA_08_1000_C || version == ID_FIFA_08_1200_C) {
            PluginLoaded();
            patch::SetUInt(0x498887 + 1, SEASON_START_YEAR);
            patch::SetUInt(0x4E5E61 + 1, SEASON_START_YEAR);
            patch::SetUInt(0x50331B + 1, SEASON_START_YEAR);
            patch::SetUInt(0x53AB39 + 1, SEASON_START_YEAR);
            patch::SetUInt(0x53CBF2 + 1, SEASON_START_YEAR);
            patch::SetUInt(0x568368 + 1, SEASON_START_YEAR);
            patch::SetUInt(0x1670505 + 1, SEASON_START_YEAR);
        }
        //else if (version == ID_FIFA_10_1000_C) {
        //    PluginLoaded();
        //    patch::SetUInt(0x4C1160 + 1, SEASON_START_YEAR);
        //    patch::SetUInt(0x4C1538 + 1, SEASON_START_YEAR);
        //    patch::SetUInt(0x4D8F3F + 1, SEASON_START_YEAR);
        //    patch::SetUInt(0x4E160B + 1, SEASON_START_YEAR);
        //    patch::SetUInt(0x528C16 + 1, SEASON_START_YEAR);
        //    patch::SetUInt(0x52A888 + 1, SEASON_START_YEAR);
        //    patch::SetUInt(0x52CB89 + 1, SEASON_START_YEAR);
        //    patch::SetUInt(0x52DFF8 + 1, SEASON_START_YEAR);
        //}
        else {
            PluginNotLoaded();
        }
    }
};

TheSeason theSeason;
