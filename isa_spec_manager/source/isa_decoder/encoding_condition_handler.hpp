// Copyright (c) 2023-2024 Advanced Micro Devices, Inc. All rights reserved.
// This code is auto-generated. Do not edit directly. Report any issues.
// This code was generated on: 2024-04-30
#ifndef ENCODING_CONDITION_HANDLER_HPP_
#include <functional>
#include <map>
namespace amdisa
{
    static bool IsArch12VopcDefault(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch12VopcInstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch12VopcVopSdwaSdstEncHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch12Sop1Default(uint64_t dword) {
        return ((dword & 0xff)) != 255;
    }

    static bool IsArch12Sop1InstLiteralHasLit0(uint64_t dword) {
        return ((dword & 0xff)) == 255;
    }

    static bool IsArch12Sop2Default(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch12Sop2InstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch12Sop2InstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch12Sop2InstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch12SopcDefault(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch12SopcInstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch12SopcInstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch12SopcInstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch12Vop1Default(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch12Vop1InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch12Vop1VopDppHasDpp(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch12Vop1VopSdwaHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch12Vop2Default(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch12Vop2InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch12Vop2InstLiteralDefault(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch12Vop2VopDppHasDpp(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch12Vop2VopSdwaHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch12Vop2VopSdwaSdstEncHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch5Vop2VopDpp16Default(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch5Vop2VopDpp8Default(uint64_t dword) {
        return (((dword & 0x1ff)) == 233) || (((dword & 0x1ff)) == 234);
    }

    static bool IsArch5Vop2Default(uint64_t dword) {
        return (((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch5Vop2InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch5Vop2InstLiteralDefault(uint64_t dword) {
        return (((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch5Vop3Default(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3InstLiteralHasLit0(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3InstLiteralHasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3InstLiteralHasLit0HasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3InstLiteralHasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5Vop3InstLiteralHasLit0HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5Vop3InstLiteralHasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5Vop3InstLiteralHasLit0HasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5Vop3SdstEncDefault(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3SdstEncInstLiteralHasLit0(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3SdstEncInstLiteralHasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3SdstEncInstLiteralHasLit0HasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3SdstEncInstLiteralHasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5Vop3SdstEncInstLiteralHasLit0HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5Vop3SdstEncInstLiteralHasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5Vop3SdstEncInstLiteralHasLit0HasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5Vop3pDefault(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3pInstLiteralHasLit0(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3pInstLiteralHasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3pInstLiteralHasLit0HasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch5Vop3pInstLiteralHasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5Vop3pInstLiteralHasLit0HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5Vop3pInstLiteralHasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5Vop3pInstLiteralHasLit0HasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch5VopcVopSdwaSdstEncDefault(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch5VopcDefault(uint64_t dword) {
        return (((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch5VopcInstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch5MimgDefault(uint64_t dword) {
        return ((((dword & 0x6) >> 1) != 1) && (((dword & 0x6) >> 1) != 2)) && (((dword & 0x6) >> 1) != 3);
    }

    static bool IsArch5MimgNsa1HasNsa1(uint64_t dword) {
        return ((dword & 0x6) >> 1) == 1;
    }

    static bool IsArch5MimgNsa2HasNsa2(uint64_t dword) {
        return ((dword & 0x6) >> 1) == 2;
    }

    static bool IsArch5MimgNsa3HasNsa3(uint64_t dword) {
        return ((dword & 0x6) >> 1) == 3;
    }

    static bool IsArch5Sop1Default(uint64_t dword) {
        return ((dword & 0xff)) != 255;
    }

    static bool IsArch5Sop1InstLiteralHasLit0(uint64_t dword) {
        return ((dword & 0xff)) == 255;
    }

    static bool IsArch5Sop2Default(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch5Sop2InstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch5Sop2InstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch5Sop2InstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch5SopcDefault(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch5SopcInstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch5SopcInstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch5SopcInstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch5Vop1Default(uint64_t dword) {
        return (((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch5Vop1VopDpp16Default(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch5Vop1VopDpp8Default(uint64_t dword) {
        return (((dword & 0x1ff)) == 233) || (((dword & 0x1ff)) == 234);
    }

    static bool IsArch5Vop1VopSdwaDefault(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch5Vop1InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch5Vop2VopSdwaDefault(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch5Vop2VopSdwaSdstEncDefault(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch6Vop3Default(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3InstLiteralHasLit0(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3InstLiteralHasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3InstLiteralHasLit0HasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3InstLiteralHasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch6Vop3InstLiteralHasLit0HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch6Vop3InstLiteralHasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch6Vop3InstLiteralHasLit0HasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch6Vop3pDefault(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3pInstLiteralHasLit0(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3pInstLiteralHasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3pInstLiteralHasLit0HasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3pInstLiteralHasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch6Vop3pInstLiteralHasLit0HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch6Vop3pInstLiteralHasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch6Vop3pInstLiteralHasLit0HasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch6VopcVopSdwaSdstEncDefault(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch6VopcDefault(uint64_t dword) {
        return (((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch6VopcInstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch6MimgDefault(uint64_t dword) {
        return ((((dword & 0x6) >> 1) != 1) && (((dword & 0x6) >> 1) != 2)) && (((dword & 0x6) >> 1) != 3);
    }

    static bool IsArch6MimgNsa1HasNsa1(uint64_t dword) {
        return ((dword & 0x6) >> 1) == 1;
    }

    static bool IsArch6MimgNsa2HasNsa2(uint64_t dword) {
        return ((dword & 0x6) >> 1) == 2;
    }

    static bool IsArch6MimgNsa3HasNsa3(uint64_t dword) {
        return ((dword & 0x6) >> 1) == 3;
    }

    static bool IsArch6Sop1Default(uint64_t dword) {
        return ((dword & 0xff)) != 255;
    }

    static bool IsArch6Sop1InstLiteralHasLit0(uint64_t dword) {
        return ((dword & 0xff)) == 255;
    }

    static bool IsArch6Sop2Default(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch6Sop2InstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch6Sop2InstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch6Sop2InstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch6SopcDefault(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch6SopcInstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch6SopcInstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch6SopcInstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch6Vop1Default(uint64_t dword) {
        return (((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch6Vop1VopDpp16Default(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch6Vop1VopDpp8Default(uint64_t dword) {
        return (((dword & 0x1ff)) == 233) || (((dword & 0x1ff)) == 234);
    }

    static bool IsArch6Vop1VopSdwaDefault(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch6Vop1InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch6Vop2VopDpp16Default(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch6Vop2VopDpp8Default(uint64_t dword) {
        return (((dword & 0x1ff)) == 233) || (((dword & 0x1ff)) == 234);
    }

    static bool IsArch6Vop2VopSdwaDefault(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch6Vop2Default(uint64_t dword) {
        return (((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch6Vop2InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch6Vop2InstLiteralDefault(uint64_t dword) {
        return (((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch6Vop2VopSdwaSdstEncDefault(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch6Vop3SdstEncDefault(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3SdstEncInstLiteralHasLit0(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3SdstEncInstLiteralHasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3SdstEncInstLiteralHasLit0HasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch6Vop3SdstEncInstLiteralHasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch6Vop3SdstEncInstLiteralHasLit0HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch6Vop3SdstEncInstLiteralHasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) != 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch6Vop3SdstEncInstLiteralHasLit0HasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3Default(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3InstLiteralHasLit0(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3InstLiteralHasLit1(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3InstLiteralHasLit0HasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3InstLiteralHasLit2(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3InstLiteralHasLit0HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3InstLiteralHasLit1HasLit2(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3InstLiteralHasLit0HasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3VopDpp16Default(uint64_t dword) {
        return ((dword & 0x1ff00000000) >> 32) == 250;
    }

    static bool IsArch8Vop3VopDpp8Default(uint64_t dword) {
        return (((dword & 0x1ff00000000) >> 32) == 233) || (((dword & 0x1ff00000000) >> 32) == 234);
    }

    static bool IsArch8Vop3pDefault(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3pInstLiteralHasLit0(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3pInstLiteralHasLit1(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3pInstLiteralHasLit0HasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3pInstLiteralHasLit2(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3pInstLiteralHasLit0HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3pInstLiteralHasLit1HasLit2(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3pInstLiteralHasLit0HasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3pVopDpp16Default(uint64_t dword) {
        return ((dword & 0x1ff00000000) >> 32) == 250;
    }

    static bool IsArch8Vop3pVopDpp8Default(uint64_t dword) {
        return (((dword & 0x1ff00000000) >> 32) == 233) || (((dword & 0x1ff00000000) >> 32) == 234);
    }

    static bool IsArch8VopcVopDpp16Default(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch8VopcVopDpp8Default(uint64_t dword) {
        return (((dword & 0x1ff)) == 233) || (((dword & 0x1ff)) == 234);
    }

    static bool IsArch8VopcDefault(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255);
    }

    static bool IsArch8VopcInstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch8MimgDefault(uint64_t dword) {
        return ((dword & 0x1)) != 1;
    }

    static bool IsArch8MimgNsa1HasNsa(uint64_t dword) {
        return ((dword & 0x1)) == 1;
    }

    static bool IsArch8Sop1Default(uint64_t dword) {
        return ((dword & 0xff)) != 255;
    }

    static bool IsArch8Sop1InstLiteralHasLit0(uint64_t dword) {
        return ((dword & 0xff)) == 255;
    }

    static bool IsArch8Sop2Default(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch8Sop2InstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch8Sop2InstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch8Sop2InstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch8SopcDefault(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch8SopcInstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch8SopcInstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch8SopcInstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch8Vop1Default(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255);
    }

    static bool IsArch8Vop1VopDpp16Default(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch8Vop1VopDpp8Default(uint64_t dword) {
        return (((dword & 0x1ff)) == 233) || (((dword & 0x1ff)) == 234);
    }

    static bool IsArch8Vop1InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch8Vop2VopDpp16Default(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch8Vop2VopDpp8Default(uint64_t dword) {
        return (((dword & 0x1ff)) == 233) || (((dword & 0x1ff)) == 234);
    }

    static bool IsArch8Vop2Default(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255);
    }

    static bool IsArch8Vop2InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch8Vop2InstLiteralDefault(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && ((((dword & 0x1ff)) != 233) && (((dword & 0x1ff)) != 234))) && (((dword & 0x1ff)) != 255);
    }

    static bool IsArch8Vop3SdstEncDefault(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3SdstEncInstLiteralHasLit0(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3SdstEncInstLiteralHasLit1(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3SdstEncInstLiteralHasLit0HasLit1(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) != 255);
    }

    static bool IsArch8Vop3SdstEncInstLiteralHasLit2(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3SdstEncInstLiteralHasLit0HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) != 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3SdstEncInstLiteralHasLit1HasLit2(uint64_t dword) {
        return ((((((dword & 0x1ff00000000) >> 32) != 250) && ((((dword & 0x1ff00000000) >> 32) != 233) && (((dword & 0x1ff00000000) >> 32) != 234))) && (((dword & 0x1ff00000000) >> 32) != 255)) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3SdstEncInstLiteralHasLit0HasLit1HasLit2(uint64_t dword) {
        return ((((dword & 0x1ff00000000) >> 32) == 255) && (((dword & 0x3fe0000000000) >> 41) == 255)) && (((dword & 0x7fc000000000000) >> 50) == 255);
    }

    static bool IsArch8Vop3SdstEncVopDpp16Default(uint64_t dword) {
        return ((dword & 0x1ff00000000) >> 32) == 250;
    }

    static bool IsArch8Vop3SdstEncVopDpp8Default(uint64_t dword) {
        return (((dword & 0x1ff00000000) >> 32) == 233) || (((dword & 0x1ff00000000) >> 32) == 234);
    }

    static bool IsArch0VopcDefault(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch0VopcInstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch0VopcVopSdwaSdstEncHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch0Sop1Default(uint64_t dword) {
        return ((dword & 0xff)) != 255;
    }

    static bool IsArch0Sop1InstLiteralHasLit0(uint64_t dword) {
        return ((dword & 0xff)) == 255;
    }

    static bool IsArch0Sop2Default(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch0Sop2InstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch0Sop2InstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch0Sop2InstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch0SopcDefault(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch0SopcInstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch0SopcInstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch0SopcInstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch0Vop1Default(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch0Vop1InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch0Vop1VopDppHasDpp(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch0Vop1VopSdwaHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch0Vop2Default(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch0Vop2InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch0Vop2InstLiteralDefault(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch0Vop2VopDppHasDpp(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch0Vop2VopSdwaHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch0Vop2VopSdwaSdstEncHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch1VopcDefault(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch1VopcInstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch1VopcVopSdwaSdstEncHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch1Sop1Default(uint64_t dword) {
        return ((dword & 0xff)) != 255;
    }

    static bool IsArch1Sop1InstLiteralHasLit0(uint64_t dword) {
        return ((dword & 0xff)) == 255;
    }

    static bool IsArch1Sop2Default(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch1Sop2InstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch1Sop2InstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch1Sop2InstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch1SopcDefault(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch1SopcInstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch1SopcInstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch1SopcInstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch1Vop1Default(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch1Vop1InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch1Vop1VopDppHasDpp(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch1Vop1VopSdwaHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch1Vop2Default(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch1Vop2InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch1Vop2InstLiteralDefault(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch1Vop2VopDppHasDpp(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch1Vop2VopSdwaHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch1Vop2VopSdwaSdstEncHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch2VopcDefault(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch2VopcInstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch2VopcVopSdwaSdstEncHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch2Sop1Default(uint64_t dword) {
        return ((dword & 0xff)) != 255;
    }

    static bool IsArch2Sop1InstLiteralHasLit0(uint64_t dword) {
        return ((dword & 0xff)) == 255;
    }

    static bool IsArch2Sop2Default(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch2Sop2InstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch2Sop2InstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch2Sop2InstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch2SopcDefault(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch2SopcInstLiteralHasLit0(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) != 255);
    }

    static bool IsArch2SopcInstLiteralHasLit1(uint64_t dword) {
        return (((dword & 0xff)) != 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch2SopcInstLiteralHasLit0HasLit1(uint64_t dword) {
        return (((dword & 0xff)) == 255) && (((dword & 0xff00) >> 8) == 255);
    }

    static bool IsArch2Vop1Default(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch2Vop1InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch2Vop1VopDppHasDpp(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch2Vop1VopSdwaHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch2Vop2Default(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch2Vop2InstLiteralHasLit(uint64_t dword) {
        return ((dword & 0x1ff)) == 255;
    }

    static bool IsArch2Vop2InstLiteralDefault(uint64_t dword) {
        return ((((dword & 0x1ff)) != 250) && (((dword & 0x1ff)) != 255)) && (((dword & 0x1ff)) != 249);
    }

    static bool IsArch2Vop2VopDppHasDpp(uint64_t dword) {
        return ((dword & 0x1ff)) == 250;
    }

    static bool IsArch2Vop2VopSdwaHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    static bool IsArch2Vop2VopSdwaSdstEncHasSdwa(uint64_t dword) {
        return ((dword & 0x1ff)) == 249;
    }

    struct EncodingConditionHandler
    {
        EncodingConditionHandler()
        {
            arch_conditions_[12]["VOPC_default"] = IsArch12VopcDefault;
            arch_conditions_[12]["VOPC_INST_LITERAL_has_lit"] = IsArch12VopcInstLiteralHasLit;
            arch_conditions_[12]["VOPC_VOP_SDWA_SDST_ENC_has_sdwa"] = IsArch12VopcVopSdwaSdstEncHasSdwa;
            arch_conditions_[12]["SOP1_default"] = IsArch12Sop1Default;
            arch_conditions_[12]["SOP1_INST_LITERAL_has_lit_0"] = IsArch12Sop1InstLiteralHasLit0;
            arch_conditions_[12]["SOP2_default"] = IsArch12Sop2Default;
            arch_conditions_[12]["SOP2_INST_LITERAL_has_lit_0"] = IsArch12Sop2InstLiteralHasLit0;
            arch_conditions_[12]["SOP2_INST_LITERAL_has_lit_1"] = IsArch12Sop2InstLiteralHasLit1;
            arch_conditions_[12]["SOP2_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch12Sop2InstLiteralHasLit0HasLit1;
            arch_conditions_[12]["SOPC_default"] = IsArch12SopcDefault;
            arch_conditions_[12]["SOPC_INST_LITERAL_has_lit_0"] = IsArch12SopcInstLiteralHasLit0;
            arch_conditions_[12]["SOPC_INST_LITERAL_has_lit_1"] = IsArch12SopcInstLiteralHasLit1;
            arch_conditions_[12]["SOPC_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch12SopcInstLiteralHasLit0HasLit1;
            arch_conditions_[12]["VOP1_default"] = IsArch12Vop1Default;
            arch_conditions_[12]["VOP1_INST_LITERAL_has_lit"] = IsArch12Vop1InstLiteralHasLit;
            arch_conditions_[12]["VOP1_VOP_DPP_has_dpp"] = IsArch12Vop1VopDppHasDpp;
            arch_conditions_[12]["VOP1_VOP_SDWA_has_sdwa"] = IsArch12Vop1VopSdwaHasSdwa;
            arch_conditions_[12]["VOP2_default"] = IsArch12Vop2Default;
            arch_conditions_[12]["VOP2_INST_LITERAL_has_lit"] = IsArch12Vop2InstLiteralHasLit;
            arch_conditions_[12]["VOP2_INST_LITERAL_default"] = IsArch12Vop2InstLiteralDefault;
            arch_conditions_[12]["VOP2_VOP_DPP_has_dpp"] = IsArch12Vop2VopDppHasDpp;
            arch_conditions_[12]["VOP2_VOP_SDWA_has_sdwa"] = IsArch12Vop2VopSdwaHasSdwa;
            arch_conditions_[12]["VOP2_VOP_SDWA_SDST_ENC_has_sdwa"] = IsArch12Vop2VopSdwaSdstEncHasSdwa;
            arch_conditions_[5]["VOP2_VOP_DPP16_default"] = IsArch5Vop2VopDpp16Default;
            arch_conditions_[5]["VOP2_VOP_DPP8_default"] = IsArch5Vop2VopDpp8Default;
            arch_conditions_[5]["VOP2_default"] = IsArch5Vop2Default;
            arch_conditions_[5]["VOP2_INST_LITERAL_has_lit"] = IsArch5Vop2InstLiteralHasLit;
            arch_conditions_[5]["VOP2_INST_LITERAL_default"] = IsArch5Vop2InstLiteralDefault;
            arch_conditions_[5]["VOP3_default"] = IsArch5Vop3Default;
            arch_conditions_[5]["VOP3_INST_LITERAL_has_lit_0"] = IsArch5Vop3InstLiteralHasLit0;
            arch_conditions_[5]["VOP3_INST_LITERAL_has_lit_1"] = IsArch5Vop3InstLiteralHasLit1;
            arch_conditions_[5]["VOP3_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch5Vop3InstLiteralHasLit0HasLit1;
            arch_conditions_[5]["VOP3_INST_LITERAL_has_lit_2"] = IsArch5Vop3InstLiteralHasLit2;
            arch_conditions_[5]["VOP3_INST_LITERAL_has_lit_0_has_lit_2"] = IsArch5Vop3InstLiteralHasLit0HasLit2;
            arch_conditions_[5]["VOP3_INST_LITERAL_has_lit_1_has_lit_2"] = IsArch5Vop3InstLiteralHasLit1HasLit2;
            arch_conditions_[5]["VOP3_INST_LITERAL_has_lit_0_has_lit_1_has_lit_2"] = IsArch5Vop3InstLiteralHasLit0HasLit1HasLit2;
            arch_conditions_[5]["VOP3_SDST_ENC_default"] = IsArch5Vop3SdstEncDefault;
            arch_conditions_[5]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0"] = IsArch5Vop3SdstEncInstLiteralHasLit0;
            arch_conditions_[5]["VOP3_SDST_ENC_INST_LITERAL_has_lit_1"] = IsArch5Vop3SdstEncInstLiteralHasLit1;
            arch_conditions_[5]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch5Vop3SdstEncInstLiteralHasLit0HasLit1;
            arch_conditions_[5]["VOP3_SDST_ENC_INST_LITERAL_has_lit_2"] = IsArch5Vop3SdstEncInstLiteralHasLit2;
            arch_conditions_[5]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0_has_lit_2"] = IsArch5Vop3SdstEncInstLiteralHasLit0HasLit2;
            arch_conditions_[5]["VOP3_SDST_ENC_INST_LITERAL_has_lit_1_has_lit_2"] = IsArch5Vop3SdstEncInstLiteralHasLit1HasLit2;
            arch_conditions_[5]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0_has_lit_1_has_lit_2"] = IsArch5Vop3SdstEncInstLiteralHasLit0HasLit1HasLit2;
            arch_conditions_[5]["VOP3P_default"] = IsArch5Vop3pDefault;
            arch_conditions_[5]["VOP3P_INST_LITERAL_has_lit_0"] = IsArch5Vop3pInstLiteralHasLit0;
            arch_conditions_[5]["VOP3P_INST_LITERAL_has_lit_1"] = IsArch5Vop3pInstLiteralHasLit1;
            arch_conditions_[5]["VOP3P_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch5Vop3pInstLiteralHasLit0HasLit1;
            arch_conditions_[5]["VOP3P_INST_LITERAL_has_lit_2"] = IsArch5Vop3pInstLiteralHasLit2;
            arch_conditions_[5]["VOP3P_INST_LITERAL_has_lit_0_has_lit_2"] = IsArch5Vop3pInstLiteralHasLit0HasLit2;
            arch_conditions_[5]["VOP3P_INST_LITERAL_has_lit_1_has_lit_2"] = IsArch5Vop3pInstLiteralHasLit1HasLit2;
            arch_conditions_[5]["VOP3P_INST_LITERAL_has_lit_0_has_lit_1_has_lit_2"] = IsArch5Vop3pInstLiteralHasLit0HasLit1HasLit2;
            arch_conditions_[5]["VOPC_VOP_SDWA_SDST_ENC_default"] = IsArch5VopcVopSdwaSdstEncDefault;
            arch_conditions_[5]["VOPC_default"] = IsArch5VopcDefault;
            arch_conditions_[5]["VOPC_INST_LITERAL_has_lit"] = IsArch5VopcInstLiteralHasLit;
            arch_conditions_[5]["MIMG_default"] = IsArch5MimgDefault;
            arch_conditions_[5]["MIMG_NSA1_has_nsa_1"] = IsArch5MimgNsa1HasNsa1;
            arch_conditions_[5]["MIMG_NSA2_has_nsa_2"] = IsArch5MimgNsa2HasNsa2;
            arch_conditions_[5]["MIMG_NSA3_has_nsa_3"] = IsArch5MimgNsa3HasNsa3;
            arch_conditions_[5]["SOP1_default"] = IsArch5Sop1Default;
            arch_conditions_[5]["SOP1_INST_LITERAL_has_lit_0"] = IsArch5Sop1InstLiteralHasLit0;
            arch_conditions_[5]["SOP2_default"] = IsArch5Sop2Default;
            arch_conditions_[5]["SOP2_INST_LITERAL_has_lit_0"] = IsArch5Sop2InstLiteralHasLit0;
            arch_conditions_[5]["SOP2_INST_LITERAL_has_lit_1"] = IsArch5Sop2InstLiteralHasLit1;
            arch_conditions_[5]["SOP2_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch5Sop2InstLiteralHasLit0HasLit1;
            arch_conditions_[5]["SOPC_default"] = IsArch5SopcDefault;
            arch_conditions_[5]["SOPC_INST_LITERAL_has_lit_0"] = IsArch5SopcInstLiteralHasLit0;
            arch_conditions_[5]["SOPC_INST_LITERAL_has_lit_1"] = IsArch5SopcInstLiteralHasLit1;
            arch_conditions_[5]["SOPC_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch5SopcInstLiteralHasLit0HasLit1;
            arch_conditions_[5]["VOP1_default"] = IsArch5Vop1Default;
            arch_conditions_[5]["VOP1_VOP_DPP16_default"] = IsArch5Vop1VopDpp16Default;
            arch_conditions_[5]["VOP1_VOP_DPP8_default"] = IsArch5Vop1VopDpp8Default;
            arch_conditions_[5]["VOP1_VOP_SDWA_default"] = IsArch5Vop1VopSdwaDefault;
            arch_conditions_[5]["VOP1_INST_LITERAL_has_lit"] = IsArch5Vop1InstLiteralHasLit;
            arch_conditions_[5]["VOP2_VOP_SDWA_default"] = IsArch5Vop2VopSdwaDefault;
            arch_conditions_[5]["VOP2_VOP_SDWA_SDST_ENC_default"] = IsArch5Vop2VopSdwaSdstEncDefault;
            arch_conditions_[6]["VOP3_default"] = IsArch6Vop3Default;
            arch_conditions_[6]["VOP3_INST_LITERAL_has_lit_0"] = IsArch6Vop3InstLiteralHasLit0;
            arch_conditions_[6]["VOP3_INST_LITERAL_has_lit_1"] = IsArch6Vop3InstLiteralHasLit1;
            arch_conditions_[6]["VOP3_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch6Vop3InstLiteralHasLit0HasLit1;
            arch_conditions_[6]["VOP3_INST_LITERAL_has_lit_2"] = IsArch6Vop3InstLiteralHasLit2;
            arch_conditions_[6]["VOP3_INST_LITERAL_has_lit_0_has_lit_2"] = IsArch6Vop3InstLiteralHasLit0HasLit2;
            arch_conditions_[6]["VOP3_INST_LITERAL_has_lit_1_has_lit_2"] = IsArch6Vop3InstLiteralHasLit1HasLit2;
            arch_conditions_[6]["VOP3_INST_LITERAL_has_lit_0_has_lit_1_has_lit_2"] = IsArch6Vop3InstLiteralHasLit0HasLit1HasLit2;
            arch_conditions_[6]["VOP3P_default"] = IsArch6Vop3pDefault;
            arch_conditions_[6]["VOP3P_INST_LITERAL_has_lit_0"] = IsArch6Vop3pInstLiteralHasLit0;
            arch_conditions_[6]["VOP3P_INST_LITERAL_has_lit_1"] = IsArch6Vop3pInstLiteralHasLit1;
            arch_conditions_[6]["VOP3P_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch6Vop3pInstLiteralHasLit0HasLit1;
            arch_conditions_[6]["VOP3P_INST_LITERAL_has_lit_2"] = IsArch6Vop3pInstLiteralHasLit2;
            arch_conditions_[6]["VOP3P_INST_LITERAL_has_lit_0_has_lit_2"] = IsArch6Vop3pInstLiteralHasLit0HasLit2;
            arch_conditions_[6]["VOP3P_INST_LITERAL_has_lit_1_has_lit_2"] = IsArch6Vop3pInstLiteralHasLit1HasLit2;
            arch_conditions_[6]["VOP3P_INST_LITERAL_has_lit_0_has_lit_1_has_lit_2"] = IsArch6Vop3pInstLiteralHasLit0HasLit1HasLit2;
            arch_conditions_[6]["VOPC_VOP_SDWA_SDST_ENC_default"] = IsArch6VopcVopSdwaSdstEncDefault;
            arch_conditions_[6]["VOPC_default"] = IsArch6VopcDefault;
            arch_conditions_[6]["VOPC_INST_LITERAL_has_lit"] = IsArch6VopcInstLiteralHasLit;
            arch_conditions_[6]["MIMG_default"] = IsArch6MimgDefault;
            arch_conditions_[6]["MIMG_NSA1_has_nsa_1"] = IsArch6MimgNsa1HasNsa1;
            arch_conditions_[6]["MIMG_NSA2_has_nsa_2"] = IsArch6MimgNsa2HasNsa2;
            arch_conditions_[6]["MIMG_NSA3_has_nsa_3"] = IsArch6MimgNsa3HasNsa3;
            arch_conditions_[6]["SOP1_default"] = IsArch6Sop1Default;
            arch_conditions_[6]["SOP1_INST_LITERAL_has_lit_0"] = IsArch6Sop1InstLiteralHasLit0;
            arch_conditions_[6]["SOP2_default"] = IsArch6Sop2Default;
            arch_conditions_[6]["SOP2_INST_LITERAL_has_lit_0"] = IsArch6Sop2InstLiteralHasLit0;
            arch_conditions_[6]["SOP2_INST_LITERAL_has_lit_1"] = IsArch6Sop2InstLiteralHasLit1;
            arch_conditions_[6]["SOP2_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch6Sop2InstLiteralHasLit0HasLit1;
            arch_conditions_[6]["SOPC_default"] = IsArch6SopcDefault;
            arch_conditions_[6]["SOPC_INST_LITERAL_has_lit_0"] = IsArch6SopcInstLiteralHasLit0;
            arch_conditions_[6]["SOPC_INST_LITERAL_has_lit_1"] = IsArch6SopcInstLiteralHasLit1;
            arch_conditions_[6]["SOPC_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch6SopcInstLiteralHasLit0HasLit1;
            arch_conditions_[6]["VOP1_default"] = IsArch6Vop1Default;
            arch_conditions_[6]["VOP1_VOP_DPP16_default"] = IsArch6Vop1VopDpp16Default;
            arch_conditions_[6]["VOP1_VOP_DPP8_default"] = IsArch6Vop1VopDpp8Default;
            arch_conditions_[6]["VOP1_VOP_SDWA_default"] = IsArch6Vop1VopSdwaDefault;
            arch_conditions_[6]["VOP1_INST_LITERAL_has_lit"] = IsArch6Vop1InstLiteralHasLit;
            arch_conditions_[6]["VOP2_VOP_DPP16_default"] = IsArch6Vop2VopDpp16Default;
            arch_conditions_[6]["VOP2_VOP_DPP8_default"] = IsArch6Vop2VopDpp8Default;
            arch_conditions_[6]["VOP2_VOP_SDWA_default"] = IsArch6Vop2VopSdwaDefault;
            arch_conditions_[6]["VOP2_default"] = IsArch6Vop2Default;
            arch_conditions_[6]["VOP2_INST_LITERAL_has_lit"] = IsArch6Vop2InstLiteralHasLit;
            arch_conditions_[6]["VOP2_INST_LITERAL_default"] = IsArch6Vop2InstLiteralDefault;
            arch_conditions_[6]["VOP2_VOP_SDWA_SDST_ENC_default"] = IsArch6Vop2VopSdwaSdstEncDefault;
            arch_conditions_[6]["VOP3_SDST_ENC_default"] = IsArch6Vop3SdstEncDefault;
            arch_conditions_[6]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0"] = IsArch6Vop3SdstEncInstLiteralHasLit0;
            arch_conditions_[6]["VOP3_SDST_ENC_INST_LITERAL_has_lit_1"] = IsArch6Vop3SdstEncInstLiteralHasLit1;
            arch_conditions_[6]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch6Vop3SdstEncInstLiteralHasLit0HasLit1;
            arch_conditions_[6]["VOP3_SDST_ENC_INST_LITERAL_has_lit_2"] = IsArch6Vop3SdstEncInstLiteralHasLit2;
            arch_conditions_[6]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0_has_lit_2"] = IsArch6Vop3SdstEncInstLiteralHasLit0HasLit2;
            arch_conditions_[6]["VOP3_SDST_ENC_INST_LITERAL_has_lit_1_has_lit_2"] = IsArch6Vop3SdstEncInstLiteralHasLit1HasLit2;
            arch_conditions_[6]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0_has_lit_1_has_lit_2"] = IsArch6Vop3SdstEncInstLiteralHasLit0HasLit1HasLit2;
            arch_conditions_[8]["VOP3_default"] = IsArch8Vop3Default;
            arch_conditions_[8]["VOP3_INST_LITERAL_has_lit_0"] = IsArch8Vop3InstLiteralHasLit0;
            arch_conditions_[8]["VOP3_INST_LITERAL_has_lit_1"] = IsArch8Vop3InstLiteralHasLit1;
            arch_conditions_[8]["VOP3_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch8Vop3InstLiteralHasLit0HasLit1;
            arch_conditions_[8]["VOP3_INST_LITERAL_has_lit_2"] = IsArch8Vop3InstLiteralHasLit2;
            arch_conditions_[8]["VOP3_INST_LITERAL_has_lit_0_has_lit_2"] = IsArch8Vop3InstLiteralHasLit0HasLit2;
            arch_conditions_[8]["VOP3_INST_LITERAL_has_lit_1_has_lit_2"] = IsArch8Vop3InstLiteralHasLit1HasLit2;
            arch_conditions_[8]["VOP3_INST_LITERAL_has_lit_0_has_lit_1_has_lit_2"] = IsArch8Vop3InstLiteralHasLit0HasLit1HasLit2;
            arch_conditions_[8]["VOP3_VOP_DPP16_default"] = IsArch8Vop3VopDpp16Default;
            arch_conditions_[8]["VOP3_VOP_DPP8_default"] = IsArch8Vop3VopDpp8Default;
            arch_conditions_[8]["VOP3P_default"] = IsArch8Vop3pDefault;
            arch_conditions_[8]["VOP3P_INST_LITERAL_has_lit_0"] = IsArch8Vop3pInstLiteralHasLit0;
            arch_conditions_[8]["VOP3P_INST_LITERAL_has_lit_1"] = IsArch8Vop3pInstLiteralHasLit1;
            arch_conditions_[8]["VOP3P_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch8Vop3pInstLiteralHasLit0HasLit1;
            arch_conditions_[8]["VOP3P_INST_LITERAL_has_lit_2"] = IsArch8Vop3pInstLiteralHasLit2;
            arch_conditions_[8]["VOP3P_INST_LITERAL_has_lit_0_has_lit_2"] = IsArch8Vop3pInstLiteralHasLit0HasLit2;
            arch_conditions_[8]["VOP3P_INST_LITERAL_has_lit_1_has_lit_2"] = IsArch8Vop3pInstLiteralHasLit1HasLit2;
            arch_conditions_[8]["VOP3P_INST_LITERAL_has_lit_0_has_lit_1_has_lit_2"] = IsArch8Vop3pInstLiteralHasLit0HasLit1HasLit2;
            arch_conditions_[8]["VOP3P_VOP_DPP16_default"] = IsArch8Vop3pVopDpp16Default;
            arch_conditions_[8]["VOP3P_VOP_DPP8_default"] = IsArch8Vop3pVopDpp8Default;
            arch_conditions_[8]["VOPC_VOP_DPP16_default"] = IsArch8VopcVopDpp16Default;
            arch_conditions_[8]["VOPC_VOP_DPP8_default"] = IsArch8VopcVopDpp8Default;
            arch_conditions_[8]["VOPC_default"] = IsArch8VopcDefault;
            arch_conditions_[8]["VOPC_INST_LITERAL_has_lit"] = IsArch8VopcInstLiteralHasLit;
            arch_conditions_[8]["MIMG_default"] = IsArch8MimgDefault;
            arch_conditions_[8]["MIMG_NSA1_has_nsa"] = IsArch8MimgNsa1HasNsa;
            arch_conditions_[8]["SOP1_default"] = IsArch8Sop1Default;
            arch_conditions_[8]["SOP1_INST_LITERAL_has_lit_0"] = IsArch8Sop1InstLiteralHasLit0;
            arch_conditions_[8]["SOP2_default"] = IsArch8Sop2Default;
            arch_conditions_[8]["SOP2_INST_LITERAL_has_lit_0"] = IsArch8Sop2InstLiteralHasLit0;
            arch_conditions_[8]["SOP2_INST_LITERAL_has_lit_1"] = IsArch8Sop2InstLiteralHasLit1;
            arch_conditions_[8]["SOP2_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch8Sop2InstLiteralHasLit0HasLit1;
            arch_conditions_[8]["SOPC_default"] = IsArch8SopcDefault;
            arch_conditions_[8]["SOPC_INST_LITERAL_has_lit_0"] = IsArch8SopcInstLiteralHasLit0;
            arch_conditions_[8]["SOPC_INST_LITERAL_has_lit_1"] = IsArch8SopcInstLiteralHasLit1;
            arch_conditions_[8]["SOPC_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch8SopcInstLiteralHasLit0HasLit1;
            arch_conditions_[8]["VOP1_default"] = IsArch8Vop1Default;
            arch_conditions_[8]["VOP1_VOP_DPP16_default"] = IsArch8Vop1VopDpp16Default;
            arch_conditions_[8]["VOP1_VOP_DPP8_default"] = IsArch8Vop1VopDpp8Default;
            arch_conditions_[8]["VOP1_INST_LITERAL_has_lit"] = IsArch8Vop1InstLiteralHasLit;
            arch_conditions_[8]["VOP2_VOP_DPP16_default"] = IsArch8Vop2VopDpp16Default;
            arch_conditions_[8]["VOP2_VOP_DPP8_default"] = IsArch8Vop2VopDpp8Default;
            arch_conditions_[8]["VOP2_default"] = IsArch8Vop2Default;
            arch_conditions_[8]["VOP2_INST_LITERAL_has_lit"] = IsArch8Vop2InstLiteralHasLit;
            arch_conditions_[8]["VOP2_INST_LITERAL_default"] = IsArch8Vop2InstLiteralDefault;
            arch_conditions_[8]["VOP3_SDST_ENC_default"] = IsArch8Vop3SdstEncDefault;
            arch_conditions_[8]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0"] = IsArch8Vop3SdstEncInstLiteralHasLit0;
            arch_conditions_[8]["VOP3_SDST_ENC_INST_LITERAL_has_lit_1"] = IsArch8Vop3SdstEncInstLiteralHasLit1;
            arch_conditions_[8]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch8Vop3SdstEncInstLiteralHasLit0HasLit1;
            arch_conditions_[8]["VOP3_SDST_ENC_INST_LITERAL_has_lit_2"] = IsArch8Vop3SdstEncInstLiteralHasLit2;
            arch_conditions_[8]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0_has_lit_2"] = IsArch8Vop3SdstEncInstLiteralHasLit0HasLit2;
            arch_conditions_[8]["VOP3_SDST_ENC_INST_LITERAL_has_lit_1_has_lit_2"] = IsArch8Vop3SdstEncInstLiteralHasLit1HasLit2;
            arch_conditions_[8]["VOP3_SDST_ENC_INST_LITERAL_has_lit_0_has_lit_1_has_lit_2"] = IsArch8Vop3SdstEncInstLiteralHasLit0HasLit1HasLit2;
            arch_conditions_[8]["VOP3_SDST_ENC_VOP_DPP16_default"] = IsArch8Vop3SdstEncVopDpp16Default;
            arch_conditions_[8]["VOP3_SDST_ENC_VOP_DPP8_default"] = IsArch8Vop3SdstEncVopDpp8Default;
            arch_conditions_[0]["VOPC_default"] = IsArch0VopcDefault;
            arch_conditions_[0]["VOPC_INST_LITERAL_has_lit"] = IsArch0VopcInstLiteralHasLit;
            arch_conditions_[0]["VOPC_VOP_SDWA_SDST_ENC_has_sdwa"] = IsArch0VopcVopSdwaSdstEncHasSdwa;
            arch_conditions_[0]["SOP1_default"] = IsArch0Sop1Default;
            arch_conditions_[0]["SOP1_INST_LITERAL_has_lit_0"] = IsArch0Sop1InstLiteralHasLit0;
            arch_conditions_[0]["SOP2_default"] = IsArch0Sop2Default;
            arch_conditions_[0]["SOP2_INST_LITERAL_has_lit_0"] = IsArch0Sop2InstLiteralHasLit0;
            arch_conditions_[0]["SOP2_INST_LITERAL_has_lit_1"] = IsArch0Sop2InstLiteralHasLit1;
            arch_conditions_[0]["SOP2_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch0Sop2InstLiteralHasLit0HasLit1;
            arch_conditions_[0]["SOPC_default"] = IsArch0SopcDefault;
            arch_conditions_[0]["SOPC_INST_LITERAL_has_lit_0"] = IsArch0SopcInstLiteralHasLit0;
            arch_conditions_[0]["SOPC_INST_LITERAL_has_lit_1"] = IsArch0SopcInstLiteralHasLit1;
            arch_conditions_[0]["SOPC_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch0SopcInstLiteralHasLit0HasLit1;
            arch_conditions_[0]["VOP1_default"] = IsArch0Vop1Default;
            arch_conditions_[0]["VOP1_INST_LITERAL_has_lit"] = IsArch0Vop1InstLiteralHasLit;
            arch_conditions_[0]["VOP1_VOP_DPP_has_dpp"] = IsArch0Vop1VopDppHasDpp;
            arch_conditions_[0]["VOP1_VOP_SDWA_has_sdwa"] = IsArch0Vop1VopSdwaHasSdwa;
            arch_conditions_[0]["VOP2_default"] = IsArch0Vop2Default;
            arch_conditions_[0]["VOP2_INST_LITERAL_has_lit"] = IsArch0Vop2InstLiteralHasLit;
            arch_conditions_[0]["VOP2_INST_LITERAL_default"] = IsArch0Vop2InstLiteralDefault;
            arch_conditions_[0]["VOP2_VOP_DPP_has_dpp"] = IsArch0Vop2VopDppHasDpp;
            arch_conditions_[0]["VOP2_VOP_SDWA_has_sdwa"] = IsArch0Vop2VopSdwaHasSdwa;
            arch_conditions_[0]["VOP2_VOP_SDWA_SDST_ENC_has_sdwa"] = IsArch0Vop2VopSdwaSdstEncHasSdwa;
            arch_conditions_[1]["VOPC_default"] = IsArch1VopcDefault;
            arch_conditions_[1]["VOPC_INST_LITERAL_has_lit"] = IsArch1VopcInstLiteralHasLit;
            arch_conditions_[1]["VOPC_VOP_SDWA_SDST_ENC_has_sdwa"] = IsArch1VopcVopSdwaSdstEncHasSdwa;
            arch_conditions_[1]["SOP1_default"] = IsArch1Sop1Default;
            arch_conditions_[1]["SOP1_INST_LITERAL_has_lit_0"] = IsArch1Sop1InstLiteralHasLit0;
            arch_conditions_[1]["SOP2_default"] = IsArch1Sop2Default;
            arch_conditions_[1]["SOP2_INST_LITERAL_has_lit_0"] = IsArch1Sop2InstLiteralHasLit0;
            arch_conditions_[1]["SOP2_INST_LITERAL_has_lit_1"] = IsArch1Sop2InstLiteralHasLit1;
            arch_conditions_[1]["SOP2_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch1Sop2InstLiteralHasLit0HasLit1;
            arch_conditions_[1]["SOPC_default"] = IsArch1SopcDefault;
            arch_conditions_[1]["SOPC_INST_LITERAL_has_lit_0"] = IsArch1SopcInstLiteralHasLit0;
            arch_conditions_[1]["SOPC_INST_LITERAL_has_lit_1"] = IsArch1SopcInstLiteralHasLit1;
            arch_conditions_[1]["SOPC_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch1SopcInstLiteralHasLit0HasLit1;
            arch_conditions_[1]["VOP1_default"] = IsArch1Vop1Default;
            arch_conditions_[1]["VOP1_INST_LITERAL_has_lit"] = IsArch1Vop1InstLiteralHasLit;
            arch_conditions_[1]["VOP1_VOP_DPP_has_dpp"] = IsArch1Vop1VopDppHasDpp;
            arch_conditions_[1]["VOP1_VOP_SDWA_has_sdwa"] = IsArch1Vop1VopSdwaHasSdwa;
            arch_conditions_[1]["VOP2_default"] = IsArch1Vop2Default;
            arch_conditions_[1]["VOP2_INST_LITERAL_has_lit"] = IsArch1Vop2InstLiteralHasLit;
            arch_conditions_[1]["VOP2_INST_LITERAL_default"] = IsArch1Vop2InstLiteralDefault;
            arch_conditions_[1]["VOP2_VOP_DPP_has_dpp"] = IsArch1Vop2VopDppHasDpp;
            arch_conditions_[1]["VOP2_VOP_SDWA_has_sdwa"] = IsArch1Vop2VopSdwaHasSdwa;
            arch_conditions_[1]["VOP2_VOP_SDWA_SDST_ENC_has_sdwa"] = IsArch1Vop2VopSdwaSdstEncHasSdwa;
            arch_conditions_[2]["VOPC_default"] = IsArch2VopcDefault;
            arch_conditions_[2]["VOPC_INST_LITERAL_has_lit"] = IsArch2VopcInstLiteralHasLit;
            arch_conditions_[2]["VOPC_VOP_SDWA_SDST_ENC_has_sdwa"] = IsArch2VopcVopSdwaSdstEncHasSdwa;
            arch_conditions_[2]["SOP1_default"] = IsArch2Sop1Default;
            arch_conditions_[2]["SOP1_INST_LITERAL_has_lit_0"] = IsArch2Sop1InstLiteralHasLit0;
            arch_conditions_[2]["SOP2_default"] = IsArch2Sop2Default;
            arch_conditions_[2]["SOP2_INST_LITERAL_has_lit_0"] = IsArch2Sop2InstLiteralHasLit0;
            arch_conditions_[2]["SOP2_INST_LITERAL_has_lit_1"] = IsArch2Sop2InstLiteralHasLit1;
            arch_conditions_[2]["SOP2_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch2Sop2InstLiteralHasLit0HasLit1;
            arch_conditions_[2]["SOPC_default"] = IsArch2SopcDefault;
            arch_conditions_[2]["SOPC_INST_LITERAL_has_lit_0"] = IsArch2SopcInstLiteralHasLit0;
            arch_conditions_[2]["SOPC_INST_LITERAL_has_lit_1"] = IsArch2SopcInstLiteralHasLit1;
            arch_conditions_[2]["SOPC_INST_LITERAL_has_lit_0_has_lit_1"] = IsArch2SopcInstLiteralHasLit0HasLit1;
            arch_conditions_[2]["VOP1_default"] = IsArch2Vop1Default;
            arch_conditions_[2]["VOP1_INST_LITERAL_has_lit"] = IsArch2Vop1InstLiteralHasLit;
            arch_conditions_[2]["VOP1_VOP_DPP_has_dpp"] = IsArch2Vop1VopDppHasDpp;
            arch_conditions_[2]["VOP1_VOP_SDWA_has_sdwa"] = IsArch2Vop1VopSdwaHasSdwa;
            arch_conditions_[2]["VOP2_default"] = IsArch2Vop2Default;
            arch_conditions_[2]["VOP2_INST_LITERAL_has_lit"] = IsArch2Vop2InstLiteralHasLit;
            arch_conditions_[2]["VOP2_INST_LITERAL_default"] = IsArch2Vop2InstLiteralDefault;
            arch_conditions_[2]["VOP2_VOP_DPP_has_dpp"] = IsArch2Vop2VopDppHasDpp;
            arch_conditions_[2]["VOP2_VOP_SDWA_has_sdwa"] = IsArch2Vop2VopSdwaHasSdwa;
            arch_conditions_[2]["VOP2_VOP_SDWA_SDST_ENC_has_sdwa"] = IsArch2Vop2VopSdwaSdstEncHasSdwa;
        }

        using EncodingsToConditionFunctions = std::map<std::string, std::function<bool(uint64_t)>>;
        std::map<uint32_t, EncodingsToConditionFunctions> arch_conditions_;
    };
}
#endif // ENCODING_CONDITION_HANDLER_HPP_
