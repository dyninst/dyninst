#include "ParseThat.h"
#include "util.h"

using namespace Dyninst;

ParseThat::ParseThat() :
	trans(T_None),
	suppress_ipc(false),
	nofork(false),
	verbosity(7),
	timeout_secs(300),
	do_trace(true),
	tracelength(0),
	print_summary_(true),
	parse_level(PL_Func),
	do_recursive(false),
	merge_tramps(false),
	inst_level_(IL_FuncEntry),
	include_libs_(false)
{}

ParseThat::~ParseThat()
{
}

bool ParseThat::setup_args(std::vector<std::string> &pt_args)
{
	pt_args.push_back(std::string("-i"));
	pt_args.push_back(utos((unsigned) inst_level_));
	pt_args.push_back(std::string("-p"));
	pt_args.push_back(utos((unsigned) parse_level));
	pt_args.push_back(std::string("-v ") + utos(verbosity));

	if (include_libs_) 
		pt_args.push_back(std::string("--include-libs"));
	
	if (merge_tramps) 
		pt_args.push_back(std::string("--merge-tramps"));

	if (rewrite_filename.length()) 
		pt_args.push_back(std::string("--binary-edit=") + rewrite_filename);

	if (do_recursive) 
		pt_args.push_back(std::string("-r"));

	if (print_summary_) 
		pt_args.push_back(std::string("--summary"));

	if (timeout_secs) 
		pt_args.push_back(std::string("-t ") + utos(timeout_secs));

	if (do_trace) 
		pt_args.push_back(std::string("-T ") + utos(tracelength));

	if (suppress_ipc) 
		pt_args.push_back(std::string("--suppress-ipc"));

	if (skip_mods.length()) 
		pt_args.push_back(std::string("--skip-mod=") + skip_mods);

	if (skip_funcs.length()) 
		pt_args.push_back(std::string("--skip-func=") + skip_funcs);

	if (limit_mod.length()) 
		pt_args.push_back(std::string("--only-mod=") + limit_mod);

	if (limit_func.length()) 
		pt_args.push_back(std::string("--only-func=") + limit_func);

	if (pt_out_name.length()) 
		pt_args.push_back(std::string("-o ") + pt_out_name);

	if (trans != T_None)
	{
		std::string tstr = std::string("--use-transactions=");

		switch(trans) 
		{
			case T_Func: tstr += std::string("func"); break;
			case T_Mod: tstr += std::string("mod"); break;
			case T_Proc: tstr += std::string("proc"); break;
			default: tstr += std::string("invalid"); break;
		};

		pt_args.push_back(tstr);
	}

	return true;
}

test_results_t ParseThat::pt_execute(std::vector<std::string> &pt_args)
{

#if defined (os_windows_test)
	
	fprintf(stderr, "%s[%d]:  skipping parseThat test for this platform for now\n", 
			FILE__, __LINE__);
	return SKIPPED;
	
#else

	char cmdbuf[2048];
	sprintf(cmdbuf, "parseThat ");

	for (unsigned int i = 0; i < pt_args.size(); ++i)
	{
		sprintf(cmdbuf, "%s %s", cmdbuf, pt_args[i].c_str());
	}

	fprintf(stderr, "%s[%d]:  about to issue parseThat command: \n\t\t'%s'\n", 
			FILE__, __LINE__, cmdbuf);

	int res = system(cmdbuf);

	if (WIFEXITED(res))
	{
		short status = WEXITSTATUS(res);
		if (0 != status)
		{
			fprintf(stderr, "%s[%d]:  parseThat cmd failed with code %d\n", 
					FILE__, __LINE__, status);
			return FAILED;
		}
	}
	else
	{
		fprintf(stderr, "%s[%d]:  parseThat cmd failed\n", FILE__, __LINE__);
		if (WIFSIGNALED(res))
		{
			fprintf(stderr, "%s[%d]:  received signal %d\n", FILE__, __LINE__, WTERMSIG(res));
		}
		return FAILED;
	}

	return PASSED;
#endif
}

test_results_t ParseThat::operator()(std::string exec_path, std::vector<std::string> &mutatee_args)
{
	
	std::vector<std::string> pt_args;
	if (!setup_args(pt_args))
	{
		fprintf(stderr, "%s[%d]:  failed to setup parseThat args\n", FILE__, __LINE__);
		return FAILED;
	}

	//  maybe want to check existence of mutatee executable here?

	pt_args.push_back(exec_path);
	for (unsigned int i = 0; i < mutatee_args.size(); ++i)
	{
		pt_args.push_back(mutatee_args[i]);
	}

	if (cmd_stdout_name.length() && cmd_stdout_name == cmd_stderr_name)
	{
		//  both to same file
		pt_args.push_back(std::string("&>") + cmd_stdout_name);
	}
	else
	{
		if (cmd_stdout_name.length())
		{
			pt_args.push_back(std::string("1>") + cmd_stdout_name);
		}
		if (cmd_stderr_name.length())
		{
			pt_args.push_back(std::string("2>") + cmd_stderr_name);
		}
	}

	return pt_execute(pt_args);
}

test_results_t ParseThat::operator()(int pid)
{
	std::vector<std::string> pt_args;
	if (!setup_args(pt_args))
	{
		fprintf(stderr, "%s[%d]:  failed to setup parseThat args\n", FILE__, __LINE__);
		return FAILED;
	}

	pt_args.push_back(std::string("--pid=") + itos(pid));

	if (cmd_stdout_name.length() && cmd_stdout_name == cmd_stderr_name)
	{
		//  both to same file
		pt_args.push_back(std::string("&>") + cmd_stdout_name);
	}
	else
	{
		if (cmd_stdout_name.length())
		{
			pt_args.push_back(std::string("1>") + cmd_stdout_name);
		}
		if (cmd_stderr_name.length())
		{
			pt_args.push_back(std::string("2>") + cmd_stderr_name);
		}
	}

	return pt_execute(pt_args);
}
