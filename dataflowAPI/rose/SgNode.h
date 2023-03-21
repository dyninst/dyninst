#if !defined(SG_NODE_H)
#define SG_NODE_H

#include <string>

class SgNode {
 public:
    virtual std::string class_name() const = 0;
    
    /*! \brief returns new style SageIII enum values */
    virtual VariantT variantT() const = 0; // MS: new variant used in tree traversal    
    
    virtual ~SgNode() {}
    SgNode() = default;
    SgNode(const SgNode&) = default;

};
#endif
