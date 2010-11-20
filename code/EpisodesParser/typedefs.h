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
typedef QHash<EpisodeID, EpisodeName> EpisodeIDNameHash;

// Store Episode durations as 8-bit uints, 255 seconds as an Episode
// duration is sufficiently long (65535 seconds, 16-bit, would be excessive).
typedef quint8 EpisodeDuration;

struct EpisodeStruct {
    EpisodeID id;
    EpisodeDuration duration;
#ifdef DEBUG
    EpisodeIDNameHash * episodeIDNameHash;
#endif
};
typedef EpisodeStruct Episode;
typedef QList<Episode> EpisodeList;

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
    EpisodeList episodes;
#ifdef DEBUG
    EpisodeIDNameHash * episodeIDNameHash;
#endif
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
    EpisodeList episodes;
#ifdef DEBUG
    EpisodeIDNameHash * episodeIDNameHash;
#endif
    HTTPStatus status;
    URL url;
};
typedef ExpandedEpisodesLogLineStruct ExpandedEpisodesLogLine;





#ifdef DEBUG
// QDebug() streaming output operators.
QDebug operator<<(QDebug dbg, const Episode & episode);
QDebug operator<<(QDebug dbg, const NamedHostID & namedHostID);
QDebug operator<<(QDebug dbg, const EpisodesLogLine & episodesLogLine);
#endif



}
#endif // TYPEDEFS_H
