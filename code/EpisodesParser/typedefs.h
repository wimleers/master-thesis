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
    EpisodeIDNameHash * IDNameHash;
#endif
};
typedef EpisodeStruct Episode;
typedef QList<Episode> EpisodeList;

// 510 is the highest HTTP status code, so 9 bits would be sufficient, but
// that's not possible, so we use 16 instead.
typedef quint16 HTTPStatus;

typedef QString URL;
typedef QString UA;

// Efficient storage of domain names.
typedef QString DomainName;
typedef quint8 DomainID;
typedef QHash<DomainName, DomainID> DomainNameIDHash;
typedef QHash<DomainID, DomainName> DomainIDNameHash;
struct DomainStruct {
    DomainID id;
    // TODO: allow multiple domains to be analyzed as one whole by providing
    // a common identifier.
#ifdef DEBUG
    EpisodeIDNameHash * IDNameHash;
#endif
};
typedef DomainStruct Domain;

// Parsed raw line from Episodes log file: no processing applied whatsoever.
struct EpisodesLogLineStruct {
    IPAddress ip;
    Time time;
    EpisodeList episodes;
    HTTPStatus status;
    URL url;
    UA ua;
    Domain domain;
#ifdef DEBUG
    EpisodeIDNameHash * episodeIDNameHash;
    DomainIDNameHash * domainIDNameHash;
#endif
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

struct UAHierarchyStruct {
    UA ua;
    QString os_name;
    QString os_major;
    QString os_minor;
    QString cpu_arch;
    QString browser_name;
    QString browser_major;
    QString browser_minor;
};
typedef UAHierarchyStruct UAHierarchy;

struct ExpandedEpisodesLogLineStruct {
    IPHierarchy ip;
    Time time;
    EpisodeList episodes;
    HTTPStatus status;
    UAHierarchy ua;
    URL url;
#ifdef DEBUG
    EpisodeIDNameHash * episodeIDNameHash;
#endif
};
typedef ExpandedEpisodesLogLineStruct ExpandedEpisodesLogLine;





#ifdef DEBUG
// QDebug() streaming output operators.
QDebug operator<<(QDebug dbg, const Episode & episode);
QDebug operator<<(QDebug dbg, const Domain & domain);
QDebug operator<<(QDebug dbg, const EpisodesLogLine & episodesLogLine);
#endif



}
#endif // TYPEDEFS_H
