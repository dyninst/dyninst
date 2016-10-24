//
// Created by ssunny on 10/16/16.
//

#ifndef DYNINST_ARCHSPECIFICFORMATTERS_H
#define DYNINST_ARCHSPECIFICFORMATTERS_H

#include <map>
#include <vector>
#include <string>

namespace Dyninst {
    namespace InstructionAPI {

        class ArchSpecificFormatter {
        public:
            virtual std::string getInstructionString(std::vector <std::string>) = 0;

            virtual std::string formatImmediate(std::string) = 0;

            virtual std::string formatDeref(std::string) = 0;

            virtual std::string formatRegister(std::string) = 0;

            virtual std::string formatBinaryFunc(std::string, std::string, std::string);

            virtual ~ArchSpecificFormatter() {}
        };

        class ArmFormatter : public ArchSpecificFormatter {
        public:
            ArmFormatter();

            virtual std::string getInstructionString(std::vector <std::string>);

            virtual std::string formatImmediate(std::string);

            virtual std::string formatDeref(std::string);

            virtual std::string formatRegister(std::string);

            virtual std::string formatBinaryFunc(std::string, std::string, std::string);

            virtual ~ArmFormatter() {}

        private:
            std::map<std::string, std::string> binaryFuncModifier;
        };
    };
};

#endif //DYNINST_ARCHSPECIFICFORMATTERS_H
