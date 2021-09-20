oo::class create ::flutio::SystemContext {

    superclass ::flutio::AbstractCommandContext

    constructor {} {
        my variable startup_config
        my variable socket_path
        my variable pid_file
        my variable log_file
        global env
        set uhome [file join $env(HOME) .flutio]
        set startup_config [file join $uhome startup.config]
        set socket_path    [file join $uhome flutio.socket]
        set log_file       [file join $uhome messages.log]
        set pid_file       [file join $uhome flutio.pid]
        set commands {hello world}
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

    method tabtab {str} {
        return ""
    }

    method interpret {line} {
        return "ok"
    }

    method set_startup_config {v} {my variable startup_config; set startup_config $v;}
    method set_socket_path {v} {my variable socket_path; set socket_path $v;}
    method set_log_file {v} {my variable log_file; set log_file $v;}
    method set_pid_file {v} {my variable pid_file; set pid_file $v;}

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
}
