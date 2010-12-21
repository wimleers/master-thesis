#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <QtGlobal>
#include <QMetaType>
#include <QList>
#include <QHostAddress>
#include <QDebug>


namespace EpisodesParser {


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

struct Episode {
    Episode() {}
    Episode(EpisodeID id, EpisodeDuration duration) : id(id), duration(duration) {}
    EpisodeID id;
    EpisodeDuration duration;
#ifdef DEBUG
    EpisodeIDNameHash * IDNameHash;
#endif
};
inline bool operator==(const Episode &e1, const Episode &e2) {
    return e1.id == e2.id && e1.duration == e2.duration;
}
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
struct Domain {
    DomainID id;
    // TODO: allow multiple domains to be analyzed as one whole by providing
    // a common identifier.
#ifdef DEBUG
    EpisodeIDNameHash * IDNameHash;
#endif
};

// Parsed raw line from Episodes log file: no processing applied whatsoever.
struct EpisodesLogLine {
    QHostAddress ip;
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



struct IPHierarchy {
    QHostAddress ip;
    QString continent;
    QString country;
    QString province;
    QString city;
    QString isp;
};

struct UAHierarchyDetails {
    UA ua;
    // OS details.
    QString os_name;
    QString os_version;
    // Browser details.
    QString browser_name;
    quint16 browser_version_major;
    quint16 browser_version_minor;
};
typedef quint16 UAHierarchyID;
typedef QHash<UAHierarchyDetails, UAHierarchyID> UAHierarchyDetailsIDHash;
typedef QHash<UAHierarchyID, UAHierarchyDetails> UAHierarchyIDDetailsHash;

struct ExpandedEpisodesLogLine {
    IPHierarchy ip;
    Time time;
    EpisodeList episodes;
    HTTPStatus status;
    UAHierarchyDetails ua;
    URL url;
#ifdef DEBUG
    EpisodeIDNameHash * episodeIDNameHash;
#endif
};





#ifdef DEBUG
// QDebug() streaming output operators.
QDebug operator<<(QDebug dbg, const Episode & episode);
QDebug operator<<(QDebug dbg, const Domain & domain);
QDebug operator<<(QDebug dbg, const EpisodesLogLine & episodesLogLine);
#endif



}

// Register metatypes to allow these types to be streamed in QTests.
Q_DECLARE_METATYPE(EpisodesParser::EpisodeList)
Q_DECLARE_METATYPE(EpisodesParser::Episode)

#endif // TYPEDEFS_H
