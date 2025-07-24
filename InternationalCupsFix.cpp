#include "InternationalCupsFix.h"

using namespace plugin;

class CJDate {
    unsigned int value;
public:
    CJDate GetTranslated(int numYears) {
        CJDate result;
        plugin::CallMethod<0x1495EFF>(this, &result, numYears);
        return result;
    }

    unsigned short GetYear() {
        return plugin::CallMethodAndReturn<unsigned short, 0x1494CC9>(this);
    }

    unsigned char GetMonth() {
        return plugin::CallMethodAndReturn<unsigned char, 0x1494CA2>(this);
    }

    unsigned char CJDate::GetDays() {
        return plugin::CallMethodAndReturn<unsigned char, 0x1494C7B>(this);
    }

    void GetYMD(unsigned short *outY, unsigned char *outM, unsigned char *outD) {
        plugin::CallMethod<0x1494AA2>(this, outY, outM, outD);
    }

    void GetYMD(unsigned int *outY, unsigned int *outM, unsigned int *outD) {
        plugin::CallMethod<0x1494A81>(this, outY, outM, outD);
    }
};


class CDBGame {
public:
    static CDBGame *GetInstance() {
        return plugin::CallAndReturn<CDBGame *, 0xF61410>();
    }

    CJDate GetStartDate() {
        CJDate result;
        plugin::CallMethod<0xF49950>(this, &result);
        return result;
    }

    CJDate GetCurrentDate() {
        CJDate result;
        plugin::CallMethod<0xF498D0>(this, &result);
        return result;
    }
};

enum eCompetitionType : unsigned char {
    COMP_ROOT = 0,
    COMP_LEAGUE = 1,
    COMP_UNKNOWN_2 = 2,
    COMP_FA_CUP = 3,
    COMP_LE_CUP = 4,
    COMP_CHALLENGE_SHIELD = 5,
    COMP_CONFERENCE_CUP = 6,
    COMP_SUPERCUP = 7,
    COMP_RELEGATION = 8,
    COMP_CHAMPIONSLEAGUE = 9,
    COMP_UEFA_CUP = 10,
    COMP_TOYOTA = 11,
    COMP_EURO_SUPERCUP = 12,
    COMP_WORLD_CLUB_CHAMP = 13,
    COMP_UIC = 14,
    COMP_QUALI_WC = 15,
    COMP_QUALI_EC = 16,
    COMP_WORLD_CUP = 17,
    COMP_EURO_CUP = 18,
    COMP_REL1 = 19,
    COMP_REL2 = 20,
    COMP_REL3 = 21,
    COMP_INDOOR = 22,
    COMP_FRIENDLY = 23,
    COMP_POOL = 24,
    COMP_UNKNOWN_25 = 25,
    COMP_UNKNOWN_26 = 26,
    COMP_UNKNOWN_27 = 27,
    COMP_RESERVE = 28,
    COMP_UNKNOWN_29 = 29,
    COMP_UNKNOWN_30 = 30,
    COMP_U20_WORLD_CUP = 31,
    COMP_CONFED_CUP = 32,
    COMP_COPA_AMERICA = 33
};

struct CCompID {
    unsigned short index = 0;
    unsigned char type = 0;
    unsigned char countryId = 0;

    std::wstring ToStr() {
        std::wstring typeName = L"UNKNOWN";
        if (type == 0) typeName = L"ROOT";
        else if (type == 1) typeName = L"LEAGUE";
        else if (type == 2) typeName = L"UNKNOWN_2";
        else if (type == 3) typeName = L"FA_CUP";
        else if (type == 4) typeName = L"LE_CUP";
        else if (type == 5) typeName = L"CHALLENGE_SHIELD";
        else if (type == 6) typeName = L"CONFERENCE_CUP";
        else if (type == 7) typeName = L"SUPERCUP";
        else if (type == 8) typeName = L"RELEGATION";
        else if (type == 9) typeName = L"CHAMPIONSLEAGUE";
        else if (type == 10) typeName = L"UEFA_CUP";
        else if (type == 11) typeName = L"TOYOTA";
        else if (type == 12) typeName = L"EURO_SUPERCUP";
        else if (type == 13) typeName = L"WORLD_CLUB_CHAMP";
        else if (type == 14) typeName = L"UIC";
        else if (type == 15) typeName = L"QUALI_WC";
        else if (type == 16) typeName = L"QUALI_EC";
        else if (type == 17) typeName = L"WORLD_CUP";
        else if (type == 18) typeName = L"EURO_CUP";
        else if (type == 19) typeName = L"REL1";
        else if (type == 20) typeName = L"REL2";
        else if (type == 21) typeName = L"REL3";
        else if (type == 22) typeName = L"INDOOR";
        else if (type == 23) typeName = L"FRIENDLY";
        else if (type == 24) typeName = L"POOL";
        else if (type == 25) typeName = L"UNKNOWN_25";
        else if (type == 26) typeName = L"UNKNOWN_26";
        else if (type == 27) typeName = L"UNKNOWN_27";
        else if (type == 28) typeName = L"RESERVE";
        else if (type == 29) typeName = L"UNKNOWN_29";
        else if (type == 30) typeName = L"UNKNOWN_30";
        else if (type == 31) typeName = L"U20_WORLD_CUP";
        else if (type == 32) typeName = L"CONFED_CUP";
        else if (type == 33) typeName = L"COPA_AMERICA";
        return plugin::Format(L"{ %d, %s, %d }", countryId, typeName.c_str(), index);
    };
};

class CDBCompetition {
public:
    unsigned char GetCompetitionType() {
        return plugin::CallMethodAndReturn<unsigned char, 0xF81C50>(this);
    }

    unsigned int GetDbType() {
        return plugin::CallMethodAndReturn<unsigned int, 0xF81960>(this);
    }

    wchar_t const *GetName() {
        return plugin::CallMethodAndReturn<wchar_t const *, 0xF81930>(this);
    }

    CCompID GetCompID() {
        CCompID result;
        plugin::CallMethod<0xF81C20>(this, &result);
        return result;
    }

    unsigned int GetNumOfRegisteredTeams() {
        return plugin::CallMethodAndReturn<unsigned int, 0xF82510>(this);
    }

    unsigned int GetNumOfTeams() {
        return plugin::CallMethodAndReturn<unsigned int, 0xF82520>(this);
    }

    unsigned char GetFlags() {
        return *plugin::raw_ptr<unsigned char>(this, 0x288);
    }

    void DumpToFile() {
        plugin::CallMethod<0xF85430>(this);
    }
};

class CDBLeague : public CDBCompetition {
public:
    void SetStartDate(CJDate date) {
        plugin::CallMethod<0x1054390>(this, date);
    }
};

CDBLeague *gComp = nullptr;
uintptr_t gOriginal_117C830 = 0;
uintptr_t gOriginal_StoreCompetition = 0;

unsigned char METHOD MyFixInternationalQuali_StoreCompetition(CDBLeague *comp) {
    gComp = comp;
    return CallMethodAndReturnDynGlobal<unsigned char>(gOriginal_StoreCompetition, comp);
}

void *MyFixInternationalQuali_Fix() {
    CJDate startDate = CDBGame::GetInstance()->GetStartDate();
    if ((gComp->GetCompetitionType() == COMP_QUALI_WC || gComp->GetCompetitionType() == COMP_QUALI_EC) && (startDate.GetYear() % 2) == 1) {
        CJDate currDate = CDBGame::GetInstance()->GetCurrentDate();
        if (startDate.GetYear() == currDate.GetYear()) {
            CJDate oneYearDecreased = startDate.GetTranslated(-1);
            gComp->SetStartDate(oneYearDecreased);
        }
    }
    return CallAndReturnDynGlobal<void *>(gOriginal_117C830);
}

void PatchInternationalCups(FM::Version v) {
    if (v.id() == ID_FM_13_1030_RLD) {
        gOriginal_StoreCompetition = patch::RedirectCall(0x105E11F, MyFixInternationalQuali_StoreCompetition);
        gOriginal_117C830 = patch::RedirectCall(0x105E2F4, MyFixInternationalQuali_Fix);
    }
}
