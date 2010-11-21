// Source: https://github.com/wimleers/QCachingLocale

#ifndef QCACHINGLOCALE_H
#define QCACHINGLOCALE_H

#include <QSystemLocale>
#include <QVariant>
#include <QMutex>

class QCachingLocale : public QSystemLocale {
public:
    virtual QVariant query(QueryType type, QVariant in) const {
        int inAsInt;
        switch(type) {
            case QSystemLocale::LanguageId:
            case QSystemLocale::CountryId:
            case QSystemLocale::DecimalPoint:
            case QSystemLocale::GroupSeparator:
            case QSystemLocale::ZeroDigit:
            case QSystemLocale::NegativeSign:
            case QSystemLocale::PositiveSign:
            case QSystemLocale::DateFormatLong:
            case QSystemLocale::DateFormatShort:
            case QSystemLocale::TimeFormatLong:
            case QSystemLocale::DateTimeFormatLong:
            case QSystemLocale::DateTimeFormatShort:
            case QSystemLocale::MeasurementSystem:
            case QSystemLocale::AMText:
            case QSystemLocale::PMText:
                // in = empty
                if (!this->cacheInNone.contains(type)) {
                    QCachingLocale::inNoneMutex.lock();
                    this->cacheInNone[type] = QSystemLocale::query(type, in);
                    QCachingLocale::inNoneMutex.unlock();
                }
                return this->cacheInNone[type];
            case QSystemLocale::TimeFormatShort:
            case QSystemLocale::DayNameLong:
            case QSystemLocale::DayNameShort:
            case QSystemLocale::MonthNameLong:
            case QSystemLocale::MonthNameShort:
                // in = int
                inAsInt = in.toInt();
                if (!this->cacheInInt.contains(type) || !this->cacheInInt[type].contains(inAsInt)) {
                    QCachingLocale::inIntMutex.lock();
                    this->cacheInInt[type][inAsInt] = QSystemLocale::query(type, in);
                    QCachingLocale::inIntMutex.unlock();
                }
                return this->cacheInInt[type][inAsInt];
            case QSystemLocale::DateToStringLong:
            case QSystemLocale::DateToStringShort:
                // in = QDate
            case QSystemLocale::TimeToStringLong:
            case QSystemLocale::TimeToStringShort:
                // in = QTime
            case QSystemLocale::DateTimeToStringLong:
            case QSystemLocale::DateTimeToStringShort:
                // in = QDateTime
                // QDate, QTime, QDateTime can be converted to unix timestamps.
                inAsInt = in.toDateTime().toTime_t();
                if (!this->cacheInInt.contains(type) || !this->cacheInInt[type].contains(inAsInt)) {
                    QCachingLocale::inIntMutex.lock();
                    this->cacheInInt[type][inAsInt] = QSystemLocale::query(type, in);
                    QCachingLocale::inIntMutex.unlock();
                }
                return this->cacheInInt[type][inAsInt];
            default:
                // In Qt 4.7, the above cases should cover *all* cases. This
                // default case only exists to ensure that nothing will break
                // in the future.
                return QSystemLocale::query(type, in);
        }
    }
private:
    // Mutexes are necessary to make the query method thread-safe (they are
    // used to serialize access for insertions to the two QHashes).
    static QMutex inNoneMutex;
    static QMutex inIntMutex;

    // Cache for QSystemLocale queries that don't use the 'in' parameter.
    mutable QHash<QueryType, QVariant> cacheInNone;
    // Cache for QSystemLocale queries that use an 'in' parameter that is either
    // of type int, or can be converted to that type.
    mutable QHash<QueryType, QHash<int, QVariant> > cacheInInt;
};

QMutex QCachingLocale::inNoneMutex;
QMutex QCachingLocale::inIntMutex;

#endif // QCACHINGLOCALE_H
