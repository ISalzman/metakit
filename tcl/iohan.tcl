# Copyright (C) 1999-2001 Jean-Claude Wippler <jcw@equi4.com>
#
# Object commands to access local files, FTP, MetaKit, and many more

package provide iohan 0.2

# This package attempts to provide a certain level of transport layer
# independence.  The model used is a "directory" with "files", though
# some of the handlers below show that this model can be stretched at
# will.  The code uses an object interface, so that only set up has a
# relation to the transport mechanism.  All handlers respond to the
# same protocol, which lets you enumerate entries, fetch/store/delete
# them, and there's a "destroy" pseudo-method to clean up the handler.
# 
# The defining calls are:
# 
#   set h [iohan::local directory]
#       # creates a handler for local files
#
#   set h [iohan::ftp url ?userid? ?password?]
#       # creates a handler for files on FTP server (needs FTP pkg)
#
#   set h [iohan::mk view]          # assumes datafile is open
#       # creates a handler for entries stored in a MetaKit datafile
#   
#   set h [iohan::tequila array]    # assumes Tequila has been set up
#       # creates a handler for entries stored on a Tequila server
#   
#   set h [iohan::snoopy rh ?spy?]  # can init a snoopy client session
#       # creates a handler which redirects to a remote "snoopy" setup
#   
#   set h [iohan::procs ?namespace?]
#       # creates a handler for direct editing of procedures
#       
#   set h [iohan::vars ?namespace?]
#       # creates a handler for direct editing of variables and arrays
#
#   set h [iohan::backup h1 h2]
#       # creates a handler redirecting to h1, all mods also sent to h2
#
#   set h [iohan::filter h ?cmd?]
#       # creates a handler based on h, all fetches filtered through cmd
#
# All handlers respond to the following commands:
# 
#   array set filelist [$h list ?wildcard?]
#       # sets up an array with all files, value is modification time
#        
#   set data [$h fetch filename]
#       # fetches the contents of a file, throws error if not present
#       
#   set timestamp [$h store filename contents]
#       # stores contents into a file, returns new modification time
#       
#   set timestamp [$h append filename contents]
#       # appends contents to a file, creating it if necessary
#       
#   set ok [$h delete filename]
#       # delete file, return 1 if it existed and was removed
#       
#   $h destroy
#       # deletes the handler command object
#
# TODO:
#   - need a way to deal with "subdirs" as well as "files"

package provide iohan 0.2

namespace eval iohan {
    variable seq 0
    
    # local

    proc local {dir} {
        variable seq
        set cmd [namespace current]::#[incr seq]
        proc $cmd {args} [format {
            eval [list local: %s] $args
        } [list $cmd $dir]]
        return $cmd
    }

    proc local: {cmd dir action args} {
        eval [list local:$action $dir] $args
    }

    proc local:destroy {dir} {
        uplevel {rename $cmd ""}
    }

    proc local:list {dir {match *}} {
        set result {}
        
        set wd [pwd]
        catch {
            cd $dir
            foreach f [lsort [glob -nocomplain $match]] {
                if [file isfile $f] {
                    lappend result $f [file mtime $f]
                }
            }
        }
        cd $wd
        
        return $result
    }

    proc local:fetch {dir file} {
        set path [file join $dir $file]
        
        set fd [open $path]
        fconfigure $fd -translation binary
        set result [read $fd [file size $path]]
        close $fd
        
        return $result
    }

    proc local:store {dir file contents} {
        set path [file join $dir $file]
        
        set fd [open $path w]
        fconfigure $fd -translation binary
        puts -nonewline $fd $contents
        close $fd
        
        return [file mtime $path]
    }

    proc local:append {dir file contents} {
        set path [file join $dir $file]
        
        set fd [open $path a+]
        fconfigure $fd -translation binary
        puts -nonewline $fd $contents
        close $fd
        
        return [file mtime $path]
    }

    proc local:delete {dir file} {
        set path [file join $dir $file]
        
        set ok [file isfile $path]
        file delete $path
        return $ok
    }

    # ftp

        # utility call to set up the proper session context
    proc ftp_session {{sess ""}} {
        upvar #0 ftp_state cb

        update idletasks
        
        if {[info exists cb(curr)]} {
            if {$cb(curr) == $sess} return
            
            tclLog [concat ftp:close $cb(curr)]
            FTP::Close
            unset cb(curr)
        }
        
        if {$sess == ""} return
        
        catch {parray FTP::ftp}
        
        if {![eval FTP::Open $sess]} {
            after 1000 ;# one retry
            if {![eval FTP::Open $sess]} {
                error "cannot connect to [lindex $sess 0]"
            }
        }
        
        tclLog [concat ftp:open $sess]
        set cb(curr) $sess
        
        FTP::Type binary
        
            # Close sessions after 2 minutes to avoid a bug in the FTP
            # package (reporting an error after 10 minutes if last failed).
            # This is harmless because open sessions act merely as cache.
            # The session will be re-opened as soon as it is (re-)used.
        after cancel [namespace current]::ftp_session
        after 120000 [namespace current]::ftp_session
    }

    proc ftp {url {user anonymous} {pw some@where.com}} {
        variable seq

        package require FTP
        set FTP::VERBOSE 0

        proc FTP::DisplayMsg {msg {state ""}} {
            switch $state {
              error     {tclLog [list ftp-error $msg]}
            }
        }
            # the following two lines are to undo all tkcon special-casing
        catch {alias ::FTP::List ""}
        catch {rename ::FTP::List_org ::FTP::List}
            
        set site $url
        set dir .
        regexp {^([^/]+)/?(.*)$} $url - site dir
        set sess [list $site $user $pw]
        
        ftp_session $sess
        
        set cmd [namespace current]::#[incr seq]
        proc $cmd {args} [format {
            eval [list ftp: %s] $args
        } [list $cmd $sess $dir]]
        return $cmd
    }

    proc ftp: {cmd sess dir action args} {
        ftp_session $sess
        if {$dir == ""} {set dir .}
        eval [list ftp:$action $dir] $args
    }

    proc ftp:destroy {dir} {
        ftp_session
        uplevel {rename $cmd ""}
    }

    proc ftp:list {dir {match *}} {
        set result {}
        
        foreach line [::FTP::List $dir] {
            if {![string match -* $line]} continue
            set rc [scan $line "%s %s %s %s %s %s %s %s %s %s %s" \
                                perm l u g size d1 d2 d3 name link linksource]
            if {![string match $match $name]} continue
            lappend result $name [FTP::ModTime $dir/$name]
        }

        return $result
    }

    proc ftp:fetch {dir file} {
        set tmp xyzF
        
        set ok [FTP::Get $dir/$file $tmp]
        
        set fd [open $tmp]
        fconfigure $fd -translation binary
        set result [read $fd [file size $tmp]]
        close $fd
        
        file delete $tmp
        if {!$ok} {error "can't fetch $file"}
            
        return $result
    }

    proc ftp:store {dir file contents} {
        set tmp "tcl[clock seconds].tmp"
        
        set fd [open $tmp w]
        fconfigure $fd -translation binary
        puts -nonewline $fd $contents
        close $fd
        
        set ok [FTP::Put $tmp $dir/$file]
        
        file delete $tmp
        if {!$ok} {error "can't store $file"}
        
        return [FTP::ModTime $dir/$file]
    }

    proc ftp:append {dir file contents} {
        set tmp xyzF
        
        set fd [open $tmp w]
        fconfigure $fd -translation binary
        puts -nonewline $fd $contents
        close $fd
        
        set ok [FTP::Append $tmp $dir/$file]
        
        file delete $tmp
        if {!$ok} {error "can't fetch $file"}
        
        return [FTP::ModTime $dir/$file]
    }

    proc ftp:delete {dir file} {
        FTP::Delete $dir/$file
    }

    # mk

    proc mk {dir} {
        variable seq

        mk::view layout $dir "name date:I contents:B"
        set cmd [namespace current]::#[incr seq]
        proc $cmd {args} [format {
            eval [list mk: %s] $args
        } [list $cmd $dir]]
        return $cmd
    }

    proc mk_dirty {dir} {
        regsub {\..*} $dir {} dir
        after cancel mk::file commit $dir
        after 10000 mk::file commit $dir
    }

    proc mk: {cmd dir action args} {
        eval [list mk:$action $dir] $args
    }

    proc mk:destroy {dir} {
        uplevel {rename $cmd ""}
    }

    proc mk:list {dir {match *}} {
        set result {}
        foreach n [mk::select $dir -glob name $match] {
            eval lappend result [mk::get $dir!$n name date]
        }
        return $result
    }

    proc mk:fetch {dir file} {
        foreach n [mk::select $dir -count 1 name $file] {
            return [mk::get $dir!$n contents]
        }
        error "cannot find MK entry '$file'"
    }

    proc mk:store {dir file contents} {
        mk_dirty $dir
        set d [clock seconds]
        foreach n [mk::select $dir -count 1 name $file] {
            mk::set $dir!$n name $file date $d contents $contents
            return $d
        }
        mk::row append $dir name $file date $d contents $contents
        return $d
    }

    proc mk:append {dir file contents} {
            # could use the new mk::channel of Mk4tcl 1.2
        if {[catch {mk:fetch $dir $file} old]} {
            set old ""
        }
        return [mk:store $dir $file $old$contents]
    }

    proc mk:delete {dir file} {
        foreach n [mk::select $dir -count 1 name $file] {
            mk_dirty $dir
            mk::row delete $dir!$n
            return 1
        }
        return 0
    }

    # tequila

    proc tequila {aname {type X}} {
        variable seq

        tequila::do Define $aname 0 $type
        set cmd [namespace current]::#[incr seq]
        proc $cmd {args} [format {
            eval [list tequila: %s] $args
        } [list $cmd $aname]]
        return $cmd
    }

    proc tequila: {cmd aname action args} {
        eval [list tequila:$action $aname] $args
    }

    proc tequila:destroy {aname} {
        uplevel {rename $cmd ""}
    }

    proc tequila:list {aname {match *}} {
        set result [tequila::do Listing $aname]
        if {$match != "*"} {
            array set a $result
            set result [array get a $match]
        }
        return $result
    }

    proc tequila:fetch {aname file} {
        tequila::do Get $aname $file
    }

    proc tequila:store {aname file contents} {
        set d [clock seconds]
        tequila::do Set $aname $file $contents $d
        return $d
    }

    proc tequila:append {aname file contents} {
        tequila::do Append $aname $file $contents
        return [clock seconds]
    }

    proc tequila:delete {aname file} {
        if {[catch {tequila::do Get $aname $file}]} {
            return 0
        }
        tequila::do Unset $aname $file
        return 1
    }

    # snoopy

    proc snoopy {remh {spy ""}} {
        variable seq

        if {$spy == ""} {
            package require snoopy
            set spy [snoopy::client]
        }
        set cmd [namespace current]::#[incr seq]
        proc $cmd {args} [format {
            eval [list snoopy: %s] $args
        } [list $cmd $spy $remh]]
        return $cmd
    }

    proc snoopy: {cmd spy remh action args} {
        if {$action == "destroy"} {
            rename $cmd ""
        }
        eval [list $spy $remh $action] $args ;# this behaves like a proxy
    }

    # procs

    proc procs {{ns ::}} {
        variable seq

        namespace eval $ns {}
        set cmd [namespace current]::#[incr seq]
        proc $cmd {args} [format {
            eval [list procs: %s] $args
        } [list $cmd $ns]]
        return $cmd
    }

    proc procs: {cmd ns action args} {
        eval [list procs:$action $ns] $args
    }

    proc procs:destroy {ns} {
        uplevel {rename $cmd ""}
    }

    proc procs:list {ns {match *}} {
        set now [clock seconds]
        set result {}
        foreach x [info procs ${ns}::$match] {
            regsub {.*::} $x {} x
            lappend result $x $now
        }
        return $result
    }

    proc procs:fetch {ns file} {
        set name ${ns}::$file
        set argv {}
        foreach x [info args $name] {
            if {[info default $name $x y]} {
                set x [list $x $y]
            }
            lappend argv $x
        }
        return [list proc $file $argv [info body $name]]
    }

    proc procs:store {ns file contents} {
        if {[llength $contents] != 4 || ![info complete $contents] ||
                [lindex $contents 0] != "proc" ||
                [lindex $contents 1] != $file} {
            error "proc ${ns}::$file cannot be saved (bad format?)"
        }
        namespace eval $ns $contents
        #uplevel proc ${ns}::$file [lindex $contents 2] [lindex $contents 3]
        return [clock seconds]
    }

    proc procs:append {ns file contents} {
        error "append to proc ${ns}::$file not supported"
        #return [clock seconds]
    }

    proc procs:delete {ns file} {
        if {[catch {rename ${ns}::$file}]} {
            return 0
        }
        return 1
    }

    # vars

        # this resembles parray, but it returns the results as a string
    proc vars_sarray {a {pattern *}} {
        upvar 1 $a array
        if {![array exists array]} {
            error "\"$a\" isn't an array"
        }
        set maxl 0
        foreach name [lsort [array names array $pattern]] {
            if {[string length $name] > $maxl} {
                set maxl [string length $name]
            }
        }
        set result {}
        foreach name [lsort [array names array $pattern]] {
            set v $array($name)
            if {[string first \n $v] >= 0} {set v [list $v]}
            lappend result [format "%-*s = %s" $maxl $name $v]
        }
        return [join $result \n]
    }

    proc vars {{ns ::}} {
        variable seq

        namespace eval $ns {}
        set cmd [namespace current]::#[incr seq]
        proc $cmd {args} [format {
            eval [list vars: %s] $args
        } [list $cmd $ns]]
        return $cmd
    }

    proc vars: {cmd ns action args} {
        eval [list vars:$action $ns] $args
    }

    proc vars:destroy {ns} {
        uplevel {rename $cmd ""}
    }

    proc vars:list {ns {match *}} {
        set now [clock seconds]
        set result [list _ $now]
        foreach x [info vars ${ns}::$match] {
            if {![array exists $x]} continue
            regsub {.*::} $x {} x
            lappend result $x $now
        }
        return $result
    }

    proc vars:fetch {ns file} {
        set name ${ns}::$file
        if {$file == "_"} {
            array set _ {}
            foreach x [info vars ${ns}::*] {
                if {[array exists $x]} continue
                if {$x == "::errorInfo"} continue
                regsub {.*::} $x {} v
                set _($v) [set $x]
            }
            set name _
        }
        return "# $file\n[vars_sarray $name]"
    }

    proc vars:store {ns file contents} {
        set d [clock seconds]
        foreach line [split $contents \n] {
            if {[string match #* $line]} continue
            if {![regexp {^([^=]+) = ?(.*)} $line - name value]} {
                error "failed to parse line: $line"
            }
            set name [string trimright $name]
            if {$file != "_"} {
                set name ${file}($name)
            }
            namespace eval $ns [list set $name $value]
        }
        return $d
    }

    proc vars:append {ns file contents} {
        error "append to var ${ns}::$file not supported"
        #return [clock seconds]
    }

    proc vars:delete {ns file} {
        if {[catch {unset ${ns}::$file}]} {
            return 0
        }
        return 1
    }

    # backup

    proc backup {mh bh} {
        variable seq

        set cmd [namespace current]::#[incr seq]
        proc $cmd {args} [format {
            eval [list backup: %s] $args
        } [list $cmd $mh $bh]]
        return $cmd
    }

    proc backup: {cmd mh bh action args} {
        eval [list backup:$action $mh $bh] $args
    }

    proc backup:destroy {mh bh} {
        uplevel {rename $cmd ""}
    }

    proc backup:list {mh bh {match *}} {
        return [$mh list $match]
    }

    proc backup:fetch {mh bh file} {
        return [$mh fetch $file]
    }

    proc backup:store {mh bh file contents} {
        $bh store $file $contents
        return [$mh store $file $contents]
    }

    proc backup:append {mh bh file contents} {
        $bh append $file $contents
        return [$mh append $file $contents]
    }

    proc backup:delete {mh bh file} {
        $bh delete $file
        return [$mh delete $file]
    }

    # filter

	proc filter_eval {text} {
		set separator "\n#-#-#-#-#-#-#-#-#-#-#-#-#-#-#-"
		regsub "$separator.*" $text {} text
		if {[catch {uplevel #0 $text} result]} {
			set result "ERROR: $::errorInfo"
		}
		if {$result != ""} {append text $separator \n $result}
		return $text
	}
	
    proc filter {h {filter filter_eval}} {
        variable seq

        set cmd [namespace current]::#[incr seq]
        proc $cmd {args} [format {
            eval [list filter: %s] $args
        } [list $cmd $h $filter]]
        return $cmd
    }

    proc filter: {cmd h filter action args} {
        eval [list filter:$action $h $filter] $args
    }

    proc filter:destroy {h filter} {
        uplevel {rename $cmd ""}
    }

    proc filter:list {h filter {match *}} {
        return [$h list $match]
    }

    proc filter:fetch {h filter file} {
        return [$filter [$h fetch $file]]
    }

    proc filter:store {h filter file contents} {
        return [$h store $file $contents]
    }

    proc filter:append {h filter file contents} {
        return [$h append $file $contents]
    }

    proc filter:delete {h filter file} {
        return [$h delete $file]
    }
}
