#!/usr/bin/perl
# The script provides a autobuild routines for:
#   - every option configurations (features, including dependant ones)
#   - every available utility executable
# It currently not provides only few functions this conception could
# be used for.
#
# I'll document it later in detail, but for a while
# it is only worth to say that this script implies a presense of certain
# commented 'tags' inside a CMakeLists.txt. They are:
#   \option
#   \dependent-option
#   \util
#   \project-subdir
# So, please, keep it in CMakeLists.txt as they are. Some POD-formatted
# info can be found at the end of file. Or, better, run
#   $ perldoc testbuild.pl
#
#                                           Crank, 12/2015

# TODO: build scripts.

use strict;
use warnings;
use Getopt::Long;
use File::Basename;
use Cwd 'abs_path';
use File::Spec;
use IPC::Open2;
use Config;
use POSIX;
use Time::HiRes qw( time );
use List::Util qw(reduce);

my %variables = (
    buildConfigs => [],
    buildOpts => {},
    optionDeps => {},
);

sub sec2hum {
    my $time = shift;
    my $days = int($time / 86400);
    $time -= ($days * 86400);
    my $hours = int($time / 3600);
    $time -= ($hours * 3600);
    my $minutes = int($time / 60);
    my $seconds = $time % 60;

    $days = $days < 1 ? '' : $days .'d ';
    $hours = $hours < 1 ? '' : $hours .'h ';
    $minutes = $minutes < 1 ? '' : $minutes . 'm ';
    $time = $days . $hours . $minutes . $seconds . 's';
    return $time;
}

sub check_line_for_tag {
    my $tag = shift;
    my $expectedPrefix = shift;
    my $inFile = shift;
    if( /^\s*\#\s*\\(?:$tag)\s*\:\s*$/ ) {
        my $str = scalar <$inFile>;
        if( $str =~ /^\s*$expectedPrefix\s*\(([^()]+)\)(?1)$/ ) {
            $str =~ s/\R//g;
            return $str;
        } elsif( $str =~ /^\s*$expectedPrefix.*$/ ) {
            #print "multiline: $str";
            my $last;
            my $startline = $.;
            my @matches;
            until( @matches = (($str .= ($last = scalar <$inFile>)) =~ /
                (^([^()])+((?2)*\(((?:(?2))+|(?3))\)(?2)*)(?2)*)
            /mx ) and $matches[0] =~ /^(?:\s*)$expectedPrefix(?:\s*)\([^)]*\)/mx ) {
                if( not $last ) {
                    print STDERR "\033[1;31mError\033[0m: unbalanced bracket is met at line $startline.\n";
                    return;
                }
            }
            $str =~ s/\R//g;
            return $str;
            #print "\033[1;32mok\033[0m:$str";
        } else {
            $str =~ s/\R//g;
            print STDERR
            "\033[1;33mWarning\033[0m: line $.:\n$str
    is tagged as \"%tag\", but doesn't match an expected build configuration format for #\\tag:\\n$expectedPrefix(...). Ignored.\n";
        }
    }
    return;
}

# XXX: {{{
sub check_unbalanced_pattern {
    my $tag = shift;
    my $expectedPrefix = shift;
    my $inFile = shift;
    if( /^\s*\#\s*\\(?:$tag)\s*\:\s*$/ ) {
        my $str = scalar <$inFile>;
        if( $str =~ /^\s*\w+\s*\([^\)]+$/ ) {
            #print "multiline: $str";
            my $last;
            my $startline = $.;
            my @matches;
            until( @matches = (($str .= ($last = scalar <$inFile>)) =~ /
                (^([^()])+      #{1,2-2
                    (           #{3
                    (?2)*\(
                        (       #{4
                            (?:(?2))+ | (?3)
                        )       #}4
                    \)(?2)*
                    )           #}3
                (?2)*)$         #}1
            /mx ) and $matches[0] =~ /^(?:\s*)$expectedPrefix(?:\s*)\([^)]*\)/mx ) {
                if( not $last ) {
                    print STDERR "Unbalanced bracket is met at line $startline.\n";
                    return;
                }# else {  # XXX
                #    print "str>", $str;
                #    print "matches>", @matches, "\n======================================\n";
                #}
            }
            if( 0 ) {
                print "MATCHES>:";
                foreach my $m (@matches) {
                    print $m, "<";
                }
                print ":<MATCHES\n";
                print "\n>", $str;
            }
            print "* $str";
        } else {
            print "~ $str";
        }
    } else {
        print "- $_";
    }
} # }}}

sub form_build_options {
    my $enabledOptsRef = shift;
    my @allOptions = @{ (shift) };
    my %enabledHash = map {$_ => 1} @$enabledOptsRef;
    #
    my @disabledOpts = grep { not $enabledHash{$_} } @allOptions;
    #
    my @enOpts = map { "-D" . $_ . "=ON" } sort @{$enabledOptsRef};
    my @dsOpts = map { "-D" . $_ . "=OFF" } sort @disabledOpts;
    #
    return (@enOpts, @dsOpts);
}

sub mktestbuild {
    my $arglist = shift;
    my $targetDir = shift;
    #
}

sub features2code {
    my $dictRef = shift;
    my $features = shift;
    my $maxLen = 0;
    $_ > $maxLen and $maxLen = $_ for values %{$dictRef};
    my @encoded = (0) x ceil( $maxLen/($Config{intsize}*8) );
    for my $opt (@{$features}) {
        my $nPos = %$dictRef{$opt};
        my $posOffset = $nPos % ($Config{intsize}*8);
        $encoded[floor($nPos / ($Config{intsize}*8))] |= (1 << $posOffset);
    }
    return @encoded;
}

sub do_build {
    my $btime = time();
    my $utilname = shift;
    my $srcdir = shift;
    my $bCfg = shift;
    my $ftCode = shift;
    my $dryRun = shift;
    my @arglist = (
            "$utilname",
            "--loglevel=1",
            "-f", "$ftCode",
            "--srcdir=$srcdir",
            "-C $bCfg",
            "cmakeopts:", @{ (shift) }, @{ (shift) }, @{ (shift) },
            "makeopts:", @{ (shift) } );
    if( $dryRun ) {
        splice @arglist, 1, 0, "-d";
    }
    my $rc;
    #print join( " ", @arglist ), "\n";
    system( @arglist );  # ${^CHILD_ERROR_NATIVE} ?
    my $elapsed = time() - $btime;

    if( -1 == $? ) {
        die "Failed to run $utilname."
    } elsif( $? & 127 ) {
        printf "child died with signal %d, %s coredump\n",
               ($? & 127),  ($? & 128) ? 'with' : 'without';
    } else {
        # Anticipated error (cmake or make)
        my $rc = $? >> 8;
        if( 10 == $rc ) {
            die "Interrupt";
        } # otherwise do nothing but return
    }

    return ( $? >> 8, $elapsed );
}

#
# ENTRY POINT
############

#
# Parse arguments:
my $rootCMakeLists = "CMakeLists.txt";
my $verbose;
my $dryRun;
my @fwddCMakeOpts;
my @fwddMakeOpts;
my $ftNumMaskExpr = '-1 < $ftCode';
my @bConfOnly;
GetOptions("root-file|L=s" => \$rootCMakeLists,
               "v|verbose" => \$verbose,
               "dry-run|d" => \$dryRun,
        "ft-mask-expr|E=s" => \$ftNumMaskExpr,
            "bconf-only=s" => \@bConfOnly,
       "fwd-cmake-opt|C=s" => \@fwddCMakeOpts,
        "fwd-make-opt|M=s" => \@fwddMakeOpts )
    or die( "Command line arguments error." );

#print "CMake options to forward   : ", join("|", @fwddCMakeOpts ), "\n";
#print "GNU make options to forward: ", join("|", @fwddMakeOpts ), "\n";
#exit;  # XXX

my $bashScriptPath = File::Spec->catfile( abs_path(dirname($0)), "runbuild.sh");  # <<< NOTE: shell-script
if( not -e $bashScriptPath ) {
    print STDERR "\033[1;31mError\033[0m: Couldn't find necessary worker script: \"$bashScriptPath\" as it doesn't exist.\n";
    die;
} elsif( not -f $bashScriptPath and not -l $bashScriptPath ) {
    print STDERR "\033[1;31mError\033[0m: Worker script path \"$bashScriptPath\" points to neither file, not a symlink.\n";
    die;
} elsif( not -x $bashScriptPath ) {
    # Sure, we can use it though by givin a parameter to /bin/sh,
    # but we wouldn't since it requires a bit more sophisticated command-line
    # parsing algorithm.
    print STDERR "\033[1;31mError\033[0m: Worker script \"$bashScriptPath\" hasn't execution permissions.\n";
    die;
}

my @inputFileNames = ( abs_path($rootCMakeLists) );
my @processedFiles;

#
# Process file:
while( @inputFileNames ) {
    my $inFilename = shift @inputFileNames;
    push @processedFiles, $inFilename;
    if( not -e $inFilename ) {
        print STDERR "\033[1;31mError\033[0m: Couldn't treat file: \"$inFilename\" as it doesn't exist. Ignored.\n";
        next;
    }
    open my $inFile, "<", $inFilename or die "Could not open $rootCMakeLists: $!";
    print "Parsing \033[0;32m$inFilename\033[0m ...\n";
    while(<$inFile>) {
        #check_unbalanced_pattern "option", "someopt", $inFile;
        my $str;
        if( $str = check_line_for_tag( "option",               "option",           $inFile ) ) {
            # TODO: support also new push_option() tag
            if( $str !~ /^\s*option\s*\(\s*(\S+)\s+\"(.*)\"\s+(ON|OFF)\s*\)\s*$/  ) {
                print STDERR "\033[1;31mError\033[0m: Unexpected form of option provided: \"$str\". Ignored.\n";
                next;
            }
            $variables{buildOpts}{$1} = [$2, $3];
        } elsif( $str = check_line_for_tag( "buildconf",       "set",              $inFile ) ) {
            if( $str !~ /^\s*set\s*\(\s*CMAKE_BUILD_TYPE\s+\"(.*)\"\s*\).*$/  ) {
                print STDERR "\033[1;31mError\033[0m: Unexpected form of build configuration provided: \"$str\". Ignored.\n";
                next;
            }
            push @{ $variables{buildConfigs} }, lc$1;
        #} elsif( $str = check_line_for_tag( "opt-dep",         "option_depend",    $inFile ) ) {
        #    if( $str !~ /^\s*option_depend\s*\(\s*(\S+)\s+(\S+)\s*\)\s*$/  ) {
        #        print STDERR "\033[1;31mError\033[0m: Unexpected form of dependency provided: \"$str\". Ignored.\n";
        #        next;
        #    }
        #    if( exists $variables{optionDeps}{$1} ) {
        #        push @{ $variables{optionDeps}{$1} }, $2;
        #    } else {
        #        @{ $variables{optionDeps}{$1} } = ($2);
        #    }
        } elsif( $str = check_line_for_tag( "project-subdir",  "add_subdirectory", $inFile ) ) {
            if( $str !~ /^\s*add_subdirectory\s*\(\s*(\S+)\s*\)\s*$/  ) {
                print STDERR "\033[1;31mError\033[0m: Unexpected form of included subdirectory provided: \"$str\". Ignored.\n";
                next;
            }
            my $fileToInclude = File::Spec->catfile( abs_path(dirname($rootCMakeLists)), $1, "CMakeLists.txt");
            if( -e $fileToInclude ){
                push @inputFileNames, $fileToInclude if not grep(/^$fileToInclude$/, @processedFiles);
            } else {
                print STDERR "\033[1;31mError\033[0m: Couldn't include: \"$fileToInclude\" as it doesn't exist. Ignored.\n";
            }
        } elsif( $str = check_line_for_tag( "opt-dep",         "option_depend",    $inFile ) ) {
            if( $str !~ /^\s*option_depend\s*\(\s*(\S+)\s+((?:\S+\s+)+)\)\s*$/g  ) {
                print STDERR "\033[1;31mError\033[0m: Unexpected form of dependency provided: \"$str\". Ignored.\n";
                next;
            }
            my $dependency = $1;
            my $depOptString = $2;
            if( not exists $variables{optionDeps}{$dependency} ) {
                @{ $variables{optionDeps}{$dependency} } = ();
            }
            #print "$dependency:\n";
            while( $depOptString =~ /(?:(?:^|\s+)(\S+))/gx) { # TODO: ignores other subgroups
                #sprint "\t", $1, "\n";  # XXX
                push @{ $variables{optionDeps}{$dependency} }, $1;
            }
            @{ $variables{optionDeps}{$dependency} } = sort @{ $variables{optionDeps}{$dependency} };
        } # ...
    }
    @{ $variables{buildConfigs} } = sort @{ $variables{buildConfigs} };
    close $inFile;
}

sub get_allowed_options {
    my $enabledOptionsRef = shift;
    my %enabledOptions = map {$_ => 1} @{$enabledOptionsRef};
    my @allowedOpts = ();
    CANDIDATE: for my $opt (keys %{$variables{"optionDeps"}}) {
        for my $dep (@{$variables{"optionDeps"}{$opt}}) {
            next CANDIDATE if not exists($enabledOptions{$dep});
        }
        push @allowedOpts, $opt;
    }
    #return @allowedOpts;
    @allowedOpts = grep {not $enabledOptions{$_}} @allowedOpts;
    @allowedOpts = do { my %seen; grep { !$seen{$_}++ } @allowedOpts };
    @allowedOpts = sort @allowedOpts;
    return @allowedOpts;
}

#
# Recursive options building
sub populate_enabled_options_list {
    my $leafOptionsRef = shift;
    my $enabledOptionsRef = shift;
    my $readySetsRef = shift;
    my $currentOptsPool = shift;
    #my $noDepsResolution = shift;
    if( not scalar @{ $leafOptionsRef } ) {
        # Check, whether or not currently enabled options allow to resolve dependant
        # option(s). Already enabled will be excluded from resolution array:
        my %alreadyConsidered = map { $_ => 1 } @{$currentOptsPool};
        my @resolvedOptions = get_allowed_options( $enabledOptionsRef );
        my @uniqueResolvedOptions = grep { not exists $alreadyConsidered{$_} } @resolvedOptions;
        #my @repeatedOprions = grep { exists $alreadyConsidered{$_} } get_allowed_options( $enabledOptionsRef );
        if( scalar @uniqueResolvedOptions ) {
            # remove "leafOptions" from resolved
            #print "1!:", join(", ", @resolvedOptions ), " .-. ",
            #             join(", ", @{$currentOptsPool}), " .#. ",
            #             join(", ", @uniqueResolvedOptions), "\n"; # TODO?
            populate_enabled_options_list( \@uniqueResolvedOptions, $enabledOptionsRef, $readySetsRef, \@resolvedOptions );
            #print "XXX\n"; # XXX
        } else {
            #print "\033[1;32mON\033[0m:",  join( ", ", @{$enabledOptionsRef} ), "\n";
            push @{$readySetsRef}, [@{$enabledOptionsRef}];
        }
    } else {
        # Acquire one option:
        my $opt = shift @{$leafOptionsRef};
        # Try with disabled option:
        #print "\033[1;32m->\033[0m:",  join( ", ", @{$enabledOptionsRef} ), "\n";
        populate_enabled_options_list( $leafOptionsRef, $enabledOptionsRef, $readySetsRef, $currentOptsPool );
        # Enable option and run:
        push @{$enabledOptionsRef}, $opt;
        populate_enabled_options_list( $leafOptionsRef, $enabledOptionsRef, $readySetsRef, $currentOptsPool );
        # Return taken option back:
        @{$enabledOptionsRef} = grep { $_ ne $opt } @{$enabledOptionsRef};
        push @{$leafOptionsRef}, $opt;
    }
    return @{$readySetsRef};
}

print "\033[1;32mParsing done\033[0m.\n";
print
"###############################################################################\n";
my @allOptions;
my @leafOptions;
my @dependantOptions;
my @orderedOptions;  # (@leafOptions, @dependantOptions)
#
# Printing results
# print available options table with descriptions:
print "\n\033[1;1mTable I\033[0m: «Available options»:
            Option name | Dft. | Description
------------------------|------|---------------------------------------------------------------------------------\n";
for my $opt (sort (keys %{$variables{"buildOpts"}})) {
format STDOUT =
@>>>>>>>>>>>>>>>>>>>>>> | @<<< | @<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
$opt, $variables{buildOpts}{$opt}[1], $variables{buildOpts}{$opt}[0]
.
    write;
    push @allOptions, $opt;
}
print "\n";

@allOptions = sort @allOptions;
@leafOptions = @allOptions;

print
"###############################################################################\n";

print "\n\033[1;1mGraph I\033[0m: «Options dependencies»:
digraph {\n";
for my $opt (keys %{$variables{"optionDeps"}}) {
    my $lbl = $opt;
    if( $lbl =~ /^build_(\S+)/ ) {
        $lbl = $1;
        print $lbl, " [shape=component, fixedsize=shape, margin=.2]\n";
    }
    print "    ", $lbl, " -> {";
    for my $dep (@{$variables{"optionDeps"}{$opt}}) {
        print $dep, " ";
    }
    print "};\n";
    @leafOptions = grep { $_ ne $opt } @leafOptions;
    push @dependantOptions, $opt;
}
print "}\n";

print
"###############################################################################\n";

@dependantOptions = sort @dependantOptions;
@leafOptions = do { my %seen; grep { !$seen{$_}++ } @leafOptions };
@leafOptions = sort @leafOptions;

# Fill option positions dictionary:
my %optionPositions;
my $optMaxNBit;
{
    my $optMaxNBit = 0;
    %optionPositions = map {$_ => ($optMaxNBit++)} reverse @leafOptions;
    %optionPositions = (%optionPositions, map {$_ => ($optMaxNBit++)} reverse @dependantOptions);
    { # xxx: debug print-out
        print "# Bitfields:\n";
        for my $key (sort { $optionPositions{$a} <=> $optionPositions{$b} } keys %optionPositions) {
            print "#     $key => $optionPositions{$key}\n";
        }
    }
    print "# nbits: $optMaxNBit\n";
    print "# nbytes: $optMaxNBit/(8*$Config{intsize}) = "
          , int(ceil($optMaxNBit/(8*$Config{intsize})))
          , "\n";
}

print
"###############################################################################\n";

my @targBConf;  # target build configurations array
if( scalar @bConfOnly ) {
    @targBConf = @bConfOnly;  # use --bconf-only, if provided
} else {
    @targBConf = @{$variables{buildConfigs}};  # use all obtained build configs
}
@targBConf = sort @targBConf;

#print "Leaf options: ",  join( ", ", @leafOptions ), "\n"; # XXX
my @leafOptionsCopy = @leafOptions;
my @buildOptions = populate_enabled_options_list(\@leafOptions, [], [], [], \@leafOptionsCopy);
# populate_enabled_options_list reverses order, so:
@leafOptions = sort @leafOptions;

#print "All available build variants: $#buildOptions.\n"; # XXX?

print "\n\033[1;1mTable II\033[0m: «Build Test Resutls»:\n";
my $optOffsetIdx = 0;
for my $strOpt (@leafOptions) {
    print " " x 23;
    print "│" x $optOffsetIdx;
    print sprintf( "┍ %x %s", $optionPositions{$strOpt}, $strOpt), "\n";
    $optOffsetIdx += 1;
}
print " " x 23;
print "│" x $optOffsetIdx;
print "\n";
for my $strOpt (@dependantOptions) {
    print " " x 23;
    print "│" x (scalar @leafOptions);
    print " ";
    print "│" x ($optOffsetIdx - @leafOptions);
    print sprintf( "┍ %x %s", $optionPositions{$strOpt}, $strOpt), "\n";
    $optOffsetIdx += 1;
}

print
"   Number ║ Ft. code ║ ";
print "│" x (scalar @leafOptions);
print " ";
print "│" x ($optOffsetIdx - @leafOptions), " ║";
for my $bCfg (@targBConf) {
    print sprintf(" %7.7s │", $bCfg );
}
print "\b║\n";

print "══════════╬══════════╬═";
print "╽" x (scalar @leafOptions);
print "═";
print "╽" x ($optOffsetIdx - @leafOptions), "═╬";
#print "═╬═════════════════════════\n";
for my $bCfg (@targBConf) {
    print "═════════╪";
}
print "\b╣\n";

# Filter obtained @buildOptions applying mask to
# obtain choosen ones:
my @buildOptionsFiltered;
for my $enableOptionsRef (@buildOptions) {
    my $ftCode = (features2code(\%optionPositions
                                , $enableOptionsRef))[0];
    if( not eval $ftNumMaskExpr ) {
        next; # omit by mask $ftNumMaskExpr
    }
    push @buildOptionsFiltered, $enableOptionsRef;
}

my $nVariant = 0;
my @elapsedTimes = 0;
for my $enableOptionsRef (@buildOptionsFiltered) {
    my $elapsedTime;
    my $remainTaime;
    map { $elapsedTime += $_ } @elapsedTimes;
    (my @enOpts, my @dsOpts) = form_build_options( $enableOptionsRef, \@allOptions );
    $remainTaime = ($elapsedTime/($nVariant + 1))*(( scalar @buildOptionsFiltered ) - $nVariant);

    my %enabledOptionsHash = map {$_ => 1} @{$enableOptionsRef};
    my $ftCode = (features2code(\%optionPositions
                                , $enableOptionsRef))[0];

    # TODO: print longer number (if code is greater than one integer)
    print sprintf( "% 9d ║ % 8x ║ "
                    , $nVariant
                    , $ftCode );
    for my $cOpt ( @leafOptions ) {
        if( exists $enabledOptionsHash{$cOpt} ) {
            print "\033[1;32m+\033[0m";
        } else {
            print ".";
        }
    }
    print " ";
    for my $cOpt ( @dependantOptions ) {
        if( exists $enabledOptionsHash{$cOpt} ) {
            print "+";
        } else {
            print ".";
        }
    }
    print " ║ \033[s\n",
          "Build #$nVariant/$#buildOptionsFiltered. Time estimation: ",
          ($elapsedTime ? sprintf("elapsed %s , remains ≈%s",
                                  sec2hum( $elapsedTime ),
                                  sec2hum( $remainTaime )) : "elapsed --- , remains --- "),
          "\033[u\033[1A\033[s";
    for my $bCfg (sort @targBConf) {
        (my $result, my $elapsedTimeOne) = do_build(
                    $bashScriptPath,
                    abs_path(dirname($rootCMakeLists)),
                    $bCfg,
                    $ftCode,
                    $dryRun,
                    \@fwddCMakeOpts,
                    \@enOpts,
                    \@dsOpts,
                    \@fwddMakeOpts
                );
        push( @elapsedTimes, $elapsedTimeOne );
        print "\033[u";
        if( 0 == $result ) {
            print "\033[0;32msucceed\033[0m\033[K";
        } elsif( 1 == $result ) {
            print "\033[1;31mscript \033[0m\033[K";
        } elsif( 2 == $result ) {
            print "\033[1;31mCMake  \033[0m\033[K";
        } elsif( 3 == $result ) {
            print "\033[1;31mcompil.\033[0m\033[K";
        } elsif( 4 == $result ) {
            print "\033[1;31mtest   \033[0m\033[K";
        } elsif( 9 == $result ) {
            print "\033[0;36mdry-run\033[0m\033[K";
        } else {
            print "\033[1;31munknown\033[0m\033[K";
        }
        print " │ \033[s";
    }
    print "\b\b║\n";
    $nVariant ++;
}

print "\033[1;32mAll done\033[0m.\n"

__END__

=encoding utf8

=head1 NAME

testbuild.pl — a walkthrough test build script for CMake projects from Goo
library.

=head1 SYNOPSIS

    tesbuild.pl [B<OPTIONS>]

e.g.:

    $ /home/crank/Projects/goo/assets/dev/testbuild.pl \
            -L /home/crank/Projects/CERN/P-348/p348g4/CMakeLists.txt \
            -C "-DCMAKE_MODULE_PATH=/home/crank/Projects/goo.install/share/cmake/Modules/" \
            -C "-DCMAKE_LIBRARY_PATH=/home/crank/Projects/CERN/P-348/p348-daq/coral/src/DaqDataDecoding/src/" \
            -M "-j 6" \
            -E '$ftCode == 0x312' \
            --bconf-only=debug

=head1 OPTIONS

=over 4

=item B<--root-file>=I<CMakeLists.txt>

A path to project's main C<CMakeLists.txt> file where tag search routine
will start from.

=item B<--dry>

Makes the script to skip actual building procedures --- only parse cmake files
and print a usual report.

=back

=head1 DESCRIPTION

This script provides automated testing build for cmake-projects which
CMakeLists.txt files supplemented with some sort of commented-out tags.

=head2 Motivation

Considering a CMake project with a set of options and few possible build
configurations (e.g. release/debug/minimal size, etc.) it is needed to
regard the tremondous amount of possible combinations. It is desirable
for developers to test each of such combination before commiting to a
release branch.

Even for few options (e.g. 4) and for few possible build configurations
(e.g. 3) one can easily find that for manual build it will require a lot
(C<2^4 * 3 = 16*3 = 48>) of manual-entered command combinations.

=head2 CMake tags required

The common format of tags is following:

    #\tagname
    cmakekw( ... )

where C<tagname> is one of the pre-defined keywords (C<option>,
C<opt-dep>, C<buildconf>, C<project-subdir>) and C<cmakekw> is a special
real CMake command (or, in case of C<opt-dep>, external CMake routine). Note,
that newline is mandatory since the C<#\tagname> string will be considered
by CMake interpreter as a comment line.

=head4 C<#\option>

Expected keyword is C<option> with appropriate CMake syntax.

=head4 C<#\opt-dep>

Expected keyword is C<option_depend> with appropriate syntax for this goo's
CMake-subroutine. First argument for this subroutine is target (dependent)
option while the rest are its dependencies.

=head4 C<#\buildconf>

Expected keyword is C<set> with C<CMAKE_BUILD_TYPE> as the first argument.
This tag makes the script considers C<set>-instruction as project-wide
actual build configuration (like C<debug> or C<release> or whatever).

=head4 C<#\project-subdir>

Expected keyword is C<add_subdirectory> with single argument provided ---
the name of directory to include (standard CMake instruction for project
located in multiple directories).

=head1 DEPENDENCIES

The C<#\opt-dep> tag considers usage of C<option_depend> CMake module
provided within Goo library distribution.

There is also a C<runbuild.sh> BASH-script located in the same directory
where this script in. This little thing provides actual build routines,
while current script provides management and information collection.

=head1 TODO

There are many interesting things that easily can be provided within
Perl abilities for text transformations.

=over 4

=item *
Running certain util (e.g. system/unit testing facility) after build displaying
its status.

=item *
Suggestions about what particular option causes failures.

=item *
Advanced logs management, including error/warnings collection and more
laconic output for some common cases.

=item *
Should support selective option exclusion.

=item *
Should support selective build exclusion (by feature codes) with ability
to exclude it in range or by mask or whatever.

=back

All this options require scalable structure. I'm currently unable to provide
enaugh efforts for it and not so experienced in Perl for it to be expedient.

=head1 AUTHOR

L<Renat (Crank) R. Dusaev|mailto:crank@qcrypt.org>

=head1 SEE ALSO

L<Goo library|https://github.com/CrankOne/Goo>

