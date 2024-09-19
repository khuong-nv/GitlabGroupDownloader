#ifndef GITLABREQUEST_H
#define GITLABREQUEST_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QNetworkReply>
#include <QAbstractListModel>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>
#include <QWaitCondition>
struct Group
{
    QUrl url;
    int id;
    QString name;
    QString fullPath;

    void print() const {
        qDebug() << url << id << name << fullPath;
    }
};

class GroupModel : public QAbstractListModel
{
    Q_OBJECT
public:

    enum GroupRoles {
        Url = Qt::UserRole + 1,
        FullPath,
        Id,
        Name
    };

    GroupModel(QObject* parent = nullptr)
        : QAbstractListModel(parent)
    {}

    void addGroup(const Group& group)
    {
        beginInsertRows(QModelIndex(), m_groups.count(), m_groups.count());
        m_groups.push_back(group);
        endInsertRows();
    }

    void clear()
    {
        beginResetModel();
        m_groups.clear();
        endResetModel();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent)
        return m_groups.count();
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (index.row() < 0 || index.row() >= m_groups.count())
            return QVariant();

        const Group &group = m_groups[index.row()];
        if (role == Url)
            return group.url.toString();
        else if (role == FullPath)
            return group.fullPath;
        else if (role == Id)
            return group.id;
        else if (role == Name)
            return group.name;
        return QVariant();
    }

    QHash<int, QByteArray> roleNames() const override {
         QHash<int, QByteArray> roles;
         roles[Url] = "url";
         roles[FullPath] = "fullpath";
         roles[Id] = "Id";
         roles[Name] = "name";
         return roles;
     }

private:
    QList<Group> m_groups;
};

class GitlabRequest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(GroupModel* model READ model CONSTANT)
public:
    GitlabRequest();
    virtual ~GitlabRequest();
    GroupModel *model() const;

    Q_INVOKABLE void download(QString url, int id, QString pathToSave);
    Q_INVOKABLE void setGitlabInfo(QString gitlabHome, QString privateToken);
    Q_INVOKABLE void requestGroups();
signals:
     void startDownload(int id, QString fileName);
     void accessError(QString error);
     void accessSuccess(QString username);
     void downloadSuccess();

public slots:
     void slot_startDownload(int id, QString fileName);
private:
     void threadDownload();
private:
    QNetworkAccessManager *m_manager;
    GroupModel* m_groupModel;
    QQueue<QPair<int, QString>> m_queueDownload;
    QMutex m_mutex;
    QSemaphore m_sem;
    QVector<QNetworkReply *> m_currentDownloads;
    QString m_gitlabHome;
    QString m_private_token;
};

#endif // GITLABREQUEST_H
