#!/bin/sh
# \
exec tclsh "$0" ${1+"$@"}

lappend auto_path [file dirname [info script]]

package require unixsocket 1.0

proc message_proc {con} {
    if {[catch {chan read $con} msg]} {
        fileevent $con readable {}
        close $con
        return
    }

    puts "got [string length $msg] $msg"

    if {[eof $con]} {
        puts "con closed"
        fileevent $con readable {}
        close $con
    }
}

proc print_thing {} {
    global con
    global i
    chan puts -nonewline $con "helloooooooooooooooooooooo $con [incr i]"
    flush $con
    after 1000 print_thing
}

set i 1
set con [unixsocket::open ./test.socket]
fconfigure $con -blocking 0 -buffering full -translation binary
fileevent $con readable [list message_proc $con]

puts "$con"

after 1000 print_thing
vwait _f_
