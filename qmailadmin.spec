#
# spec file for package qmailadmin
#
# Copyright (c) 2019 SUSE LINUX GmbH, Nuernberg, Germany.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://bugs.opensuse.org/
#


%define		sname		qmailadmin
%define		tdir %{_datadir}/toaster
%define		vdir /home/vpopmail
%define		qdir %{_localstatedir}/qmail
%if 0%{?suse_version}
%define		apache_confd	%{_sysconfdir}/apache2/conf.d
BuildRequires:  apache2
BuildRequires:  automake
Requires:       apache2
# FIXME: use proper Requires(pre/post/preun/...)
PreReq:         permissions
%else
%define		apache_confd	%{_sysconfdir}/httpd/conf.d
%endif
Version:        1.2.15
Release:        0
URL:            http://www.inter7.com/qmailadmin
Source0:        qmailadmin-%{version}.tar.bz2
Source1:        help.tar.bz2
Source2:        qmailadmin-rpmlintrc
Source3:        qmailadmin.permissions
Source4:        qmailadmin.conf
Patch0:         qmailadmin_lang_de.diff
Patch1:         mailinglist.c.ezmlm7.patch
Patch2:         header.html.patch
BuildRequires:  ezmlm-idx
BuildRequires:  qmail-skel-devel
BuildRequires:  vpopmail-devel
Requires:       ezmlm-idx
Requires:       maildrop
Requires:       qmail
Requires:       qmail-autorespond
Requires:       vpopmail
Obsoletes:      qmailadmin-toaster < %{version}
Provides:       qmailadmin-toaster = %{version}
%if 0%{?spambox:1}
Name:           qmailadmin-spambox
Summary:        Web Administration for qmail/vpopmail with Spambox suuport
License:        GPL-2.0-only
Provides:       qmailadmin = %{version}
%else
Name:           qmailadmin
Summary:        Web Administration for qmail/vpopmail
License:        GPL-2.0-only
# FIXME: This is the correct group for Fedora
Group:          Networking/Other
Conflicts:      qmailadmin-spambox
%endif
%if 0%{?suse_version}
Group:          Productivity/Networking/Email/Utilities
%else
# FIXME: use correct group, see "https://en.opensuse.org/openSUSE:Package_group_guidelines"
Group:          Networking/Other
%endif
%if 0%{?rhel_version} || 0%{?centos_version} || 0%{?fedora_version}
BuildRequires:  httpd
Requires:       httpd
%endif

%description
QmailAdmin is a free software package that provides a web interface
for managing a  qmail  system with virtual domains. This version is
for use with the vpopmail program. It provides admin for
adding/deleting users, Aliases, Forwards, Mailing lists and
Autoresponders.

%prep
%setup -q -n %{sname}-%{version}
%patch0
%patch1
%patch2

cp %{SOURCE1} help.tar.bz2
tar xfj help.tar.bz2
rm -f help.tar.bz2

#dont change owner to allow builds without root
sed -i -e 's/-o @vpopuser@//g ; s/-g @vpopgroup@//g' Makefile.am

%build
%define cflags %(echo %{optflags} | sed -e 's/$/ -fPIE/' )
%define ldflags %(echo %{optflags} | sed -e 's/$/ -pie/' )

export CFLAGS="%{cflags}"
export LDFLAGS="%{ldflags}"

autoreconf --install
%configure \
 --datadir=%{_datadir}/%{sname} \
 --enable-htmllibdir=%{_datadir}/%{sname} \
 --enable-htmldir=%{_datadir}/%{sname} \
 --enable-cgibindir=%{_datadir}/%{sname} \
 --enable-imageurl=/%{sname}/images \
 --enable-imagedir=%{_datadir}/%{sname}/images \
 --enable-cgipath=/qmailadmin/index.cgi \
 --enable-vpopmaildir=%{vdir} \
 --enable-qmaildir=%{qdir} \
 --enable-ezmlmdir=%{_bindir} \
 --enable-autoresponder-path=%{_bindir} \
 --enable-vpopuser=vpopmail \
 --enable-vpopgroup=vchkpw \
 --enable-maxusersperpage=12 \
 --enable-maxaliasesperpage=12 \
 --enable-modify-quota=y \
 --enable-ezmlm-mysql=n \
 --enable-help=y \
%if 0%{?spambox:1}
 --enable-modify-spam \
 --enable-spam-command="|%{_localstatedir}/qmail/bin/preline %{_bindir}/maildrop -A 'Content-Filter: maildrop-toaster' %{_sysconfdir}/mail/mailfilter" \
%endif
--enable-domain-autofill=n

make %{?_smp_mflags}

%install
mkdir -p %{buildroot}%{_sysconfdir}/permissions.d
install -m644 %{SOURCE2} %{buildroot}%{_sysconfdir}/permissions.d/qmailadmin

install -d %{buildroot}%{_datadir}/%{sname}
%make_install

pushd %{buildroot}%{_datadir}/%{sname}
  ln -s qmailadmin index.cgi
popd

# module to be inserted into toaster-web-admin
echo "

<!-- qmailadmin.module -->
<tr>
	<td align=\"right\" width=\"47%\">Edit Users, mailing lists, forwarders</td>
	<td width=\"6%\"></td>
	<td align=\"left\" width=47%\"><input type=\"button\" value=\"%{name}-%{version}\" class=\"inputs\" onClick=\"location.href='/qmailadmin';\"></td>
</tr>
<!-- qmailadmin.module -->

" > %{buildroot}%{_datadir}/%{sname}/%{sname}.module

# apache configuration file
mkdir -p %{buildroot}/%{apache_confd}
install -m644 %{SOURCE4} %{buildroot}/%{apache_confd}/%{sname}.conf
sed -e 's|QMAILADMINDIR|%{_datadir}/%{sname}|' -i %{buildroot}/%{apache_confd}/%{sname}.conf

# Install online help
install -d %{buildroot}%{_datadir}/%{sname}/images/help
cp -R $RPM_BUILD_DIR/%{sname}-%{version}/help/* %{buildroot}%{_datadir}/%{sname}/images/help/

%triggerin -- control-panel-toaster
# Insert into toaster-web-admin
if [ -d %{tdir}/include ] ; then
	ln -fs %{_datadir}/%{sname}/%{sname}.module %{tdir}/include
fi

%triggerun -- control-panel-toaster
# Delete from toaster-web-admin
if [ -L %{tdir}/include/%{sname}.module ] ; then
	rm %{tdir}/include/%{sname}.module
fi

%post
%set_permissions %{_datadir}/qmailadmin/qmailadmin

%verifyscript
%verify_permissions -e %{_datadir}/qmailadmin/qmailadmin

%files
%defattr(0644,root,root,0755)
%config %{_sysconfdir}/permissions.d/%{sname}
%dir %{_datadir}/%{sname}
%{_datadir}/%{sname}/html
%{_datadir}/%{sname}/lang
%{_datadir}/%{sname}/images

%{_datadir}/%{sname}/%{sname}.module

%license COPYING
%doc AUTHORS BUGS ChangeLog FAQ NEWS README* TODO TRANSLATORS

%config(noreplace) %{apache_confd}/%{sname}.conf

%verify(not mode) %attr(0755,vpopmail,vchkpw) %{_datadir}/%{sname}/index.cgi
%verify(not mode) %attr(6755,vpopmail,vchkpw) %{_datadir}/%{sname}/qmailadmin

%changelog
