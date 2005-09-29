#!/usr/bin/perl -w

use strict;
use POSIX;


########################################
# Global Variables
#
my $useLog = 0;
my $test_driver = "$ENV{'PWD'}/test_driver";
my $testLimit = 10;
my $timeout = 60;

########################################
# Sub Declarations
#

sub ParseParameters();
sub RunTest($);
sub PDScriptDir();


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

   # Set Environment Variables
   if ( not defined($ENV{'PDSCRDIR'}) )
   {
      $ENV{'PDSCRDIR'} = PDScriptDir();
   }

   if ( not defined($ENV{'RESUMELOG'}) )
   {
      $ENV{'RESUMELOG'} = "/tmp/test_driver.resumelog.$$";
   }
   $ENV{'LD_LIBRARY_PATH'} .= ":.";

   # Prepend timer script
   my $totalTimeout = $testLimit * $timeout;
   my $timerString = "$ENV{'PDSCRDIR'}/timer.pl -t $totalTimeout ";
   $testString = $timerString . $testString;


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
      if (/\-log/ )
      {
         $useLog = 1;
      }
   }
}

########################################
# Main Code
#
ParseParameters();

my $result = 0;
my $invocation = 0;

# Return value of 2 indicates that we have run out of tests to run
while ( $result != 2 )
{
   $result = RunTest($invocation);
   $invocation++;
}

# Call Reap here
