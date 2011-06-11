#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <QtGlobal>
#include <QMetaType>
#include <QList>
#include <QHostAddress>
#include <QStringList>
#include <QString>
#include <QHash>

#ifdef DEBUG
#include <QDebug>
#endif


namespace EpisodesParser {


typedef uint Time;

// Efficient storage of Episode names: don't store the actual names, use
// 8-bit IDs instead. This allows for 256 different Episode names, which
// should be more than sufficient.
typedef QString EpisodeName;
typedef quint8 EpisodeID;
typedef QHash<EpisodeName, EpisodeID> EpisodeNameIDHash;
typedef QHash<EpisodeID, EpisodeName> EpisodeIDNameHash;

// Store Episode durations as 16-bit uints.
typedef quint16 EpisodeDuration;
// The EpisodeDuration will be discretized to an EpisodeSpeed for association
// rule mining.
typedef QString EpisodeSpeed;

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
// that's not possible, so we use 16 bits instead.
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



struct Location{
    QString continent;
    QString country;
    QString region;
    QString city;
    QString isp;

    // @TODO this is a likely performance bottleneck.
    // @TRICKY: Note that we don't check continent and country, we assume each
    // (region, city, isp) tuple is unique on its own!
    bool operator==(const Location & other) const {
        return (this->region == other.region
                && this->city == other.city
                && this->isp == other.isp);
    }

    /**
     * Generate the items for this Location, to allow for association rule
     * mining. This takes the concept hierarchy into account.
     *
     * @return
     *   The association rule items for this Location.
     */
    QStringList generateAssociationRuleItems() const {
        static const QString prefix = "location:";
        static const QString s = ":";

        return QStringList() // Granular locations.
                             << prefix + this->continent
                             << prefix + this->continent + s + this->country
                             << prefix + this->continent + s + this->country + s + this->region
                             // ISP per country (global ISP does not make sense).
                             << prefix + "isp" + s + this->country + s + this->isp;
    }
};
typedef quint32 LocationID;
typedef QHash<Location, LocationID> LocationToIDHash;
typedef QHash<LocationID, Location> LocationFromIDHash;
uint qHash(const Location & location);

struct UAHierarchyDetails {
    // OS details.
    QString platform;
    // Browser details.
    QString browser_name;
    QString browser_version;
    quint16 browser_version_major;
    quint16 browser_version_minor;
    bool is_mobile;

    // @TODO this is a likely performance bottleneck.
    bool operator==(const UAHierarchyDetails & other) const {
        return (this->platform == other.platform && this->browser_name == other.browser_name && this->browser_version == other.browser_version);
    }

    /**
     * Generate the items for this UA, to allow for association rule
     * mining. This takes the concept hierarchy into account.
     *
     * @return
     *   The association rule items for this Location.
     */
    QStringList generateAssociationRuleItems() const {
        static const QString prefix = "ua:";
        static const QString s = ":";

        const QString browser_version_major = QString::number(this->browser_version_major);
        const QString browser_version_minor = QString::number(this->browser_version_minor);

        QStringList items;

        // Platform-specific browsers.
        items << prefix + this->platform
              << prefix + this->platform + s + this->browser_name
              << prefix + this->platform + s + this->browser_name + s + browser_version_major
              << prefix + this->platform + s + this->browser_name + s + browser_version_major + s + browser_version_minor;

        // Mobile or not.
        if (this->is_mobile)
            items << prefix + "isMobile";

        return items;
    }
};
typedef quint16 UAHierarchyID;
typedef QHash<UAHierarchyDetails, UAHierarchyID> UAHierarchyDetailsIDHash;
typedef QHash<UAHierarchyID, UAHierarchyDetails> UAHierarchyIDDetailsHash;
uint qHash(const UAHierarchyDetails & ua);

struct ExpandedEpisodesLogLine {
    LocationID location;
    Time time;
    EpisodeList episodes;
    HTTPStatus status;
    URL url;
    UAHierarchyID ua;

    LocationFromIDHash * locationFromIDHash;
    UAHierarchyIDDetailsHash * uaHierarchyIDDetailsHash;
#ifdef DEBUG
    EpisodeIDNameHash * episodeIDNameHash;
#endif
};





#ifdef DEBUG
// QDebug() streaming output operators.
QDebug operator<<(QDebug dbg, const Episode & episode);
QDebug operator<<(QDebug dbg, const Domain & domain);
QDebug operator<<(QDebug dbg, const EpisodesLogLine & episodesLogLine);
QDebug operator<<(QDebug dbg, const Location & location);
QDebug operator<<(QDebug dbg, const UAHierarchyDetails & ua);
QDebug operator<<(QDebug dbg, const ExpandedEpisodesLogLine & episodesLogLine);
#endif



}

// Register metatypes to allow these types to be streamed in QTests.
Q_DECLARE_METATYPE(EpisodesParser::EpisodeList)
Q_DECLARE_METATYPE(EpisodesParser::Episode)

#endif // TYPEDEFS_H
