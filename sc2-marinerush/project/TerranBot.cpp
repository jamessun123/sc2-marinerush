
#include "TerranBot.h"

void TerranBot::OnGameStart() {
	std::cout << "are you ready to lose" << std::endl;
	startLocation_ = Observation()->GetStartLocation();
	expansions_ = search::CalculateExpansionLocations(Observation(), Query());
}

//what to do on the current step
void TerranBot::OnStep() {
	TryBuildSupplyDepot();
	TryBuildRefinery();
	TryBuildBarracks();
	TryExpand(ABILITY_ID::BUILD_COMMANDCENTER, UNIT_TYPEID::TERRAN_SCV);
}

void TerranBot::OnUnitCreated(const Unit& unit) {}

void TerranBot::OnBuildingConstructionComplete(const Unit& unit) {
	switch (unit.unit_type.ToType()) {
	case(UNIT_TYPEID::TERRAN_BARRACKS): {
		size_t numLabs = CountUnitType(UNIT_TYPEID::TERRAN_BARRACKSTECHLAB);
		if (numLabs < 1) {
			Actions()->UnitCommand(unit, ABILITY_ID::BUILD_TECHLAB);
		}
		else {
			Actions()->UnitCommand(unit, ABILITY_ID::BUILD_REACTOR);
		}
	}
	case(UNIT_TYPEID::TERRAN_TECHLAB): {
		Actions()->UnitCommand(unit, ABILITY_ID::RESEARCH_STIMPACK); //upon creation, start stim
	}
	}
}

//What to do when you find an idle unit
void TerranBot::OnUnitIdle(const Unit& unit) {
	switch (unit.unit_type.ToType()) {
	case UNIT_TYPEID::TERRAN_COMMANDCENTER: {
		if(unit.assigned_harvesters <= unit.ideal_harvesters + 2)
			Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_SCV);
		break;
	}
	case UNIT_TYPEID::TERRAN_SCV: {
		uint64_t mineral_target;
		if (MaxUseRefinery(mineral_target)) {
			Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
			break;
		}
		if (FindNearestMineralPatch(unit.pos, mineral_target)) {
			Actions()->UnitCommand(unit, ABILITY_ID::SMART, mineral_target);
		}
		break;
	}
	case UNIT_TYPEID::TERRAN_BARRACKS: {
		
		Actions()->UnitCommand(unit, ABILITY_ID::TRAIN_MARINE);
		break;
	}
	
	case UNIT_TYPEID::TERRAN_MARINE: {
		const GameInfo& game_info = Observation()->GetGameInfo();
		if(CountUnitType(UNIT_TYPEID::TERRAN_MARINE) > 20){ 
			Actions()->UnitCommand(unit, ABILITY_ID::ATTACK_ATTACK, game_info.enemy_start_locations.front()); 
		}

		break;
	}
	default: {
		break;
	}
	}
}

//returns the number of this unit that exist (for you)
size_t TerranBot::CountUnitType(UNIT_TYPEID unit_type) {
	return Observation()->GetUnits(Unit::Alliance::Self, IsUnit(unit_type)).size();
}

//does what it says
bool TerranBot::FindNearestMineralPatch(const Point2D& start, uint64_t& target) {
	Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	for (const auto& u : units) {
		if (u.unit_type == UNIT_TYPEID::NEUTRAL_MINERALFIELD) {
			float d = DistanceSquared2D(u.pos, start);
			if (d < distance) {
				distance = d;
				target = u.tag;
			}
		}
	}
	if (distance == std::numeric_limits<float>::max()) {
		return false;
	}

	return true;
}

bool TerranBot::FindNearestVespeneGeyeser(const Point2D& start, uint64_t& target) {
	Units units = Observation()->GetUnits(Unit::Alliance::Neutral);
	float distance = std::numeric_limits<float>::max();
	for (const auto& u : units) {
		if (u.unit_type == UNIT_TYPEID::NEUTRAL_VESPENEGEYSER) {
			float d = DistanceSquared2D(u.pos, start);
			if (d < distance) {
				distance = d;
				target = u.tag;
			}
		}
	}
	if (distance == std::numeric_limits<float>::max()) {
		return false;
	}

	return true;
}

bool TerranBot::MaxUseRefinery(uint64_t& target) {
	Units refineries = Observation()->GetUnits
			(Unit::Alliance::Self, IsUnit(UNIT_TYPEID::TERRAN_REFINERY));
	//units has a list of all refineries that belong to you
	if (refineries.size() == 0)
		return false;

	for (const auto& refinery : refineries) {
		if (refinery.assigned_harvesters < refinery.ideal_harvesters) {
			//if you have less than 3 harvesters on gas, fill me up
			target = refinery.tag;
			return true;
		}
	}

	return false;
}

//Builds a structure at a random location
bool TerranBot::TryBuildStructure(ABILITY_ID ability_type_for_structure, UNIT_TYPEID unit_type) {
	const ObservationInterface* observation = Observation();

	// Also get an scv to build the structure.
	Unit unit_to_build;
	Units units = observation->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		if (unit.unit_type == unit_type) {
			unit_to_build = unit;
		}
	}

	if (ability_type_for_structure == ABILITY_ID::BUILD_REFINERY) {
		uint64_t place;
		if (FindNearestVespeneGeyeser(unit_to_build.pos, place)) {
			//if you found a vespene geyeser
			Actions()->UnitCommand(unit_to_build,
				ability_type_for_structure,
				place);
			return true;
		}
	}

	//now, find where to place the building
	float rx = GetRandomScalar();
	if (rx > 0 && rx < 0.5)
		rx += 0.5;
	else if (rx <= 0 && rx > -0.5)
		rx -= 0.5;
	float ry = GetRandomScalar();
	if (ry > 0 && ry < 0.5)
		ry += 0.5;
	else if (ry <= 0 && ry > -0.5)
		ry -= 0.5;
	//don't place in the mineral line, it triggers me

	Point2D placement = Point2D(unit_to_build.pos.x + rx * 15.0f, unit_to_build.pos.y + ry * 15.0f);

	Actions()->UnitCommand(unit_to_build,
		ability_type_for_structure,
		placement);

	return true;
}

//Builds a structure at a location you decide
bool TerranBot::TryBuildStructure(ABILITY_ID ability_type_for_structure, Point3D location, UNIT_TYPEID unit_type) {
	const ObservationInterface* observation = Observation();

	// Also get an scv to build the structure.
	Unit unit_to_build;
	Units units = observation->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		if (unit.unit_type == unit_type) {
			unit_to_build = unit;
		}
	}

	Actions()->UnitCommand(unit_to_build,
		ability_type_for_structure,
		location);
	return true;
}

//builds the barracks (if you have the reqs)
bool TerranBot::TryBuildBarracks() {
	const ObservationInterface* observation = Observation();

	if (CountUnitType(UNIT_TYPEID::TERRAN_SUPPLYDEPOT) < 1) {
		return false;
	}

	size_t numRax = CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS);

	if ((observation->GetFoodUsed() < 22 && numRax == 1) )//|| numRax > 2) {
		return false;
//	}
	/* UNCOMMENT IF YOU WANT HELLA AGGRO
	//leave enough minerals to build more marines, 
	//then if you have enough minerals to build one, build barracks
	if (observation->GetMinerals() > (150 + 50 * CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS))) {
		return TryBuildStructure(ABILITY_ID::BUILD_BARRACKS);
	}*/

	return TryBuildStructure(ABILITY_ID::BUILD_BARRACKS);
}

//builds supply depot.  what do you think
bool TerranBot::TryBuildSupplyDepot() {
	const ObservationInterface* observation = Observation();

	size_t numRax = CountUnitType(UNIT_TYPEID::TERRAN_BARRACKS);

	// If we are not supply capped, don't build a supply depot.
	if (observation->GetFoodUsed() < observation->GetFoodCap() - 2 - numRax)
		return false;

	Units units = observation->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		for (const auto& order : unit.orders) {
			if (order.ability_id == ABILITY_ID::BUILD_SUPPLYDEPOT) {
				return false;
			}
		}
	}

	// Try and build a depot. Find a random SCV and give it the order.
	return TryBuildStructure(ABILITY_ID::BUILD_SUPPLYDEPOT);
}

//attempts to build a refinery on nearby vespene geyesers
//you only want refineries on 15 scv and 21 scv
bool TerranBot::TryBuildRefinery() {
	const ObservationInterface* observation = Observation();
	int currentSupply = observation->GetFoodUsed();

	if (!((currentSupply == 15 && CountUnitType(UNIT_TYPEID::TERRAN_REFINERY) == 0))) {
		//don't even bother with building refinery unless you have these scv counts
		return false;
	}

	//don't build refinery if someone else is building it
	Units units = observation->GetUnits(Unit::Alliance::Self);
	for (const auto& unit : units) {
		for (const auto& order : unit.orders) {
			if (order.ability_id == ABILITY_ID::BUILD_REFINERY) {
				return false;
			}
		}
	}

	return TryBuildStructure(ABILITY_ID::BUILD_REFINERY);
}

//attempt to expand when you have 20 scvs
bool TerranBot::TryExpand(AbilityID build_ability, UnitTypeID worker_type) {
	const ObservationInterface* observation = Observation();

	if (observation->GetFoodUsed() != 19)
		return false;

	float minimum_distance = std::numeric_limits<float>::max();
	Point3D closest_expansion;
	for (const auto& expansion : expansions_) {
		float current_distance = Distance2D(startLocation_, expansion);
		if (current_distance < .01f) {
			continue;
		}

		if (current_distance < minimum_distance) {
			if (Query()->Placement(build_ability, expansion)) {
				closest_expansion = expansion;
				minimum_distance = current_distance;
			}
		}
	}
	//only update staging location up till 3 bases.
	if (TryBuildStructure(build_ability, closest_expansion)) {
		staging_location_ = Point3D(((staging_location_.x + closest_expansion.x) / 2),
			((staging_location_.y + closest_expansion.y) / 2),
			((staging_location_.z + closest_expansion.z) / 2));
		return true;
	}
	return false;

}