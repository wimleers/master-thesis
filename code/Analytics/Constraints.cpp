#include "Constraints.h"

namespace Analytics {

    Constraints::Constraints() {
        this->highestPreprocessedItemID = ROOT_ITEMID;
    }

    /**
     * Add an item constraint of a given constraint type. When
     * frequent itemsets are being generated, only those will be considered
     * that match the constraints defined here.
     *
     * @param item
     *   An item name.
     * @param type
     *   The constraint type.
     */
    void Constraints::addItemConstraint(ItemName item, ItemConstraintType type) {
        if (!this->itemConstraints.contains(type))
            this->itemConstraints.insert(type, QSet<ItemName>());
        this->itemConstraints[type].insert(item);
    }

    /**
     * Set the requirements for frequent itemset. Wildcards are allowed, e.g.
     * "episode:*" will match "episode:foo", "episode:bar", etc.
     *
     * Note: wilcard items will be expanded to their corresponding item ids in
     *       FPGrowth::scanTransactions().
     *
     * @param contraints
     *   A list of constraints.
     * @param type
     *   The item constraint type.
     */
    void Constraints::setItemConstraints(const QSet<ItemName> & constraints, ItemConstraintType type) {
        this->itemConstraints.insert(type, constraints);
    }

    /**
     * Get the item IDs for a given item constraint type. Clearly, this only
     * returns item IDs after all item IDs have been preprocessed.
     *
     * @param type
     *   An item constraint type to get all item IDs for.
     * @return
     *   All item IDs for the given item constraint type.
     */
    QSet<ItemID> Constraints::getItemIDsForConstraintType(ItemConstraintType type) const {
        QSet<ItemID> preprocessedItemIDs;

        foreach (ItemName key, this->preprocessedItemConstraints[type].keys())
            preprocessedItemIDs.unite(this->preprocessedItemConstraints[type][key]);

        return preprocessedItemIDs;
    }

    /**
     * Consider the given item ID -> name hash for use with constraints, but
     * only if its size exceeds the highest preprocessed item, or no items have
     * been preprocessed yet.
     *
     * This method is supposed to be used when all item IDs are already known.
     *
     * @param hash
     *   An item ID -> name hash.
     */
    void Constraints::preprocessItemIDNameHash(const ItemIDNameHash & hash) {
        if (this->highestPreprocessedItemID == ROOT_ITEMID || this->highestPreprocessedItemID + 1 < (uint) hash.size()) {
            this->clearPreprocessedItems();

            foreach (ItemID itemID, hash.keys()) {
                ItemName itemName = hash[itemID];
                this->preprocessItem(itemName, itemID);
            }
        }
    }

    /**
     * Consider the given item for use with constraints: store its item id in
     * an optimized data structure to allow for fast constraint checking
     * during frequent itemset generation.
     *
     * @param name
     *   An item name.
     * @param id
     *   The corresponding item ID.
     */
    void Constraints::preprocessItem(const ItemName & name, ItemID id) {
        QRegExp rx;
        rx.setPatternSyntax(QRegExp::Wildcard);

        // Store the item IDs that correspond to the wildcard item
        // constraints.
        ItemConstraintType constraintType;
        for (int i = CONSTRAINT_POSITIVE_MATCH_ALL; i <= CONSTRAINT_NEGATIVE_MATCH_ANY; i++) {
            constraintType = (ItemConstraintType) i;

            if (!this->itemConstraints.contains(constraintType))
                continue;

            foreach (ItemName constraint, this->itemConstraints[constraintType]) {
                // Map ItemNames to ItemIDs.
                if (constraint.compare(name) == 0) {
                    this->addPreprocessedItemConstraint(constraintType, "non-wildcards", id);
                }
                // Map ItemNames with wildcards in them to *all* corresponding
                // ItemIDs.
                else if (constraint.contains('*')) {
                    rx.setPattern(constraint);
                    if (rx.exactMatch(name))
                        this->addPreprocessedItemConstraint(constraintType, constraint, id);
                }
            }
        }

        // Always keep the highest preprocessed item ID.
        if (this->highestPreprocessedItemID == ROOT_ITEMID || id > this->highestPreprocessedItemID)
            this->highestPreprocessedItemID = id;
    }

    /**
     * Remove the given item id from the optimized constraint storage data
     * structure, because it is infrequent.
     *
     * @param id
     *   The item id to remove.
     */
    void Constraints::removeItem(ItemID id) {
        ItemConstraintType type;
        for (int i = CONSTRAINT_POSITIVE_MATCH_ALL; i <= CONSTRAINT_NEGATIVE_MATCH_ANY; i++) {
            type = (ItemConstraintType) i;

            if (!this->preprocessedItemConstraints.contains(type))
                continue;

            foreach (ItemName constraint, this->preprocessedItemConstraints[type].keys())
                this->preprocessedItemConstraints[type][constraint].remove(id);
        }
    }

    /**
     * Check if the given itemset matches the defined constraints.
     *
     * @param itemset
     *   An itemset to check the constraints for.
     * @return
     *   True if the itemset matches the constraints, false otherwise.
     */
    bool Constraints::matchItemset(const ItemIDList & itemset) const {
        for (int i = CONSTRAINT_POSITIVE_MATCH_ALL; i <= CONSTRAINT_NEGATIVE_MATCH_ANY; i++) {
            ItemConstraintType type = (ItemConstraintType) i;
            foreach (ItemName category, this->preprocessedItemConstraints[type].keys()) {
                if (!Constraints::matchItemsetHelper(itemset, type, this->preprocessedItemConstraints[type][category]))
                    return false;
            }
        }

        return true;
    }

    /**
     * Check if a particular frequent itemset search space will be able to
     * match the defined constraints. We can do this by matching all
     * constraints over the itemset *and* prefix paths support counts
     * simultaneously (since this itemset will be extended with portions of
     * the prefix paths).
     *
     * @param itemset
     *   An itemset to check the constraints for.
     * @param prefixPathsSupportCounts
     *   A list of support counts for the prefix paths in this search space.
     * @return
     *   True if the itemset matches the constraints, false otherwise.
     */
    bool Constraints::matchSearchSpace(const ItemIDList & frequentItemset, const QHash<ItemID, SupportCount> & prefixPathsSupportCounts) const {
        for (int i = CONSTRAINT_POSITIVE_MATCH_ALL; i <= CONSTRAINT_NEGATIVE_MATCH_ANY; i++) {
            ItemConstraintType type = (ItemConstraintType) i;
            foreach (ItemName category, this->preprocessedItemConstraints[type].keys()) {
                if (!Constraints::matchSearchSpaceHelper(frequentItemset, prefixPathsSupportCounts, type, this->preprocessedItemConstraints[type][category]))
                    return false;
            }
        }

        return true;
    }


    //------------------------------------------------------------------------
    // Protected methods.

    /**
     * Helper function for Constraints::matchItemSet().
     */
    bool Constraints::matchItemsetHelper(const ItemIDList & itemset, ItemConstraintType type, const QSet<ItemID> & constraintItems) {
        foreach (ItemID id, constraintItems) {
            switch (type) {
            case CONSTRAINT_POSITIVE_MATCH_ALL:
                if (!itemset.contains(id))
                    return false;
                break;

            case CONSTRAINT_POSITIVE_MATCH_ANY:
                if (itemset.contains(id))
                    return true;
                break;

            case CONSTRAINT_NEGATIVE_MATCH_ALL:
                if (itemset.contains(id))
                    return false;
                break;

            case CONSTRAINT_NEGATIVE_MATCH_ANY:
                if (!itemset.contains(id))
                    return true;
                break;
            }
        }

        // In case we haven't returned yet: in the case of the "all matches",
        // this is a good thing, since we haven't had any bad encounters.
        // Hence we return true for those. For the "any matches", it's the
        // other way around.
        switch (type) {
        case CONSTRAINT_POSITIVE_MATCH_ALL:
        case CONSTRAINT_NEGATIVE_MATCH_ALL:
            return true;
            break;

        case CONSTRAINT_POSITIVE_MATCH_ANY:
        case CONSTRAINT_NEGATIVE_MATCH_ANY:
            return false;
            break;
        }

        // Satisfy the compiler.
        return false;
    }

    /**
     * Helper function for Constraints::matchSearchSpace().
     */
    bool Constraints::matchSearchSpaceHelper(const ItemIDList & frequentItemset, const QHash<ItemID, SupportCount> & prefixPathsSupportCounts, ItemConstraintType type, const QSet<ItemID> & constraintItems) {
        foreach (ItemID id, constraintItems) {
            switch (type) {
            case CONSTRAINT_POSITIVE_MATCH_ALL:
                if (!frequentItemset.contains(id) && prefixPathsSupportCounts[id] == 0)
                    return false;
                break;

            case CONSTRAINT_POSITIVE_MATCH_ANY:
                if (frequentItemset.contains(id) || prefixPathsSupportCounts[id] > 0)
                    return true;
                break;

            case CONSTRAINT_NEGATIVE_MATCH_ALL:
                if (prefixPathsSupportCounts[id] > 0)
                    return false;
                break;

            case CONSTRAINT_NEGATIVE_MATCH_ANY:
                if (prefixPathsSupportCounts[id] == 0)
                    return true;
                break;
            }
        }

        // In case we haven't returned yet: in the case of the "all matches",
        // this is a good thing, since we haven't had any bad encounters.
        // Hence we return true or those. For the "any matches", it's the
        // other way around.
        switch (type) {
        case CONSTRAINT_POSITIVE_MATCH_ALL:
        case CONSTRAINT_NEGATIVE_MATCH_ALL:
            return true;
            break;

        case CONSTRAINT_POSITIVE_MATCH_ANY:
        case CONSTRAINT_NEGATIVE_MATCH_ANY:
            return false;
            break;
        }

        // Satisfy the compiler.
        return false;
    }

    /**
     * Store a preprocessed item constraint in the optimized constraint data
     * structure.
     *
     * @param type
     *   The item constraint type.
     * @param category
     *   The category, either "non-wildcards" or a constraint that contains a
     *   wildcard ('*').
     * @param id
     *   The item id.
     */
    void Constraints::addPreprocessedItemConstraint(ItemConstraintType type, const ItemName & category, ItemID id) {
        if (!this->preprocessedItemConstraints.contains(type))
            this->preprocessedItemConstraints.insert(type, QHash<ItemName, QSet<ItemID> >());
        if (!this->preprocessedItemConstraints[type].contains(category))
            this->preprocessedItemConstraints[type].insert(category, QSet<ItemID>());
        this->preprocessedItemConstraints[type][category].insert(id);
    }
}
