static int otherFunctionCalled = 0;

static void otherFunction() { otherFunctionCalled = 1; }

void external(int callOtherFunction) {
  if(callOtherFunction)
    otherFunction();
}
