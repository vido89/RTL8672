#
# iwidgets.tcl
# ----------------------------------------------------------------------
# Invoked automatically by [incr Tk] upon startup to initialize
# the [incr Widgets] package.
# ----------------------------------------------------------------------
#  AUTHOR: Mark L. Ulferts               EMAIL: mulferts@spd.dsccc.com
#
#  @(#) $Id: iwidgets.tcl,v 1.1 2003/01/21 22:29:36 hunt Exp $
# ----------------------------------------------------------------------
#                Copyright (c) 1995  Mark L. Ulferts
# ======================================================================
# See the file "license.terms" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.

package require Tcl 8.4
package require Tk 8.4
package require Itcl 3.2
package require Itk 3.2

namespace eval ::iwidgets {
    namespace export *

    variable library [file dirname [info script]]
    variable version 4.0.1
}

lappend auto_path $iwidgets::library \
                  [file join $iwidgets::library generic] \
                  [file join $iwidgets::library scripts]

package provide Iwidgets $iwidgets::version

# For now we need to import all of the itcl functions into the global
# namespace. This should be removed once iwidgets are upgraded to use the
# itcl:: names directly.

namespace import -force itcl::*
