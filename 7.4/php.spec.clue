16c16
< %global __requires_exclude libclntsh.*
---
> 

databases.

%package pdo_oci
Summary: PDO_OCI extension
Group: Development/Languages

%description pdo_oci
blank

--
      --enable-pdo=shared \
      --with-pdo-odbc=shared,unixODBC,%{_root_prefix} \
      --with-pdo-oci=shared,instantclient,/usr,12.1 \
      --with-pdo-mysql=shared,mysqlnd \
      --with-pdo-pgsql=shared,%{_root_prefix} \
--
for mod in pgsql odbc ldap snmp xmlrpc \
    mysqlnd mysql mysqli pdo_mysql \
    pdo_oci \
%if %{with_imap}
    imap \
--
%files dba -f files.dba
%files pdo -f files.pdo
%files pdo_oci -f files.pdo_oci
%if %{with_tidy}
%files tidy -f files.tidy
--
- really enable pcntl (#142903)
- don't build with --enable-safe-mode (#148969)
- use "Instant Client" libraries for oci8 module (Kai Bolay, #149873)

* Fri Feb 18 2005 Joe Orton <jorton@redhat.com> 5.0.3-3
