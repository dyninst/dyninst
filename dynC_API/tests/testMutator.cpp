//tester.C
#include <iostream>
#include <fstream>
#include <string>

#include "dynC.h"
#include "BPatch.h"
#include "BPatch_process.h"
#include "BPatch_snippet.h"
#include "BPatch_function.h"

#define STATEMENT_PATH "../testStatements"
#define STATEMENT_PATH_2 "../testStatements3"
const char *MUTATEE_PATH = "myMutatee";
const char *MUTATEE_ARGS[3];
const char *MODULE_NAME = "testMutatee.cpp";


BPatch bpatch;

int main(){

   using std::string;
   using std::ifstream;
   using std::cout;
   using std::endl;
   using std::getline;

   ifstream myfile(STATEMENT_PATH);
   ifstream myfile2(STATEMENT_PATH_2);

  cout << "Starting binary " << MUTATEE_PATH  << "... ";
  BPatch_addressSpace * appProc;
  bool rewrite = false;
  if(rewrite){
     appProc = bpatch.openBinary(MUTATEE_PATH, true);
  }else{
     appProc = bpatch.processCreate(MUTATEE_PATH, MUTATEE_ARGS);
  }
  cout << "complete" << endl;
  if (!appProc) return -1;

  BPatch_image *appImage = appProc->getImage();
  BPatch_module *mutatee = appImage->findModule(MODULE_NAME);

  if (mutatee == NULL){cout << "Bad Mutatee!" << endl;}

  const std::vector<BPatch_function *> * functions = mutatee->getProcedures();  

      
     std::string fileString;
     std::string line;
     if(!myfile.is_open()){
        fprintf(stderr, "Error: Unable to open file\n");
        exit(-1);
     }
     while(!myfile.eof())
     {
        std::getline(myfile, line);
        fileString += line + "\n";
     }

     std::string fileString2;
     if(!myfile2.is_open()){
        fprintf(stderr, "Error: Unable to open file\n");
        exit(-1);
     }
     while(!myfile2.eof())
     {
        std::getline(myfile2, line);
        fileString2 += line + "\n";
     }

     appProc->malloc(*appImage->findType("long"), std::string("globalVar"));
     

  std::vector<BPatch_point *> * entry_points = (*functions)[0]->findPoint(BPatch_entry);
  std::vector<BPatch_point *> * exit_points = (*functions)[0]->findPoint(BPatch_exit);;
  
  for(unsigned int n = 1; n < functions->size(); n++){
     entry_points->push_back((*(*functions)[n]->findPoint(BPatch_entry))[0]);
     exit_points->push_back((*(*functions)[n]->findPoint(BPatch_exit))[0]);
  }

  
/////////////////////////////////////////////////////////
  for(unsigned int i = 0; i < entry_points->size(); ++i){
     if((*entry_points)[i] == NULL){
        printf("entry point %d is null \n", i);
     }
     char str[] = "printf(\"Flow!\\n\");"; 
     BPatch_snippet *retSnippet = dynC_API::createSnippet(fileString, *(*entry_points)[i], "entrySnippet");
     if (retSnippet != NULL){
        appProc->insertSnippet(*retSnippet, *(*entry_points)[i]);       
     }
     BPatch_snippet *retSnippet2 = dynC_API::createSnippet(strdup(fileString2.c_str()), *(*exit_points)[i], "entrySnippet");
     if (retSnippet != NULL){
        appProc->insertSnippet(*retSnippet2, *(*exit_points)[i]);       
     }
     
//     BPatch_snippet *auxSnippet = dynC_API::createSnippet(&myfile2, "AuxSnippet");
  }
/////////////////////////////////////////////////////////


//  appProc->insertSnippet(*auxSnippet, *exit_points);


  printf("Snippet's inserted!\n");
  if(!rewrite){
     BPatch_process *aProc = static_cast<BPatch_process *>(appProc);
     aProc->continueExecution();

  while (!aProc->isTerminated()){
     bpatch.waitForStatusChange();
  }

  if (aProc->terminationStatus() == ExitedNormally) {
     printf("Application exited with code %d\n", aProc->getExitCode());
  }
  else if (aProc->terminationStatus() == ExitedViaSignal) {
     printf("Application exited with signal %d\n", aProc->getExitSignal());
  }
  else {
     printf("Unknown application exit\n");
  }
  }else{
     BPatch_binaryEdit *aProc = static_cast<BPatch_binaryEdit *>(appProc);
     aProc->writeFile("myMutatee.out");
  }

   return 0;
}
