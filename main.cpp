#include "decompile.h"
#include <fstream>

const char *helpMsg =
R"(Usage: decompiler <cs|cm|scm> input_file [options] output_file

Decompiler Options:
    -help                    Show this information.
    -about                   Show credits information.

Output Options:
    -hexlabel                Hex format for labels offsets.
    -clip                    Copy output to the clipboard.
    -ignoreunk               Ignore unknown opcodes.
)";

const char *creditsMsg =
R"(Basic decompiler for .cs, .cm and .scm scripts.
Powered By: Israel.

2018.
)";

enum ErrorCode {
    Success,
    ModeArg,
    ModeErr,
    InputArg,
    InputErr,
    OptionErr,
    OutputArg
};

auto readArgs(char **argv, Files& files, Mode& mode, Options& options) {
    if (*(++argv)) {
        if (!strcmp(*argv, "-help")) {
            options.help = true;
            return ErrorCode::Success;
        } else if (!strcmp(*argv, "-about")) {
            options.about = true;
            return ErrorCode::Success;
        } else if (!strcmp(*argv, "cs")) {
            mode = Mode::CS;
        } else if (!strcmp(*argv, "cm")) {
            mode = Mode::CM;
        } else if (!strcmp(*argv, "scm")) {
            mode = Mode::SCM;
        } else {
            return ErrorCode::ModeErr;
        }
    } else {
        return ErrorCode::ModeArg;
    }

    if (*(++argv)) {
        files.input = *argv;
    } else {
        return ErrorCode::InputArg;
    }

    ErrorCode result = ErrorCode::OptionErr;
    
    while (*(++argv)) {
        if (**argv == '-')
        {
            if (!strcmp(*argv, "-hexlabel")) {
                result = ErrorCode::Success;
                options.hexlabel = true;
            } else if (!strcmp(*argv, "-clip")) {
                result = ErrorCode::Success;
                options.clip = true;
            } else if (!strcmp(*argv, "-ignoreunk")) {
                result = ErrorCode::Success;
                options.ignoreunk = true;
            } else return ErrorCode::OptionErr;
        } else {
            files.output = *argv;

            if (!*(++argv)) {
                result = ErrorCode::Success;
            } else {
                result = ErrorCode::OptionErr;
            }

            break;
        }
    }

    if (std::fstream(files.input).fail()) {
        result = ErrorCode::InputErr;
    } else if (files.output == nullptr) {
        result = ErrorCode::OutputArg;
    }

    return result;
}

int main(int argc, char **argv)
{
    Files files;
    Mode mode = Mode::None;
    Options options;

    int result = readArgs(argv, files, mode, options);
    
    if (result == ErrorCode::Success) {
        if (options.help) {
            printf(helpMsg);
            return EXIT_SUCCESS;
        } else if (options.about) {
            printf(creditsMsg);
            return EXIT_SUCCESS;
        }
    } else {
        switch (result)
        {
        case ErrorCode::ModeArg:
            fprintf(stderr, "Type -help to view help information.");
            break;
        case ErrorCode::ModeErr:
            fprintf(stderr, "Unrecognized mode, type -help to view available modes.");
            break;
        case ErrorCode::InputArg:
            fprintf(stderr, "No input file typed, type -help to view usage.");
            break;
        case ErrorCode::InputErr:
            fprintf(stderr, "Unable to open input file.");
            break;
        case ErrorCode::OptionErr:
            fprintf(stderr, "Unrecognized option, type -help to view available options.");
            break;
        case ErrorCode::OutputArg:
            fprintf(stderr, "No output file typed, type -help to view usage.");
        }

        return EXIT_FAILURE;
    }

    FILE *f = fopen(files.input, "rb");

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    byte *buffer = new byte[size];

    fread(buffer, size, 1, f);
    fclose(f);

    byte *temp = buffer;

    while (*(uint64_t*)++temp != 0x000002F100524156) {
        if (temp >= (buffer + size)) {
            break;
        }
    }
    
    return decompile(buffer, (temp - buffer), files, mode, options);
}
