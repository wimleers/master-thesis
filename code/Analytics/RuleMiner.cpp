#include "RuleMiner.h"

namespace Analytics {

    //------------------------------------------------------------------------
    // Public static methods.

    /**
     * An exact implementation of algorithm 6.2 on page 351 in the textbook.
     */
    QList<AssociationRule> RuleMiner::mineAssociationRules(QList<FrequentItemset> frequentItemsets, Confidence minimumConfidence, const Constraints & ruleConsequentConstraints, const FPGrowth * fpgrowth) {
        QList<AssociationRule> associationRules;
        QList<ItemIDList> consequents;
        bool hasConstraints = !ruleConsequentConstraints.empty();

        // Iterate over all frequent itemsets.
        foreach (FrequentItemset frequentItemset, frequentItemsets) {
            // It's only possible to generate an association rule if there are at
            // least two items in the frequent itemset.
            if (frequentItemset.itemset.size() >= 2) {
                // Generate all 1-item consequents.
                consequents.clear();
                foreach (ItemID itemID, frequentItemset.itemset) {
                    ItemIDList consequent;
                    consequent.append(itemID);

                    // Store this consequent whenever no constraints are
                    // defined, or when constraints are defined and the
                    // consequent matches the constraints.
                    if (!hasConstraints || ruleConsequentConstraints.matchItemset(consequent))
                        consequents.append(consequent);
                }

#ifdef RULEMINER_DEBUG
                qDebug() << "Generating rules for frequent itemset" << frequentItemset << " and consequents " << consequents;
#endif

                // Attempt to generate association rules for this frequent
                // itemset and store the results.
                associationRules.append(RuleMiner::generateAssociationRulesForFrequentItemset(frequentItemset, consequents, minimumConfidence, fpgrowth));
            }
        }
        return associationRules;
    }

    QList<AssociationRule> RuleMiner::mineAssociationRules(QList<FrequentItemset> frequentItemsets, Confidence minimumConfidence, const Constraints & ruleConsequentConstraints, const PatternTree & patternTree, uint from, uint to) {
        QList<AssociationRule> associationRules;
        QList<ItemIDList> consequents;
        bool hasConstraints = !ruleConsequentConstraints.empty();

        // Iterate over all frequent itemsets.
        foreach (FrequentItemset frequentItemset, frequentItemsets) {
            // It's only possible to generate an association rule if there are at
            // least two items in the frequent itemset.
            if (frequentItemset.itemset.size() >= 2) {
                // Generate all 1-item consequents.
                consequents.clear();
                foreach (ItemID itemID, frequentItemset.itemset) {
                    ItemIDList consequent;
                    consequent.append(itemID);

                    // Store this consequent whenever no constraints are
                    // defined, or when constraints are defined and the
                    // consequent matches the constraints.
                    if (!hasConstraints || ruleConsequentConstraints.matchItemset(consequent))
                        consequents.append(consequent);
                }

#ifdef RULEMINER_DEBUG
                qDebug() << "Generating rules for frequent itemset" << frequentItemset << " and consequents " << consequents;
#endif

                // If no valid consequents could be found (due to none if the
                // items matching the constraints), then don't attempt to
                // generate association rules.
                if (consequents.isEmpty())
                    continue;

                // Attempt to generate association rules for this frequent
                // itemset and store the results.
                associationRules.append(RuleMiner::generateAssociationRulesForFrequentItemset(frequentItemset, consequents, minimumConfidence, patternTree, from, to));
            }
        }
        return associationRules;
    }


    //------------------------------------------------------------------------
    // Protected static methods.

    /**
     * A.k.a. "ap-genrules", but slightly different to fix a bug in that
     * algorithm: it accepts consequents of size 1, but doesn't generate
     * antecedents for these, instead it immediately generates consequents of
     * size 2. Algorithm 6.3 on page 352 in the textbook.
     * This variation of that algorithm fixes that.
     */
    QList<AssociationRule> RuleMiner::generateAssociationRulesForFrequentItemset(FrequentItemset frequentItemset, QList<ItemIDList> consequents, Confidence minimumConfidence, const FPGrowth * fpgrowth) {
        Q_ASSERT_X(consequents.size() > 0, "RuleMiner::generateAssociationRulesForFrequentItemset", "List of consequents may not be empty.");

        QList<AssociationRule> associationRules;
        SupportCount antecedentSupportCount;
        ItemIDList antecedent;
        Confidence confidence;
        unsigned int k = frequentItemset.itemset.size(); // Size of the frequent itemset.
        unsigned int m = consequents[0].size(); // Size of a single consequent.

        // Iterate over all given consequents.
        foreach (ItemIDList consequent, consequents) {
            // Get the antecedent for the current consequent, so we
            // effectively get a candidate association rule.
            antecedent = RuleMiner::getAntecedent(frequentItemset.itemset, consequent);
            antecedentSupportCount = fpgrowth->calculateSupportCount(antecedent);

            // If the confidence is sufficiently high, we've found an
            // association rule that meets our requirements.
            confidence = 1.0 * frequentItemset.support / antecedentSupportCount;
#ifdef RULEMINER_DEBUG
            qDebug () << "confidence" << confidence << ", frequent itemset support" << frequentItemset.support << ", antecedent support" << antecedentSupportCount << ", antecedent" << antecedent << ", consequent" << consequent;
#endif
            if (confidence >= minimumConfidence) {
                AssociationRule rule(antecedent, consequent, frequentItemset.support, confidence);
#ifdef DEBUG
                rule.IDNameHash = frequentItemset.IDNameHash;
#endif
                associationRules.append(rule);
            }
            // If the confidence is not sufficiently high, delete this
            // consequent, to prevent other consequents to be generated that
            // build upon this one since those consequents would have the same
            // insufficiently high confidence in the best case.
            else
                consequents.removeAll(consequent);
        }

        // If there are still consequents left (i.e. consequents with a
        // sufficiently high confidence), and the size of the consequents
        // still alows them to be expanded, expand the consequents and attempt
        // to generate more association rules with them.
        if (consequents.size() >= 2 && k > m + 1) {
            QList<ItemIDList> candidateConsequents = RuleMiner::generateCandidateItemsets(consequents);
            associationRules.append(RuleMiner::generateAssociationRulesForFrequentItemset(frequentItemset, candidateConsequents, minimumConfidence, fpgrowth));
        }

        return associationRules;
    }

    QList<AssociationRule> RuleMiner::generateAssociationRulesForFrequentItemset(FrequentItemset frequentItemset, QList<ItemIDList> consequents, Confidence minimumConfidence, const PatternTree & patternTree, uint from, uint to) {
        Q_ASSERT_X(consequents.size() > 0, "RuleMiner::generateAssociationRulesForFrequentItemset", "List of consequents may not be empty.");

        QList<AssociationRule> associationRules;
        SupportCount antecedentSupportCount;
        ItemIDList antecedent;
        Confidence confidence;
        unsigned int k = frequentItemset.itemset.size(); // Size of the frequent itemset.
        unsigned int m = consequents[0].size(); // Size of a single consequent.

        // Iterate over all given consequents.
        foreach (ItemIDList consequent, consequents) {
            // Get the antecedent for the current consequent, so we
            // effectively get a candidate association rule.
            antecedent = RuleMiner::getAntecedent(frequentItemset.itemset, consequent);

            // Calculate the support for the frequent itemsets over the given
            // range.
            TiltedTimeWindow * ttw = patternTree.getPatternSupport(antecedent);
            Q_ASSERT(ttw != NULL);
            antecedentSupportCount = ttw->getSupportForRange(from, to);
            confidence = 1.0 * frequentItemset.support / antecedentSupportCount;

            // If the confidence is sufficiently high, we've found an
            // association rule that meets our requirements.
#ifdef RULEMINER_DEBUG
            qDebug () << "confidence" << confidence << ", frequent itemset support" << frequentItemset.support << ", antecedent support" << antecedentSupportCount << ", antecedent" << antecedent << ", consequent" << consequent;
#endif

            if (confidence >= minimumConfidence) {
                AssociationRule rule(antecedent, consequent, frequentItemset.support, confidence);
#ifdef DEBUG
                rule.IDNameHash = frequentItemset.IDNameHash;
#endif
                associationRules.append(rule);
            }
            // If the confidence is not sufficiently high, delete this
            // consequent, to prevent other consequents to be generated that
            // build upon this one since those consequents would have the same
            // insufficiently high confidence in the best case.
            else
                consequents.removeAll(consequent);
        }

        // If there are still consequents left (i.e. consequents with a
        // sufficiently high confidence), and the size of the consequents
        // still alows them to be expanded, expand the consequents and attempt
        // to generate more association rules with them.
        if (consequents.size() >= 2 && k > m + 1) {
            QList<ItemIDList> candidateConsequents = RuleMiner::generateCandidateItemsets(consequents);
            associationRules.append(RuleMiner::generateAssociationRulesForFrequentItemset(frequentItemset, candidateConsequents, minimumConfidence, patternTree, from, to));
        }

        return associationRules;
    }

    /**
     * Build the antecedent for this candidate consequent, which are all items
     * in the frequent itemset except for those in the candidate consequent.
     */
    ItemIDList RuleMiner::getAntecedent(const ItemIDList & frequentItemset, const ItemIDList & consequent) {
        ItemIDList antecedent;
        foreach (ItemID itemID, frequentItemset)
            if (!consequent.contains(itemID))
                antecedent.append(itemID);
        return antecedent;
    }

    /**
     * A.k.a. "apriori-gen", but without pruning because we're already working
     * with frequent itemsets, which were generated by the FP-growth
     * algorithm. We solely need this function for association rule mining,
     * not for frequent itemset generation.
     */
    QList<ItemIDList> RuleMiner::generateCandidateItemsets(const QList<ItemIDList> & frequentItemsubsets) {
        // Phase 1: candidate generation.
        QList<ItemIDList> candidateItemsets;
        int allButOne = frequentItemsubsets[0].size() - 1;
        foreach (ItemIDList frequentItemsubset, frequentItemsubsets) {
            foreach (ItemIDList otherFrequentItemsubset, frequentItemsubsets) {
                if (allButOne == 0) {
                    if (frequentItemsubset[0] == otherFrequentItemsubset[0]) {
                        break;
                    }
                }
                else {
                    // Iterate over all but the last item in this frequent
                    // itemsubset.
                    for (int i = 0; i < allButOne; i++) {
                        if (frequentItemsubset[i] != otherFrequentItemsubset[i])
                            break;
                    }
                }

                // The first all-but-one items of the two frequent itemsubsets
                // matched! Now generate a candidate itemset, whose:
                // - first k-1 == allButOne items are copied from the first
                //   frequent itemsubset (outer)
                // - last (k == allButOne + 1) item is copied from the second
                //   frequent itemsubset (inner)
                ItemIDList candidateItemset;
                candidateItemset.append(frequentItemsubset);
                candidateItemset.append(otherFrequentItemsubset[allButOne]);
                // Store this candidate set.
                candidateItemsets.append(candidateItemset);
            }
        }

        // Phase 2: candidate pruning.
        // Not necessary!

        return candidateItemsets;
    }
}
