#!/usr/bin/env perl

# Based on make_names.pl
#
# Copyright (C) 2005-2022 Apple Inc. All rights reserved.
# Copyright (C) 2009, Julien Chaffraix <jchaffraix@webkit.org>
# Copyright (C) 2009 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
# Copyright (C) 2009 Robert Hogan <robert@roberthogan.net>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1.  Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 2.  Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
# 3.  Neither the name of Apple Inc. ("Apple") nor the names of
#     its contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# This script reads Version.xcconfig and returns either or both of the major and minor
# WebKit version numbers. It is currently used by WebKit.pri.

use strict;

use Config;
use Getopt::Long;
use File::Path;

my $usage = "generate-webkitversion.pl --config Configurations/Version.xcconfig --outputDir <outputdir>";

my $major_version = "";
my $minor_version = "";
# The appropriate Apple-maintained Version.xcconfig file for WebKit version information is in Configurations/.
my $configFile = "./Configurations/Version.xcconfig";
my $outputDir = "";

GetOptions('config=s' => \$configFile,
    'outputDir=s' => \$outputDir);

die "You must specify a --config <file> " unless (length($configFile));
die "You must specify a --outputDir <outputdir> " unless (length($outputDir));

die "${configFile} does not exist: use --config <file> to specify its correct location." unless (-e $configFile);
die "$outputDir/ does not exist: use --outputDir <directory> to specify the location of an output directory that exists" unless (-e "$outputDir");

unless (open INPUT, "<", $configFile) { print STDERR "File does not exist: $configFile\n";}
while (my $line = <INPUT>) {
    chomp $line;
    if ($line =~ /^MAJOR_VERSION\s+=\s+\d+;/) {
      $line =~ s/^(MAJOR_VERSION)\s+(=)\s+(\d+);/$3/;
      $major_version = $line;
    }
    if ($line =~ /^MINOR_VERSION\s+=\s+\d+;/) {
      $line =~ s/^(MINOR_VERSION)\s+(=)\s+(\d+);/$3/;
      $minor_version = $line;
    }
}

$major_version = "531" unless (length($major_version));
$minor_version = "3" unless (length($minor_version));

my $webKitVersionPath = "$outputDir/WebKitVersion.h";

printWebKitVersionHeaderFile("$webKitVersionPath");

sub printLicenseHeader
{
    my $F = shift;
    print F "/*
 * THIS FILE IS AUTOMATICALLY GENERATED, DO NOT EDIT.
 *
 *
 * Copyright (C) 2009-2022 Apple Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */


";
}

sub printWebKitVersionHeaderFile
{
    my $headerPath = shift;
    my $F;
    open F, ">$headerPath";

    printLicenseHeader($F);

    print F "#ifndef WebKitVersion_h\n";
    print F "#define WebKitVersion_h\n\n";

    print F "#define WEBKIT_MAJOR_VERSION $major_version\n";
    print F "#define WEBKIT_MINOR_VERSION $minor_version\n\n";

    print F "#endif //WebKitVersion_h\n";

    close F;
}



