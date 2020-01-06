#include "SignalGroup.h"

/*==========================================================================*/

SignalGroup::SignalGroup(long nr)
: nr_(nr)
, origNr_(nr)
, editable_(false)
, name_("")
, origName_("")
, amber_(2)
, origAmber_(2)
, red_(0)
, origRed_(0)
, minRed_(0)
, origMinRed_(0)
, green_(0)
, origGreen_(0)
, minGreen_(0)
, origMinGreen_(0)
{
}

/*--------------------------------------------------------------------------*/

SignalGroup::SignalGroup(long nr, bool editable)
: nr_(nr)
, origNr_(editable? 0: nr)
, editable_(editable)
, name_("")
, origName_("")
, amber_(2)
, origAmber_(2)
, red_(0)
, origRed_(0)
, minRed_(0)
, origMinRed_(0)
, green_(0)
, origGreen_(0)
, minGreen_(0)
, origMinGreen_(0)
{
}

/*--------------------------------------------------------------------------*/

void SignalGroup::EditModus(bool editable)
{
  editable_ = editable;
}

/*--------------------------------------------------------------------------*/

void SignalGroup::Revert(void)
{
  nr_ = origNr_;
  name_ = origName_;
  amber_ = origAmber_;
  red_ = origRed_;
  minRed_ = origMinRed_;
  green_ = origGreen_;
  minGreen_ = origMinGreen_;
}

/*--------------------------------------------------------------------------*/

void SignalGroup::SetDefault(void)
{
  name_ = "";
  amber_ = 0;
  red_ = 0;
  minRed_ = 0;
  green_ = 0;
  minGreen_ = 0;
}

/*==========================================================================*/

SignalControl::SignalControl(long scNr)
: scNr_(scNr)
, editable_(false)
{

}

/*--------------------------------------------------------------------------*/

SignalControl::~SignalControl()
{
  ClearSignalGroups();
}

/*--------------------------------------------------------------------------*/

void SignalControl::ComputeUpdatedSignalGroups(void)
{
  for (SignalGroups::iterator groupIter = groups_.begin();
       groupIter != groups_.end();
       ++groupIter) {
    SignalGroup* group = groupIter->second;
    if (group->GetOrigNr() == 0) {
      // new signal group
      newGroups_.push_back(group);
      continue;
    }

    if (group->GetNr() == 0) {
      // deleted signal group
      deletedGroups_.push_back(group);
      continue;
    }

    if (group->GetNr() != group->GetOrigNr()) {
      // modified signal group
      changedGroups_.push_back(group);
      continue;
    }
  }
}

/*--------------------------------------------------------------------------*/

void SignalControl::ClearSignalGroups(void)
{
  changedGroups_.clear();
  newGroups_.clear();
  deletedGroups_.clear();

  for (SignalGroups::iterator groupIter = groups_.begin();
       groupIter != groups_.end();
       ++groupIter) {
    delete groupIter->second;
  }

  groups_.clear();
}

/*--------------------------------------------------------------------------*/

void SignalControl::RevertSignalGroups(void)
{
  // Remove created 
  SignalGroups::iterator groupIter = groups_.begin();

  while (groupIter != groups_.end()) {
    if (groupIter->second->GetOrigNr() == 0) {
      groupIter = groups_.erase(groupIter);
    }
    else {
      ++groupIter;
    }
  }
  
  // Enable deleted and revert each group
  groupIter = groups_.begin();

  while (groupIter != groups_.end()) {
    SignalGroup* group = groupIter->second;

    if (group->GetNr() == 0) {
      groupIter = groups_.erase(groupIter);
      groups_.insert(std::make_pair(group->GetOrigNr(), group));
    }
    else {
      ++groupIter;
    }
  }

  groupIter = groups_.begin();

  while (groupIter != groups_.end()) {
    groupIter->second->Revert();
    ++groupIter;
  }
}

/*--------------------------------------------------------------------------*/

void SignalControl::AddSignalGroup(long sgNo)
{
  SignalGroups::iterator groupIter = groups_.find(-sgNo);

  if (groupIter != groups_.end()) {
    SignalGroup* group = groupIter->second;
    groups_.erase(groupIter);
    groups_.insert(std::make_pair(sgNo, group));
  }
  else {
    groups_.insert(std::make_pair(sgNo, new SignalGroup(sgNo, editable_)));
  }
}

/*--------------------------------------------------------------------------*/

void SignalControl::RemoveSignalGroup(long sgNo)
{
  SignalGroups::iterator groupIter = groups_.find(sgNo);

  if (groupIter != groups_.end()) {
    if (editable_) {
      SignalGroup* group = groupIter->second;
      groups_.erase(groupIter);

      if (group->GetOrigNr() != 0) {
        group->SetNr(0);
        group->SetDefault();
        groups_.insert(std::make_pair(-group->GetOrigNr(), group));
      }
    }
  }
}

/*--------------------------------------------------------------------------*/

SignalGroup* SignalControl::GetSignalGroup(long sgNo, bool includeDeleted)
{
  SignalGroups::iterator groupIter = groups_.find(sgNo);

  if (groupIter != groups_.end()) {
    return groupIter->second;
  }

  if (includeDeleted) {
    groupIter = groups_.find(-sgNo);

    if (groupIter != groups_.end()) {
      return groupIter->second;
    }
  }

  return NULL;
}

/*--------------------------------------------------------------------------*/

bool SignalControl::RenameSignalGroup(long oldSgNo, long newSgNo) {
  SignalGroups::iterator oldGroupIter = groups_.find(oldSgNo);
  SignalGroups::iterator newGroupIter = groups_.find(newSgNo);

  if (oldGroupIter == groups_.end() || newGroupIter != groups_.end()) {
    return false;
  }

  SignalGroup* group = oldGroupIter->second;
  groups_.erase(oldGroupIter);
  group->SetNr(newSgNo);
  groups_.insert(std::make_pair(newSgNo, group));
  return true;
}

/*--------------------------------------------------------------------------*/

void SignalControl::EditModus(bool editable)
{
  editable_ = editable;

  for (SignalGroups::iterator groupIter = groups_.begin();
       groupIter != groups_.end();
       ++groupIter) {
    groupIter->second->EditModus(editable);
  }
}

/*==========================================================================*/
