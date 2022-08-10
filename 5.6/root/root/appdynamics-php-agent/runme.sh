#!/bin/sh
#
#
# Copyright 2013 AppDynamics.
# All rights reserved.
#
#
# Self Service Install script for AppDynamics PHP agent
#
agentVersionId="4.2.8.2GA.11548.cf31a1eb84fb5e6c011dc4ce3afac0bb40cb96fd"
canonicalNameOfThisFile=`readlink -f "$0"`
containingDir=`dirname "$canonicalNameOfThisFile"`

echo "Installing AppDynamics PHP agent: ${agentVersionId}"

varFileBaseName="installVars"
varFile="${containingDir}/${varFileBaseName}"

rpmBaseName=""
tarballBaseName="appdynamics-php-agent.tar.bz2"

if [ ! -r "${varFile}" ] ; then
    echo "Unable to find install variables at: ${varFile}" >&2
    echo "Please contact customer support." >&2
    exit 1
fi

. "${varFile}"

if [ -z "${APPD_CONF_CONTROLLER_HOST}" ] ; then
    echo "${varFileBaseName} did not set APPD_CONF_CONTROLLER_HOST!" >&2
    echo "Please contact customer support" >&2
    exit 1
fi

if [ -z "${APPD_CONF_CONTROLLER_PORT}" ] ; then
    echo "${varFileBaseName} did not set APPD_CONF_CONTROLLER_PORT!" >&2
    echo "Please contact customer support" >&2
    exit 1
fi

if [ -z "${APPD_CONF_ACCOUNT_NAME}" -a -n "${APPD_CONF_ACCESS_KEY}" ] ; then
    echo "${varFileBaseName} set APPD_CONF_ACCESS_KEY, but not APPD_CONF_ACCOUNT_NAME." >&2
    echo "Please contact customer support" >&2
    exit 1
fi

if [ -z "${APPD_CONF_ACCESS_KEY}" -a -n "${APPD_CONF_ACCOUNT_NAME}" ] ; then
    echo "${varFileBaseName} set APPD_CONF_ACCOUNT_NAME, but not APPD_CONF_ACCESS_KEY." >&2
    echo "Please contact customer support" >&2
    exit 1
fi

if [ -z "${APPD_CONF_HTTP_PROXY_HOST}" -a -n "${APPD_CONF_HTTP_PROXY_PORT}" ] ; then
    echo "${varFileBaseName} set APPD_CONF_HTTP_PROXY_PORT, but not APPD_CONF_HTTP_PROXY_HOST." >&2
    echo "Please contact customer support" >&2
    exit 1
fi

if [ -z "${APPD_CONF_HTTP_PROXY_PORT}" -a -n "${APPD_CONF_HTTP_PROXY_HOST}" ] ; then
    echo "${varFileBaseName} set APPD_CONF_HTTP_PROXY_HOST, but not APPD_CONF_HTTP_PROXY_PORT." >&2
    echo "Please contact customer support" >&2
    exit 1
fi

if [ -z "${APPD_CONF_HTTP_PROXY_USER}" -a -n "${APPD_CONF_HTTP_PROXY_PASSWORD_FILE}" ] ; then
    echo "${varFileBaseName} set APPD_CONF_HTTP_PROXY_PASSWORD_FILE, but not APPD_CONF_HTTP_PROXY_USER." >&2
    echo "Please contact customer support" >&2
    exit 1
fi

if [ -z "${APPD_CONF_HTTP_PROXY_PASSWORD_FILE}" -a -n "${APPD_CONF_HTTP_PROXY_USER}" ] ; then
    echo "${varFileBaseName} set APPD_CONF_HTTP_PROXY_USER, but not APPD_CONF_HTTP_PROXY_PASSWORD_FILE." >&2
    echo "Please contact customer support" >&2
    exit 1
fi

rpmFileName="${containingDir}/${rpmBaseName}"
tarFileName="${containingDir}/${tarballBaseName}"

if [ ! -f "${rpmFileName}" -a ! -f "${tarFileName}" ]; then
    echo "Unable to find rpm file ( '${rpmFileName}' ) or tar file ( '${tarFileName}' )" >&2
    echo "Please contact customer support" >&2
    exit 1
fi

[ -n "${APPD_CONF_CONTROLLER_HOST}" ] && export APPD_CONF_CONTROLLER_HOST
[ -n "${APPD_CONF_CONTROLLER_PORT}" ] && export APPD_CONF_CONTROLLER_PORT
[ -n "${APPD_CONF_APP}" ] && export APPD_CONF_APP
[ -n "${APPD_CONF_TIER}" ] && export APPD_CONF_TIER
[ -n "${APPD_CONF_NODE}" ] && export APPD_CONF_NODE
[ -n "${APPD_CONF_ACCOUNT_NAME}" ] && export APPD_CONF_ACCOUNT_NAME
[ -n "${APPD_CONF_ACCESS_KEY}" ] && export APPD_CONF_ACCESS_KEY
[ -n "${APPD_CONF_SSL_ENABLED}" ] && export APPD_CONF_SSL_ENABLED
[ -n "${APPD_CONF_HTTP_PROXY_HOST}" ] && export APPD_CONF_HTTP_PROXY_HOST
[ -n "${APPD_CONF_HTTP_PROXY_PORT}" ] && export APPD_CONF_HTTP_PROXY_PORT
[ -n "${APPD_CONF_HTTP_PROXY_USER}" ] && export APPD_CONF_HTTP_PROXY_USER
[ -n "${APPD_CONF_HTTP_PROXY_PASSWORD_FILE}" ] && export APPD_CONF_HTTP_PROXY_PASSWORD_FILE
[ -n "${APPD_CONF_WITH_SELINUX}" ] && export APPD_CONF_WITH_SELINUX
[ -n "${APPD_CONF_LOG_DIR}" ] && export APPD_CONF_LOG_DIR
[ -n "${APPD_CONF_PROXY_CTRL_DIR}" ] && export APPD_CONF_PROXY_CTRL_DIR

if [ -f "${rpmFileName}" ]; then
    rpm -i "${rpmFileName}"
else
    tar -xjf "${tarFileName}" --overwrite --strip-components 1 -C "${containingDir}"
    APPD_AUTO_MODE="install" \
        "${containingDir}/install.sh"
fi
