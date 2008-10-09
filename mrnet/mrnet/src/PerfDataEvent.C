#include "PerfDataEvent.h"
#include "CommunicationNode.h"
#include "mrnet/MRNet.h"

#include <iostream>
using namespace std;

namespace MRN {

bool send_PacketToGUI( PacketPtr& connect_pkt, PacketPtr& data_pkt )
{
#if !defined(os_windows)
    char * GUI_hostname_ptr;
    Port iGUIPort;
    connect_pkt->unpack( "%s %uhd", &GUI_hostname_ptr, &iGUIPort ); 
    std::string strGUIHostName( GUI_hostname_ptr );

    int sock_fd_To_GUI=0;
    if(connectHost(&sock_fd_To_GUI, strGUIHostName, iGUIPort ) == -1){
        std::cout << "connect_to_GUIhost() failed\n";               
        close( sock_fd_To_GUI );
        return false;
    }

    Message msgToGUI;
    msgToGUI.add_Packet( data_pkt );
    if( msgToGUI.send( sock_fd_To_GUI ) == -1 ) {
        std::cout << "Message.send failed\n";
        close( sock_fd_To_GUI );
        return false;
    }
    close( sock_fd_To_GUI );
#endif
    return true;
}


bool handle_PerfGuiInit( PacketPtr& connect_pkt )
{
    char * topo_ptr = network->get_NetworkTopology()->get_TopologyStringPtr();
    mrn_dbg( 5, mrn_printf(FLF, stderr, "topology: (%p), \"%s\"\n", topo_ptr, topo_ptr ));

    PacketPtr packetToGUI( new Packet( 0, PROT_GUI_INIT, "%s", topo_ptr ) );
    mrn_dbg( 5, mrn_printf(FLF, stderr, "topology: (%p), \"%s\"\n", topo_ptr, topo_ptr ));
    free( topo_ptr );

    return send_PacketToGUI( connect_pkt, packetToGUI );
}


bool handle_PerfDataCPU( PacketPtr& connect_pkt, Rank my_rank )
{
#if defined(os_linux)
    static bool first_time = true;

    static long long llUser = 0;
    static long long llNice = 0;
    static long long llKernel = 0;
    static long long llIdle = 0;
    static long long llTotal = 0;

    FILE *f;
    if( (f = fopen("/proc/stat", "r")) == NULL ) {
        std::cout << "open /proc/stat failed\n";
        fflush( stdout );
        return false;
    }
    else
    {
        long long currUser = 0;
        long long currNice = 0;
        long long currKernel = 0;
        long long currIdle = 0;
        long long currTotal = 0;/* The very first line should be cpu */
        if((fscanf(f, "cpu %lld %lld %lld %lld",
                   &currUser, &currNice, &currKernel, &currIdle)) != 4) {
            std::cout << "parse /proc/stat failed\n";
            fflush( stdout );      
            fclose(f);
            return false;
        }
        else {
            fclose(f);
            currTotal = currUser + currNice + currKernel + currIdle;
                
            if( first_time ) {
                llUser = currUser;
                llNice = currNice;
                llKernel = currKernel;
                llIdle = currIdle;
                llTotal = currTotal;
                first_time = false;
                return true;
            }
            else {

                float fUserCPUPercent = 100.0;
                float fNiceCPUPercent = 100.0;
                float fKernelCPUPercent = 100.0;
                float fIdleCPUPercent = 100.0;
                float denom = ((float)currTotal - (float)llTotal);

                fUserCPUPercent *= ((float)currUser - (float)llUser) / denom; 
                fNiceCPUPercent *= ((float)currNice - (float)llNice) / denom; 
                fKernelCPUPercent *= ((float)currKernel - (float)llKernel) / denom; 
                fIdleCPUPercent *= ((float)currIdle - (float)llIdle) / denom;

                llUser = currUser;
                llNice = currNice;
                llKernel = currKernel;
                llIdle = currIdle;
                llTotal = currTotal;

                PacketPtr packetToGUI( new Packet( 0, PROT_GUI_CPUPERCENT, "%f %f %f %f %ud",
                                                   fUserCPUPercent, fNiceCPUPercent, fKernelCPUPercent,
                                                   fIdleCPUPercent, my_rank ) );
                
                return send_PacketToGUI( connect_pkt, packetToGUI );
            }
        }
    }
#endif // defined(os_linux)

    return true;
}

}
