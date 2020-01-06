#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

/*==========================================================================*/

/**
 * @brief Contains all properties of a signal group.
 *
 * The signal group allows reverting changes, if the
 * editing was canceled. If further properties are required
 * for a signal group, add them in this class using the following
 * pattern:
 *
 * @code
 * template<typename T>
 * T GetValue() {
 *   return value_;
 * }
 *
 * template<typename T>
 * T GetOrigValue() {
 *   return origValue_;
 * }
 *
 * template<typename T>
 * void SetValue(T value) {    
 *   if (!editable_) {
 *     origValue_ = value;
 *   }
 *   value_ = value;
 * }
 * @endcode
 *
 * You should also add default values for the new properties. This
 * requires updating the SetDefault method.
 */
class SignalGroup
{
public:
  SignalGroup(long nr);
  SignalGroup(long nr, bool editable);

  long GetNr() {
    return nr_;
  }
  long GetOrigNr() {
    return origNr_;
  }
  std::string GetName(){
    return name_;
  }
  unsigned int GetAmber() {
    return amber_;
  }
  unsigned int GetRed() {
    return red_;
  }
  unsigned int GetMinRed() {
    return minRed_;
  }
  unsigned int GetGreen() {
    return green_;
  }
  unsigned int GetMinGreen() {
    return minGreen_;
  }

  void SetNr(long nr) {    
    if (!editable_) {
      origNr_ = nr;
    }
    nr_ = nr;
  }
  void SetName(std::string & name) {
    if (!editable_) {
      origName_ = name;
    }
    name_ = name;
  }
  void SetAmber(unsigned int amber) {
    if (!editable_) {
      origAmber_ = amber;
    }
    amber_ = amber;
  }
  void SetRed(unsigned int red) {
    if (!editable_) {
      origRed_ = red;
    }
    red_ = red;
  }
  void SetMinRed(unsigned int minRed) {
    if (!editable_) {
      origMinRed_ = minRed;
    }
    minRed_ = minRed;
  }
  void SetGreen(unsigned int green) {
    if (!editable_) {
     origGreen_ = green;
    }
    green_ = green;
  }
  void SetMinGreen(unsigned int minGreen) {
    if (!editable_) {
      origMinGreen_ = minGreen;
    }
    minGreen_ = minGreen;
  }

  /**
   * @brief Compares two signal groups using the signal group number.
   *
   * @param The signal group to compare with.
   *
   * @return <code>true</code>, if both object equal, otherwise <code>false</code>.
   */
  bool operator== (SignalGroup* group) {
    return nr_ == group->nr_;
  }

  /**
   * @brief Sets, whether the signal group is edited or not.
   * 
   * @param editable Whether the signal group is edited or not.
   */
  void EditModus(bool editable);

  /**
   * @brief Undo all made changes.
   *
   * The function reverts all changes made on the signal group.
   * Update this function, if you require additional properties.
   *
   */
  void Revert(void);

  /**
   * @brief Sets all values to the default.
   *
   * Update this function, if you require additional properties.
   */
  void SetDefault(void);

private:
  bool editable_;
  long nr_;
  long origNr_;
  std::string name_;
  std::string origName_;
  unsigned int amber_;
  unsigned int origAmber_;
  unsigned int red_;
  unsigned int origRed_;
  unsigned int minRed_;
  unsigned int origMinRed_;
  unsigned int green_;
  unsigned int origGreen_;
  unsigned int minGreen_;
  unsigned int origMinGreen_;
};

typedef std::map<long, SignalGroup*> SignalGroups;

/*==========================================================================*/

/**
 * @brief Contains all properties of a signal control including the
 *        signal groups.
 *
 * Add here additional properties for the signal control using
 * the pattern described in the SignalGroup class.
 */
class SignalControl
{
public:
  SignalControl(long nr);
  virtual ~SignalControl();

  std::string GetFileName() {
    return fileName_;
  }

  long GetNumber() {
    return scNr_;
  }

  std::string & FileName() {
    return fileName_;
  }

  SignalGroups & SignalGroups() {
    return groups_;
  }

  std::vector<SignalGroup*> & NewGroups() {
    return newGroups_;
  }

  /**
   * @brief Sets, whether the signal group is edited or not.
   *
   * All signal groups are also marked as edited.
   * 
   * @param editable Whether the signal group is edited or not.
   */
  void EditModus(bool editable);

  /**
   * @brief Adds a new signal group with the given id.
   *
   * An existing signal group with the id is erased, before
   * the new one is added.
   *
   * @param sgNo The signal group number.
   */
  void AddSignalGroup(long sgNo);

  /**
   * @brief Removes an existing signal group with the given id.
   *
   * If the signal control is in the edit mode, the signal group
   * is not really removed but marked as removed.
   *
   * @param sgNo The signal group number.
   */
  void RemoveSignalGroup(long sgNo);

  /**
   * @brief Gets the signal group with the given id.
   *
   * @param sgNo The signal group number.
   * @param includeDeleted Whether already deleted signal 
   *                       groups should be considered or not.
   */
  SignalGroup* GetSignalGroup(long sgNo, bool includeDeleted = false);

  /**
   * @brief Changes the number of the given signal group.
   *
   * @param oldSgNo The old signal group number.
   * @param newSgNo The new signal group number.
   *
   * @return <code>true</code> if the renaming succeeded, otherwise <code>false</code>.
   */
  bool RenameSignalGroup(long oldSgNo, long newSgNo);

  /**
   * @brief Makes the changes permanent, if the signal group was in edit mode before.
   */
  void ComputeUpdatedSignalGroups(void);

  /**
   * @brief Removes all signal groups.
   */
  void ClearSignalGroups(void);

  /**
   * @brief Reverts all changed made on the signal groups.
   */
  void RevertSignalGroups(void);

private:
  bool editable_;
  long scNr_;
  std::string fileName_;
  std::vector<SignalGroup*> newGroups_;
  std::vector<SignalGroup*> changedGroups_;
  std::vector<SignalGroup*> deletedGroups_;
  std::map<long, SignalGroup*> groups_;
};

typedef std::map<long, SignalControl* > SignalControls;

/*==========================================================================*/