/**
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
 * Modified for Windows by Alexander Kaspar <alexander.kaspar@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "networkpackage.h"
#include "kclogger.h"
//#include "deviceidhelper.h"
//#include "filetransferjob.h"
//#include "pluginloader.h"
#include "kdeconnectconfig.h"

#include <QMetaObject>
#include <QMetaProperty>
#include <QByteArray>
#include <QDataStream>
#include <QDateTime>
#include <QJsonDocument>
#include <QDebug>


QDebug operator<<(QDebug s, const NetworkPackage& pkg)
{
    s.nospace() << "NetworkPackage(" << pkg.type() << ':' << pkg.body();
    if (pkg.hasPayload()) {
        s.nospace() << ":withpayload";
    }
    s.nospace() << ')';
    return s.space();
}

const int NetworkPackage::ProtocolVersion = 7;

NetworkPackage::NetworkPackage(const QString& type, const QVariantMap &body)
    : mId(QString::number(QDateTime::currentMSecsSinceEpoch()))
    , mType(type)
    , mBody(body)
	, mPayload()
	, mPayloadSize(0)
{

}

void NetworkPackage::createIdentityPackage(NetworkPackage* np)
{
	KdeConnectConfig* config = KdeConnectConfig::instance();

	const QString id = config->deviceId();
    np->mId = QString::number(QDateTime::currentMSecsSinceEpoch());
    np->mType = PACKAGE_TYPE_IDENTITY;
	np->mPayload = QSharedPointer<QIODevice>();
	np->mPayloadSize = 0;
    np->set("deviceId", id);
	np->set("deviceName", config->name());
	np->set("deviceType", config->deviceType());
    np->set("protocolVersion",  NetworkPackage::ProtocolVersion);
	//TODO:
	QStringList tmpList;
	tmpList << "kdeconnect.ping";
	np->set("incomingCapabilities", tmpList);
	np->set("outgoingCapabilities", tmpList);

	//qDebug() << "createIdentityPackage" << np;
}

template<class T>
QVariantMap qobject2qvariant(const T* object)
{
    QVariantMap map;
    auto metaObject = T::staticMetaObject;
    for(int i = metaObject.propertyOffset(); i < metaObject.propertyCount(); ++i) {
        QMetaProperty prop = metaObject.property(i);
        map.insert(QString::fromLatin1(prop.name()), prop.readOnGadget(object));
    }

    return map;
}

QByteArray NetworkPackage::serialize() const
{
    //Object -> QVariant
	//QVariantMap variant;
	//variant["id"] = mId;
	//variant["type"] = mType;
	//variant["body"] = mBody;
	QVariantMap variant = qobject2qvariant(this);

    if (hasPayload()) {
		qDebug() << "NetworkPackage: Serializing payloadTransferInfo";
		KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Serializing payloadTransferInfo");
		variant["payloadSize"] = payloadSize();
        variant["payloadTransferInfo"] = mPayloadTransferInfo;
    }

    //QVariant -> json
    auto jsonDocument = QJsonDocument::fromVariant(variant);
    QByteArray json = jsonDocument.toJson(QJsonDocument::Compact);
    if (json.isEmpty()) {
		qDebug() << "NetworkPackage: Serialization error:";
		KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "Serialization error");
	} else {
		//if (!isEncrypted()) {
		//	qDebug() << "Serialized package:" << json;
		//}
        json.append('\n');
    }

    return json;
}

template <class T>
void qvariant2qobject(const QVariantMap& variant, T* object)
{
    for ( QVariantMap::const_iterator iter = variant.begin(); iter != variant.end(); ++iter )
    {
        const int propertyIndex = T::staticMetaObject.indexOfProperty(iter.key().toLatin1());
        if (propertyIndex < 0) {
			qWarning() << "NetworkPackage: missing property" << object << iter.key();
			//KcLogger::instance()->write(QtMsgType::QtDebugMsg, prefix, "Missing property");
			continue;
        }

        QMetaProperty property = T::staticMetaObject.property(propertyIndex);
        bool ret = property.writeOnGadget(object, *iter);
        if (!ret) {
			qWarning() << "NetworkPackage: couldn't set" << object << "->" << property.name() << '=' << *iter;
			//KcLogger::instance()->write(QtMsgType::QtInfoMsg, prefix, "couldn't set" + object + "->" + property.name() + '=' + *iter);
		}
    }
}


bool NetworkPackage::unserialize(const QByteArray& a, NetworkPackage* np)
{
    //Json -> QVariant
    QJsonParseError parseError;
    auto parser = QJsonDocument::fromJson(a, &parseError);
    if (parser.isNull()) {
		qDebug() << "NetworkPackage: Unserialization error:" << parseError.errorString();
		//KcLogger::instance()->write(QtMsgType::QtDebugMsg, prefix, "Unserialization error:" + parseError.errorString());
		return false;
    }

    auto variant = parser.toVariant().toMap();
    qvariant2qobject(variant, np);

    np->mPayloadSize = variant["payloadSize"].toInt(); //Will return 0 if was not present, which is ok
    if (np->mPayloadSize == -1) {
        np->mPayloadSize = np->get<int>("size", -1);
    }
    np->mPayloadTransferInfo = variant["payloadTransferInfo"].toMap(); //Will return an empty qvariantmap if was not present, which is ok

    //Ids containing characters that are not allowed as dbus paths would make app crash
    if (np->mBody.contains("deviceId"))
    {
        QString deviceId = np->get<QString>("deviceId");
		KdeConnectConfig::filterNonExportableCharacters(deviceId);
        np->set("deviceId", deviceId);
    }

    return true;

}

/*
FileTransferJob* NetworkPackage::createPayloadTransferJob(const QUrl &destination) const
{
    return new FileTransferJob(payload(), payloadSize(), destination);
}
*/
