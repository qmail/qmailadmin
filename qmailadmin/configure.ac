#
# $Id$
# Copyright (c) 1999-2009 Inter7 Internet Technologies, Inc.
#

AC_INIT(qmailadmin, 1.4.0)
AC_CANONICAL_HOST
AC_CONFIG_HEADER(config.h)

if test -z "$CFLAGS"
then
   AC_SUBST(CFLAGS, "")
fi

if test -z "$LDFLAGS"
then
   AC_SUBST(LDFLAGS, "")
fi

AC_DEFINE_UNQUOTED(QA_PACKAGE,"$PACKAGE_NAME")
AC_DEFINE_UNQUOTED(QA_VERSION,"$PACKAGE_VERSION")
AC_SUBST(QA_PACKAGE)
AC_SUBST(QA_VERSION)

#
# Build packages and programs
#

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_ISC_POSIX

#
# Libraries
#

AC_CHECK_LIB(crypt, crypt)
AC_CHECK_LIB(shadow, crypt)
AC_CHECK_LIB(dl, dlopen)

#
# Headers
#

AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h ctype.h dirent.h errno.h float.h pwd.h stdarg.h stdio.h stdlib.h string.h sys/param.h sys/stat.h sys/types.h sys/wait.h time.h unistd.h vlimits.h])

#
# Compiler requirements
#

AC_C_CONST
AC_C_INLINE

#
# Definitions
#

AC_C_CONST
AC_TYPE_UID_T

#
# Functions
#

AC_CHECK_FUNCS([dup2 memset mkdir strcasecmp strchr strdup strncasecmp strrchr strstr closedir malloc stat])

#
# vpopmail vconfig location
#

vconfigpaths="/home/vpopmail/bin/vconfig /usr/local/bin/vconfig /usr/bin/vconfig /usr/sbin/vconfig /sbin/vconfig /bin/vconfig /usr/local/sbin/vconfig ~vpopmail/bin/vconfig"

AC_ARG_WITH(vconfig, [  --with-vconfig=PATH     Path to vpopmail vconfig binary ], [ vconfigpaths="$withval $vconfigpaths" ])
AC_MSG_CHECKING(for vpopmail binary vconfig)

unset vconfig

for f in $vconfigpaths
do
   if test -x $f
   then
	  vconfig=$f
   fi
done

if test -z "$vconfig"
then
   AC_MSG_RESULT(not found)
   AC_MSG_ERROR([Unable to find vconfig binary.  Please use --with-vconfig to set path to vconfig binary. 
                  If you do not have a vconfig binary, please update to the 5.5 branch of vpopmail])
fi

AC_MSG_RESULT($vconfig)

#
# Check vpopmail configurations
#

AC_MSG_CHECKING(vpopmail configuration)

#
# Check version
#

allowed_versions="5.5.0"

vpopmail_version=`$vconfig v`
if test -z "$vpopmail_version"
then
   AC_MSG_RESULT(unable to determine vpopmail version)
   AC_MSG_ERROR(Could not determine your installed version of vpopmail)
fi

unset running_version 

for v in "$allowed_versions"
do
   if test "$vpopmail_version" == $v
   then
	  running_version=$v
	  break
   fi
done

if test -z "$running_version"
then
   AC_MSG_RESULT(not running required version)
   AC_MSG_ERROR([The version of vpopmail you're using, "$vpopmail_version", is not compatible with this release of qmailadmin.
				  Acceptable versions: "$allowed_versions"
				  Please or downgrade vpopmail accordingly.])
fi

#
# Include and Library depedencies
#

etcdir=`$vconfig c`
auth_incs=`cat $etcdir/inc_deps`
auth_libs=`cat $etcdir/lib_deps`
AC_SUBST(auth_incs)
AC_SUBST(auth_libs)

#
# vpopmail user and group
#

vpopuser=`$vconfig U`

AC_ARG_ENABLE(vpopuser, [  --enable-vpopuser=USER        User vpopmail was installed as [vpopmail].],
    vpopuser="$enableval",
    [
        if test "$vpopuser" = ""
        then
            AC_MSG_ERROR([Unable to find your vpopuser user, specify --enable-vpopuser.])
        fi
    ] )
AC_DEFINE_UNQUOTED(VPOPUSER,"$vpopuser","")
AC_SUBST(vpopuser)
   
vpopgroup=`$vconfig G`
AC_ARG_ENABLE(vpopgroup, [  --enable-vpopgroup=GROUP      Group vpopmail was installed as [vchkpw].],
    vpopgroup="$enableval",
    [
        if test "$vpopgroup" = ""
        then
            AC_MSG_ERROR([Unable to find your vpopgroup group, specify --enable-vpopgroup.])
        fi
    ] )
AC_DEFINE_UNQUOTED(VPOPGROUP,"$vpopgroup","")
AC_SUBST(vpopgroup)

#
# All vpopmail configurations gathered
#

AC_MSG_RESULT(ok)

#
# Webserver configurations
#

htmldir=""
for f in /home/httpd/html /usr/local/apache/htdocs /usr/local/www/data /var/www/html /var/www/htdocs /var/www
do
    if test -d $f
    then
        htmldir=$f
    fi
done
AC_ARG_ENABLE(htmldir, [  --enable-htmldir=DIR      HTTP server's base html dir.],
    htmldir="$enableval",
    [
        if test "$htmldir" = ""
        then
            AC_MSG_ERROR([Unable to find your html base directory, specify --enable-htmldir.])
        fi
    ] )
AC_SUBST(htmldir)


cgibindir=""

for f in /home/httpd /usr/local/apache /usr/local/www /var/lib/httpd /var/www /usr/local/www /usr/lib $htmldir
do
    if test -d $f/cgi-bin
    then
        cgibindir=$f/cgi-bin
    fi
done

AC_ARG_ENABLE(cgibindir, [  --enable-cgibindir=DIR    HTTP server's cgi-bin directory.],
    cgibindir="$enableval",
    [
        if test "$cgibindir" = ""
        then
            AC_MSG_ERROR([Unable to find your cgi-bin directory, specify --enable-cgibindir.])
        fi
    ] )
AC_SUBST(cgibindir)

imageurl="/images/qmailadmin"
AC_ARG_ENABLE(imageurl, [  --enable-imageurl=URL     URL to image files [/images/qmailadmin].],
    imageurl="$enableval",
    [
        if test "$imageurl" = ""
        then
            AC_MSG_ERROR([Unable to find your html url, specify --enable-imageurl.])
        fi
    ] )
AC_SUBST(imageurl)
AC_DEFINE_UNQUOTED(IMAGEURL, "$imageurl","")

imagedir=""
for f in /home/httpd/html /usr/local/apache/htdocs /usr/local/www/data /var/www/html $htmldir
do
    if test -d $f
    then
        imagedir=$f$imageurl
    fi
done
AC_ARG_ENABLE(imagedir, [  --enable-imagedir=DIR     Where to put the image files [DocumentRoot/images/qmailadmin].],
    imagedir="$enableval",
    [
        if test "$imagedir" = ""
        then
            AC_MSG_ERROR([Unable to find your html image directory, specify --enable-imagedir.])
        fi
    ] )
AC_SUBST(imagedir)

htmllibdir="/usr/local/share/qmailadmin"

AC_ARG_ENABLE(htmllibdir, [  --enable-htmllibdir=DIR   qmailadmin HTML library directory.],
    htmllibdir="$enableval",
    [
        if test "$htmllibdir" = ""
        then
            AC_MSG_ERROR([Unable to find your htmllibdir directory, specify --enable-htmllibdir.])
        fi
    ] )
AC_SUBST(htmllibdir,$htmllibdir)
AC_DEFINE_UNQUOTED(HTMLLIBDIR, "$htmllibdir","")

#
# qmail directory
# 

qmaildir=`$vconfig q`

AC_ARG_ENABLE(qmaildir, [  --enable-qmaildir=DIR     directory where qmail is installed.],
    qmaildir="$enableval",
    [
        if test "$qmaildir" = ""
        then
            AC_MSG_ERROR([Unable to find your qmaildir directory, specify --enable-qmaildir.])
        fi
    ] )
AC_DEFINE_UNQUOTED(QMAILDIR,"$qmaildir","")

#
# ezmlm directory
#

ezmlmdir=""
for f in /usr/local/bin/ezmlm /usr/bin /usr/sbin /usr/local/bin
do
    if test -f $f/ezmlm-make
    then
        ezmlmdir=$f
    fi
done

AC_ARG_ENABLE(ezmlmdir, [  --enable-ezmlmdir=PATH       Path to ezmlm binaries or =n to disable.],
    ezmlmdir="$enableval",
    [
        if test "$ezmlmdir" = ""
        then
            AC_MSG_ERROR([Unable to find your ezmlmdir user, specify --enable-ezmlmdir.])
        fi
    ] )
AC_DEFINE_UNQUOTED(EZMLMDIR,"$ezmlmdir","")

cgipath="/cgi-bin/qmailadmin"

AC_ARG_ENABLE(cgipath, [  --enable-cgipath=PATH     URL path for cgi [/cgi-bin/qmailadmin].],
    cgipath="$enableval",
    [
        if test "$cgipath" = ""
        then
            AC_MSG_ERROR([Unable to find your cgipath directory, specify --enable-cgipath.])
        fi
    ] )
AC_DEFINE_UNQUOTED(CGIPATH,"$cgipath","")

autorespond_path=""
for f in /usr/local/bin /usr/bin /usr/sbin /usr/local/bin /var/qmail/bin
do
    if test -f $f/autorespond
    then
        autorespond_path=$f
    fi
done
AC_ARG_ENABLE(autoresponder-path, [  --enable-autoresponder-path=PATH   Path to autoresponder directory.],
AC_ARG_ENABLE(autoresponder-bin, [], autorespond_path="$enableval", [])
    autorespond_path="$enableval",
    [
        if test "$autorespond_path" = ""
        then
            AC_MSG_ERROR([Unable to find your autorespond program directory, specify --enable-autoresponder-path.])
        fi
    ] )
AC_DEFINE_UNQUOTED(AUTORESPOND_PATH,"$autorespond_path","")

mysql=yes
AC_ARG_ENABLE(ezmlm-mysql, [  --disable-ezmlm-mysql         Disable MySQL options for mailing lists.],
    mysql="$enableval",)
case $mysql in
1*|y*|Y*)
	mysql=yes
        AC_DEFINE_UNQUOTED(ENABLE_MYSQL,1,"")
        ;;
*)
        mysql=no
        ;;
esac

maxusersperpage=15
AC_ARG_ENABLE(maxusersperpage, [  --enable-maxusersperpage=#    Number of users to display on a page [15].],
    maxusersperpage="$enableval",)
AC_DEFINE_UNQUOTED(MAXUSERSPERPAGE,$maxusersperpage,"")

maxaliasesperpage=25
AC_ARG_ENABLE(maxaliasesperpage, [  --enable-maxaliasesperpage=#  Number of aliases to display on a page [25].],
    maxaliasesperpage="$enableval",)
AC_DEFINE_UNQUOTED(MAXALIASESPERPAGE,$maxaliasesperpage,"")

NO_CACHE=no
AC_ARG_ENABLE(no-cache, [  --enable-no-cache   Prohibit caching via http headers.],
    NO_CACHE="$enableval",)

case $NO_CACHE in
1*|y*|Y*)
	NO_CACHE=1
        AC_DEFINE_UNQUOTED(NO_CACHE,$NO_CACHE,"")
        echo "checking whether we should prohibit caching... yes"
        ;;
*)
        echo "checking whether we should prohibit caching... no"
        ;;
esac

IPAUTH=yes
AC_ARG_ENABLE(ipauth, [  --disable-ipauth    Disable IP address checks (NAT/proxy systems may not use the same IP for the whole session).],
    IPAUTH="$enableval",)

case $IPAUTH in
1*|y*|Y*)
	IPAUTH=1
        AC_DEFINE_UNQUOTED(IPAUTH,$IPAUTH,"")
        echo "checking whether to require ip authentication... yes"
        ;;
*)
        echo "checking whether to require ip authentication... no"
        ;;
esac

USER_INDEX=yes
AC_ARG_ENABLE(user-index, [  --disable-user-index   Don't display user index on show users page.],
    USER_INDEX="$enableval",)

case $USER_INDEX in
1*|y*|Y*)
	USER_INDEX=1
        AC_DEFINE_UNQUOTED(USER_INDEX,$USER_INDEX,"")
        echo "checking whether to display user index... yes"
        ;;
*)
        echo "checking whether to display user index... no"
        ;;
esac

AC_ARG_ENABLE(modify-quota, [  --enable-modify-quota  Allow domain admin (postmaster) to modify user quotas.],
    MODIFY_QUOTA="$enableval",)

case $MODIFY_QUOTA in
1*|y*|Y*)
	MODIFY_QUOTA=yes
        AC_DEFINE_UNQUOTED(MODIFY_QUOTA,1,"")
        echo "checking whether to allow admins to modify user quotas... yes"
        ;;
*)
        MODIFY_QUOTA=no
        echo "checking whether to allow admins to modify user quotas... no"
        ;;
esac

AC_ARG_ENABLE(domain-autofill, [  --enable-domain-autofill  Autofill the domain on login page based on the hostname that accesses qmailadmin.],
   DOMAIN_AUTOFILL="$enableval",)

case $DOMAIN_AUTOFILL in
1*|y*|Y*)
      DOMAIN_AUTOFILL=yes
      AC_DEFINE_UNQUOTED(DOMAIN_AUTOFILL,1,"")
      echo "checking whether to autofill domain name on login page... yes"
      ;;
*)
      DOMAIN_AUTOFILL=no
      echo "checking whether to autofill domain name on login page... no"
      ;;
esac


CATCHALL_ENABLED=yes
AC_ARG_ENABLE(catchall, [  --disable-catchall  Disable the use of catchall accounts.],
   CATCHALL_ENABLED="$enableval",)

case $CATCHALL_ENABLED in
1*|y*|Y*)
      CATCHALL_ENABLED=yes
      AC_DEFINE_UNQUOTED(CATCHALL_ENABLED,1,"")
      echo "checking whether to enable catchall accounts... yes"
      ;;
*)
      CATCHALL_ENABLED=no
      echo "checking whether to enable catchall accounts... no"
      ;;
esac


TRIVIAL_PASSWORD_ENABLED=yes
AC_ARG_ENABLE(trivial-password, [  --disable-trivial-password  Disable the use of trivial password.],
   TRIVIAL_PASSWORD_ENABLED="$enableval",)

case $TRIVIAL_PASSWORD_ENABLED in
1*|y*|Y*)
      TRIVIAL_PASSWORD_ENABLED=yes
      AC_DEFINE_UNQUOTED(TRIVIAL_PASSWORD_ENABLED,1,"")
      echo "checking whether to enable trivial password... yes"
      ;;
*)
      TRIVIAL_PASSWORD_ENABLED=no
      echo "checking whether to enable trivial password... no"
      ;;
esac

AC_ARG_ENABLE(spamcmd-needs-email, [  --enable-spamcmd-needs-email  Append the user's email to the end of the spam command.],
    SPAM_NEED_EMAIL="$enableval",)

case $SPAM_NEED_EMAIL in
1*|y*|Y*)
      SPAM_NEED_EMAIL=yes
      AC_DEFINE_UNQUOTED(MODIFY_SPAM_NEED_EMAIL,1,"")
      echo "checking whether to append the email to the end of the spam command... yes"
      ;;
*)
      SPAM_NEED_EMAIL=no
      echo "checking whether to append the email to the end of the spam command... no"
      ;;
esac

AC_ARG_ENABLE(modify-spam, [  --enable-modify-spam  Allow users to enable/disable spam checking.],
    MODIFY_SPAM="$enableval",)

case $MODIFY_SPAM in
1*|y*|Y*)
      MODIFY_SPAM=yes
      AC_DEFINE_UNQUOTED(MODIFY_SPAM,1,"")
      echo "checking whether to allow spam check enable/disable... yes"
      ;;
*)
      MODIFY_SPAM=no
      echo "checking whether to allow spam check enable/disable... no"
      ;;
esac

spam_command="|preline /usr/local/bin/maildrop /etc/mailfilter"
AC_ARG_ENABLE(spam_command, [  --enable-spam-command=CMD     Remember to use '|preline' at the beginning, [|preline /usr/local/bin/maildrop /etc/mailfilter].],
    spam_command="$enableval",
)
AC_DEFINE_UNQUOTED(SPAM_COMMAND, "$spam_command","")
AC_SUBST(SPAM_COMMAND)

HELP=no
AC_ARG_ENABLE(help, [  --enable-help  Display help links on login page.],
    HELP="$enableval",)

case $HELP in
1*|y*|Y*)
        enable_help="yes"
        AC_SUBST(enable_help)
        AC_DEFINE_UNQUOTED(HELP,1,"")
        echo "checking whether to show help pages... yes"
        ;;
*)
        enable_help="no"
        echo "checking whether to show help pages... no"
        ;;
esac


EZMLMIDX=no
if test -f $ezmlmdir/ezmlm-moderate
then
	EZMLMIDX=yes
fi

case $EZMLMIDX in
1*|y*|Y*)
                EZMLMIDX=1
                AC_DEFINE_UNQUOTED(EZMLMIDX,$EZMLMIDX,"")
		echo "checking for ezmlm-idx... yes"
		ezmlmidx="yes"
        ;;
*)
		echo "checking for ezmlm-idx... no"
		ezmlmidx="no"
        ;;
esac

AC_OUTPUT(Makefile)

echo ""
echo "            $PACKAGE_NAME $PACKAGE_VERSION"
echo "            Current settings"
echo "---------------------------------------"
echo "       cgi-bin dir = "$cgibindir""
echo "          html dir = "$htmldir""
echo "         image dir = "$imagedir""
echo "         image URL = "$imageurl""
echo "      template dir = "$htmllibdir""
echo "         qmail dir = "$qmaildir""
echo "   autorespond dir = "$autorespond_path""
echo "         ezmlm dir = "$ezmlmdir""
echo "         ezmlm idx = "$ezmlmidx""
echo "   mysql for ezmlm = "$mysql""
echo "              help = "$enable_help""
echo "      modify quota = "$MODIFY_QUOTA""
echo "   domain autofill = "$DOMAIN_AUTOFILL""
echo " catchall accounts = "$CATCHALL_ENABLED""
echo " trivial passwords = "$TRIVIAL_PASSWORD_ENABLED""
echo " modify spam check = "$MODIFY_SPAM""

if test "$MODIFY_SPAM" = "yes"
then
        echo "      spam command = "$spam_command""
fi

