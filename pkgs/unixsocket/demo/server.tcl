#!/bin/sh
# \
exec tclsh "$0" ${1+"$@"}

lappend auto_path [file dirname [file dirname [info script]]]

package require unixsocket 1.0

proc echo {con} {

    if {[catch {chan read $con} msg]} {
        fileevent $con readable {}
        close $con
        return
    }

    if {[string length $msg] >= 0} {
        puts "got message: $msg from $con"
        chan puts $con "$msg"
        flush $con
    }

    if {[eof $con]} {
        puts "con closed"
        fileevent $con readable {}
        close $con
    }
}

proc accept_proc {con} {
    puts "accept_proc $con"
    fconfigure $con -blocking 0 -buffering full -translation binary
    fileevent $con readable [list echo $con]
}


unixsocket::listen ./test.socket accept_proc

vwait _f_
