#include "output.h"

template <class T> T script::getData()
{
    T value = *(T*)bytecode;
    bytecode += sizeof(T);

    return value;
}

char* script::getString(char* out, size_t size)
{
    strncpy(out, reinterpret_cast<char*>(bytecode), size);

    out[size] = 0;
    jumpPos(size);

    return out;
}

void script::jumpPos(size_t pos) {
    bytecode += pos;
}

BYTE script::getDataType() {
    return getData<BYTE>();
}

WORD script::getOpcode() {
    return getData<WORD>();
}

BYTE* script::currBytes() {
    return bytecode;
}

bool output::clip(const std::string& text)
{
    if (auto ga = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1))
    {
        OpenClipboard(HWND_DESKTOP);
        EmptyClipboard();

        memcpy(GlobalLock(ga), text.c_str(), text.size() + 1);
        GlobalUnlock(ga);
        SetClipboardData(CF_TEXT, ga);

        CloseClipboard();
        GlobalFree(ga);

        return true;
    }

    return false;
}

bool operatorEQ(WORD opcode) {
    return (opcode >= 0x0004 && opcode <= 0x0007)
        // || (opcode >= 0x0038 && opcode <= 0x0046)
        || (opcode >= 0x0084 && opcode <= 0x008B)
        || (opcode == 0x04AE || opcode == 0x04AF)
        || (opcode == 0x06D1 || opcode == 0x06D2)
        // || (opcode == 0x08F9 || opcode == 0x08FA)
        || (opcode == 0x05AA || opcode == 0x05AD);
}

bool operatorADD(WORD opcode) {
    return (opcode >= 0x0008 && opcode <= 0x000B)
        || (opcode >= 0x0058 && opcode <= 0x005F);
}

bool operatorSUB(WORD opcode) {
    return (opcode >= 0x000C && opcode <= 0x000F)
        || (opcode >= 0x0060 && opcode <= 0x0067);
}

bool operatorMUL(WORD opcode) {
    return (opcode >= 0x0010 && opcode <= 0x0013)
        || (opcode >= 0x0068 && opcode <= 0x006F);
}

bool operatorDIV(WORD opcode) {
    return (opcode >= 0x0014 && opcode <= 0x0017)
        || (opcode >= 0x0070 && opcode <= 0x0077);
}

bool operatorGT(WORD opcode) {
    return (opcode >= 0x0018 && opcode <= 0x0027)
        || (opcode >= 0x04B0 && opcode <= 0x04B3);
}

bool operatorBE(WORD opcode) {
    return (opcode >= 0x0028 && opcode <= 0x0037)
        || (opcode >= 0x04B4 && opcode <= 0x04B7);
}

size_t output::point() {
    return out.length();
}

int output::argsCount() {
    return argsc[currentOpcode];
}

void output::clear() {
    out.clear();
}

void output::newLine(size_t count) {
    for (size_t i = 0; i < count; ++i) {
        out.append("\n");
    }
}

void output::tab(size_t count) {
    for (size_t i = 0; i < count; ++i) {
        out.append("\t");
    }
}

size_t output::printf(const char *format, ...)
{
    __int8 *args;

    __crt_va_start(args, format);
    size_t len = vsprintf(tempBuffer, format, args);
    __crt_va_end(args);

    out.append(tempBuffer);
    return len;
}

bool output::isCmdOperator() {
    return (currentOpcode >= 0x0004 && currentOpcode <= 0x0037)
        || (currentOpcode >= 0x0058 && currentOpcode <= 0x0077)
        || (currentOpcode >= 0x0084 && currentOpcode <= 0x008B)
        || (currentOpcode >= 0x04AE && currentOpcode <= 0x04B7)
        || (currentOpcode == 0x05AA || currentOpcode == 0x05AD)
        || (currentOpcode == 0x06D1 || currentOpcode == 0x06D2);
        //|| (currentOpcode == 0x08F9 || currentOpcode == 0x08FA);
}

bool output::addCommand(WORD opcode, script& sc_struct)
{
    currentOpcode = opcode & 0x7FFF;
    
    if (!commands[currentOpcode].empty())
    {
        signed iBackup = (size_t)sc_struct.currBytes();
        std::string sBackup = out;

        if (opcode & 0x8000) {
            printf("\n\tNOT ");
        } else {
            printf("\n\t");
        }

        if (!isCmdOperator()) {
            printf(commands[currentOpcode].c_str());
        }

        auto counter = (argsCount() == -1 ? 35 : argsCount());

        for (int i = 1; i <= counter && counter != -1; ++i)
        {
            switch (sc_struct.getDataType())
            {
            case DataType::DT_END:
                if (counter == 35) {
                    counter = -1;
                    break;
                } else {
                    sc_struct.jumpPos(-((signed)sc_struct.currBytes() - iBackup));
                    out = sBackup;

                    return false;
                }
            case DataType::DT_DWORD:
                addDword(sc_struct.getData<DWORD>(), i - 1);
                break;
            case DataType::DT_VAR:
                addVar(sc_struct.getData<WORD>());
                break;
            case DataType::DT_LVAR:
                addLVar(sc_struct.getData<WORD>(), i - 1);
                break;
            case DataType::DT_BYTE:
                addByte(sc_struct.getData<BYTE>(), i - 1);
                break;
            case DataType::DT_WORD:
                addWord(sc_struct.getData<WORD>());
                break;
            case DataType::DT_FLOAT:
                addFloat(sc_struct.getData<FLOAT>());
                break;
            case DataType::DT_VAR_ARRAY:
                //...
                break;
            case DataType::DT_LVAR_ARRAY:
                addLVarArray(sc_struct.getData<WORD>(), sc_struct.getData<WORD>());
                sc_struct.jumpPos(2);
                break;
            case DataType::DT_TEXTLABEL:
                char text[8];
                sc_struct.getString(text, sizeof(text));
                //...
                addTextLabel(text);
                break;
            case DataType::DT_VAR_TEXTLABEL:
                //...
                break;
            case DataType::DT_LVAR_TEXTLABEL:
                //...
                addLVarTextLabel(sc_struct.getData<WORD>(), i - 1);
                break;
            case DataType::DT_VAR_TEXTLABEL_ARRAY:
                //...
                break;
            case DataType::DT_LVAR_TEXTLABEL_ARRAY:
                //...
                break;
            case DataType::DT_VARLEN_STRING:
            {
                char text[0xFF];
                sc_struct.getString(text, sc_struct.getData<BYTE>());

                addVarLenString(text);
                break;
            }
            case DataType::DT_STRING:
                //...
                break;
            case DataType::DT_VAR_STRING:
                //...
                break;
            case DataType::DT_LVAR_STRING:
                addLVarString(sc_struct.getData<WORD>());
                break;
            case DataType::DT_VAR_STRING_ARRAY:
                //...
                break;
            case DataType::DT_LVAR_STRING_ARRAY:
                //...
                break;
            default:
                sc_struct.jumpPos(-((signed)sc_struct.currBytes() - iBackup));
                out = sBackup;

                return false;
            }

            if (i == 1)
            {
                if (operatorEQ(opcode & 0x7FFF)) {
                    printf(" =");
                }
                else if (operatorADD(opcode & 0x7FFF)) {
                    printf(" +=");
                }
                else if (operatorSUB(opcode & 0x7FFF)) {
                    printf(" -=");
                }
                else if (operatorMUL(opcode & 0x7FFF)) {
                    printf(" *=");
                }
                else if (operatorDIV(opcode & 0x7FFF)) {
                    printf(" /=");
                }
                else if (operatorGT(opcode & 0x7FFF)) {
                    printf(" >");
                }
                else if (operatorBE(opcode & 0x7FFF)) {
                    printf(" >=");
                }
            }
        }

        return true;
    }

    return false;
}

void output::addByte(BYTE param, size_t argIndex)
{
    // comment IF AND/OR
    switch (currentOpcode)
    {
    case Commands::IF:
        this->printf(" %d // IF", (char)param);

        if (param >= 1 && param <= 7) {
            this->printf(" AND with %d ", param + 1);
        }

        if (param >= 21 && param <= 27) {
            this->printf(" OR with %d ", param - 19);
        }

        if (param != 0) {
            this->printf("conditions.");
        }

        return;
    case Commands::CLEO_CALL:
        if (argIndex == 1) {
            param = 0;
            break;
        }
    case Commands::CLEO_RETURN:
        if (argIndex == 0) param = 0;
    }

    this->printf(" %d", (char)param);
}

void output::addDword(DWORD param, size_t argIndex)
{
    switch (currentOpcode)
    {
    case GOTO_IF_FALSE:
    case GOTO:
    case GOSUB:
    case ELSE_GOSUB:
    case SWITCH_START:
    case SWITCH_CONTINUED:
    case CLEO_CALL:
    case SKIP_CUTSCENE_START_INTERNAL:
    case GET_LABEL_POINTER:
    case START_NEW_SCRIPT:
    case LAUNCH_MISSION:
        if (types[currentOpcode].size() > 0)
        {
            if (types[currentOpcode][argIndex] == Types::_LABEL)
            {
                if (std::find(labelsOffsets.begin(), labelsOffsets.end(), -(signed)param) == labelsOffsets.end()) {
                    labelsOffsets.push_back(-(signed)param);
                }

                if (options.hexlabel) {
                    this->printf(" label_%X", -(signed)param);
                } else {
                    this->printf(" label_%d", -(signed)param);
                }

                break;
            }
        }
    default:
        this->printf(" %d", param);
    }
}

void output::addFloat(FLOAT param)
{
    auto point = this->printf(" %.7g", param);

    if (out.substr(out.length() - point).find(".") == out.npos) {
        this->printf(".0");
    }
}

void output::addWord(WORD param) {
    this->printf(" %d", param);
}

void output::addLVar(WORD param, size_t argIndex) {
    if (types[currentOpcode].size() > 0)
    {
        switch (types[currentOpcode][argIndex])
        {
        case Types::_INT:
        case Types::_PARAM:
            this->printf(" int%d", param);

            if (std::find(ints.begin(), ints.end(), param) == ints.end()) {
                ints.push_back(param);
            }
            
            break;
        case Types::_FLOAT:
            this->printf(" float%d", param);

            if (std::find(floats.begin(), floats.end(), param) == floats.end()) {
                floats.push_back(param);
            }
            
            break;
        case Types::_TEXT_LABEL:
        case Types::_STRING:
            if (std::find(texts.begin(), texts.end(), param) != texts.end()) {
                this->printf(" $text_%d", param);
            } else if (std::find(texts16.begin(), texts16.end(), param) != texts16.end()) {
                this->printf(" $text16_%d", param);
            } else {
                this->printf(" $int%d", param);
            }
        }
    }
}

void output::addVar(WORD param) {
    this->printf(" global_%d", param / 4);
}

void output::addLVarTextLabel(WORD param, size_t argIndex)
{
    if (std::find(texts.begin(), texts.end(), param) == texts.end()) {
        texts.push_back(param);
    }

    if (types[currentOpcode][argIndex] == Types::_STRING) {
        this->printf(" $text_%d", param);
    } else {
        this->printf(" text_%d", param);
    }
}

void output::addLVarString(WORD param)
{
    this->printf(" text16_%d", param);

    if (std::find(texts16.begin(), texts16.end(), param) == texts16.end()) {
        texts16.push_back(param);
    }
}

void output::addLVarArray(WORD param, WORD param2) {
    this->printf(" v%d[v%d]", param2, param);
}

void output::addVarArray(WORD param, WORD param2) {
    this->printf(" global_%d[global_%d]", param2, param);
}

void output::addTextLabel(std::string param)
{
    auto pos = param.find("%");

    while (pos != param.npos)
    {
        param.insert(pos++, "%");
        pos = param.find("%", ++pos);
    }

    this->printf(" %s", param.c_str());
}

void output::addVarLenString(std::string param)
{
    auto pos = param.find("%");

    while (pos != param.npos)
    {
        param.insert(pos++, "%");
        pos = param.find("%", ++pos);
    }

    this->printf(" \"%s\"", param.c_str());
}

void output::createLabels(std::map<int, size_t> points)
{
    std::sort(labelsOffsets.begin(), labelsOffsets.end(), std::greater<>());

    for (auto a : labelsOffsets)
    {
        if (options.hexlabel) {
            sprintf(tempBuffer, "\n\nlabel_%X:", a);
        } else {
            sprintf(tempBuffer, "\n\nlabel_%d:", a);
        }

        if (points[a]) out.insert(points[a], tempBuffer);
    }
}

output::output(Files f, Mode m, Options o) : files(f), mode(m), options(o)
{
    if (mode == Mode::CS) {
        out = "SCRIPT_START\n{";
    } else if (mode == Mode::CM) {
        out = "MISSION_START\n{";
    }
    
    xml_document<> doc;

    std::ifstream theFile("commands.xml");
    std::vector<char> buf((std::istreambuf_iterator<char>(theFile)), std::istreambuf_iterator<char>());
    buf.push_back('\0');

    doc.parse<0>(&buf[0]);

    xml_node<> *node = doc.first_node("GTA3Script")->first_node("Commands")->first_node("Command");

    for (auto chil = node; chil; chil = chil->next_sibling()) {
        auto args = chil->first_node("Args");
        int index = strtol(chil->first_attribute("ID")->value(), nullptr, 16);
        int argsCount = 0;

        if (args)
        {
            auto optional = args->last_node("Arg")->first_attribute("Optional");

            if (optional)
            {
                if (!strcmp(optional->value(), "true")) {
                    argsCount = -1;
                }
            }

            args = args->first_node("Arg");

            for (; args; args = args->next_sibling())
            {
                char *att = args->first_attribute("Type")->value();

                if (!strcmp(att, "INT")) types[index].push_back(Types::_INT);
                if (!strcmp(att, "FLOAT")) types[index].push_back(Types::_FLOAT);
                if (!strcmp(att, "TEXT_LABEL")) types[index].push_back(Types::_TEXT_LABEL);
                if (!strcmp(att, "STRING")) types[index].push_back(Types::_STRING);
                if (!strcmp(att, "LABEL")) types[index].push_back(Types::_LABEL);
                if (!strcmp(att, "CONSTANT")) types[index].push_back(Types::_CONSTANT);
                if (!strcmp(att, "PARAM")) types[index].push_back(Types::_PARAM);

                if (argsCount != -1) argsCount++;
            }
        }
        
        commands[index] = chil->first_attribute("Name")->value();
        argsc[index] = argsCount;
    }
}

output::~output()
{
    if (!out.empty())
    {
        for (int stage = 1; stage <= 2; ++stage)
        {
            size_t start_pos = 0;

            while ((start_pos = out.find((stage == 1 ? "\t " : "\tNOT  "), start_pos)) != out.npos)
            {
                out.replace(start_pos, (stage == 1 ? 2 : 6), (stage == 1 ? "\t" : "\tNOT "));
                start_pos += (stage == 1 ? 2 : 6);
            }
        }

        std::string temp;

        if (!ints.empty())
        {
            temp = "\tLVAR_INT";

            for (auto a : ints)
            {
                sprintf(tempBuffer, " int%i", a);
                temp.append(tempBuffer);
            }

            temp.append("\n");
        }

        if (!floats.empty())
        {
            temp.append("\tLVAR_FLOAT");

            for (auto a : floats)
            {
                sprintf(tempBuffer, " float%i", a);
                temp.append(tempBuffer);
            }

            temp.append("\n");
        }

        if (!texts.empty())
        {
            temp.append("\tLVAR_TEXT_LABEL");

            for (auto a : texts)
            {
                sprintf(tempBuffer, " text_%i", a);
                temp.append(tempBuffer);
            }

            temp.append("\n");
        }

        if (!texts16.empty())
        {
            temp.append("\tLVAR_TEXT_LABEL16");
            
            for (auto a : texts16)
            {
                sprintf(tempBuffer, " text16_%i", a);
                temp.append(tempBuffer);
            }

            temp.append("\n");
        }

        temp.append("\n");
        out.insert(15, temp);

        if (out.substr(out.length() - commands[0x0A93].length()) == commands[0x0A93]) {
            out.erase(out.length() - commands[0x0A93].length() - 2);
        }

        if (mode == Mode::CS) {
            this->printf("\n}\nSCRIPT_END");
        } else if (mode == Mode::CM) {
            this->printf("\n}\nMISSION_END");
        }

        FILE *f = fopen(files.output, "w");

        fprintf(f, out.c_str());
        fclose(f);

        if (options.clip) clip(out);
    }
}
