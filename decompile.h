#pragma once

#ifndef DECOMPILE_H
#define DECOMPILE_H

#include "output.h"

#include <stdio.h>
#include <windows.h>

enum DataType
{
    DT_END,
    DT_DWORD,
    DT_VAR,
    DT_LVAR,
    DT_BYTE,
    DT_WORD,
    DT_FLOAT,
    DT_VAR_ARRAY,
    DT_LVAR_ARRAY,
    DT_TEXTLABEL,
    DT_VAR_TEXTLABEL,
    DT_LVAR_TEXTLABEL,
    DT_VAR_TEXTLABEL_ARRAY,
    DT_LVAR_TEXTLABEL_ARRAY,
    DT_VARLEN_STRING,
    DT_STRING,
    DT_VAR_STRING,
    DT_LVAR_STRING,
    DT_VAR_STRING_ARRAY,
    DT_LVAR_STRING_ARRAY
};

int decompile(byte*, size_t, struct Files, enum class Mode, struct Options);

class script
{
    byte *bytecode;
public:

    template <class T> T getData();
    char *getString(char*, size_t);

    void jumpPos(size_t);
    BYTE *currBytes();

    WORD getOpcode();
    BYTE getDataType();

    script(byte *sc) : bytecode(sc) {}
    script() = default;
};

#endif
