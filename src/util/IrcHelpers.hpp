#pragma once

#include <IrcMessage>
#include <QString>
#include <QUuid>

namespace chatterino {

inline QString parseTagString(const QString &input)
{
    QString output = input;
    output.detach();

    auto length = output.length();

    for (int i = 0; i < length - 1; i++)
    {
        if (output[i] == '\\')
        {
            QChar c = output[i + 1];

            switch (c.cell())
            {
                case 'n': {
                    output.replace(i, 2, '\n');
                }
                break;

                case 'r': {
                    output.replace(i, 2, '\r');
                }
                break;

                case 's': {
                    output.replace(i, 2, ' ');
                }
                break;

                case '\\': {
                    output.replace(i, 2, '\\');
                }
                break;

                case ':': {
                    output.replace(i, 2, ';');
                }
                break;

                default: {
                    output.remove(i, 1);
                }
                break;
            }

            length--;
        }
    }

    return output;
}

inline QDateTime calculateMessageTime(const Communi::IrcMessage *message)
{
    // Check if message is from recent-messages API
    if (message->tags().contains("historical"))
    {
        bool customReceived = false;
        auto ts =
            message->tags().value("rm-received-ts").toLongLong(&customReceived);
        if (!customReceived)
        {
            ts = message->tags().value("tmi-sent-ts").toLongLong();
        }

        return QDateTime::fromMSecsSinceEpoch(ts);
    }

    // If present, handle tmi-sent-ts tag and use it as timestamp
    if (message->tags().contains("tmi-sent-ts"))
    {
        auto ts = message->tags().value("tmi-sent-ts").toLongLong();
        return QDateTime::fromMSecsSinceEpoch(ts);
    }

    // Some IRC Servers might have server-time tag containing UTC date in ISO format, use it as timestamp
    // See: https://ircv3.net/irc/#server-time
    if (message->tags().contains("time"))
    {
        QString timedate = message->tags().value("time").toString();

        auto date = QDateTime::fromString(timedate, Qt::ISODate);
        date.setTimeSpec(Qt::TimeSpec::UTC);
        return date.toLocalTime();
    }

    // Fallback to current time
    return QDateTime::currentDateTime();
}

// generateClearchatUUID generates a deterministic UUID from the tags of the
// given CLEARCHAT message. The same message with the same room-id, target-user-id,
// and tmi-sent-ts tags will return the same UUID.
inline QString generateClearchatUUID(const Communi::IrcMessage *message)
{
    auto roomId = message->tag("room-id").toString();
    auto targetId = message->tag("target-user-id").toString();
    auto timestamp = message->tag("tmi-sent-ts").toString();

    auto data = roomId + "_" + targetId + "_" + timestamp;
    QUuid uuid = QUuid::createUuidV5("chatterino2", data);

    return uuid.toString();
}

}  // namespace chatterino
