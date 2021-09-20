oo::class create ::flutio::SystemContext {

    superclass ::flutio::AbstractCommandContext

    constructor {} {
        my variable startup_config
        my variable socket_path
        my variable pid_file
        my variable log_file
        global env
        set flutio_home [file join $env(HOME) .flutio]
        set startup_config [file join $flutio_home startup.config]
        set socket_path    [file join $flutio_home flutio.socket]
        set log_file       [file join $flutio_home messages.log]
        set pid_file       [file join $flutio_home flutio.pid]
        set commands {hello world}
    }

    method startup_config {} {my variable startup_config; return $startup_config;}
    method socket_path {} {my variable socket_path; return $socket_path;}
    method log_file {} {my variable log_file; return $log_file;}
    method pid_file {} {my variable pid_file; return $pid_file;}

    method isvalid? {} {
        my variable startup_config
        my variable socket_path
        my variable pid_file
        my variable log_file

        foreach f [list $startup_config $socket_path $pid_file $log_file] {
            set d [file dirname $f]
            if {![file exists $d]} {
                file mkdir [file dirname $d]
            }
            if {![file writable $d]} {
                puts stderr "Sysconfig error: $d is not writeable"
                return 0;
            }
        }
        return 1
    }

    method dump {} {
        my variable startup_config
        my variable socket_path
        my variable pid_file
        my variable log_file

        set systime [clock seconds]
        set data {}
        lappend data "# [clock format $systime -format {%D %T}]"
        lappend data "context /system"
        lappend data "startup-config $startup_config"
        lappend data "socket-path $socket_path"
        lappend data "pid-file $pid_file"
        lappend data "log-file $log_file"
        puts $data
        return [join $data "\n"]
    }

    method interpret {line} {
        puts "lkjlk"
    }
}
