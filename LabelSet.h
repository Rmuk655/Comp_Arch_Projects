#ifndef LABELSET_H
#define LABELSET_H

#include <string>
#include <unordered_map>


class LabelSet {
public:
    // Returns label at programCounter if it exists; nullptr otherwise
    // labelDefintion is used to check if a label is a label definition
    std::string getLabel(int programCounter,bool labelDef=false) const;
    // Sets label or label defintion at programCounter 
    void  setLabel(int programCounter, std::string label, bool labelDef = false) ;

    // Returns programCounter of a label if it exists
    int getProgramCounter(std::string label) const;
   // Check if a label exists given the label name
    bool isPresent(std::string label) const;
   // Check if a label exists given the programCounter
    bool isPresent(int programCounter) const;
    void clear() ;
 
    const std::unordered_map<int, std::string>& getAllLabels() const;
    // address → name (reverse map)
    std::unordered_map<int, std::string> addressToLabels;  
private:
    // label name to line index
    mutable std::unordered_map<std::string, int> labeltoAddress; 
    // These are label definitions needed only for round tripping from assembler to deassembler
    std::unordered_map<int, bool> labelDefinitions;  

};

#endif // LABELSET_H
