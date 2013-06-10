#include "MachSyscall.h"
#include "PCProcess.h"
#include "Event.h"

namespace Dyninst {
namespace ProcControlAPI {

MachSyscall makeFromEvent(const EventSyscall * ev)
{
    Process::const_ptr proc = ev->getProcess();
    Architecture arch = proc->getArchitecture();
    OSType os = proc->getOS();
    Platform plat(arch,os);
    MachSyscall::SyscallIDPlatform syscallNumber = ev->getSyscallNumber();
    MachSyscall::SyscallName syscallName = MachSyscall::nameLookup(plat, syscallNumber);
    return MachSyscall(plat, syscallNumber, syscallName);
}

MachSyscall makeFromID(Process::ptr proc, MachSyscall::SyscallIDIndependent id)
{
    Architecture arch = proc->getArchitecture();
    OSType os = proc->getOS();
    Platform plat(arch,os);
    return MachSyscall::makeFromPlatform(plat, id);
}

}
}
