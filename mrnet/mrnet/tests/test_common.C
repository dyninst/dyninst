#include "mrnet/tests/test_common.h"
#include "mrnet/tests/timer.h"

#include <stdio.h>
using namespace MRN_test;

Test::SubTest::SubTest(const std::string & subtest_name, FILE *f)
    :name(subtest_name), fout(f), status(NOTRUN)
{
    fprintf( fout, "    **Starting SubTest: \"%s\"\n", name.c_str() );
    timer.start();
}

void Test::SubTest::end(TestStatus s)
{
    timer.end();
    status=s;
    switch(status){
    case SUCCESS:
        fprintf(fout, "      %s: SUCCESS! (%lf secs)\n\n", name.c_str(),
                timer.duration() );
        break;
    case COMPLETED:
        fprintf(fout, "      %s: COMPLETED! (%lf secs)\n\n", name.c_str(),
                timer.duration() );
        break;
    case FAILURE:
        fprintf(fout, "      %s: FAILURE! (%lf secs)\n\n", name.c_str(),
                timer.duration() );
        break;
    default:
        fprintf(fout, "      %s: Unknown test status! (%lf secs)\n\n",
                name.c_str(),
                timer.duration() );
        break;
    }
}

void Test::SubTest::print_status()
{
    switch(status){
    case SUCCESS:
        fprintf(fout, "%s: SUCCESS! (%lf secs)\n", name.c_str(),
                timer.duration() );
        break;
    case COMPLETED:
        fprintf(fout, "%s: COMPLETED! (%lf secs)\n", name.c_str(),
                timer.duration() );
        break;
    case FAILURE:
        fprintf(fout, "%s: FAILURE! (%lf secs)\n", name.c_str(),
                timer.duration() );
        break;
    default:
        fprintf(fout, "%s: Unknown test status! (%lf secs)\n", name.c_str(),
                timer.duration() );
        break;
    }
}

Test::Test(const char *test_name, FILE *f)
    :name(test_name), fout(f), num_failures(0)
{
    fprintf( fout, "  * Starting Test: \"%s\"\n", name.c_str() );
    timer.start();
}

void Test::end_Test( )
{
    timer.end();

    fprintf(fout, "Test %s Complete. There were %d failures\n",
            name.c_str(), num_failures);

    if( num_failures ){
        std::map<std::string, SubTest*>::iterator iter;

        fprintf(fout, "Sub-test Summary:\n");
        for( iter = subtests.begin(); iter != subtests.end(); iter++ ){
            (*iter).second->print_status();
        }
    }
}

int Test::start_SubTest(const std::string & subtest_name)
{
    SubTest * subtest;
    std::map<std::string, SubTest*>::iterator iter;

    iter = subtests.find( subtest_name );
    if( iter != subtests.end() ){
        //subtest already exists
        fprintf(fout, "Subtest %s already exists\n", subtest_name.c_str() );
        return -1;
    }

    subtest = new SubTest(subtest_name, fout);
    subtests[subtest_name] = subtest;

    return 0;
}

int Test::end_SubTest(const std::string & subtest_name, TestStatus status)
{
    SubTest * subtest;
    std::map<std::string, SubTest*>::iterator iter;

    iter = subtests.find( subtest_name );
    if( iter == subtests.end() ){
        //subtest does not already exist
        fprintf(fout, "Subtest %s does not exist\n", subtest_name.c_str() );
        return -1;
    }

    subtest = (*iter).second;
    subtest->end(status);

    if(status == FAILURE){
        num_failures++;
    }

    return 0;
}

void Test::print(const char *s, const std::string& subtest_name)
{
    if(subtest_name == ""){
        fprintf(fout, "%s", s);
    }
    else{
        fprintf(fout, "      %s: %s", subtest_name.c_str(), s);
    }
}
