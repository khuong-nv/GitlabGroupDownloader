#include "GitlabRequest.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <thread>
#include <QUrlQuery>
#include <QFile>
#include <QDataStream>
#include <QProcess>
#include <QDir>
#include <QSettings>
GitlabRequest::~GitlabRequest()
{
    delete m_manager;
    delete m_groupModel;
}

GitlabRequest::GitlabRequest() : QObject(), m_sem(0), m_gitlabHome(""), m_private_token("")
{
    m_manager = new QNetworkAccessManager(this);
    m_groupModel = new GroupModel();

    std::thread(&GitlabRequest::threadDownload, this).detach();

    connect(this, &GitlabRequest::startDownload, this, &GitlabRequest::slot_startDownload);
}

void GitlabRequest::requestGroups()
{
    m_groupModel->clear();

    // Perform a GET request
    QUrl url(m_gitlabHome + "/api/v4/groups");

    QUrlQuery query;
    query.addQueryItem("private_token", m_private_token);
    query.addQueryItem("per_page", "1000");
    url.setQuery(query);
    QNetworkRequest request(url);

    QNetworkReply* reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray response = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            QJsonArray jsonArray = jsonDoc.array();

            for (const QJsonValue &value : jsonArray)
            {
                QJsonObject obj = value.toObject();

                Group group;
                group.url = QUrl(obj["web_url"].toString());
                group.id = obj["id"].toInt();
                group.name = obj["name"].toString();
                group.fullPath = obj["full_path"].toString();
                m_groupModel->addGroup(group);
            }

            qDebug() << "Group size: " << jsonArray.size();
        }
        else
        {
            qDebug() << "Error REQUEST: " << reply->errorString();
        }
        reply->deleteLater();
    });

}

void GitlabRequest::download(QString url, int id, QString pathToSave)
{
    (void) url;

    QUrl path_url(pathToSave);

    m_mutex.lock();
    m_queueDownload.push_back(QPair<int, QString>(id, path_url.toLocalFile()));
    m_mutex.unlock();
}

void GitlabRequest::setGitlabInfo(QString gitlabHome, QString privateToken)
{
    m_groupModel->clear();

    if(gitlabHome.isEmpty())
    {
        emit accessError("Gitlab URL is empty");
        return;
    }

    if(privateToken.isEmpty())
    {
        emit accessError("Personal access token is empty");
        return;
    }

    if(gitlabHome.back() == '/')
        gitlabHome.remove(gitlabHome.size() - 1, 1);

    qDebug() << "Try access: " << gitlabHome << privateToken << this;
    QUrl url(gitlabHome + "/api/v4/user");
    QUrlQuery query;
    query.addQueryItem("private_token", privateToken);
    url.setQuery(query);
    QNetworkRequest request(url);
    QNetworkReply* reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray response = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            QJsonObject obj = jsonDoc.object();
            m_gitlabHome = gitlabHome;
            m_private_token = privateToken;
            emit accessSuccess(obj["name"].toString());
        }
        else
        {
            qDebug() << "Auth Error: " << reply->errorString();
            emit accessError(reply->errorString());
        }
        reply->deleteLater();
    });

    // Save to .config file
    QSettings settings(".gdm_config", QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    settings.setValue("gitlab_home_url", gitlabHome);
    settings.setValue("private_key", privateToken);
}

GroupModel *GitlabRequest::model() const
{
    return m_groupModel;
}

void GitlabRequest::slot_startDownload(int id, QString fileName, QString baseFolder)
{
    QUrl url(m_gitlabHome + "/api/v4/groups/" + QString::number(id) + "/projects");
    QString tmp_folder = baseFolder + QFileInfo(fileName).baseName();
    QUrlQuery query;
    query.addQueryItem("private_token", m_private_token);
    query.addQueryItem("per_page", "1000");
    url.setQuery(query);
    QNetworkRequest request(url);
    QNetworkReply* reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError)
        {
            QByteArray response = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            QJsonArray jsonArray = jsonDoc.array();

            for (const QJsonValue &value : jsonArray) {
                QJsonObject obj = value.toObject();
                int idProject = obj["id"].toInt();
                QString nameProject = obj["name"].toString();

                QUrl url_download(m_gitlabHome + "/api/v4/projects/" + QString::number(idProject) + "/repository/archive.zip");
                url_download.setQuery(query);
                QNetworkRequest request(url_download);
                QNetworkReply* download_reply = m_manager->get(request);
                m_currentDownloads.push_back(download_reply);

                connect(download_reply, &QNetworkReply::finished, [=](){
                    QByteArray b = download_reply->readAll();


                    QString path_to_zip = tmp_folder + "/" + nameProject + ".zip";
                    QFile file(path_to_zip);
                    file.open(QIODevice::WriteOnly);
                    file.write(b);

                    qDebug() << "Download success project: " << nameProject << "Byte Size:" << b.size();
                    m_currentDownloads.removeAll(download_reply);
                    download_reply->deleteLater();

                    if(m_currentDownloads.empty())
                    {
                        m_sem.release();
                    }
                });
            }
        }
        reply->deleteLater();
    });



    QUrl urlSubGroup(m_gitlabHome + "/api/v4/groups/" + QString::number(id) + "/subgroups");

    QUrlQuery querySubGroup;
    querySubGroup.addQueryItem("private_token", m_private_token);
    querySubGroup.addQueryItem("per_page", "1000");
    urlSubGroup.setQuery(querySubGroup);
    QNetworkRequest requestSubGroup(urlSubGroup);
    QNetworkReply* replySubGroup = m_manager->get(requestSubGroup);
    connect(replySubGroup, &QNetworkReply::finished, [=]() {
        if (replySubGroup->error() == QNetworkReply::NoError)
        {
            QByteArray response = replySubGroup->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            QJsonArray jsonArray = jsonDoc.array();

            for (const QJsonValue &value : jsonArray) {
                QJsonObject obj = value.toObject();
                int idGroup = obj["id"].toInt();
                QString nameGroup = obj["name"].toString();
                QString tmp_folder_subGroup = tmp_folder + "/";

                QString tmp_folder2 = tmp_folder_subGroup + QFileInfo(nameGroup).baseName();
                if(QDir(tmp_folder2).exists())
                    QProcess::execute("rm -rf " + tmp_folder2);

                QDir().mkdir(tmp_folder2);

                slot_startDownload(idGroup, nameGroup, tmp_folder_subGroup);
            }
        }
        replySubGroup->deleteLater();
    });
}

void GitlabRequest::threadDownload()
{
    while(true)
    {
        m_mutex.lock();
        QQueue<QPair<int, QString>> queue;
        queue.swap(m_queueDownload);
        m_mutex.unlock();

        for(auto pair: queue)
        {
            qDebug() << "==> Start download group: " << pair.second;
            QString tmp_folder = QFileInfo(pair.second).baseName();
            if(QDir(tmp_folder).exists())
                QProcess::execute("rm -rf " + tmp_folder);

            QDir().mkdir(tmp_folder);
            m_currentDownloads.clear();
            emit startDownload(pair.first, pair.second, "./");
            m_sem.acquire();

            QSettings settings(".gdm_config", QSettings::IniFormat);
            QString password = settings.value("default_password_7z").toString();


//            QString cmd = "zip -qq -r " + pair.second + " " + tmp_folder;
            if(QFile(pair.second).exists())
            {
                QFile(pair.second).remove();
            }

            QString cmd = QString("7z a -sdel %1 %2 %3").arg(password.isEmpty() ? "" : ("-mhe=on -p" + password)).arg(pair.second).arg(tmp_folder);
            qDebug() << "CMD:" << cmd;
            system(cmd.toStdString().c_str());

//            QDir(tmp_folder).removeRecursively();
            qDebug() << "==> Downloaded success group: " << pair.second;
        }

        emit downloadSuccess();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
