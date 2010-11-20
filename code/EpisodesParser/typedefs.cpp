#ifdef DEBUG

#include "typedefs.h"


namespace EpisodesParser {

    QDebug operator<<(QDebug dbg, const NamedEpisode & namedEpisode) {
        dbg.nospace() << namedEpisode.lookup->value(namedEpisode.episode.id).toStdString().c_str()
                      << "("
                      << namedEpisode.episode.id
                      << ") = "
                      << namedEpisode.episode.duration;

        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const NamedEpisodeList & namedEpisodeList) {
        QString episodeOutput;
        NamedEpisode tmp;
        tmp.lookup = namedEpisodeList.lookup;

        //dbg.nospace() << "[size=" << namedEpisodeList.episodes.size() << "] ";
        dbg.nospace() << "{";

        for (int i = 0; i < namedEpisodeList.episodes.size(); i++) {
            if (i > 0)
                dbg.nospace() << ", ";

            // Generate output for episode.
            tmp.episode = namedEpisodeList.episodes[i];
            episodeOutput.clear();
            QDebug(&episodeOutput) << tmp;

            dbg.nospace() << episodeOutput.toStdString().c_str();
        }
        dbg.nospace() << "}";

        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const EpisodesLogLine & episodesLogLine) {
        NamedEpisodeList nel;
        nel.episodes = episodesLogLine.episodes;
        nel.lookup = episodesLogLine.episodeIDNameHash;

        dbg.nospace() << "{\n"
                      << "ip = " << episodesLogLine.ip << ", \n"
                      << "time = " << episodesLogLine.time << ", \n"
                      << "episodes = " << nel << ", \n"
                      << ", more to come!\n"
                      << "}";

        return dbg.nospace();
    }

}
#endif
