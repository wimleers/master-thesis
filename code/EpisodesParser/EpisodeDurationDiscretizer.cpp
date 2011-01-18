#include "EpisodeDurationDiscretizer.h"

namespace EpisodesParser {
    EpisodeDurationDiscretizer::EpisodeDurationDiscretizer() {
    }

    bool EpisodeDurationDiscretizer::parseCsvFile(const QString & csvFile) {
        this->csvFile = csvFile;

        QFile csv(this->csvFile);
        if (!csv.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qCritical("Could not open '%s' file for reading: %s.", qPrintable(this->csvFile), qPrintable(csv.errorString()));
            exit(1);
        }
        else {
            QTextStream in(&csv);
            QStringList parts;
            EpisodeName episodeName;
            EpisodeSpeed episodeSpeed;
            EpisodeDuration maxDuration;

            while (!in.atEnd()) {
                parts = in.readLine().split(',');
                episodeName = parts[0];

                // Build the hierarchical map:
                // EpisodeName -> EpisodeSpeed -> max duration for this speed.
                QMap<EpisodeDuration, EpisodeSpeed> map;
                this->thresholds.insert(episodeName, map);
                for (int i = 1; i < parts.length(); i += 2) {
                    episodeSpeed = parts[i];
                    if (i < parts.length() - 1)
                        maxDuration = parts[i+1].toInt();
                    else
                        maxDuration = -1; // This will automatically map to the highest value supported, right now that is 65535.
                    this->thresholds[episodeName].insert(maxDuration, episodeSpeed);
                }
            }

            return true;
        }
    }

    EpisodeSpeed EpisodeDurationDiscretizer::mapToSpeed(const EpisodeName & name, const EpisodeDuration & duration) const {
        EpisodeDuration maxDuration;
        foreach (maxDuration, this->thresholds[name].keys()) {
            if (duration <= maxDuration)
                return this->thresholds[name][maxDuration];
        }

        qCritical("The duration %d for the Episode '%s' could not be mapped to a discretized speed.", duration, qPrintable(name));
        return "satisfy the compiler";
    }
}
