#!/usr/bin/env perl

use strict;
use warnings;

package ASTNode {
    sub new {
        my $class = shift;
        my $self = {
            _token => shift,
            _type => shift,
            _parent_node => shift,
            _left_node => shift,
            _right_node => shift,
            _child_nodes => shift,
        };
        bless $self, $class;
        return $self;
    }
}

sub build_ast {
    my (@tokens) = @_;
    my $ast = parse_exp(\@tokens);

    return $ast;
}

1;
