/*
    FIFA Manager API
    Authors: Dmitry (Dmitri,DK,DK22)
*/
#pragma once
#include <Windows.h>
#include <stdio.h>
#include <string>
#include "Other.h"

namespace plugin
{
    struct message_gen {
        template <typename... ArgTypes>
        static bool msga(std::string const &title, int iconType, std::string const &format, ArgTypes... args) {
            MessageBoxA(GetActiveWindow(), FormatStatic(format, args...), title.c_str(), iconType);
            return false;
        }

        template <typename... ArgTypes>
        static bool msgw(std::wstring const &title, int iconType, std::wstring const &format, ArgTypes... args) {
            MessageBoxW(GetActiveWindow(), FormatStatic(format, args...), title.c_str(), iconType);
            return false;
        }
    };

    template <typename... ArgTypes> bool Message(std::string const &format, ArgTypes... Args) {
        return message_gen::msga("FIFA Manager Message", MB_ICONINFORMATION, format, Args...);
    }

    template <typename... ArgTypes> bool Warning(std::string const &format, ArgTypes... Args) {
        return message_gen::msga("FIFA Manager Warning", MB_ICONWARNING, format, Args...);
    }

    template <typename... ArgTypes> bool Error(std::string const &format, ArgTypes... Args) {
        return message_gen::msga("FIFA Manager Error", MB_ICONERROR, format, Args...);
    }

    template <typename... ArgTypes> bool Message(std::wstring const &format, ArgTypes... Args) {
        return message_gen::msgw(L"FIFA Manager Message", MB_ICONINFORMATION, format, Args...);
    }

    template <typename... ArgTypes> bool Warning(std::wstring const &format, ArgTypes... Args) {
        return message_gen::msgw(L"FIFA Manager Warning", MB_ICONWARNING, format, Args...);
    }

    template <typename... ArgTypes> bool Error(std::wstring const &format, ArgTypes... Args) {
        return message_gen::msgw(L"FIFA Manager Error", MB_ICONERROR, format, Args...);
    }

    template <typename... ArgTypes> bool InternalMessage(std::string const &format, ArgTypes... Args) {
        return message_gen::msga("FIFA Manager ASI Loader", MB_ICONINFORMATION, format, Args...);
    }

    template <typename... ArgTypes> bool InternalWarning(std::string const &format, ArgTypes... Args) {
        return message_gen::msga("FIFA Manager ASI Loader", MB_ICONWARNING, format, Args...);
    }

    template <typename... ArgTypes> bool InternalError(std::string const &format, ArgTypes... Args) {
        return message_gen::msga("FIFA Manager ASI Loader", MB_ICONERROR, format, Args...);
    }

    template <typename... ArgTypes> bool InternalMessage(std::wstring const &format, ArgTypes... Args) {
        return message_gen::msgw(L"FIFA Manager ASI Loader", MB_ICONINFORMATION, format, Args...);
    }

    template <typename... ArgTypes> bool InternalWarning(std::wstring const &format, ArgTypes... Args) {
        return message_gen::msgw(L"FIFA Manager ASI Loader", MB_ICONWARNING, format, Args...);
    }

    template <typename... ArgTypes> bool InternalError(std::wstring const &format, ArgTypes... Args) {
        return message_gen::msgw(L"FIFA Manager ASI Loader", MB_ICONERROR, format, Args...);
    }
}
