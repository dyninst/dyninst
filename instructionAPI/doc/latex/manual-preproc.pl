#!/usr/bin/perl

use File::Temp "tempfile";

# Edit subsection numbering in the main document:
# 2 levels of subsection in the TOC, 2 levels numbered
# Reorder classes to match group ordering
open(CONFIG, "LatexConfig");
while(<CONFIG>)
{
    s/\#.*$//;
    chomp;
    if(/CLASSLIST =(.*)/)
    {
	@classList = split /[\W|,]+/, $1;
    }
}

sub reorderClasses
{
    my($line, $class, $fh, $curfile) = @_;
    my @tempClassList = @classList;
    # keep going through this block of class includes, stuffing the class name and
    # original line into a hash as key/value, respectively
    do
    {
	$foundClasses{$class} = $line;
	print "Found $class\n";
	$line = <$curfile>;
    }
    while(($class) = ($line =~ /\\input\{(?:class|struct).*_\d+_\d+.*_\d+_\d+(.*)\}/));
    # pull out all of the elements of the hash that appear in classlist, in order
    while(defined($class = shift @tempClassList))
    {
	print $fh $foundClasses{$class};
	print "Including $class: $foundClasses{$class}\n";
	delete $foundClasses{$class};
    }
    # pull out any remaining elements, in alphabetical order
    foreach $key (sort keys %foundClasses)
    {
	print "Including $key: $foundClasses{$key}\n";
	print $fh $foundClasses{$key};
    }
    undef %foundclasses;
    # Print the line that didn't match, so we can go back cleanly
    print $fh $line;
}


while(@ARGV)
{
    $file = shift @ARGV;
    print "Preprocessing $file...\n";
    open CURFILE, $file;
    ($temp, $tempname) = tempfile();
    while($line = <CURFILE>)
    {
	if($line =~ s/\{\\footrulewidth\}\{0.4pt\}/\{\\footrulewidth\}\{0pt\}/)
	{
	    print $temp '\\renewcommand{\\headrulewidth}{0pt}';
	    print $temp "\n";
	    print $temp '\\setlength{\\headheight}{15pt}';
	    print $temp "\n";
	}
	if($line =~ /^Reimplemented/ || $line =~ /^Implemented/ || $line =~ /^Implements/ ||
	   $line =~ /^Inherits/ || $line =~ /^Inherited by/ || $line =~ /Namespace Documentation/ ||
	   $line =~ /namespaceDyninst/)
	{
	    next;
	}
	$line =~ s/INSTRUCTION\\_\\\-EXPORT//g;
	$line =~ s/Main Page/Introduction/;
	$line =~ s/Module Documentation/Modules and Abstractions/;
	$line =~ s/Class Documentation/Class Reference/;
	$line =~ s/\\\w*section\{Detailed Description\}//;
	$line =~ s/\\(\w*)section\{Constructor \\\& Destructor Documentation\}/\\$1section\{Constructors \\\& Destructors\}/;
	$line =~ s/\\(\w*)section\{Member Function Documentation\}/\\$1section\{Member Functions\}/;
	$line =~ s/\\(\w*)section\{Member Typedef Documentation\}/\\$1section\{Member Typedefs\}/;
	$line =~ s/\\(\w*)section\{(.*)Documentation\}/\\$1section\{$2\}/;
	$line =~ s/\\(\w*)subsection\{(.*)Class Reference\}/\\$1subsection\{$2Class\}/;
	$line =~ s/\\(\w*)subsection\{(.*)Struct Reference\}/\\$1subsection\{$2Struct\}/;
	$line =~ s/\\bf/\\textbf/g;

	# remove section headers if they're tagged for removal
	$line =~ s/\\\w*section\{REMOVE\}//;
	if($line =~ /\\input\{(?:class|struct).*_\d+_\d+.*_\d+_\d+(.*)\}/)
	{
	    # this will take over until we're done with the class includes
	    reorderClasses($line, $1, $temp, CURFILE);
	}
	else
	{
	    print $temp $line;
	}
    };
    close CURFILE;
    close $temp;
    `cp $tempname $file`;
};
