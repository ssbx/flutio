package require TclOO
package provide flutiolib 1.0

oo::class create ::flutio::AbstractCommandContext {
    constructor {} {
        if {![lindex [self call] 1]} {
            return -code error \
                "class '[info object class [self]]' is abstract"
        }
    }

    method dump {} {
        return -code error \
            "method '[info object class [self]]' dump is abstract"

    }

    method interpret {l} {
        return -code error \
            "method '[info object class [self]]' interpret is abstract"
    }
}
