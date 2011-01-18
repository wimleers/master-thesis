#ifdef DEBUG

#include "typedefs.h"


namespace EpisodesParser {

    QDebug operator<<(QDebug dbg, const Episode & e) {
        dbg.nospace() << e.IDNameHash->value(e.id).toStdString().c_str()
                      << "("
                      << e.id
                      << ") = "
                      << e.duration;

        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const Domain & d) {
        dbg.nospace() << d.IDNameHash->value(d.id).toStdString().c_str()
                      << "(" << d.id << ")";

        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const EpisodeList & el) {
        QString episodeOutput;

        //dbg.nospace() << "[size=" << namedEpisodeList.episodes.size() << "] ";
        dbg.nospace() << "{";

        for (int i = 0; i < el.size(); i++) {
            if (i > 0)
                dbg.nospace() << ", ";

            // Generate output for episode.
            episodeOutput.clear();
            QDebug(&episodeOutput) << el[i];

            dbg.nospace() << episodeOutput.toStdString().c_str();
        }
        dbg.nospace() << "}";

        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const EpisodesLogLine & line) {
        const static char * eol = ", \n";

        dbg.nospace() << "{\n"
                      << "IP = " << line.ip << eol
                      << "time = " << line.time << eol
                      << "episodes = " << line.episodes << eol
                      << "status = " << line.status << eol
                      << "URL = " << line.url << eol
                      << "user-agent = " << line.ua << eol
                      << "domain = " << line.domain << eol
                      << "}";

        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const IPHierarchy & ip) {
        dbg.nospace() << ip.ip
                      << " ("
                      << ip.continent << " > " << ip.country << " > " << ip.city
                      << ", " << ip.isp
                      << ")";
        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const UAHierarchyDetails & ua) {
        dbg.nospace() << ua.browser_name.toStdString().c_str() << " " << ua.browser_version.toStdString().c_str()
                      << " (" << ua.browser_version_major << ", " << ua.browser_version_minor << ")"
                      << " on " << ua.platform.toStdString().c_str();
        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const ExpandedEpisodesLogLine & line) {
        const static char * eol = ", \n";

        dbg.nospace() << "{\n"
                      << "IP = " << line.ip << eol
                      << "time = " << line.time << eol
                      << "episodes = " << line.episodes << eol
                      << "status = " << line.status << eol
                      << "URL = " << line.url << eol
                      << "user-agent = " << line.ua << eol
                      << "}";

        return dbg.nospace();
    }
}
#endif
