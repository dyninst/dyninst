#ifndef PDUTIL_RESOURCE_H
#define PDUTIL_RESOURCE_H


enum ResourceType
{
    OtherResourceType           = 0,
    CategoryResourceType        = 1,
    MachineResourceType         = 2,
    ProcessResourceType         = 3,
    ThreadResourceType          = 4,
    ModuleResourceType          = 5,
    FunctionResourceType        = 6,
    LoopResourceType            = 7,
    MessageGroupResourceType    = 8,
    MessageTagResourceType      = 9,
    CondVarResourceType         = 10,
    MutexResourceType           = 11,
    RWLockResourceType          = 12,
    SemaphoreResourceType       = 13
};


#if defined(__cplusplus)

// (Eventually) base class for resource objects 
// in both Paradyn front end and daemon.
// (currently) unused.

class Resource
{
private:
    pdstring name;
    Resource* parent;
    ResourceType type;
    unsigned int mdlType;
    timeStamp createTime;
    void* data;
    bool suppressed;

public:
    Resource( pdstring _name,
                Resource* _parent,
                ResourceType _type,
                unsigned int _mdlType,
                timeStamp _createTime,
                void* _data,
                bool _suppressed )
      : name( _name ),
        parent( _parent ),
        type( _type ),
        mdlType( _mdlType ),
        createTime( _createTime ),
        data( _data ),
        suppressed( _suppressed )
    { }
    
};

#endif /* defined(__cplusplus) */


#endif /* PDUTIL_RESOURCE_H */
