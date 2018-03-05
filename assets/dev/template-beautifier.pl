my @stack;

# usage:
#   $ perl ./template-beautifier.pl < ./output.txt

while( my $line=<> ) {
    format_cxx_template($line);
}

#print "match \"$1\" at char $stack[-1]\n";  # xxx

sub format_cxx_template {
    my ($line) = @_;
    if( $line =~ /([<,>])/ ) {
        #print "match \"$1\" at char $stack[-1]\n";  # xxx
        if( '<' eq $1 ) {
            push @stack, ($-[0]);
            my $head = substr $line, 0, $stack[-1];
            my $tail = substr $line, $stack[-1] + 1;
            print "$head\033[1;37m<\033[0m ";
            format_cxx_template( $tail )
        } elsif ( ',' eq $1 ) {
            my $head = substr $line, 0, $-[0];
            my $tail = substr $line, $-[0] + 1;
            if( $head ) {
                print "$head\n";
                indent_stacked();
            }
            print "\033[1;37m,\033[0m";
            format_cxx_template( $tail )
        } elsif( '>' eq $1 ) {
            my $head = substr $line, 0, $-[0];
            my $tail = substr $line, $-[0] + 1;
            if( $head =~ /^ *$/ ) {
                print "\033[1;37m>\033[0m\n";
            } else {
                print "$head \033[1;37m>\033[0m\n";
            }
            #indent_stacked();
            #print ">\n";
            pop @stack;
            indent_stacked();
            format_cxx_template( $tail )
        } else {
            print "XXX";
        }
    }
}

sub indent_stacked {
    my $i = 0;
    for( @stack ) {
        if( $i ) {
            print "\033[1;30m.\033[0m" x $_;
            print "\033[1;30m:\033[0m";
        } else {
            print "\033[1;30m.\033[0m" x ($_ - 1);
            print "\033[1;30m:\033[0m";
        }
        $i ++;
    }
}

