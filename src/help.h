/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
static void print_help()
{
   fprintf(stderr, "General Information:\n");
   fprintf(stderr, "-------------------------\n");
   fprintf(stderr, "Usage: runTests [OPTION]...\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "runTests is a wrapper program for running test_driver. It monitors test_driver\n");
   fprintf(stderr, "for crashes, and if one happens it reports the crash point and restarts\n");
   fprintf(stderr, "test_driver.  It should be used when running multiple tests and looking for\n");
   fprintf(stderr, "failures.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Usage: test_driver [OPTION]...\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "test_driver is the executable that loads mutators runs Dyninst/Symtab/etc.\n");
   fprintf(stderr, "It should be used when debugging.  For example if you want to debug a single\n");
   fprintf(stderr, "test run in gdb you should use test_driver.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "test_driver and runTests take the same arguments.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Specifying tests to run:\n");
   fprintf(stderr, "-------------------------\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "You can use the following options to specify which tests should be run.  If\n");
   fprintf(stderr, "multiple options are specified on the same command line, then options that\n");
   fprintf(stderr, "specified the same class of thing are 'OR'd together.  Options from seperate\n");
   fprintf(stderr, "classes are 'AND'd together when determining what tests to run.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "For example, the command: 'runTests -dyninst -create -low -none -gcc -g++'\n");
   fprintf(stderr, "runs all 'Dyninst' tests in 'create' mode with optimization level 'low' or\n");
   fprintf(stderr, "'none' and compiler 'gcc' or 'g++'.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Tests\n");
   fprintf(stderr, "  -test <test>,<test>,...           specifies test mutators to run.\n");
   fprintf(stderr, "                                    Wildcards are valid in the test name.\n");
   fprintf(stderr, "  -mutatee <mutatee>,<mutatee>,...  specifies test mutatees to run\n");
   fprintf(stderr, "                                    Wildcards are valid in the mutatee name.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Create/Attach modes\n");
   fprintf(stderr, "  -create                           Run tests using create mode\n");
   fprintf(stderr, "  -attach                           Run tests using attach mode\n");
   fprintf(stderr, "  -rewriter                         Run tests in binary rewriter tests\n");
   fprintf(stderr, "  -noclean                          Do no clean rewriter produced binaries\n");
   fprintf(stderr, "  -allmode                          Run all create modes\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Components\n");
   fprintf(stderr, "  -dyninst                          Run DyninstAPI tests\n");
   fprintf(stderr, "  -symtab                           Run SymtabAPI tests\n");
   fprintf(stderr, "  -allcomp                          Run all components\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Mutatee Optimization Levels\n");
   fprintf(stderr, "  -max                              Run mutatees with max optimization\n");
   fprintf(stderr, "  -high                             Run mutatees with high optimization\n");
   fprintf(stderr, "  -low                              Run mutatees with low optimization\n");
   fprintf(stderr, "  -none                             Run mutatees with no optimization\n");
   fprintf(stderr, "  -allopt                           Run all optimization levels\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Mutatee Compilers\n");
   fprintf(stderr, "  -<compiler name>                  Run mutatees compiled with <compiler name>\n");
   fprintf(stderr, "                                    Valid choices for <compiler name> are:\n");
   fprintf(stderr, "                                    gcc, g++, g77, icc, icpc, pgcc, pgcxx, \n");
   fprintf(stderr, "                                    cc, CC, cxx, VC, VC++, sun_cc, xlc, xlC,\n");
   fprintf(stderr, "                                    ibm_as, masm, nasm, nocompiler\n");
   fprintf(stderr, "  -allcompilers                     Run all compilers\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "ABIs\n");
   fprintf(stderr, "  -32                               Run mutatees that use a 32 bit ABI\n");
   fprintf(stderr, "  -64                               Run mutatees that use a 64 bit ABI\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Link Types\n");
   fprintf(stderr, "  -dynamiclink                      Run mutatees that are dynamically linked\n");
   fprintf(stderr, "  -staticlink                       Run mutatees that are statically linked\n");
   fprintf(stderr, "\n");   
   fprintf(stderr, "Other\n");
   fprintf(stderr, "  -all                              The same as '-allmode', '-allcomp', and \n");
   fprintf(stderr, "                                    '-allcompilers'.\n");
   fprintf(stderr, "  -full                             Same as '-all', but adds '-allopt'.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Test Output Levels\n");
   fprintf(stderr, "------------------\n");
   fprintf(stderr, "  -log [logfile]                    Outputs test logging information to \n");
   fprintf(stderr, "                                    'logfile'. This contains debug \n");
   fprintf(stderr, "                                    information that may be relevant to why \n");
   fprintf(stderr, "                                    tests pass or fail. If logfile is not\n");
   fprintf(stderr, "                                    present then output is sent to stderr\n");
   fprintf(stderr, "  -v+\n");
   fprintf(stderr, "  -v++\n");
   fprintf(stderr, "  -verbose                          These options control output levels of\n");
   fprintf(stderr, "                                    debug information from test_driver. These\n");
   fprintf(stderr, "                                    are useful for debugging problems in the\n");
   fprintf(stderr, "                                    testing infrastructure.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "  -dboutput [file]                  Instead of printing human readable output, \n");
   fprintf(stderr, "                                    test_driver should print a SQL file that\n");
   fprintf(stderr, "                                    is suitable for submission to a database.\n");
   fprintf(stderr, "  -memcpu                           Gather memory usage and cpu elapsed time\n");
   fprintf(stderr, "                                    information for each test.\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Running Tests in Parallel\n");
   fprintf(stderr, "-------------------------\n");
   fprintf(stderr, "  -j [number]                       Run [number] copies of test_driver in\n");
   fprintf(stderr, "                                    parallel. As a rule-of-thumb, run one copy\n");
   fprintf(stderr, "                                    of test_driver per CPU core. (This option\n");
   fprintf(stderr, "                                    is only valid when passed to runTests)\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "  -hosts [host1] [host2] ...        Use ssh to run test_driver instances on\n");
   fprintf(stderr, "                                    the given hosts. Use with the -j option\n");
   fprintf(stderr, "                                    when running on multiple hosts. (This\n");
   fprintf(stderr, "                                    option is only valid when passed to\n");
   fprintf(stderr, "                                    runTests)\n");
}
