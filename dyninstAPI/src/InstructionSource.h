#ifndef INSTRUCTION_SOURCE_H
#define INSTRUCTION_SOURCE_H

class InstructionSource
{
public:
    InstructionSource() {}
    virtual ~InstructionSource() {}
    virtual bool isValidAddress(const Address&) const { return true; }
    virtual void* getPtrToInstruction(Address) const = 0;
    virtual unsigned int getAddressWidth() const = 0;
};

#endif // INSTRUCTION_SOURCE_H
