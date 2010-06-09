#if !defined(FREEBSD_H_)
#define FREEBSD_H_

#include "proccontrol/h/Generator.h"
#include "proccontrol/h/Event.h"
#include "proccontrol/h/Decoder.h"
#include "proccontrol/h/Handler.h"
#include "proccontrol/src/int_process.h"
#include "proccontrol/src/sysv.h"
#include "common/h/dthread.h"

using namespace Dyninst;
using namespace ProcControlAPI;

class GeneratorFreeBSD : public GeneratorMT
{
    public: 
        GeneratorFreeBSD();
        virtual ~GeneratorFreeBSD();

        virtual bool initialize();
        virtual bool canFastHandle();
        virtual ArchEvent *getEvent(bool block);
};

class ArchEventFreeBSD : public ArchEvent
{
    public:
        int status;
        pid_t pid;
        bool interrupted;
        int error;

        ArchEventFreeBSD(bool inter_);
        ArchEventFreeBSD(pid_t p, int s);
        ArchEventFreeBSD(int e);

        virtual ~ArchEventFreeBSD();
};

class DecoderFreeBSD : public Decoder
{
    public:
        DecoderFreeBSD();
        virtual ~DecoderFreeBSD();
        virtual unsigned getPriority() const;
        virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events);
        Dyninst::Address adjustTrapAddr(Dyninst::Address address, Dyninst::Architecture arch);
};

class freebsd_process : public sysv_process
{
 public:
    freebsd_process(Dyninst::PID p, std::string e, std::vector<std::string> a);
    freebsd_process(Dyninst::PID pid_, int_process *p);
    virtual ~freebsd_process();

    virtual bool plat_create();
    virtual bool plat_attach();   
    virtual bool plat_forked();
    virtual bool post_forked();
    virtual bool plat_execed();
    virtual bool plat_detach();
    virtual bool plat_terminate(bool &needs_sync);

    virtual bool plat_readMem(int_thread *thr, void *local, 
                             Dyninst::Address remote, size_t size);
    virtual bool plat_writeMem(int_thread *thr, void *local, 
                              Dyninst::Address remote, size_t size);

    virtual bool needIndividualThreadAttach();
    virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps);
    virtual Dyninst::Architecture getTargetArch();
    virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address, unsigned size);
    virtual bool independentLWPControl();
    virtual bool plat_individualRegAccess();
};

class freebsd_thread : public int_thread
{
 public:
    freebsd_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l);

    freebsd_thread();
    virtual ~freebsd_thread();

    virtual bool plat_cont();
    virtual bool plat_stop();
    virtual bool plat_getAllRegisters(int_registerPool &reg);
    virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val);
    virtual bool plat_setAllRegisters(int_registerPool &reg);
    virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val);
    virtual bool attach();
};

#endif
