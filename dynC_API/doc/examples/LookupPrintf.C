std::vector<BPatch_function*> printf_func;
appImage->findFunction("printf", printf_func);
BPatch_function* BPF_printf = printf_func[0];
