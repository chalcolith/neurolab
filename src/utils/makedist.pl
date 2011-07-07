#!/usr/bin/perl -w

use strict;
use Cwd;

my $qt_base_dir = '~/QtSDK/Desktop/Qt/473/gcc';

my $cwd = getcwd();
if ($cwd =~ /^(.*src)/)
{
    chdir($1);
    $cwd = getcwd();
}

die "You must be in the src directory to run makedist.\n" unless -d 'utils';

my $is_darwin = `uname` =~ 'Darwin';

print "cleaning...\n";
&run('sh utils/distclean.sh');
unlink 'utils/incversion/incversion';

print "updating...\n";
&run('hg -q pull -u');

&build('utils/incversion', 'incversion.pro');

# get version and hg id
my $version;

if ($#ARGV > 0 && $ARGV[0] eq '-bump')
{
    $version = `utils/incversion/incversion version.txt`;
    &run("hg commit -m \"makedist: incremented version.txt to $version\"");
    &run("hg tag \"$version\"");
}
elsif ($#ARGV > 0 && $ARGV[0] eq '-v')
{
    $version = $ARGV[1];
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

if ($is_darwin)
{
    &run("cp thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/LICENSE.LGPL $release_dir/licenses/qt/LICENSE.LGPL");
}
else
{
    &run("cp $qt_base_dir/../../../../Licenses/NQSThirdPartyAgreement.txt $release_dir/licenses/qt");
}

&run("cp thirdparty/qtpropertybrowser/qtpropertybrowser-2.5_1-opensource/LICENSE.LGPL $release_dir/licenses/qtpropertybrowser/LICENSE.LGPL");
&run("cp ../LICENSE.txt $release_dir");
&run("cp ../README.txt $release_dir");
&run("cp ../doc/manual.pdf $release_dir/NeuroLab_UserManual.pdf");

&run("cp -a ../samples/*.nln ../samples/*.nnn $release_dir/samples");

if ($is_darwin)
{
    &run("cp -a $build_dir/release/neurolab.app $release_dir");
    #&run("macdeployqt $release_dir/neurolab.app -no-strip");
    &run("utils/macdeploy --copyqt --release $release_dir");
    &run("find $release_dir -name '*_debug' | xargs rm -f ");

    &run("mkdir -p $distrib_dir/dmg");
    my $dmg_file = "$distrib_dir/dmg/neurocogling-$release-osx.dmg";
    &run("rm -f $dmg_file");
    &run("hdiutil create $dmg_file -volname $release -fs HFS+ -srcfolder $release_dir");
}
else
{
    &run("cp -a $qt_base_dir/lib/libQtCore.so* $release_dir");
    &run("cp -a $qt_base_dir/lib/libQtSvg.so* $release_dir");
    &run("cp -a $qt_base_dir/lib/libQtGui.so* $release_dir");

    &run("cp -a $build_dir/release/* $release_dir");

    &run("strip $release_dir/neurolab $release_dir/lib*");
    &run("mv $release_dir/neurolab $release_dir/neurolab-bin");
    &run("cp -a utils/neurolab_run $release_dir/neurolab");

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
}

# functions
sub build
{
    my ($path, $project) = @_;
    unless (-d $path)
    {
        mkdir $path or die "Unable to make $path: $!\n";
    }

    print "building $project in $path\n";

    chdir $path or die "Unable to cd to $path: $!\n";

    my $cmd = $is_darwin
        ? "qmake $project -spec macx-g++ CONFIG-=debug CONFIG+=release"
        : "$qt_base_dir/bin/qmake $project -spec linux-g++ CONFIG-=debug CONFIG+=release";

    my $retval = system($cmd);
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
