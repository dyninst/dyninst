/*
 * Copyright (c) 2024 Advanced Micro Devices, Inc. All rights reserved.
 */
#include "cli_command_generate_inst_desc.h"

// C++ libraries.
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

// Local libraries.
#include "amdisa_structures.h"
#include "isa_xml_reader.h"

namespace amdisa
{
    enum class kSupportedFormat
    {
        kUndefined,
        kText,
        kCsv
    };

    static const std::map<std::string, kSupportedFormat> kMapStringToSupportedFormats = {{"text", kSupportedFormat::kText}, {"csv", kSupportedFormat::kCsv}};

    // Constants.
    static const char* kDescriptionTagOpen  = "<Description>";
    static const char* kDescriptionTagClose = "</Description>";

    // Information constant strings.
    static const char* kStringInfoGenerateStart      = "Info: Generating instruction descriptions...";
    static const char* kStringInfoGenerateSuccessful = "Info: Instruction generation completed.";

    // Error constant strings.
    static const char* kStringErrorGenerateFailed = "Error: Instruction generation failed.";
    static const char* kStringErrorFileOpenFailed = "Error: Instruction generation failed. Could not open the file.";

    // Internal class to handle different printing formating.
    struct InstructionData
    {
        std::string instruction_name;
        std::string instruction_description;
    };

    // Base class for the instruction serializer.
    class IInstructionSerializer
    {
    public:
        bool Initialize(const std::string& output_file_path)
        {
            desc_file_.open(output_file_path);
            is_init_ = desc_file_.is_open();
            if (is_init_)
            {
                PrintHeader();
            }
            return is_init_;
        }

        // Prints single line to file.
        virtual void PrintLine(const InstructionData& data) = 0;
        virtual ~IInstructionSerializer(){};

    protected:
        std::ofstream desc_file_;
        bool          is_init_ = false;

        // Prints the header of the file if the format requires.
        virtual void PrintHeader() = 0;
    };

    // Print as text.
    class TextSerializer : public IInstructionSerializer
    {
    public:
        void PrintHeader() override
        {
            // This format does not have a header.
        }

        void PrintLine(const InstructionData& data) override
        {
            if (is_init_)
            {
                desc_file_ << data.instruction_name << std::endl;
                desc_file_ << data.instruction_description << std::endl;
                desc_file_ << std::endl;
            }
        }
    };

    // Print as CSV.
    class CsvSerializer : public IInstructionSerializer
    {
    public:
        void PrintHeader() override
        {
            if (is_init_)
            {
                desc_file_ << "Instruction,Description\n";
            }
        }

        void PrintLine(const InstructionData& data) override
        {
            if (is_init_)
            {
                std::string description = data.instruction_description.substr(0, data.instruction_description.length() - 1);
                desc_file_ << data.instruction_name << ",";
                desc_file_ << "\"" << description << "\"\r\n";
            }
        }
    };

    // Function for creating different types of printers.
    static std::shared_ptr<IInstructionSerializer> GetSerializer(kSupportedFormat format)
    {
        std::shared_ptr<IInstructionSerializer> ret;
        switch (format)
        {
        case kSupportedFormat::kCsv:
            ret = std::make_shared<CsvSerializer>();
            break;
        default:
            ret = std::make_shared<TextSerializer>();
            break;
        }
        return ret;
    }

    bool CliCommandGenerateInstDesc::Execute(std::string& err_message)
    {
        bool is_executed = false;
        std::cout << kStringInfoGenerateStart << std::endl;

        // Pick the printing format.
        kSupportedFormat format = kSupportedFormat::kUndefined;
        if (kMapStringToSupportedFormats.count(desc_format_) > 0)
        {
            format = kMapStringToSupportedFormats.at(desc_format_);
        }
        std::shared_ptr<IInstructionSerializer> serializer = GetSerializer(format);

        bool is_init = serializer->Initialize(output_desc_path_);
        if (is_init)
        {
            IsaSpec     spec_data;
            std::string read_err_message;
            bool        is_xml_read = IsaXmlReader::ReadSpec(input_xml_path_, spec_data, read_err_message);
            if (is_xml_read)
            {
                // Print instructions.
                for (const auto& instruction : spec_data.instructions)
                {
                    // Remove opening description tag if present.
                    std::string description = instruction.description;
                    size_t      open_pos    = description.find(kDescriptionTagOpen);
                    if (open_pos != std::string::npos)
                    {
                        description.erase(open_pos, strlen(kDescriptionTagOpen));
                    }

                    // Remove closing description tag if present.
                    size_t close_pos = description.find(kDescriptionTagClose);
                    if (close_pos != std::string::npos)
                    {
                        description.erase(close_pos, strlen(kDescriptionTagClose));
                    }

                    // Write to file.
                    serializer->PrintLine({instruction.name, description});
                }
                is_executed = true;
                std::cout << kStringInfoGenerateSuccessful << std::endl;
            }
            else
            {
                err_message = kStringErrorGenerateFailed;
                err_message += ". " + read_err_message;
            }
        }
        else
        {
            err_message = kStringErrorFileOpenFailed;
        }

        return is_executed;
    }
}  // namespace amdisa
