#git:framework/web/wrt-installer
Name:       wrt-installer
Summary:    Installer for tizen Webruntime
Version:    0.1.175_w13
Release:    1
Group:      Development/Libraries
License:    Apache-2.0
URL:        N/A
Source0:    %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  pkgconfig(appsvc)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(libxml-2.0)
BuildRequires:  pkgconfig(openssl)
BuildRequires:  pkgconfig(dpl-efl)
BuildRequires:  pkgconfig(cert-svc-vcore)
BuildRequires:  pkgconfig(dpl-event-efl)
BuildRequires:  pkgconfig(dpl-utils-efl)
BuildRequires:  pkgconfig(dpl-wrt-dao-ro)
BuildRequires:  pkgconfig(dpl-wrt-dao-rw)
BuildRequires:  pkgconfig(wrt-commons-i18n-dao-ro)
BuildRequires:  pkgconfig(wrt-commons-widget-interface-dao)
BuildRequires:  pkgconfig(security-install)
BuildRequires:  pkgconfig(capi-security-privilege-manager)
BuildRequires:  pkgconfig(ecore-x)
BuildRequires:  pkgconfig(xmlsec1)
BuildRequires:  pkgconfig(libidn)
BuildRequires:  pkgconfig(libiri)
BuildRequires:  pkgconfig(libpcrecpp)
BuildRequires:  pkgconfig(pkgmgr-installer)
BuildRequires:  pkgconfig(pkgmgr-parser)
BuildRequires:  pkgconfig(pkgmgr-types)
BuildRequires:  pkgconfig(pkgmgr-info)
BuildRequires:  pkgconfig(pkgmgr)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(cert-svc)
BuildRequires:  pkgconfig(utilX)
BuildRequires:  pkgconfig(wrt-plugins-types)
BuildRequires:  pkgconfig(shortcut)
BuildRequires:  pkgconfig(capi-appfw-app-manager)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-system-device)
BuildRequires:  pkgconfig(app2sd)
BuildRequires:  pkgconfig(web-provider)
BuildRequires:  pkgconfig(libprivilege-control)
BuildRequires:  pkgconfig(libsmack)
BuildRequires:  pkgconfig(storage)
BuildRequires:  pkgconfig(uuid)
BuildRequires:  libss-client-devel
Requires: libss-client
Requires: xmlsec1
Requires: wrt-plugins-tizen

%description
Description: Wrt Installer for Tizen apps and Wac apps

%prep
%setup -q

%define with_tests 0
%if "%{WITH_TESTS}" == "ON" || "%{WITH_TESTS}" == "Y" || "%{WITH_TESTS}" == "YES" || "%{WITH_TESTS}" == "TRUE" || "%{WITH_TESTS}" == "1"
    %define with_tests 1
%endif

%build
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"

export LDFLAGS+="-Wl,--rpath=/usr/lib -Wl,--hash-style=both -Wl,--as-needed"
LDFLAGS="$LDFLAGS"

cmake . -DCMAKE_INSTALL_PREFIX=/usr \
        -DTIZEN_VERSION=%{tizen_version} \
        -DDPL_LOG=ON \
        -DLB_SUPPORT=ON \
        -DCMAKE_BUILD_TYPE=%{?build_type:%build_type} \
        %{?WITH_TESTS:-DWITH_TESTS=%WITH_TESTS}
make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}
%make_install

%clean
rm -rf %{buildroot}

%post
/sbin/ldconfig

#symlink for package manager
%define pkg_manager_backend_path "/usr/etc/package-manager/backend"
ln -sf /usr/bin/wrt-installer %{pkg_manager_backend_path}/wgt
ln -sf %{pkg_manager_backend_path}/wgt %{pkg_manager_backend_path}/Wgt
ln -sf %{pkg_manager_backend_path}/wgt %{pkg_manager_backend_path}/wGt
ln -sf %{pkg_manager_backend_path}/wgt %{pkg_manager_backend_path}/wgT
ln -sf %{pkg_manager_backend_path}/wgt %{pkg_manager_backend_path}/WGt
ln -sf %{pkg_manager_backend_path}/wgt %{pkg_manager_backend_path}/wGT
ln -sf %{pkg_manager_backend_path}/wgt %{pkg_manager_backend_path}/WgT
ln -sf %{pkg_manager_backend_path}/wgt %{pkg_manager_backend_path}/WGT

#for booting recovery
mkdir -p /opt/share/widget/temp_info

# for downloadable Application icons path
mkdir -p /opt/share/icons/default/small

%postun -p /sbin/ldconfig

%files
%manifest wrt-installer.manifest
%attr(755,root,root) %{_bindir}/wrt-installer
/usr/etc/package-manager/backendlib/libwgt.so
%attr(644,root,root) /usr/etc/wrt-installer/*.xsd
%{_datadir}/license/%{name}
%if %{with_tests}
    %attr(755,root,root) %{_bindir}/wrt-installer-tests-*
    /opt/share/widget/tests/installer/widgets/*
    /opt/share/widget/tests/installer/configs/*
%endif
