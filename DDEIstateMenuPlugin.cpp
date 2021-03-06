//
// Created by septemberhx on 2020/5/13.
//

#include "DDEIstateMenuPlugin.h"
#include "utils/network_traffic_filter.h"
#include <QJsonObject>

#define PLUGIN_STATE_KEY "enable"

DDEIstateMenuPlugin::DDEIstateMenuPlugin(QObject *parent) : QObject(parent) {
    m_statsCollector = new StatsCollector();
    m_statsCollector->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::started, m_statsCollector, &StatsCollector::start);
    connect(&m_workerThread, &QThread::finished, m_statsCollector, &QObject::deleteLater);
    connect(this->m_statsCollector, &StatsCollector::processListUpdated, this, &DDEIstateMenuPlugin::updateProcessList);
    m_workerThread.start();
}

const QString DDEIstateMenuPlugin::pluginName() const {
    return QStringLiteral("dde_istate_menus");
}

void DDEIstateMenuPlugin::init(PluginProxyInterface *proxyInter) {
    this->m_proxyInter = proxyInter;

    this->netspeedPlugin = new DDENetspeedPlugin();
    this->netspeedPlugin->init(this->m_proxyInter);
    this->datetimePlugin = new DatetimePlugin();
    this->datetimePlugin->init(this->m_proxyInter);

    this->m_refreshTimer = new QTimer(this);
    this->m_refreshTimer->setInterval(2000);
    connect(this->m_refreshTimer, &QTimer::timeout, this, &DDEIstateMenuPlugin::fetchSystemData);
    this->m_refreshTimer->start();

    std::thread nethogs_monitor_thread(&NetworkTrafficFilter::nethogsMonitorThreadProc);
    nethogs_monitor_thread.detach();

    if (!pluginIsDisable()) {
//        this->m_proxyInter->itemAdded(this, this->pluginName());
        this->m_proxyInter->itemAdded(this->netspeedPlugin, this->netspeedPlugin->pluginName());
        this->m_proxyInter->itemAdded(this->datetimePlugin, this->datetimePlugin->pluginName());
    }
}

bool DDEIstateMenuPlugin::pluginIsAllowDisable() {
    return true;
}

bool DDEIstateMenuPlugin::pluginIsDisable() {
    return !(m_proxyInter->getValue(this, PLUGIN_STATE_KEY, true).toBool());
}

void DDEIstateMenuPlugin::pluginStateSwitched() {
    m_proxyInter->saveValue(this, PLUGIN_STATE_KEY, pluginIsDisable());
    m_proxyInter->saveValue(this->netspeedPlugin, PLUGIN_STATE_KEY, pluginIsDisable());
    m_proxyInter->saveValue(this->datetimePlugin, PLUGIN_STATE_KEY, pluginIsDisable());

    if (pluginIsDisable()) {
//        m_proxyInter->itemRemoved(this, pluginName());
        m_proxyInter->itemRemoved(this->datetimePlugin, this->datetimePlugin->pluginName());
        m_proxyInter->itemRemoved(this->netspeedPlugin, this->netspeedPlugin->pluginName());
        return;
    }

//    m_proxyInter->itemAdded(this, pluginName());
    this->m_proxyInter->itemAdded(this->netspeedPlugin, this->netspeedPlugin->pluginName());
    this->m_proxyInter->itemAdded(this->datetimePlugin, this->datetimePlugin->pluginName());
}

const QString DDEIstateMenuPlugin::pluginDisplayName() const {
    return QStringLiteral("DDE Istate Menus");
}

void DDEIstateMenuPlugin::pluginSettingsChanged() {
    if (pluginIsDisable()) {
//        m_proxyInter->itemRemoved(this, pluginName());
        m_proxyInter->itemRemoved(this->datetimePlugin, this->datetimePlugin->pluginName());
        m_proxyInter->itemRemoved(this->netspeedPlugin, this->netspeedPlugin->pluginName());
        return;
    }

//    m_proxyInter->itemAdded(this, pluginName());
    this->m_proxyInter->itemAdded(this->datetimePlugin, this->datetimePlugin->pluginName());
    this->m_proxyInter->itemAdded(this->netspeedPlugin, this->netspeedPlugin->pluginName());
}

QWidget *DDEIstateMenuPlugin::itemWidget(const QString &itemKey) {
    Q_UNUSED(itemKey)
    return nullptr;
}

void DDEIstateMenuPlugin::fetchSystemData() {
}

void DDEIstateMenuPlugin::updateProcessList(QList<ProcessEntry> procList) {
    QSet<pid_t> procIdSet;
//    for (auto iter = procList.begin(); iter != procList.end(); ++iter) {
//        if (procIdSet.contains(iter->getPID())) {
//            procList.erase(iter);
//        } else {
//            procIdSet.insert(iter->getPID());
//        }
//    }

    if (this->netspeedPlugin != nullptr) {

        this->netspeedPlugin->updateProcesses(procList);
    }
}
