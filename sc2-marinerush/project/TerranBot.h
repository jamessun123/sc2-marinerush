#ifndef TERRANBOT_H
#define TERRANBOT_H

#include "sc2api/sc2_api.h"
#include "sc2lib/sc2_search.h"
#include <iostream>

using namespace sc2;


/*This bot will be hard coded to do a 5 minute marine 1/1 stim timing push.
Most of the code will be in the micro.
*/
class TerranBot : public Agent {
public:
	virtual void OnGameStart() final;

	//what to do each step of the game
	virtual void OnStep() final;

	//does stuff when you create UNITS
	virtual void OnUnitCreated(const Unit& unit);

	//what to do on idle units
	virtual void OnUnitIdle(const Unit& unit) final;

	virtual void OnBuildingConstructionComplete(const Unit& unit);

private:

	//counts the number of this unit
	size_t CountUnitType(UNIT_TYPEID unit_type);

	// is used to make scvs go mine gas
	bool MaxUseRefinery(uint64_t& target);

	//finding natural resources
	bool FindNearestMineralPatch(const Point2D& start, uint64_t& target);
	bool FindNearestVespeneGeyeser(const Point2D& start, uint64_t& target);

	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV);
	bool TryBuildStructure(ABILITY_ID ability_type_for_structure, Point3D location, UNIT_TYPEID unit_type = UNIT_TYPEID::TERRAN_SCV);

	bool TryBuildBarracks();
	bool TryBuildRefinery();
	bool TryBuildSupplyDepot();

	bool TryExpand(AbilityID build_ability, UnitTypeID worker_type);

	std::vector<Point3D> expansions_;
	Point3D startLocation_;
	Point3D staging_location_;
};

#endif