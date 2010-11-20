#ifdef DEBUG

#include "typedefs.h"


namespace EpisodesParser {

    QDebug operator<<(QDebug dbg, const Episode & e) {
        dbg.nospace() << e.episodeIDNameHash->value(e.id).toStdString().c_str()
                      << "("
                      << e.id
                      << ") = "
                      << e.duration;

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
        dbg.nospace() << "{\n"
                      << "ip = " << line.ip << ", \n"
                      << "time = " << line.time << ", \n"
                      << "episodes = " << line.episodes << ", \n"
                      << ", more to come!\n"
                      << "}";

        return dbg.nospace();
    }

}
#endif
