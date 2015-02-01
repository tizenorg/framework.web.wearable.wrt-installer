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
#include <installer_logic.h>
#include <installer_controller.h>
#include <dpl/string.h>
#include <dpl/foreach.h>
#include <dpl/wrt-dao-rw/feature_dao.h>
#include <dpl/wrt-dao-rw/plugin_dao.h>
#include <widget_install/job_widget_install.h>
#include <widget_uninstall/job_widget_uninstall.h>
#include <plugin_install/job_plugin_install.h>
#include <job_exception_base.h>
#include <plugin_install/plugin_objects.h>
#include <installer_log.h>

using namespace WrtDB;

namespace Logic {
InstallerLogic::InstallerLogic() :
    m_job(0),
    m_NextHandle(0)
{}

InstallerLogic::~InstallerLogic()
{
    if (m_job) {
        delete m_job;
    }
}

void InstallerLogic::Initialize()
{
    _D("Done");
}

void InstallerLogic::Terminate()
{
    //TODO how to delete, if it is still running, paused and so on
    if(m_job)
        m_job->SetPaused(true);

    _D("Done");
}

Jobs::JobHandle InstallerLogic::AddAndStartJob()
{
    Jobs::JobHandle handle = GetNewJobHandle();
    m_job->SetJobHandle(handle);
    //Start job
    CONTROLLER_POST_EVENT(InstallerController,
                          InstallerControllerEvents::NextStepEvent(m_job));

    return handle;
}

//InstallWidget, UninstallWidget InstallPlugin method are almost the same
// But each Job has different constructor, so creating new Job is specific
Jobs::JobHandle InstallerLogic::InstallWidget(
    const std::string & widgetPath,
    const std::string & pkgId,
    const Jobs::WidgetInstall::WidgetInstallationStruct &
    installerStruct)
{
    if(m_job)
    {
        _E("Job is in progress. It is impossible to add new job");
        return -1;
    }

    _D("New Widget Installation:");

    m_job = new Jobs::WidgetInstall::JobWidgetInstall(widgetPath, pkgId, installerStruct);

    return AddAndStartJob();
}

Jobs::JobHandle InstallerLogic::UninstallWidget(
    const std::string & widgetPkgName,
    const
    WidgetUninstallationStruct &uninstallerStruct)
{
    if(m_job)
    {
        _E("Job is in progress. It is impossible to add new job");
        return -1;
    }

    _D("New Widget Uninstallation");

    m_job  =
        new Jobs::WidgetUninstall::JobWidgetUninstall(widgetPkgName,
                                                      uninstallerStruct);

      return AddAndStartJob();
}

Jobs::JobHandle InstallerLogic::InstallPlugin(
    std::string const & pluginPath,     // TODO change type to PluginPath
    const PluginInstallerStruct &
    installerStruct)
{
    if(m_job)
    {
        _E("Job is in progress. It is impossible to add new job");
        return -1;
    }

    _D("New Plugin Installation");

    // TODO Conversion to PluginPath is temporary
    m_job =
        new Jobs::PluginInstall::JobPluginInstall(PluginPath(pluginPath), installerStruct);

    // before start install plugin, reset plugin data which is stopped
    // during installing. (PluginDAO::INSTALLATION_IN_PROGRESS)
    ResetProgressPlugins();

    return AddAndStartJob();
}

#define TRANSLATE_JOB_EXCEPTION() \
    _rethrown_exception.getParam()
#define TRANSLATE_JOB_MESSAGE() \
    _rethrown_exception.GetMessage()

bool InstallerLogic::NextStep(Jobs::Job *job)
{
    Try {
        bool stepSucceded = job->NextStep();

        job->SendProgress();

        if (stepSucceded) {
            return !job->IsPaused();
        }

        if (!job->GetAbortStarted()) {
            //job successfully finished

            //send finished callback
            job->SendFinishedSuccess();

            switch (job->GetInstallationType()) {
            case Jobs::PluginInstallation:
                InstallWaitingPlugins();
                break;
            default: //because of warning
                break;
            }
        } else {
            //job abort process completed
            job->SendFinishedFailure();
        }

        //clean job
        delete job;
        m_job=0;

        return false;
    } catch (Jobs::JobExceptionBase &exc) {
        //start revert job
        LOGE("Exception occured: %s(%d). Reverting job...", exc.GetMessage().c_str(), exc.getParam());

        bool hasAbortSteps = job->Abort();
        job->SetAbortStarted(true);
        job->SaveExceptionData(exc);

        if (!hasAbortSteps) {
            //no AbortSteps
            job->SendFinishedFailure();

            //clean job
            delete job;
            m_job=0;
        }
        return hasAbortSteps;
    }
}

//TODO implement me
bool InstallerLogic::AbortJob(const Jobs::JobHandle & /*handle*/)
{
    _W("Not implemented");
    return true;
}
void InstallerLogic::InstallWaitingPlugins()
{
    PluginHandleSetPtr waitingPlugins;

    waitingPlugins =
        PluginDAO::getPluginHandleByStatus(PluginDAO::INSTALLATION_WAITING);

    FOREACH(it, *waitingPlugins)
    {
        resolvePluginDependencies(*it);
    }
}

void InstallerLogic::ResetProgressPlugins()
{
    PluginHandleSetPtr progressPlugins;

    progressPlugins =
        PluginDAO::getPluginHandleByStatus(PluginDAO::INSTALLATION_IN_PROGRESS);

    FOREACH(it, *progressPlugins) {
        FeatureHandleListPtr featureListPtr =
            FeatureDAOReadOnly::GetFeatureHandleListForPlugin(*it);
        FOREACH(ItFeature, *featureListPtr) {
            FeatureDAO::UnregisterFeature(*ItFeature);
        }
        PluginDAO::unregisterPlugin(*it);
    }
}

bool InstallerLogic::resolvePluginDependencies(PluginHandle handle)
{
    PluginHandleSetPtr dependencies(new PluginHandleSet);

    PluginObjects::ObjectsPtr requiredObjects =
        PluginDAO::getRequiredObjectsForPluginHandle(handle);

    PluginHandle depHandle =
        Jobs::PluginInstall::JobPluginInstall::INVALID_HANDLE;

    FOREACH(requiredObject, *requiredObjects)
    {
        depHandle =
            PluginDAO::getPluginHandleForImplementedObject(*requiredObject);

        if (depHandle ==
            Jobs::PluginInstall::JobPluginInstall::INVALID_HANDLE)
        {
            _E("Library implementing: %s NOT FOUND", (*requiredObject).c_str());

            //PluginDAO::SetPluginInstallationStatus(INSTALLATION_WAITING);
            return false;
        }
        dependencies->insert(depHandle);
    }

    PluginDAO::registerPluginLibrariesDependencies(handle, dependencies);
    PluginDAO::setPluginInstallationStatus(handle,
                                           PluginDAO::INSTALLATION_COMPLETED);

    return true;
}
}

