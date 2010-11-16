#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <QtGlobal>
#include <QList>
#include <QDebug>


namespace EpisodesParser {


typedef quint32 IPAddress;
typedef uint Time;

// Efficient storage of Episode names: don't store the actual names, use
// 8-bit IDs instead. This allows for 256 different Episode names, which
// should be more than sufficient.
typedef QString EpisodeName;
typedef quint8 EpisodeID;
typedef QHash<EpisodeName, EpisodeID> EpisodeNameIDHash;

// Store Episode durations as 8-bit uints, 255 seconds as an Episode
// duration is sufficiently long (65535 seconds, 16-bit, would be excessive).
typedef quint8 EpisodeDuration;

typedef QHash<EpisodeID, EpisodeDuration> Episode;

// 510 is the highest HTTP status code, so 9 bits would be sufficient, but
// that's not possible, so we use 16 instead.
typedef quint16 HTTPStatus;

typedef QString URL;

// Efficient storage of hosts (cfr. efficient storage of Episode names).
typedef QString HostName;
typedef quint8 HostID;
typedef QHash<HostName, HostID> HostNameIDHash;

// Parsed raw line from Episodes log file: no processing applied whatsoever.
struct EpisodesLogLineStruct {
    IPAddress ip;
    Time time;
    QList<Episode> episodes;
    HTTPStatus status;
    URL url;
    HostID host;
};
typedef EpisodesLogLineStruct EpisodesLogLine;



struct IPHierarchyStruct {
    IPAddress ip;
    QString continent;
    QString country;
    QString province;
    QString city;
    QString isp;
};
typedef IPHierarchyStruct IPHierarchy;

struct ExpandedEpisodesLogLineStruct {
    IPHierarchy ip;
    Time time;
    QList<Episode> episodes;
    HTTPStatus status;
    URL url;
};
typedef ExpandedEpisodesLogLineStruct ExpandedEpisodesLogLine;





#ifdef DEBUG
// Alternative types that support named output.
struct NamedEpisodeIDStruct { EpisodeID id; EpisodeNameIDHash lookup; };
typedef NamedEpisodeIDStruct NamedEpisodeID;
struct NamedHostIDStruct { HostID host; HostNameIDHash lookup; };
typedef NamedHostIDStruct NamedHostID;

// QDebug() streaming output operators.
QDebug operator<<(QDebug dbg, const NamedEpisodeID & namedEpisodeID);
QDebug operator<<(QDebug dbg, const NamedHostID & namedHostID);
QDebug operator<<(QDebug dbg, const EpisodesLogLine & episodesLogLine);
#endif



}
#endif // TYPEDEFS_H
