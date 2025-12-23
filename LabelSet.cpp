/**
 * @class LabelSet
 * @brief Manages mappings between labels and program counters for an assembler or similar tool.
 *
 * This class provides functionality to associate string labels with integer program counters,
 * track label definitions, and perform lookups in both directions.
 *
 * Methods:
 * - void clear(): Clears all label mappings and definitions.
 * - std::string getLabel(int programCounter, bool labelDef) const:
 *      Retrieves the label at the specified program counter, optionally only if it's a label definition.
 * - void setLabel(int programCounter, std::string label, bool labelDef):
 *      Associates a label with a program counter and optionally marks it as a label definition.
 * - int getProgramCounter(std::string label) const:
 *      Returns the program counter associated with a given label, or -1 if not found.
 * - bool isPresent(std::string label) const:
 *      Checks if a label exists in the mapping.
 * - bool isPresent(int programCounter) const:
 *      Checks if a program counter has an associated label.
 *
*/
#include "LabelSet.h"

void LabelSet::clear() {
        // Clear all label mappings
        labeltoAddress.clear();
        addressToLabels.clear();
        labelDefinitions.clear();
}

// Returns label at programCounter if it exists; "" otherwise
std::string LabelSet::getLabel(int programCounter, bool labelDef) const {
    if (addressToLabels.count(programCounter)) {
        if (!labelDef || (labelDef && labelDefinitions.count(programCounter) && labelDefinitions.at(programCounter))) {
            return addressToLabels.at(programCounter);
        }
    }
    return "";
}


// Sets label or label definition at programCounter
// labelDefintion is used to check if a label is a label definition
void LabelSet::setLabel(int programCounter, std::string label, bool labelDef)  {
    addressToLabels[programCounter] = label;
    labeltoAddress[label] = programCounter;
    if (labelDef) {
        labelDefinitions[programCounter] = true;
    }

}

// Returns programCounter of a label if it exists
int LabelSet::getProgramCounter(std::string label) const {
    for (const auto& pair : addressToLabels) {
        if (pair.second == label) {
            return pair.first;
        }
    }
    return -1;
}

// Check if a label exists given the label name
bool LabelSet::isPresent(std::string label) const {
    return labeltoAddress.count(label) > 0;
}


// Check if a label exists given the programCounter
 bool LabelSet::isPresent(int programCounter) const {
    return addressToLabels.count(programCounter) > 0;
 }


 const std::unordered_map<int, std::string>& LabelSet::getAllLabels() const {
    return addressToLabels;
}