#!/usr/bin/perl
#
# (this is a unix script that calls evolve_batch)
#
# 5/3/2006
#
# Run evolve simulator and updload results to a ftp directory.
#
# This is the script used to run the "One Year Of Alife" project.
#
# To use,
#
# STEP 1: Configure these variables:
# ==================================
#
# $local_dir	= Actual path to root of your project directory (where the sim files will be read/written)
# $duration	= How long to run between uploads (24h means 24 hours)
# $progress_html= The name of the progress file to produce
# 
# $ftp_host	= where to upload 'progress.html'
# $ftp_user	= user name name to use
# $ftp_pass	= password to use
# $ftp_directory= directory to place 'prograss.html'
#
# STEP 2: Create Day Zero sim file
# ================================
# Create your first simulation file and rename it to 'day-0000.evolve'.
# Place this file into $local_dir
#
# STEP 3: Copy evolve_batch
# =========================
# Make sure a copy of the 'evolve_batch' executable is stored in
# the $local_dir directory.
#
# STEP 4: Run the script
# ======================
# Run this script as follows: ./alife_deamon.sh
#
# You're done.
#
# WHAT THIS SCRIPT DOES:
# =====================
#
#	1. Finds the last day-xxxx.evolve file (the file with the largest xxxx number)
#
#	2. Zip this file into day-xxxx.zip
#
#	3. Rebuilds progress.html by calling 'evolve_batch p filename.evolve'
#	   for every day-XXXX.evolve file. Build a basic HTML document.
#
#	4. Upload day-xxxx.zip and progress.html to the ftp site.
#
#	5. Run evolve_batch for 24 hours (or whatever time period was specified)
#
#		"evolve_batch s 24h day-0099.evolve day-0100.evolve"
#
#	6. Goto step 1.
#
# If you need to quit the program use ctrl-C to abort the program.
#
# When you restart the program it will start simulating using the file with
# the largest 'xxxx' (day-xxxx.evolve) number.
#
#

use Net::FTP;
use Time::localtime;
use File::stat;

$local_dir	= "/home/kjs/Development/evolve/alife";
$duration	= "24h";
$progress_html	= "progress.html";

$ftp_host	= "myhost.com";
$ftp_user	= "myusername";
$ftp_pass	= "mypassword";
$ftp_directory	= "/my/alife/directory";

$html_title	= "One Year of ALIFE Progress";

######################################################################
# Insert commas into argument
#
sub commify
{
    local($_) = shift;
    1 while s/^(-?\d+)(\d{3})/$1,$2/;
    return $_;
}

#######################################################################
#
# log routine
#
sub LOG
{
	local($message) = @_;

	$now=`date`;
	chop $now;
	print "$now $message\n";
}

#######################################################################
#
# Error routine
#
sub ERROR
{
	local($error_message) = @_;

	$now=`date`;
	chop $now;
	print "$now ERROR: $error_message\n";
}

#######################################################################
#
# This routine ftp's the shit
#
#
sub ftp_shit
{
	local($zip_file, $html_file) = @_;

	&LOG("begin ftp connection to ${ftp_user}@${ftp_host}");

	$ftp = Net::FTP->new($ftp_host, Debug => 0);
	if( ! $ftp ) {
		&ERROR("Cannot connect to $ftp_host: $@");
		return;
	}

	$result = $ftp->login($ftp_user, $ftp_pass);
	if( ! $result ) {
		&ERROR("Cannot login to $ftp_host as $ftp_user: $@");
		return;
	}


	$result = $ftp->cwd($ftp_directory);
	if( ! $result ) {
		&ERROR("Cannot change dir to $ftp_directory: $@");
		return;
	}

	$ftp->binary;
	if( ! $result ) {
		&ERROR("Cannot change mode to binary: $@");
		return;
	}

	$ftp->put($zip_file);
	if( ! $result ) {
		&ERROR("Cannot put file $zip_file: $@");
		return;
	}

	$ftp->ascii;
	if( ! $result ) {
		&ERROR("Cannot change mode to ascii: $@");
		return;
	}

	$ftp->put($html_file);
	if( ! $result ) {
		&ERROR("Cannot put file $html_file: $@");
		return;
	}

	$ftp->quit;

	&LOG("end ftp connection.");
}

#######################################################################
#
# Returns a list of all the simulation files
#
#
sub get_sim_files {
	local($result);
	local(@simfiles);

	@simfiles = ();

	$result = opendir(DIR, $local_dir);
	if( ! result ) {
		&ERROR("Cannot open directory $local_dir");
		return @simfiles;
	}

	@simfiles = grep(/day-[0-9][0-9][0-9][0-9]\.evolve/, readdir(DIR));
	@simfiles = sort(@simfiles);

	closedir(DIR);

	return @simfiles;
}

######################################################################
#
# Examine the simulation file and return a structure containing
# the parameters from the file.
#
sub simfile_info
{
	local($simfile) = @_;
	local($result, $cmd, $line, $parm, $value);
	local(%info);

	$cmd = "${local_dir}/evolve_batch p ${local_dir}/$simfile";

	$info{'created'} = ctime(stat("${local_dir}/$simfile")->mtime);

	$result = open(CMD, "$cmd|");
	if( ! $result ) {
		&ERROR("cannot run '$cmd'");
	}

	$info{'day'} = (split(/[.-]/, $simfile))[1] + 0;

	$info{'basename'} = $simfile;

	while( $line = <CMD>) {
		($parm, $value) = split(/[ \t\n]+/, $line);
		$info{$parm} = $value;
	}

	close(CMD);

	return %info;

}

######################################################################
#
# Write header crap to  html file.
#
sub progress_header {
	print HTML <<END_OF_HTML;
<HTML>
<HEAD>
<TITLE>$html_title</TITLE>
<BODY>
<P>
<A HREF="index.html">Back</A>
<P>
<H1>$html_title</H1>
This page contains the current progress of the Evolve ALIFE simulator.
The most recent simulations are at the bottom of the page. Each simulation
is compressed inside of a WinZip file.
<P>
END_OF_HTML

}

######################################################################
#
# Write trailer html crap.
#
sub progress_trailer
{
	print HTML <<END_OF_HTML;
<P>
<A HREF="index.html">Back</A>
</BODY>
</HTML>
END_OF_HTML

}

sub progress_item
{
	local(%info) = @_;
	local($zipfile);

	$info{'step'}		= &commify($info{'step'});
	$info{'nborn'}		= &commify($info{'nborn'});
	$info{'ndie'}		= &commify($info{'ndie'});
	$info{'norganism'}	= &commify($info{'norganism'});
	$info{'num_cells'}	= &commify($info{'num_cells'});
	$info{'num_instructions'} = &commify($info{'num_instructions'});
	$info{'num_organic'}	= &commify($info{'num_organic'});
	$info{'num_spores'}	= &commify($info{'num_spores'});

	$zipfile = $info{'basename'};
	$zipfile =~ s/.evolve/.zip/;

	print HTML <<END_OF_HTML;
<CENTER>
<TABLE BORDER=1 WIDTH="70%">
<TR>
<TD BGCOLOR="e0e0ff" ALIGN=CENTER><B>DAY: $info{'day'}</B></TD>
<TD COLSPAN=2 ALIGN=RIGHT><B>$info{'created'}</B></TD>

<TR>
<TD BGCOLOR="e0e0ff" ALIGN=CENTER COLSPAN=3><B>$info{'basename'}</B></TD>

<TR>
<TD>Steps: $info{'step'}</TD>
<TD>Births: $info{'nborn'}</TD>
<TD>Deaths: $info{'ndie'}</TD>

<TR>
<TD>Organisms: $info{'norganism'}</TD>
<TD>Cells: $info{'num_cells'}</TD>
<TD>Instructions: $info{'num_instructions'}</TD>

<TR>
<TD>Organic: $info{'num_organic'}</TD>
<TD>Spores: $info{'num_spores'}</TD>
<TD>Sex: $info{'num_sexual'}</TD>

<TR>
<TD ALIGN=CENTER COLSPAN=2></TD>
<TD BGCOLOR="ff2010" ALIGN=CENTER><A HREF="$zipfile"><B>DOWNLOAD</B></A></TD>

</TABLE>
</CENTER>
<P><BR><P>

END_OF_HTML

}

######################################################################
#
# We examine $local_dir and rebuild the progress.html
# file based on all the simulation files located here.
#
#
sub rebuild_progress_html
{
	local($f, $result, $file);
	local(@simfiles, %info);

	&LOG("rebuilding $progress_html");

	$file = "${local_dir}/$progress_html";

	$result = open(HTML, ">$file");
	if( ! $result ) {
		&ERROR("Unable to open $file");
		return;
	}

	&progress_header;

	@simfiles = &get_sim_files;

	foreach $f (@simfiles) {
		%info = &simfile_info($f);
		&progress_item(%info);
	}

	&progress_trailer;

	close(HTML);
}


######################################################################
#
# Query the $local_dir and figure out the filenames for the
# current simulation, and the next simulation to use.
#
sub get_filenames
{
	local(@result, @simfiles, $curr_sim, $curr_zip, $next_sim, $next_zip);
	local($day);

	@simfiles = &get_sim_files;

	$curr_sim = $simfiles[$#simfiles];

	$day = (split(/[.-]/, $curr_sim))[1] + 0;

	$curr_zip = sprintf "day-%04d.zip", $day;

	$day = $day + 1;
	$next_sim = sprintf "day-%04d.evolve", $day;
	$next_zip = sprintf "day-%04d.zip", $day;

	@result = ($curr_sim, $curr_zip, $next_sim, $next_zip);

	return @result
}

#######################################################################
#
# Run the simulator
#
sub simulate
{
	local($sim1, $sim2) = @_;
	local($cmd, $f1, $f2);
	local($rc);


	$f1 = "${local_dir}/$sim1";
	$f2 = "${local_dir}/$sim2";

	$cmd = "${local_dir}/evolve_batch s $duration $f1 $f2";

	&LOG("Running: $cmd");

	$rc = system($cmd);
	$rc &= 127;
	if( $rc != 0 ) {
		die "command: $cmd, signal: $rc";
	}
}

######################################################################
# run zip
#
sub zipit
{
	local($sim, $zip) = @_;
	local($cmd);
	local($rc);

	$cmd = "(cd $local_dir; zip $zip $sim)";

	&LOG("Running: $cmd");

	$rc = system($cmd);
	$rc &= 127;
	if( $rc != 0 ) {
		die "command: $cmd, signal: $rc";
	}
}

#######################################################################
# MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN
#
# This script is intended to run forever.
#
# When it starts is looking in '$local_dir' and
# finds the latest simulation file and then
# uses that file as a starting point to simulate for
# another day.
#
# When we first run, we rebuild the progress file, and also upload
# the current sim file.
#
# When the simulator completes we upload
# the new file to the server, and rebuild the 'progress.html'
# file.
#
#
#
MAIN:
{
	local($curr_sim, $curr_zip, $next_sim, $next_zip);
	local($cnt);

	&LOG("alife_deamon started.");
	&LOG("working directory is $local_dir");

	$cnt = 1;
	while(1) {
		($curr_sim, $curr_zip, $next_sim, $next_zip) = &get_filenames;

		if( $cnt == 1 ) {
			&LOG("FIRST TIME: rebuild and zip & upload current file");
			&zipit($curr_sim, $curr_zip);
			&rebuild_progress_html;
			&ftp_shit($curr_zip, $progress_html);
			&LOG("\n\n");
		}

		&simulate($curr_sim, $next_sim);

		&zipit($next_sim, $next_zip);

		&rebuild_progress_html;

		&ftp_shit($next_zip, $progress_html);

		print "\n\n";

		$cnt = $cnt + 1;
	}

}

