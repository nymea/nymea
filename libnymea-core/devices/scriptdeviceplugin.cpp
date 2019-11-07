#include "scriptdeviceplugin.h"

#include <QQmlEngine>
#include <QDir>
#include <QJsonDocument>

#include "loggingcategories.h"

ScriptDevicePlugin::ScriptDevicePlugin(QObject *parent) : DevicePlugin(parent)
{

}

bool ScriptDevicePlugin::loadScript(const QString &fileName)
{

    QFileInfo fi(fileName);
    QString metaDataFileName = fileName + "on";

    QFile metaDataFile(metaDataFileName);
    if (!metaDataFile.open(QFile::ReadOnly)) {
        qCWarning(dcDeviceManager()) << "Failed to open plugin metadata at" << metaDataFileName;
        return false;
    }
    QJsonParseError error;
    QByteArray data = metaDataFile.readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &error);
    metaDataFile.close();

    if (error.error != QJsonParseError::NoError) {
        int errorOffset = error.offset;
        int newLineIndex = data.indexOf("\n");
        int lineIndex = 1;
        while (newLineIndex > 0 && errorOffset > newLineIndex) {
            data.remove(0, newLineIndex + 2);
            errorOffset -= (newLineIndex + 2);
            newLineIndex = data.indexOf("\n");
            lineIndex++;
        }
        if (newLineIndex >= 0) {
            data = data.left(newLineIndex);
        }
        QString spacer;
        for (int i = 0; i < errorOffset; i++) {
            spacer += ' ';
        }
        QDebug dbg = qWarning(dcDeviceManager()).nospace().noquote();
        dbg << metaDataFileName << ":" << lineIndex << ":" << errorOffset + 2 << ": error: JSON parsing failed: " << error.errorString() << ": " << data.trimmed() << endl;
        dbg << data << endl;
        dbg << spacer << "^";
        return false;
    }
    m_metaData = QJsonObject::fromVariantMap(jsonDoc.toVariant().toMap());


    QFile scriptFile(fileName);
    if (!scriptFile.open(QIODevice::ReadOnly)) {
        return false;
    }

    m_engine = new QJSEngine(this);
    m_engine->installExtensions(QJSEngine::ConsoleExtension);

    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();
    QJSValue result = m_engine->evaluate(contents, fileName);
    if (result.isError()) {
        qCWarning(dcDeviceManager()) << "Error evaluating script" << fileName << result.toString();
        return false;
    }

    return true;
}

QJsonObject ScriptDevicePlugin::metaData() const
{
    return m_metaData;
}

void ScriptDevicePlugin::init()
{

}
