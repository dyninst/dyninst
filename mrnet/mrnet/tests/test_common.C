/****************************************************************************
 * Copyright © 2003-2007 Dorian C. Arnold, Philip C. Roth, Barton P. Miller *
 *                  Detailed MRNet usage rights in "LICENSE" file.          *
 ****************************************************************************/

#include "test_common.h"
#include "timer.h"

#include <stdio.h>
#include <math.h>

namespace MRN_test{
std::map< MRN::DataType, std::string> Type2String;
std::map< MRN::DataType, const char *> Type2FormatString;

class StaticInitializer{
public:
    StaticInitializer();
};

StaticInitializer si; //Used to initialize maps

bool compare_Float(float f1, float f2, int sig)
{
    if( fabs(f1 - f2) > pow( (double)10, sig * -1) ) {
        return false;
    }

    return true;
}

bool compare_Double(double f1, double f2, int sig)
{
    if( fabs(f1 - f2) > pow( (double)10, sig * -1) ) {
        return false;
    }

    return true;
}

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

Test::~Test()
{
    std::map<std::string, SubTest*>::iterator iter;
    for( iter = subtests.begin(); iter != subtests.end(); iter++ )
        delete iter->second;
}

void Test::end_Test( )
{
    timer.end();

    if( num_failures == 0 ){
        fprintf(fout, "Test %s Complete. All tests passed.\n", name.c_str() );
    }
    else{
        fprintf(fout, "Test %s Complete. There were %d failures\n",
                name.c_str(), num_failures);
    }

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

StaticInitializer::StaticInitializer()
{
    Type2FormatString[ CHAR_T ] = "%c";
    Type2FormatString[ UCHAR_T ] = "%uc";
    Type2FormatString[ INT16_T ] = "%hd";
    Type2FormatString[ UINT16_T ] = "%uhd";
    Type2FormatString[ INT32_T ] = "%d";
    Type2FormatString[ UINT32_T ] = "%ud";
    Type2FormatString[ INT64_T ] = "%ld";
    Type2FormatString[ UINT64_T ] = "%uld";
    Type2FormatString[ FLOAT_T ] = "%f";
    Type2FormatString[ DOUBLE_T ] = "%lf";

    Type2String[ CHAR_T ] = "char_t";
    Type2String[ UCHAR_T ] = "uchar_t";
    Type2String[ INT16_T ] = "int16_t";
    Type2String[ UINT16_T ] = "uint16_t";
    Type2String[ INT32_T ] = "int32_t";
    Type2String[ UINT32_T ] = "uint32_t";
    Type2String[ INT64_T ] = "int64_t";
    Type2String[ UINT64_T ] = "uint64_t";
    Type2String[ FLOAT_T ] = "float_t";
    Type2String[ DOUBLE_T ] = "double_t";
}

void val2string( char * ostring, void * ival, MRN::DataType itype )
{
    ostring = NULL;
    switch(itype){
    case CHAR_T:
        sprintf(ostring, "%c", *((char_t *)ival) );
        break;
    case UCHAR_T:
        sprintf(ostring, "%c", *((uchar_t *)ival) );
        break;
    case INT16_T:
        sprintf(ostring, "%d", *((uint16_t *)ival) );
        break;
    case UINT16_T:
        sprintf(ostring, "%hu", *((uint16_t *)ival) );
        break;
    case INT32_T:
        sprintf(ostring, "%d", *((uint32_t *)ival) );
        break;
    case UINT32_T:
        sprintf(ostring, "%u", *((uint32_t *)ival) );
        break;
    case INT64_T:
        sprintf(ostring, "%Ld", *((int64_t *)ival) );
        break;
    case UINT64_T:
        sprintf(ostring, "%Lu", *((uint64_t *)ival) );
        break;
    case FLOAT_T:
        sprintf(ostring, "%f", *((float *)ival) );
        break;
    case DOUBLE_T:
        sprintf(ostring, "%lf", *((double *)ival) );
        break;
    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
    case FLOAT_ARRAY_T:
    case DOUBLE_ARRAY_T:
    case STRING_T:
    case UNKNOWN_T:
    default:
        ostring=NULL;
    }
}

bool compare_Vals( void * ival1, void * ival2, MRN::DataType itype )
{
    switch(itype){
    case CHAR_T:
        if( *((char_t *)ival1) == *((char_t *)ival2) )
            return true;
    case UCHAR_T:
        if( *((uchar_t *)ival1) == *((uchar_t *)ival2) )
            return true;
    case INT16_T:
        if( *((int16_t *)ival1) == *((int16_t *)ival2) )
            return true;
    case UINT16_T:
        if( *((uint16_t *)ival1) == *((uint16_t *)ival2) )
            return true;
    case INT32_T:
        if( *((int32_t *)ival1) == *((int32_t *)ival2) )
            return true;
    case UINT32_T:
        if( *((uint32_t *)ival1) == *((uint32_t *)ival2) )
            return true;
    case INT64_T:
        if( *((int64_t *)ival1) == *((int64_t *)ival2) )
            return true;
    case UINT64_T:
        if( *((uint64_t *)ival1) == *((uint64_t *)ival2) )
            return true;
    case FLOAT_T:
        return compare_Float( *((float *)ival1), *((float *)ival2), 4 );
    case DOUBLE_T:
        return compare_Double( *((double *)ival1), *((double *)ival2), 4 );
    case CHAR_ARRAY_T:
    case UCHAR_ARRAY_T:
    case INT16_ARRAY_T:
    case UINT16_ARRAY_T:
    case INT32_ARRAY_T:
    case UINT32_ARRAY_T:
    case INT64_ARRAY_T:
    case UINT64_ARRAY_T:
    case FLOAT_ARRAY_T:
    case DOUBLE_ARRAY_T:
    case STRING_T:
    case UNKNOWN_T:
    default:
        return false;
    }
}
}
