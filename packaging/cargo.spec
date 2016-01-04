%define script_dir %{_sbindir}
# Default platform is Tizen, setup Fedora with --define 'platform_type FEDORA'
%{!?platform_type:%define platform_type "TIZEN"}
# Default build with systemd
%{!?without_systemd:%define without_systemd 0}

Name:           cargo
Epoch:          1
Version:        0.1.2
Release:        0
Source0:        %{name}-%{version}.tar.gz
License:        Apache-2.0
Group:          Security/Other
Summary:        Cargo libraries
BuildRequires:  cmake
BuildRequires:  boost-devel

%description
This package provides Cargo libraries.

%files

%prep
%setup -q

%build
%{!?build_type:%define build_type "RELEASE"}

%if %{build_type} == "DEBUG" || %{build_type} == "PROFILING" || %{build_type} == "CCOV"
    CFLAGS="$CFLAGS -Wp,-U_FORTIFY_SOURCE"
    CXXFLAGS="$CXXFLAGS -Wp,-U_FORTIFY_SOURCE"
%endif

%cmake . -DVERSION=%{version} \
         -DCMAKE_BUILD_TYPE=%{build_type} \
         -DSCRIPT_INSTALL_DIR=%{script_dir} \
         -DSYSTEMD_UNIT_DIR=%{_unitdir} \
         -DDATA_DIR=%{_datadir} \
         -DPYTHON_SITELIB=%{python_sitelib} \
         -DWITHOUT_SYSTEMD=%{?without_systemd}
make -k %{?jobs:-j%jobs}

%install
make install DESTDIR=${RPM_BUILD_ROOT}

%clean
rm -rf %{buildroot}

## libcargo-utils-devel Package ################################################
%package -n libcargo-utils-devel
Summary:            Development Cargo Utils static library
Group:              Development/Libraries
BuildRequires:      pkgconfig(glib-2.0)

%description -n libcargo-utils-devel
The package provides libcargo-utils development tools and static library.

%files -n libcargo-utils-devel
%defattr(644,root,root,755)
%{_libdir}/libcargo-utils.a
%attr(755,root,root) %{_libdir}/libcargo-utils.a
%{_includedir}/cargo-utils

## libLogger Package ###########################################################
%package -n libLogger
Summary:            Logger library
Group:              Security/Other
%if !%{without_systemd}
BuildRequires:      pkgconfig(libsystemd-journal)
%endif
Requires(post):     /sbin/ldconfig
Requires(postun):   /sbin/ldconfig

%description -n libLogger
The package provides libLogger library.

%post -n libLogger -p /sbin/ldconfig

%postun -n libLogger -p /sbin/ldconfig

%files -n libLogger
%defattr(644,root,root,755)
%{_libdir}/libLogger.so.0
%attr(755,root,root) %{_libdir}/libLogger.so.%{version}

%package -n libLogger-devel
Summary:        Development logger library
Group:          Development/Libraries
Requires:       libLogger = %{epoch}:%{version}-%{release}

%description -n libLogger-devel
The package provides libLogger development tools and libs.

%files -n libLogger-devel
%defattr(644,root,root,755)
%{_libdir}/libLogger.so
%{_includedir}/logger
%{_libdir}/pkgconfig/libLogger.pc

## libcargo-devel Package ##########################################################
%package -n libcargo-devel
Summary:        Development C++ object serialization library
Group:          Development/Libraries
Requires:       boost-devel
Requires:       pkgconfig(libLogger)

%description -n libcargo-devel
The package provides libcargo development tools and libs.

%files -n libcargo-devel
%defattr(644,root,root,755)
%{_includedir}/cargo
%{_libdir}/pkgconfig/libcargo.pc

## libcargo-gvariant Package ##################################################
%package -n libcargo-gvariant-devel
Summary:        Development cargo GVariant module
Group:          Development/Libraries
BuildRequires:  pkgconfig(glib-2.0)
Requires:       libcargo-devel = %{epoch}:%{version}-%{release}
Requires:       boost-devel
Requires:       pkgconfig(libLogger)

%description -n libcargo-gvariant-devel
The package provides libcargo GVariant development module.

%files -n libcargo-gvariant-devel
%defattr(644,root,root,755)
%{_includedir}/cargo-gvariant
%{_libdir}/pkgconfig/libcargo-gvariant.pc

## libcargo-json Package ######################################################
%package -n libcargo-json
Summary:            Cargo Json module
Group:              Security/Other
%if %{platform_type} == "TIZEN"
BuildRequires:      libjson-devel >= 0.10
Requires:           libjson >= 0.10
%else
BuildRequires:      json-c-devel
Requires:           json-c
%endif
Requires(post):     /sbin/ldconfig
Requires(postun):   /sbin/ldconfig

%description -n libcargo-json
The package provides libcargo Json module.

%post -n libcargo-json -p /sbin/ldconfig

%postun -n libcargo-json -p /sbin/ldconfig

%files -n libcargo-json
%defattr(644,root,root,755)
%{_libdir}/libcargo-json.so.0
%attr(755,root,root) %{_libdir}/libcargo-json.so.%{version}

%package -n libcargo-json-devel
Summary:        Development cargo Json module
Group:          Development/Libraries
Requires:       libcargo-devel = %{epoch}:%{version}-%{release}
Requires:       libcargo-json = %{epoch}:%{version}-%{release}
Requires:       boost-devel
Requires:       pkgconfig(libLogger)
%if %{platform_type} == "TIZEN"
Requires:       libjson-devel >= 0.10
%else
Requires:       json-c-devel
%endif

%description -n libcargo-json-devel
The package provides libcargo Json development module.

%files -n libcargo-json-devel
%defattr(644,root,root,755)
%{_libdir}/libcargo-json.so
%{_includedir}/cargo-json
%{_libdir}/pkgconfig/libcargo-json.pc

## libcargo-sqlite Package ##########################################################
%package -n libcargo-sqlite
Summary:            Cargo SQLite KVStore module
Group:              Security/Other
BuildRequires:      pkgconfig(sqlite3)
Requires(post):     /sbin/ldconfig
Requires(postun):   /sbin/ldconfig

%description -n libcargo-sqlite
The package provides libcargo SQLite KVStore library.

%post -n libcargo-sqlite -p /sbin/ldconfig

%postun -n libcargo-sqlite -p /sbin/ldconfig

%files -n libcargo-sqlite
%defattr(644,root,root,755)
%{_libdir}/libcargo-sqlite.so.0
%attr(755,root,root) %{_libdir}/libcargo-sqlite.so.%{version}

%package -n libcargo-sqlite-devel
Summary:        Cargo SQLite KVStore development module
Group:          Development/Libraries
Requires:       libcargo-sqlite = %{epoch}:%{version}-%{release}
Requires:       pkgconfig(sqlite3)
Requires:       boost-devel
Requires:       pkgconfig(libLogger)
Requires:       pkgconfig(libcargo)

%description -n libcargo-sqlite-devel
The package provides libcargo SQLite KVStore development module.

%files -n libcargo-sqlite-devel
%defattr(644,root,root,755)
%{_libdir}/libcargo-sqlite.so
%{_includedir}/cargo-sqlite
%{_libdir}/pkgconfig/libcargo-sqlite.pc

## libcargo-fd Package ##########################################################
%package -n libcargo-fd
Summary:            Cargo file descriptor I/O module
Group:              Security/Other
Requires(post):     /sbin/ldconfig
Requires(postun):   /sbin/ldconfig

%description -n libcargo-fd
The package provides libcargo file descriptor I/O module.

%post -n libcargo-fd -p /sbin/ldconfig

%postun -n libcargo-fd -p /sbin/ldconfig

%files -n libcargo-fd
%defattr(644,root,root,755)
%{_libdir}/libcargo-fd.so.0
%attr(755,root,root) %{_libdir}/libcargo-fd.so.%{version}

%package -n libcargo-fd-devel
Summary:        Development C++ object serialization library
Group:          Development/Libraries
Requires:       libcargo-devel = %{epoch}:%{version}-%{release}
Requires:       libcargo-fd = %{epoch}:%{version}-%{release}
Requires:       boost-devel
Requires:       pkgconfig(libLogger)

%description -n libcargo-fd-devel
The package provides libcargo file descriptor I/O module.

%files -n libcargo-fd-devel
%defattr(644,root,root,755)
%{_libdir}/libcargo-fd.so
%{_includedir}/cargo-fd
%{_libdir}/pkgconfig/libcargo-fd.pc

## libcargo-sqlite-json Package ##########################################################
%package -n libcargo-sqlite-json-devel
Summary:        Cargo SQLite with Json defaults development module
Group:          Development/Libraries
Requires:       libcargo-sqlite-devel = %{epoch}:%{version}-%{release}
Requires:       libcargo-json-devel = %{epoch}:%{version}-%{release}
Requires:       boost-devel
Requires:       pkgconfig(libLogger)

%description -n libcargo-sqlite-json-devel
The package provides libcargo SQLite with Json defaults development module.

%files -n libcargo-sqlite-json-devel
%defattr(644,root,root,755)
%{_includedir}/cargo-sqlite-json
%{_libdir}/pkgconfig/libcargo-sqlite-json.pc

## libcargo-ipc Package #######################################################
%package -n libcargo-ipc
Summary:            Cargo IPC library
Group:              Security/Other
%if !%{without_systemd}
BuildRequires:      pkgconfig(libsystemd-daemon)
%endif
Requires:           libcargo-fd
Requires:           libuuid
BuildRequires:      libuuid-devel
Requires(post):     /sbin/ldconfig
Requires(postun):   /sbin/ldconfig

%description -n libcargo-ipc
The package provides libcargo-ipc library.

%post -n libcargo-ipc -p /sbin/ldconfig

%postun -n libcargo-ipc -p /sbin/ldconfig

%files -n libcargo-ipc
%defattr(644,root,root,755)
%{_libdir}/libcargo-ipc.so.0
%attr(755,root,root) %{_libdir}/libcargo-ipc.so.%{version}

%package -n libcargo-ipc-devel
Summary:        Development cargo IPC library
Group:          Development/Libraries
Requires:       libcargo-ipc = %{epoch}:%{version}-%{release}
Requires:       pkgconfig(libcargo-fd)
Requires:       pkgconfig(libLogger)
Requires:       pkgconfig(libcargo)

%description -n libcargo-ipc-devel
The package provides libcargo-ipc development tools and libs.

%files -n libcargo-ipc-devel
%defattr(644,root,root,755)
%{_libdir}/libcargo-ipc.so
%{_includedir}/cargo-ipc
%{_libdir}/pkgconfig/libcargo-ipc.pc

## libcargo-validator Package #######################################################
%package -n libcargo-validator
Summary:            Cargo Validator library
Group:              Security/Other
Requires(post):     /sbin/ldconfig
Requires(postun):   /sbin/ldconfig

%description -n libcargo-validator
The package provides libcargo-validator library.

%post -n libcargo-validator -p /sbin/ldconfig

%postun -n libcargo-validator -p /sbin/ldconfig

%files -n libcargo-validator
%defattr(644,root,root,755)
%{_libdir}/libcargo-validator.so.0
%{_libdir}/pkgconfig/libcargo-validator.pc
%attr(755,root,root) %{_libdir}/libcargo-validator.so.%{version}

%package -n libcargo-validator-devel
Summary:        Development Cargo Validator library
Group:          Development/Libraries
Requires:       libcargo-validator = %{epoch}:%{version}-%{release}
Requires:       pkgconfig(libLogger)
Requires:       pkgconfig(libcargo)

%description -n libcargo-validator-devel
The package provides libcargo-validator development tools and libs.

%files -n libcargo-validator-devel
%defattr(644,root,root,755)
%{_libdir}/libcargo-validator.so
%{_includedir}/cargo-validator
%{_libdir}/pkgconfig/libcargo-validator.pc

## Test Package ################################################################
%package tests
Summary:          Cargo Tests
Group:            Development/Libraries
Requires:         python
%if %{platform_type} == "TIZEN"
Requires:         python-xml
%endif
Requires:         boost-test

%description tests
Cargo Unit tests.

%post tests
%if !%{without_systemd}
systemctl daemon-reload || :
systemctl enable cargo-socket-test.socket || :
systemctl start cargo-socket-test.socket || :
%endif

%preun tests
%if !%{without_systemd}
systemctl stop cargo-socket-test.socket || :
systemctl disable cargo-socket-test.socket || :
%endif

%postun tests
%if !%{without_systemd}
systemctl daemon-reload || :
%endif

%files tests
%defattr(644,root,root,755)
%attr(755,root,root) %{_bindir}/cargo-unit-tests
%if !%{without_systemd}
%attr(755,root,root) %{_bindir}/cargo-socket-test
%endif
%attr(755,root,root) %{script_dir}/cargo_all_tests.py
%attr(755,root,root) %{script_dir}/cargo_launch_test.py
%{script_dir}/cargo_test_parser.py
%if !%{without_systemd}
%{_unitdir}/cargo-socket-test.socket
%{_unitdir}/cargo-socket-test.service
%endif
%dir /etc/cargo/tests/utils
%config /etc/cargo/tests/utils/*.txt
