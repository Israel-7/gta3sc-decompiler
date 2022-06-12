#pragma once

#ifndef OUTPUT_H
#define OUTPUT_H

#define _CRT_SECURE_NO_WARNINGS
#include "decompile.h"
#include "rapidxml.hpp"

#include <algorithm>
#include <fstream>
#include <functional>
#include <map>
#include <stdarg.h>
#include <vector>
#include <windows.h>

using namespace rapidxml;

enum Commands
{
    GOTO = 0x0002,
    GOTO_IF_FALSE = 0x004D,
    START_NEW_SCRIPT = 0x004F,
    GOSUB = 0x0050,
    LAUNCH_MISSION = 0x00D7,
    SKIP_CUTSCENE_START_INTERNAL = 0x0707,
    SWITCH_START = 0x0871,
    SWITCH_CONTINUED = 0x0872,
    ELSE_GOSUB = 0x0AA0,
    CLEO_CALL = 0x0AB1,
    CLEO_RETURN = 0x0AB2,
    GET_LABEL_POINTER = 0x0AC6,

    IF = 0x00D6
};

enum Types
{
    _INT,
    _FLOAT,
    _TEXT_LABEL,
    _STRING,
    _LABEL,
    _CONSTANT,
    _PARAM
};

struct Files {
    const char *input;
    const char *output;

    Files() :
        input(nullptr),
        output(nullptr)
    {}
};

enum class Mode {
    None,
    CS,
    CM,
    SCM
};

struct Options {
    bool help;
    bool about;
    bool hexlabel;
    bool clip;
    bool ignoreunk;

    Options() :
        help(false), about(false),
        hexlabel(false), clip(false),
        ignoreunk(false)
    {}
};

class output {
    WORD currentOpcode;

    std::map<WORD, std::string> commands;
    std::map<WORD, uint32_t> argsc;
    std::map<WORD, std::vector<byte>> types;

    std::vector<int> labelsOffsets;

    std::vector<int> ints;
    std::vector<int> floats;
    std::vector<int> texts;
    std::vector<int> texts16;

    char tempBuffer[512];

    Files files;
    Mode mode;
    Options options;

public:
    std::string out;

    size_t point();
    void clear();
    void newLine(size_t);
    void tab(size_t);
    int argsCount();

    size_t printf(const char*, ...);
    bool addCommand(WORD, class script&);

    void addByte(BYTE, size_t);
    void addWord(WORD);
    void addDword(DWORD, size_t);
    void addFloat(FLOAT);

    void addLVar(WORD, size_t);
    void addLVarTextLabel(WORD, size_t);
    void addLVarString(WORD);

    void addTextLabel(std::string);
    void addVarLenString(std::string);

    void addVar(WORD);

    void addLVarArray(WORD, WORD);
    void addVarArray(WORD, WORD);

    bool isCmdOperator();

    template <class T>
    bool varAdded(T);

    void createLabels(std::map<int, size_t>);
    bool clip(const std::string&);

    output(Files, Mode, Options);
    ~output();
};

#endif
