#include "SerialGraph.h"
#include "utils.h"
#include <sstream>

namespace MRN
{

void SerialGraph::add_Leaf(std::string ihostname, Port iport, Rank irank )
{
    std::ostringstream hoststr;

    hoststr << "[" << ihostname << ":" << iport << ":" << irank << ":0" << "]";
    _byte_array += hoststr.str();

    _num_nodes++; _num_backends++;
}

void SerialGraph::add_SubTreeRoot(std::string ihostname, Port iport, Rank irank )
{
    std::ostringstream hoststr;

    hoststr << "[" << ihostname << ":" << iport << ":" << irank << ":1";
    _byte_array += hoststr.str();

    _num_nodes++;
}

bool SerialGraph::is_RootBackEnd( void ) const
{
    //format is "host:port:rank:[0|1]: where 0 means back-end and 1 means internal node
    unsigned int idx,num_colons;
    for( idx=0,num_colons=0; num_colons<3; idx++ ){
        if(_byte_array[idx] == ':'){
            num_colons++;
        }
    }
    if( _byte_array[idx] == '0' ) {
        return true;
    }
    else{
        return false;
    }
}

std::string SerialGraph::get_RootHostName()
{
    std::string retval;
    size_t begin=1, end=1; //Byte array begins [ xxx ...

    //find first ':'
    end = _byte_array.find(':', begin);
    assert ( end != std::string::npos );

    retval = _byte_array.substr(begin, end-begin);

    return retval;
}

Port SerialGraph::get_RootPort()
{
    size_t begin, end;
    Port retval;

    begin = _byte_array.find(':', 2);
    assert( begin != std::string::npos );
    begin++;
    end = _byte_array.find(' ', begin);
    std::string port_string = _byte_array.substr(begin, end-begin);
    retval = atoi(port_string.c_str());

    return retval;
}

Rank SerialGraph::get_RootRank(){
    Rank retval = UnknownRank;
    size_t begin=0, end=1; //Byte array begins [ xxx ...

    //find 2nd ':'
    begin = _byte_array.find(':', begin);
    assert(begin != std::string::npos );
    begin++;
    begin = _byte_array.find(':', begin);
    assert(begin != std::string::npos );
    begin++;
    end = _byte_array.find(' ', begin);

    std::string rankstring = _byte_array.substr(begin, end-begin);

    retval = atoi(rankstring.c_str());

    return retval;
}

SerialGraph * SerialGraph::get_MySubTree( std::string &ihostname, Port iport, Rank irank )
{
    std::ostringstream hoststr;
    size_t begin, cur, end;

    hoststr << "[" << ihostname << ":" << iport << ":" << irank; 
    mrn_dbg( 5, mrn_printf(FLF, stderr, "SubTreeRoot:\"%s\" byte_array:\"%s\"\n",
                           hoststr.str().c_str(), _byte_array.c_str() ));

    begin = _byte_array.find( hoststr.str() );
    if( begin == std::string::npos ) {
        mrn_dbg( 1, mrn_printf(FLF, stderr,
                               "No SubTreeRoot:\"%s\" found in byte_array:\"%s\"\n",
                               hoststr.str().c_str(), _byte_array.c_str() ));
        return NULL;
    }

    //now find matching ']'
    cur=begin;
    end=1;
    int num_leftbrackets=1, num_rightbrackets=0;
    while(num_leftbrackets != num_rightbrackets){
        cur++,end++;
        if( _byte_array[cur] == '[')
            num_leftbrackets++;
        else if( _byte_array[cur] == ']')
            num_rightbrackets++;
    }

    SerialGraph * retval = new SerialGraph( _byte_array.substr(begin, end));
    
    mrn_dbg( 5, mrn_printf(FLF, stderr, "returned sg byte array :\"%s\"\n",
                           retval->_byte_array.c_str() )); 
    return retval;
}

SerialGraph * SerialGraph::get_NextChild()
{
    SerialGraph * retval;
    size_t begin, end, cur;
    const char * buf = _byte_array.c_str();

    if( _buf_idx == std::string::npos ||
        _byte_array.find('[',_buf_idx) == std::string::npos ){
        return NULL;
    }

    cur=begin=_buf_idx;
    end=1;
    int num_leftbrackets=1, num_rightbrackets=0;
    while(num_leftbrackets != num_rightbrackets){
        cur++, end++;
        if( buf[cur] == '[')
            num_leftbrackets++;
        else if( buf[cur] == ']')
            num_rightbrackets++;
    }

    _buf_idx = cur + 1;

    retval = new SerialGraph( _byte_array.substr(begin, end));
    
    mrn_dbg( 5, mrn_printf(FLF, stderr, "returned sg byte array :\"%s\"\n",
                           retval->_byte_array.c_str() )); 
    return retval;
}

} /* namespace MRN */
