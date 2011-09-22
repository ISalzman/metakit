# preamble, included by all MetaKit tests

source defs.tcl

if {[info commands mk::file] == ""} {
  set x [info sharedlibextension]
    # assume we're in tcl/test/, try debug version first if it exists
    # normally, builds happen in builds/ but check unix/ just in case
  foreach d {builds/Mk4tcl_d unix/Mk4tcl_d builds/Mk4tcl unix/Mk4tcl} {
    if {![catch {load ../../$d$x} Mk4tcl]} break
  }
  unset d x
}

package require Mk4tcl

S { 
  # do this before each test
} {
  # do this after each test
  foreach {db path} [mk::file open] {
    mk::file close $db
  }
}
