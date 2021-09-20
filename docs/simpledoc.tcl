#=Simple Doc
#TCL documentation generator
#See end of file for copyright notice and license terms.
#Convert Tcl script with structured text style markup to HTML.

#to test: cd to the directory containing this script and try:

#tclsh simpledoc.tcl simpledoc.tcl

#This will generate a file called simpledoc.tcl.html in the same directory. Opening this in a browser should show this text nicely fomatted.

#==Usage
#Command line arg of file or directory to process. If it is a directory it can be followed by an optional arg specifying the output directory (by relative path, html by default), and then optional args list extensions to process (.tcl by default).

#If a doc.css file is found in the top level of a directory being processed it is copied to the destination directory and linked to from each file, otherwise the default style is included inline in each file.

#===Warning
#Any file of that looks like it was previously generated by this script, with the form *.ext.html will be deleted. This usually means that any file in the destination directory that looks like *.tcl.html

#===Examples

#tclsh simpledoc.tcl myfile.tcl

#will generate myfile.tcl.html in the same directory

#tclsh simpledoc.tcl mydir

#will generate html versions of all .tcl files in mydir/html

#tclsh simpledoc.tcl mydir .

#will generate html versions of all .tcl files in mydir into mydir itself

#tclsh simpledoc.tcl mydir html tcl txt

# will generate html versions of all .tcl and .txt files in mydir in myder/html

#==Markup

#===Description
#Markup understood within comment lines. All lines start with a "#", followed by the following - e.g. a line starting with an asterisk means a like stating with a "#", followed by an asterisk. See unprocessed version of this file for examples.
#* A line starting with an asterisk is a list item
#* A line starting with "1." starts a numbered list. Subsequent list items should be asterisks
#* A vertical bar "|" starts a table cell
#* A line starting with an equals sign is a heading. The number of equals signs is the level of the heading so "=" means h1, == means h2, etc.

#===Examples

# #Just an ordinary paragraph of text. This should all be on one line, so use a text editor that can wrap

# #A blank line starts a new para,
# #but a new line just a break

#becomes:

#Just an ordinary paragraph of text. This should all be on one line, so use a text editor that can wrap. Of course a browser will wrap the html version in any case. Lines of code may not wrap if you use your own style sheet.

#A blank line starts a new para,
#but a new line just a break

# #=====Heading 5
#becomes:

#=====Heading 5

# #1. One
# #* Two
# #* Three
#becomes:

#1. One
#* Two
#* Three

# #* One
# #* Two
# #* Three
#becomes:

#* One
#* Two
#* Three

# #|col1|col2|col3|
# #|is|a|table|

#|col1|col2|col3|
#|is|a|table|

#==Bugs
#===tcl2html
#* Extraneous comment markup - see style in initialisation section for example
#* Highlighting as proc names within comments - see comments in do_file for example

#==To do

#* Delete unnneded subdirectories of destination - recurse and delete empty dirs?
#* Link to procs from wherever they are used. It may cease to be a simple script if this is done!

#==Initialisation

package require fileutil

set htmldir html

set header {<html><head><title>$page_title</title>$style</head>}
#default style is over-ridden if doc.css exists
set style {<style type="text/css">
body {color: #fff; background-color: #000148 }
pre {color: #0f0; background-color: #000; padding: 1em; white-space: pre-wrap}
a {color: #09f}
table
{border-width: 1px; border-style: solid; border-spacing: 0; border-collapse: collapse;}
td {border: 1px solid; padding: 0.25em}

.tcl-comment {color: #999}
.tcl-proc {color: #9f9; font-weight: bold}
.tcl-str {color: #f77}
.tcl-var {color: #0ff}
</style>
<body>}
set style_type inline
set footer {</body>}

#=Procedure definitions

#==do_file
# Handles a single file.

#It separates the file, line by line into code and comment chunks and passes them to he appropriate functions to markup.

proc do_file {infile outfile} {
	global header
	global footer
	global style
	global style_type
	if {$style_type eq {file}} {
		global style_file
		set style "<link rel=\"stylesheet\" href=\"[::fileutil::relative $infile $style_file]\" type=\"text/css\">"
	}
	set anchor 0 ;#used to generate unique anchors
	set index {}
	set page_title {}
	set fd [open $infile r]
	set mode comment
	set out {};# variable that holds chunks of intermediate output
	set html {};# variable that holds final output
	#iterate over lines in file, adding to $out until it swtiches from comments to text or vice-versa, at which point call appropriate proc to process it.

	#Processed results are appended to $html
	while {[gets $fd line] >= 0} {
		set tline [string trim $line]
		#blank lines can be comment or code
		if {$tline eq {}} {
			append out \n
			continue
		}
		if {[string index $tline 0] eq {#}} {
			set new comment
			set line [string trimleft $tline #]
		} else {
			set new code
		}
		#if more of the same then just keep appending to $out to process later
		if {$mode eq $new} {
			append out \n$line
			continue
		}
		#if we get to this point we need to process what we have so far and switch modes
		if {$mode eq {comment}} {
			append html [text2html $out]
		} else {
			append html [tcl2html [regsub -all -- {\t} $out {    }]]
		}
		set mode $new
		set out $line
	}
	#repeated code - not nice but easy - to close last chunk of code or comment
	if {$mode eq {comment}} {
		append html [text2html $out]
	} else {
		append html [tcl2html [regsub -all -- {\t} $out {    }]]
	}
	if {![file exists [file dirname $outfile]]} {file mkdir [file dirname $outfile]}
	set fd [open $outfile w+]
	puts $fd [subst $header]
	if {$index ne {}} {
		puts $fd $index
		puts $fd "<hr />"
	}
	puts $fd $html
	puts $fd [subst $footer]
	close $fd
}

#==do_directory

proc do_directory {dir ext} {
	global htmldir
	global style
	global style_file
	global style_type
	set dir [file normalize $dir]
	#Path to destination dir, and create it if it does not exist
	set destdir [file normalize [file join $dir $htmldir]]
	if {![file exists $destdir]} {file mkdir $destdir}
	#If css file exists, copy to destination directory, unless it already exists there.
	if {[file exists [file join $dir doc.css]]} {
		set style {}
		set style_file [file join $destdir doc.css]
		set style_type file
		if {![file exists [file join $destdir doc.css]] && ($dir ne $destdir)} {file copy [file join $dir doc.css] [file join $destdir doc.css]}
	}
	#iterate over list of extenions
	foreach e $ext {
		#Delete existing files that look like htmlized source.
		foreach file [::fileutil::findByPattern $destdir -glob -- *.$e.html] {
			puts "deleting: $file"
			file delete $file
		}
		#generate html version of each file with that extension.
		foreach file [::fileutil::findByPattern $dir -glob -- *.$e] {
			set f [::fileutil::stripPath $dir [file normalize $file]]
			puts "generating: $file"
			if {[lindex [file split $f] 0] eq $htmldir} {continue}
			do_file $file [file join $destdir ${f}.html]
		}
	}
}

#==text2html
#Convert list of lines of structured text to HTML,
#Arguments, text to be converted, current mode of conversion, which should be provided by the last return value.

proc text2html {x} {
	upvar anchor anchor
	upvar index index
	upvar page_title page_title
	set out {}
	set mode para
	set x [string map {& &amp; < &lt; > &gt;} [string trim $x]]
	set x [split $x \n]
	lappend x {}
	foreach i $x {
		set i [string trim $i]
		if {($mode == {list}) || ($mode == {enum})} {
			if {[string index $i 0] == {*}} {
				append out <li>[string trim [string range $i 1 end]]</li>
				continue
			}
			if {$mode == {list}} {
				append out </ul><p>$i</p>
				set mode para
				continue
			} {
				append out </ol><p>$i</p>
				set mode para
				continue
			}
		}
		if {[string index $i 0] == {|}} {
			if {$mode != {table}} {
				set mode table
				append out <table>
			}
			append out <tr><td>[string map {| </td><td>} [string trim $i |]]</td></tr>
			continue
		}
		if {$mode == {table}} {
		append out </table>
		set mode para
		}
		if {[string index $i 0] == {*}} {
			append out <ul><li>[string trim [string range $i 1 end]]</li>
			set mode list
			continue
		}
		if {[string range $i 0 1] == {1.}} {
			append out <ol><li>[string trim [string range $i 2 end]]</li>
			set mode enum
			continue
		}
		# Doing headings needs to both pick up "=" at begining and allow a lowest heading of h5
		if {[string index $i 0] == {=}} {
			set trimeq [string trimleft $i =]
			set level [expr [string length $i] - [string length $trimeq]]
			set mlevel [expr min($level,5)]
			set title_text [string range $i $level end]
			if {$page_title == {} && $mlevel == 1} {set page_title $title_text}
			append out "<h$mlevel><a name=\"[incr anchor]\">$title_text</a></h$mlevel>"
			append index "<a href=\"#$anchor\">$title_text</a><br>"
			set mode para
			continue
		}
		if {$mode == {break}} {
			if {$i == {}} {
			append out </p>
			set mode para
			} {append out <br>$i}
			continue
		}
		if {$i != {}} {
			append out <p>$i
			set mode break
		}
	}
	return $out
}

#Adapted from Jeff Hobb's tcl2html
#Need to check license, but appears to be GPL compatible as derived code is included in OpenACS.
#==tcl2html

set COMMANDS [concat \
	[info commands] \
	[array names auto_index] \
	{break continue default elseif else} \
	]

array set HTML {
    comment	{<span class="tcl-comment">}
    /comment	</span>
    proc	{<span class="tcl-proc">}
    /proc	</span>
    str		{<span class="tcl-str">}
    /str	</span>
    var		{<span class="tcl-var">}
    /var	</span>
}

proc tcl2html {data} {
    global HTML COMMANDS

    set ws "\t\n "; #whitespace

	set data [string map {& &amp; < &lt; > &gt;} $data]

    lappend html <pre>

    ## Here we handle the framing of commands
    set cmds (^|\[\[\{\"\;$ws\])([join $COMMANDS |])(\[^a-z0-9\]|$)
    regsub -all $cmds $data \\1$HTML(proc)\\2$HTML(/proc)\\3 data

    ## Here we handle the framing of variables
    ## This regexp handles simple vars and {}, but skips any array () index.
    #set var  {(^|[^\\])(\$\{[^\}]*\}|\$[a-z0-9_]+)}
    ## This one tries to handle all vars, but not perfect if you use
    ## funny characters - hey, why are you making your code unreadable!
    set var  {(^|[^\\])(\$\{[^\}]*\}|\$[a-z0-9_]+(\([^\)]*\))?)}

    regsub -nocase -all $var $data \\1$HTML(var)\\2$HTML(/var) data

    set cont	0
    set inComment 0
    # To check for line beginning with #
    set cmnt	"^(\[$ws\;\]*\[^\\\\#\]?)(\#)"
    # To check for line beginning with # after a ;
    set cmnt2	"(\[$ws\;\]+\[^\\\\#\]?)(\#)"
    # To check for line ending in '\', but not '\\'
    set bs	"(^|\[^\\\\\])\\\\\[$ws\]*\$"
    foreach line [split $data \n] {
	if {![string length [string trim $line]]} {
	    set line {}
	} elseif {($cont && $inComment) || [regexp $cmnt $line]} {
	    if !$inComment {
		regsub $cmnt $line "\\1$HTML(comment)\\2" line
		set inComment 1
	    }
	} else {
	    if $inComment {
		lappend html $HTML(/comment)
		set inComment 0
	    }
	    if {[regsub $cmnt2 $line "\\1$HTML(comment)\\2" line]} {
		set inComment 1
	    }
	}
	lappend html $line
	##### Does this line continue?
	set cont [regexp $bs $line]
    }
    if $inComment { lappend html $HTML(/comment) }

    #lappend html </pre>;#removed because it generated an extra blank line at the end.

    return [join $html \n]</pre>
}

# All procs now defined, call procs to process file or directory specified in command line args

set infile [lindex $argv 0]
if [file isdirectory $infile] {
	#If we have been given a destination directory, override the default, then if we have extensions specified override that default.
	if {[llength $argv] > 1} {set htmldir [lindex $argv 1]}
	set ext [lrange $argv 2 end]
	if {$ext eq {}} {set ext tcl}
	do_directory $infile $ext
} elseif [file isfile $infile] {
	#if it is not a directory, check if it is just a single file
	do_file $infile $infile.html
} else {
	error "$infile is neither a regular file nor a directory"
}

#==Copyright
#Copyright (c) 2008, 2017 Graeme Pietersz. Licensed under the GPL version 2 or later. Incorporates BSD licensed code by Jeff Hobbs.
#This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 2 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program.  If not, see http://www.gnu.org/licenses/