#include "Zcore.hpp"
#include "ExecSingleton.hpp"

Zcore::Zcore()
{
    _app.description("Zcore start");
    _app.footer("Zcore finished");
    _dbTServers.clear();
}

Zcore::~Zcore()
{
    ExecSingleton::getInstance().freeSession();
}

int Zcore::setCmdsParse(int argc, char **argv)
{
    std::string path, appName, keydbIpPort = "";
    _app.add_option("-a,--address", _hostIp, "connect host address for installation")->required();
    _app.add_option("-w,--password", _hostPwd, "connect host password for installation");
    _app.add_option("-k,--keypath", _hostKeyPath, "connect host API key path for installation");
    _app.add_option("-u,--username", _hostUserName, "connect host user name for installation")->required();
    std::function<void(std::string)> funcApp = std::bind(&Zcore::installCallback, this, std::placeholders::_1);
    _app.add_option("-i,--installapp", appName, "install application")->each(funcApp);
    std::function<void(std::string)> funcPath = std::bind(&Zcore::pathCallback, this, std::placeholders::_1);
    _app.add_option("-p,--path", path, "select application install path")->each(funcPath);

    std::string master = "";
    std::function<void(std::string)> funcYudb = std::bind(&Zcore::yudbMasterDeploy, this, std::placeholders::_1);
    _app.add_option("--ymaster", master, "deploy YugabyteDB and create master node")->each(funcYudb);
    funcYudb = std::bind(&Zcore::yudbTServerDeploy, this, std::placeholders::_1);
    _app.add_option("--tserver", master, "deploy YugabyteDB and create tserver node")->each(funcYudb);

    std::function<void(std::string)> funcKeydb = std::bind(&Zcore::keydbDeploy, this, std::placeholders::_1);
    _app.add_option("--keydb", keydbIpPort, "deploy database named keydb (Execute only one by one)")->each(funcKeydb);
    _app.add_option("--keydbclusters", _keydbClusters, "set keydb cluster");
    std::function<void(std::string)> funcKeydbCluster = std::bind(&Zcore::keydbClusterSet, this, std::placeholders::_1);
    bool flag = false;
    _app.add_flag("-d", flag, "deploy keydb deploy")->each(funcKeydbCluster);

    CLI11_PARSE(_app, argc, argv);
    return 0;
}

void Zcore::installCallback(std::string app)
{
    connectHost();
    ExecSingleton::getInstance().installCallback(app);
}

void Zcore::pathCallback(std::string path)
{
    ExecSingleton::getInstance().pathCallback(path);
    // the session needs to be released because of pathCallback execution after install
}

void Zcore::yudbMasterDeploy(std::string master)
{
    connectHost();
    ExecSingleton::getInstance().yugabyteDeploy(master);
    ExecSingleton::getInstance().freeSession();
}

void Zcore::yudbTServerDeploy(std::string master)
{
    LOG(INFO) << "yudb TServer Deploy";
    connectHost();
    ExecSingleton::getInstance().setMasterIp(master);
    ExecSingleton::getInstance().yugabyteDeploy(_hostIp, true);
    ExecSingleton::getInstance().freeSession();
}

void Zcore::keydbDeploy(std::string port)
{
    connectHost();
    ExecSingleton::getInstance().keydbDeploy(port);
    ExecSingleton::getInstance().freeSession();
}

void Zcore::keydbClusterSet(std::string port)
{
    connectHost();
    ExecSingleton::getInstance().keydbClusterSet(_keydbClusters);
    ExecSingleton::getInstance().freeSession();
}

void Zcore::connectHost()
{
    nlohmann::json hostInfo;
    hostInfo["ip"] = _hostIp;
    hostInfo["password"] = _hostPwd;
    hostInfo["path"] = _hostKeyPath;
    hostInfo["userName"] = _hostUserName;
    LOG(ERROR) << hostInfo;

    int ret = ExecSingleton::getInstance().connect(hostInfo);
    LOG(INFO) << "connect status " << ret;
}
