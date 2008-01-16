
#include "parRegion.h"
#include "image-func.h"

#include "process.h"

image_parRegion::image_parRegion(image_func * imageFunc)
{
   regionIf_ = imageFunc;
  parentIf_ = NULL;
}

image_parRegion::image_parRegion(Address firstOffset, image_func * imageFunc)
{
   regionIf_ = imageFunc;
  parentIf_ = NULL;
  firstInsnOffset_ = firstOffset;
}

image_func * image_parRegion::getAssociatedFunc(){return regionIf_;}

void image_parRegion::printDetails()
{  
  printf("\n********************************************************\n");
  
  if (parentIf_)
    printf("Parent func is %s\n", parentIf_->prettyName().c_str());
  else
    printf("Parent func NULL\n");
  if (regionIf_)
    printf("Outlined func is %s\n", regionIf_->prettyName().c_str());
  else 
    printf("Outlined func is NULL\n");
  
  printf("Begin address - 0x%lx\n", firstInsnOffset_);
  printf("End address - 0x%lx\n", lastInsnOffset_);

  printf("Region Type is ...");
  if(regionType == OMP_NONE)
    printf("OMP_NONE\n");
  else if (regionType == OMP_PARALLEL)
    printf("OMP_PARALLEL\n");
  else if (regionType == OMP_DO_FOR)
    printf("OMP_DO_FOR\n");
  else if (regionType == OMP_SECTIONS)
    printf("OMP_SECTIONS\n");
  else if (regionType == OMP_SINGLE)
    printf("OMP_SINGLE\n");
  else if (regionType == OMP_PAR_DO)
    printf("OMP_PAR_DO\n");
  else if (regionType == OMP_PAR_SECTIONS)
    printf("OMP_PAR_SECTIONS\n");
  else if (regionType == OMP_MASTER)
    printf("OMP_MASTER\n");
  else if (regionType == OMP_CRITICAL)
    printf("OMP_CRITICAL\n");
  else if (regionType == OMP_BARRIER)
    printf("OMP_BARRIER\n");
  else if (regionType == OMP_ATOMIC)
    printf("OMP_ATOMIC\n");
  else if (regionType == OMP_FLUSH)
    printf("OMP_FLUSH\n");
  else if (regionType == OMP_ORDERED)
    printf("OMP_ORDERED\n");
  else if (regionType == OMP_DO_FOR_LOOP_BODY)
    printf("OMP_DO_FOR_LOOP_BODY\n");
  else
    printf("UNKNOWN\n");

  /*
  std::map<const char *, int>::iterator iter;

  printf("Mapped Items ...\n");
  for (iter = clauses.begin(); iter != clauses.end(); iter++)
    {
      printf("Directive %s - Value %d\n", iter->first, iter->second); 
    }
  */

 printf("********************************************************\n");

}

void image_parRegion::decodeClauses(int bitmap)
{
  /* Par do always evaluates to '0' for power, which could be quite misleading to someone asking for the value, so 
     for this purpose we just have it always return '1', as in, the thing is executed.  In the cases that
     someone actually does put something like if(0) in the code, the par do section isn't even generated,
     so this becomes moot */
  if (regionType == OMP_PAR_DO)
    clauses["IF"] = 1;

  if (regionType == OMP_PARALLEL || regionType == OMP_PAR_SECTIONS)
    clauses["IF"] = bitmap & 0x0001;
  
  bitmap >>= 2;

  if (regionType == OMP_DO_FOR || regionType == OMP_SECTIONS || regionType == OMP_SINGLE)
    clauses["NOWAIT"] = bitmap & 0x0001;

  bitmap >>=2;

  if (regionType == OMP_DO_FOR || regionType == OMP_PAR_DO)
    clauses["ORDERED"] = bitmap & 0x0001;
  
}

void image_parRegion::setRegionType(parRegType rt)
{
  regionType = rt;
  setClause("REGION_TYPE", (int) rt);
} 

void image_parRegion::setClause(const char *key, int value)
{
  clauses[key] = value;
}

void image_parRegion::setClauseLoc(const char *key, Address value)
{
  clauses[key] = value;
}

int_parRegion::int_parRegion(image_parRegion *ip, Address baseAddr, int_function * iFunc)
{
  ip_ = ip;
  addr_ = baseAddr + ip->get_address();
  endAddr_ = addr_ + ip->getSize();
  intFunc_ = iFunc;
}

int_parRegion::~int_parRegion()
{}


int int_parRegion::getClause(const char * key)
{
  return  ip_->getClause(key);   
}

int image_parRegion::getClause(const char * key)
{
  if (clauses.find(key) != clauses.end())
    return clauses[key];
  else
    return -1;
}


Address int_parRegion::getClauseLoc(const char * key)
{
  return  ip_->getClauseLoc(key);   
}

Address image_parRegion::getClauseLoc(const char * key)
{
  if (clauses.find(key) != clauses.end())
    return clauses[key];
  else
    return -1;
}


int int_parRegion::replaceOMPParameter(const char * key, int value)
{
  //  parReg->replaceOMPParameter(key,value);
  
  Address writeAddy = getClauseLoc(key);

  Address writeValue = 0x39000000;

  if (value > 0 )
    writeValue += (unsigned) value;

  AddressSpace * p = intFunc_->proc();

  //printf("About to write value 0x%x to address 0x%x\n", writeValue, writeAddy);
  
  if (!p->writeDataSpace((void *) writeAddy, sizeof(writeValue), &writeValue))
    fprintf(stderr, "%s[%d]:  writeDataSpace failed\n", FILE__, __LINE__);
  
  return 0;
}



