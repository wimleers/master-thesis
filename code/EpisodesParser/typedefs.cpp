#include "typedefs.h"

#ifdef DEBUG
namespace EpisodesParser {

    QDebug operator<<(QDebug dbg, const EpisodesLogLine & episodesLogLine) {
        dbg.nospace() << "{"
                      << "ip = " << episodesLogLine.ip
                      << ", time = " << episodesLogLine.time
                      << ", more to come!"
                      << "}";

        return dbg.nospace();
    }

}
#endif
