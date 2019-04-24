// DynInst
#include "BPatch.h"
#include "BPatch_binaryEdit.h"
#include "BPatch_image.h"
#include "BPatch_function.h"
#include "BPatch_object.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"

#include <stdlib.h>
#include <stdio.h>
#include <iterator>
#include <string>
#include <vector>
#include <iterator>

// patchAPI
#include "PatchMgr.h"

using namespace std;
using namespace Dyninst;
using namespace PatchAPI;


// a simple example, just insert a bunch of no ops
class NoopSnippet : public Snippet {

public:
  bool generate(Point *pt, Buffer &buffer){
    uint8_t byte = 0x90;
    cout << "inserting a no op @" << pt << endl;
    for(int i = 0; i < 10; i++){
      buffer.push_back(byte);
    }
    return true;
  }

};



int main(int argc, const char *argv[]) {

  if(argc != 3){
    cerr << "Usage:\n\t" << argv[0] << " <input binary> <output binary path>" << endl;
    return 1;
  }

  const char* input_binary = argv[1];
  const char* output_binary = argv[2];

  BPatch bpatch;

  BPatch_binaryEdit* app = bpatch.openBinary(input_binary, false);

  if(app == NULL){
    return 0;
  }

  cout << "app OK" << endl;

  BPatch_image* image = app->getImage();

  if(image == NULL){
    return 0;
  }
  
  cout << "image OK" << endl;

  PatchMgrPtr patchMgr = PatchAPI::convert(image);

  vector<BPatch_object*> objects;

  image->getObjects(objects);

  int ocount = objects.size();

  cout << "objects: " <<  ocount << endl;

  if(ocount <= 0){
    return 0;
  }

  BPatch_object* batchObj = objects[0];

  // Not mentioned in A.2 of https://dyninst.org/sites/default/files/manuals/dyninst/patchAPI.pdf
  // But found in the header file:  "BPatch_object.h"
  PatchObject* binobj = PatchAPI::convert(batchObj);

  Patcher patcher(patchMgr);

  NoopSnippet::Ptr snippet = NoopSnippet::create(new NoopSnippet);

  vector<PatchFunction*> functions;

  binobj->funcs(back_inserter(functions));


  for(vector<PatchFunction*>::iterator funIter = functions.begin(); funIter != functions.end(); funIter++){
    PatchFunction *fun = *funIter;

    vector<Point*> f_entryPoints;
    patchMgr->findPoints(PatchAPI::Scope(fun), PatchAPI::Point::FuncEntry, back_inserter(f_entryPoints));


    cout << fun->name() << " has:\n\t" << f_entryPoints.size() << " entry points" << endl;

    for(vector<Point*>::iterator pointIter = f_entryPoints.begin(); pointIter!= f_entryPoints.end(); pointIter++){
      Point* point = *pointIter;
      cerr << "Patching @ " << point << endl;
      patcher.add(PushBackCommand::create(point, snippet));
    }

  }

  patcher.commit();

  cout << "Commited" << endl;

  app->writeFile(output_binary);

  cout << "Written" << endl;


}
