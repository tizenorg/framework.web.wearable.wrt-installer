/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <regex.h>
#include <sys/stat.h>
#include <widget_uninstall/job_widget_uninstall.h>
#include <widget_uninstall/widget_uninstall_errors.h>
#include <widget_uninstall/task_check.h>
#include <widget_uninstall/task_db_update.h>
#include <widget_uninstall/task_remove_files.h>
#include <widget_uninstall/task_remove_custom_handlers.h>
#include <widget_uninstall/task_smack.h>
#include <widget_uninstall/task_uninstall_ospsvc.h>
#include <widget_uninstall/task_delete_pkginfo.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <pkg-manager/pkgmgr_signal.h>
#include <app2ext_interface.h>
#include <dpl/utils/path.h>
#include <installer_log.h>

using namespace WrtDB;

namespace { //anonymous
const char* REG_TIZEN_PKGID_PATTERN = "^[a-zA-Z0-9]{10}$";
const int PKGID_LENTH = 10;
const DPL::Utils::Path PRELOAD_INSTALLED_PATH("/usr/apps");

bool checkDirectoryExist(const std::string& pkgId)
{
    DPL::Utils::Path installPath(GlobalConfig::GetUserInstalledWidgetPath());
    installPath /= pkgId;
    return installPath.Exists();
}
}

namespace Jobs {
namespace WidgetUninstall {

class UninstallerTaskFail :
    public DPL::TaskDecl<UninstallerTaskFail>
{
  private:
    WidgetStatus m_status;

    void StepFail()
    {
        if (WidgetStatus::NOT_INSTALLED == m_status) {
            ThrowMsg(Jobs::WidgetUninstall::Exceptions::WidgetNotExist,
                     "Widget does not exist");
        } else if (WidgetStatus::PREALOAD == m_status) {
            ThrowMsg(Jobs::WidgetUninstall::Exceptions::Unremovable,
                     "Widget cann't uninstall");
        } else {
            Throw(Jobs::WidgetUninstall::Exceptions::Base);
        }
    }

  public:
    UninstallerTaskFail(WidgetStatus status) :
        DPL::TaskDecl<UninstallerTaskFail>(this),
        m_status(status)

    {
        AddStep(&UninstallerTaskFail::StepFail);
    }
};

JobWidgetUninstall::JobWidgetUninstall(
    const std::string & tizenPkgId,
    const WidgetUninstallationStruct &
    uninstallerStruct) :
    Job(Uninstallation),
    JobContextBase<WidgetUninstallationStruct>(uninstallerStruct),
    m_id(tizenPkgId),
    m_exceptionCaught(Jobs::Exceptions::Success)
{
    using namespace PackageManager;
    m_context.removeStarted = false;
    m_context.removeFinished = false;
    m_context.removeAbnormal = false;
    m_context.uninstallStep = UninstallerContext::UNINSTALL_START;
    m_context.job = this;

    Try
    {
        WidgetStatus status = getWidgetStatus(tizenPkgId);

        if (WidgetStatus::Ok == status) {
            //TODO: check and save type

            WrtDB::WidgetDAOReadOnly dao(*m_context.tzAppIdList.begin());
            m_context.tzPkgid = DPL::ToUTF8String(dao.getTizenPkgId());
            m_context.locations = WidgetLocation(m_context.tzPkgid);
            m_context.locations->registerAppid(DPL::ToUTF8String(*m_context.tzAppIdList.begin()));
            m_context.installedPath =
                DPL::Utils::Path(*dao.getWidgetInstalledPath());
            m_context.manifestFile = getManifestFile();
            PackagingType packagingType = dao.getPackagingType();

            _D("Widget model exists. Pkg id : %s", m_context.tzPkgid.c_str());

            // send start signal of pkgmgr
            if (GetInstallerStruct().pkgmgrInterface->setPkgname(m_context.tzPkgid))
            {
                GetInstallerStruct().pkgmgrInterface->startJob(InstallationType::Uninstallation);
            }

            AddTask(new TaskCheck(m_context));
            if (packagingType == PKG_TYPE_HYBRID_WEB_APP) {
                AddTask(new TaskUninstallOspsvc(m_context));
            }
            AddTask(new TaskDeletePkgInfo(m_context));
            AddTask(new TaskDbUpdate(m_context));
            AddTask(new TaskSmack(m_context));

            AddTask(new TaskRemoveCustomHandlers(m_context));
            AddTask(new TaskRemoveFiles(m_context));
        } else if (WidgetStatus::NOT_INSTALLED == status ||
                WidgetStatus::PREALOAD == status) {
            AddTask(new UninstallerTaskFail(status));
        } else if (WidgetStatus::ABNORMAL == status) {
            m_context.locations = WidgetLocation(m_context.tzPkgid);
            m_context.removeAbnormal = true;
            AddTask(new TaskRemoveFiles(m_context));
        } else {
            AddTask(new UninstallerTaskFail(WidgetStatus::UNRECOGNIZED));
        }
    } Catch(WidgetDAOReadOnly::Exception::Base) {
        AddTask(new UninstallerTaskFail(WidgetStatus::UNRECOGNIZED));
    }
}

// regexec() function does not work properly in specific locale (ex, Estonian)
// So, change locale temporally before call regcomp and regexec
class ScopeLocale {
public:
    ScopeLocale(){
        currentLocale = setlocale(LC_ALL , NULL);
        if (NULL == setlocale(LC_ALL, "C")) {
            _W("Failed to change locale to \"C\"");
        }
    }

    ~ScopeLocale() {
        if (NULL == setlocale(LC_ALL, currentLocale.c_str())) {
            _W("Failed to set previous locale");
        }
    }

private:
    std::string currentLocale;
};

WidgetStatus JobWidgetUninstall::getWidgetStatus(const std::string &id)
{
    ScopeLocale locale;

    regex_t regx;
    if(regcomp(&regx, REG_TIZEN_PKGID_PATTERN, REG_NOSUB | REG_EXTENDED)!=0){
        _D("Regcomp failed");
    }
    std::string pkgId = id;
    DPL::Utils::Path installPath;

    if ((regexec(&regx, id.c_str(),
            static_cast<size_t>(0), NULL, 0) != REG_NOERROR)) {
        //appid was passed
        pkgId = id.substr(0, PKGID_LENTH);

        //Service app cannot uninstall by appid
        WrtDB::WidgetDAOReadOnly dao(DPL::FromUTF8String(id));
        if( dao.getWidgetType().appType == APP_TYPE_TIZENWEBSERVICE ){
            _E("Service app cannot uninstall by appid");
            return WidgetStatus::NOT_INSTALLED;
        }
    }

    m_context.tzAppIdList = WrtDB::WidgetDAOReadOnly::getTzAppIdList(DPL::FromUTF8String(pkgId));
    if( m_context.tzAppIdList.empty() ){
        if(checkDirectoryExist(pkgId)) {
            _E("installed widget status is abnormal");

            return WidgetStatus::ABNORMAL;
        }
        return WidgetStatus::NOT_INSTALLED;
    }

    return WidgetStatus::Ok;
}

std::string JobWidgetUninstall::getRemovedTizenId() const
{
    return m_id;
}

bool JobWidgetUninstall::getRemoveStartedFlag() const
{
    return m_context.removeStarted;
}

bool JobWidgetUninstall::getRemoveFinishedFlag() const
{
    return m_context.removeFinished;
}

DPL::Utils::Path JobWidgetUninstall::getManifestFile() const
{
    std::ostringstream manifest_name;
    manifest_name << m_context.tzPkgid << ".xml";
    DPL::Utils::Path manifestFile;

    const DPL::Utils::Path PRELOAD_INSTALLED_PATH("/usr/apps/" + m_context.tzPkgid);
    const DPL::Utils::Path USR_PACKAGES_PATH("/usr/share/packages");
    const DPL::Utils::Path OPT_PACKAGES_PATH("/opt/share/packages");

    if (PRELOAD_INSTALLED_PATH == m_context.installedPath) {
        _D("This widget is preloaded.");
        manifestFile = USR_PACKAGES_PATH;
    } else {
        manifestFile = OPT_PACKAGES_PATH;
    }

    manifestFile /= manifest_name.str();
    _D("Manifest file : %s", manifestFile.Fullpath().c_str());

    return manifestFile;
}

void JobWidgetUninstall::SendProgress()
{
    using namespace PackageManager;
    if (!getRemoveStartedFlag() ||
        (getRemoveStartedFlag() && getRemoveFinishedFlag()))
    {
        if (NULL != GetInstallerStruct().progressCallback) {
            // send progress signal of pkgmgr
            std::ostringstream percent;
            percent << static_cast<int>(GetProgressPercent());

            _D("Call widget uninstall progressCallback");
            GetInstallerStruct().progressCallback(
                GetInstallerStruct().userParam,
                GetProgressPercent(), GetProgressDescription());
        }
    }
}

void JobWidgetUninstall::SendFinishedSuccess()
{
    using namespace PackageManager;
    // send signal of pkgmgr
    GetInstallerStruct().pkgmgrInterface->endJob(m_exceptionCaught);

    _D("Call widget uninstall success finishedCallback");
    GetInstallerStruct().finishedCallback(GetInstallerStruct().userParam,
                                          getRemovedTizenId(),
                                          Jobs::Exceptions::Success);
}

void JobWidgetUninstall::SendFinishedFailure()
{
    using namespace PackageManager;

    LOGE(COLOR_ERROR "Error in uninstallation step: %d" COLOR_END, m_exceptionCaught);
    LOGE(COLOR_ERROR "Message: %s" COLOR_END, m_exceptionMessage.c_str());
    fprintf(stderr, "[Err:%d] %s", m_exceptionCaught, m_exceptionMessage.c_str());

    // send signal of pkgmgr
    GetInstallerStruct().pkgmgrInterface->endJob(m_exceptionCaught);

    _D("Call widget uninstall failure finishedCallback");
    GetInstallerStruct().finishedCallback(GetInstallerStruct().userParam,
                                          getRemovedTizenId(),
                                          m_exceptionCaught);
    _D("[JobWidgetUninstall] Asynchronous failure callback status sent");
}

void JobWidgetUninstall::SaveExceptionData(const Jobs::JobExceptionBase &e)
{
    m_exceptionCaught = static_cast<Jobs::Exceptions::Type>(e.getParam());
    m_exceptionMessage = e.GetMessage();
}
} //namespace WidgetUninstall
} //namespace Jobs
