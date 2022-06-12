#include "decompile.h"

int decompile(byte *in, size_t size, Files files, Mode mode, Options options)
{
    script sc_struct(in);
    output code(files, mode, options);

    std::map<int, size_t> offsets;
    size_t counter = 0;
    size_t n = 0;
    
    while (sc_struct.currBytes() < (in + size))
    {
        offsets[sc_struct.currBytes() - in] = code.point();
        uint16_t opcode = sc_struct.getOpcode();
        
        size_t point = code.point();

        if (!code.addCommand(opcode, sc_struct))
        {
            sc_struct.jumpPos(-2);

            if (options.ignoreunk)
            {
                if (counter > 0) offsets.erase(sc_struct.currBytes() - in);

                if (counter++ == 0)
                {
                    code.newLine(1);
                    code.tab(1);

                    code.printf("DUMP", sc_struct.getData<uint8_t>());

                    code.newLine(1);
                    code.tab(2);

                    code.printf("%02X ", sc_struct.getData<uint8_t>());
                    code.printf("%02X ", sc_struct.getData<uint8_t>());
                }

                code.printf("%02X ", sc_struct.getData<uint8_t>());

                if (++n >= 14)
                {
                    code.newLine(1);
                    code.tab(2);

                    n = 0;
                }

                if (sc_struct.currBytes() >= (in + size))
                {
                    code.newLine(1);
                    code.tab(1);

                    code.printf("ENDDUMP");
                    n = 0;
                }

            } else {
                uint32_t offset = (sc_struct.currBytes() - in);
                fprintf(stderr, "Unknown opcode %04X on offset %X", sc_struct.getOpcode(), offset);

                code.clear();
                return EXIT_FAILURE;
            }
        } else if (counter > 0) {
            code.out.insert(point, "\n\tENDDUMP");
            counter = n = 0;
        }
    }
    
    code.createLabels(offsets);
    return EXIT_SUCCESS;
}
