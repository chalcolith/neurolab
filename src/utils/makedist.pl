#!/usr/bin/perl -w

use strict;
use Cwd;

my $qt_base_dir = '/opt/qtsdk-2010.04';

my $cwd = getcwd();
if ($cwd =~ /utils$/)
{
    chdir('..');
    $cwd = getcwd();
}

print "cleaning...\n";
&run('sh utils/distclean.sh');
unlink 'utils/incversion/incversion';

print "updating...\n";
&run('hg -q update');

&build('utils/incversion', 'incversion.pro');

# get version and hg id
my $version;

if ($#ARGV > 0 && $ARGV[0] eq '-bump')
{
    $version = `utils/incversion/incversion version.txt`;
    &run("hg commit -m \"makedist: incremented version.txt to $version\"");
    &run("hg tag \"$version\"");
}
else
{
    $version = `utils/incversion/incversion -nobump version.txt`;
}

die "unable to get version\n" unless defined $version;
print "version is $version\n";

my $hg_id = `hg -q id`;
die "unable to get hg id\n" unless defined $hg_id;
$hg_id =~ s/^\s+//;
$hg_id =~ s/\s+$//;
print "hg id is $hg_id\n";

# build neurolab
my $build_dir = '../neurolab_all-build-desktop';

print "build neurolab...\n";
&build($build_dir, '../src/neurolab_all.pro');

# create release directory
my $distrib_dir = '../distrib';
my $release = "neurolab-$version-$hg_id";
my $release_dir = "$distrib_dir/$version/$release";

print "create release directory $release_dir...\n";
&run("rm -rf $release_dir");
&run("mkdir -p $release_dir/licenses/qt");
&run("mkdir -p $release_dir/licenses/qtpropertybrowser");
&run("mkdir -p $release_dir/samples");

# copy files
print "copying files...\n";
&run("cp $qt_base_dir/qt/LICENSE.LGPL $release_dir/licenses/qt/LICENSE.LGPL");
&run("cp thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/LICENSE.LGPL $release_dir/licenses/qtpropertybrowser/LICENSE.LGPL");
&run("cp ../LICENSE.txt $release_dir");
&run("cp ../README.txt $release_dir");
&run("cp -a $qt_base_dir/qt/lib/libQtCore.so* $release_dir");
&run("cp -a $qt_base_dir/qt/lib/libQtSvg.so* $release_dir");
&run("cp -a $qt_base_dir/qt/lib/libQtGui.so* $release_dir");

&run("cp -a $build_dir/release/* $release_dir");

&run("strip $release_dir/neurolab $release_dir/lib*");
&run("mv $release_dir/neurolab $release_dir/neurolab-bin");
&run("cp -a utils/neurolab_run $release_dir/neurolab");

&run("cp -a ../samples/*.nln ../samples/*.nnn $release_dir/samples");

# make tgz file
print "creating tgz file...\n";
&run("mkdir -p $distrib_dir/tgz");

my $zipfile = "neurocogling-neurolab-$version-$hg_id-linux.tgz";

$zipfile =~ s/linux/linux64/ if `uname -a` =~ 'x86_64';

my $pwd = `pwd`;
chdir "$distrib_dir/$version";
&run("tar czf ../tgz/$zipfile $release");
chdir $pwd;

print "created $zipfile\n";

# functions
sub build
{
    my ($path, $project) = @_;
    unless (-d $path)
    {
        mkdir $path or die "Unable to make $path: $!\n";
    }

    chdir $path or die "Unable to cd to $path: $!\n";

    my $retval = system("$qt_base_dir/qt/bin/qmake $project -spec linux-g++ CONFIG-=debug CONFIG+=release");
    $retval = system('make') if $retval == 0;

    chdir $cwd;
    die "aborted: $!\n" if $retval != 0;
}

sub run
{
    my ($cmd) = @_;
    my $retval = system($cmd);
    die "aborted: $!\n" if $retval != 0;
}
