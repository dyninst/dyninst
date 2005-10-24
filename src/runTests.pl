#!/usr/bin/perl -w
# $Id: runTests.pl,v 1.3 2005/10/24 03:55:38 bpellin Exp $

use strict;
use POSIX;


########################################
# Global Variables
#
my $useLog = 0;
my $logfile = '';
my $test_driver = "$ENV{'PWD'}/test_driver";
my $testLimit = 10;
my $timeout = 120;

my $BASE_DIR;
my $TLOG_DIR;
my $PDSCRDIR;

########################################
# Sub Declarations
#

sub ParseParameters();
sub SetupVars();
sub RunTest($);
sub PDScriptDir();


########################################
# SetupVars
#
sub SetupVars()
{
   # Determine Base dir
   if ( not defined $ENV{'PARADYN_BASE'} ) {
      $BASE_DIR=$ENV{'DYNINST_ROOT'};
   } else {
      $BASE_DIR=$ENV{'PARADYN_BASE'};
   }

   $PDSCRDIR = "$BASE_DIR/scripts";

   # Determine Test log dir
   if ( not defined $ENV{'PDTST'} ) {
      $TLOG_DIR = "$BASE_DIR/log/tests";
   } else {
      $TLOG_DIR = $ENV{'PDTST'};
   }
   $TLOG_DIR .= "2";
      
   # Determine Test log file
   my $BUILDNUM = "$PDSCRDIR/buildnum";
   my $BUILD_ID;
   if ( defined $ENV{'PARADYN_BASE'} and -f $BUILDNUM ) {
      $BUILD_ID = `$BUILDNUM`;
   } else {
      $BUILD_ID = `date '+%Y-%m-%d'`;
   }
   chomp $BUILD_ID;


   if ( $useLog != 0 ) {
      if ( $logfile eq "" )
      {
         $logfile = "$TLOG_DIR/$ENV{'PLATFORM'}/$BUILD_ID";
      }

      print "   ... output to $logfile\n";

      my $TESTSLOGDIR=`dirname $logfile`;
      chomp $TESTSLOGDIR;
      if ( not -d $TESTSLOGDIR ) {
         print "$TESTSLOGDIR dose not exist (yet)!\n";
         system("mkdir -p $TESTSLOGDIR");
         if ( not -d $TESTSLOGDIR ) {
            print "$TESTSLOGDIR creation failed - aborting!\n";
            exit(1);
         } else {
            print "$TESTSLOGDIR created for test logs!\n";
         }
      }

      if ( -f $logfile ) {
         print "File exists\n";
      } else {
         system("touch $logfile");
      }

      if ( -f "$logfile.gz" ) {
         print "File.gz exists\n";
      }
   }

}

########################################
# RunTest
#
#TODO: Setup enviroment variables
sub RunTest($) {
   my $iteration = shift;

   my $testString = "$test_driver -enable-resume -limit $testLimit";
   if ( $iteration != 0 )
   {
      $testString .= " -use-resume";
   }
   if ( $useLog != 0 )
   {
      $testString .= " -log";
   }

   #$testString .= " " . (join " ", @ARGV);

   # Set Environment Variables
   $ENV{'PDSCRDIR'} = $PDSCRDIR;

   if ( not defined($ENV{'RESUMELOG'}) )
   {
      $ENV{'RESUMELOG'} = "/tmp/test_driver.resumelog.$$";
   }
   $ENV{'LD_LIBRARY_PATH'} = ".:" . $ENV{'LD_LIBRARY_PATH'};

   # Prepend timer script
   my $totalTimeout = $timeout;
   my $timerString = "$ENV{'PDSCRDIR'}/timer.pl -t $totalTimeout ";
   $testString = $timerString . $testString;

   if ( $useLog != 0 )
   {
      $testString .= " 2>&1 | tee -a $logfile";
   }

   # Run Test Program
   system($testString);
   return WEXITSTATUS($?);

}

########################################
# PDScriptDir
#
sub PDScriptDir()
{
   my $UW_path = '/p/paradyn/builds/scripts';
   my $UMD_path = '/fs/dyninst/dyninst/current/scripts';

   if ( -e $UW_path )
   {
      return $UW_path;
   }
   if ( -e $UMD_path )
   {
      return $UMD_path;
   }

   print "Error finding pdscript dir, set the environment variable PDSCRDIR\n";
   exit(1);
}

########################################
# Parse Parameters
#
sub ParseParameters()
{
   while ($_ = shift(@ARGV)) {
      if (/^-log/ )
      {
         $useLog = 1;
         if ( $#ARGV >= 0 and not ($ARGV[0] =~ /^-/) )
         {
            $logfile = shift @ARGV;
         }
      }
   }
}

########################################
# Main Code
#
ParseParameters();
SetupVars();

my $result = 0;
my $invocation = 0;


# Return value of 2 indicates that we have run out of tests to run
while ( $result != 2)
{
   $result = RunTest($invocation);
   $invocation++;
}

if ( $useLog != 0 )
{
   system("gzip $logfile");
   print "Test log $logfile.gz written.\n";
}
