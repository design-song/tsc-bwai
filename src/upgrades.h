
struct upgrade_type {
	BWAPI::UpgradeType game_upgrade_type;
	BWAPI::TechType game_tech_type;

	double minerals_cost, gas_cost;
	unit_type*builder_type;
	int build_time;
	a_vector<unit_type*> required_units;
	a_string name;
	int level;
	upgrade_type*prev;
	a_vector<unit_type*> what_uses;
};
namespace upgrades {
	upgrade_type*get_upgrade_type(BWAPI::UpgradeType game_upgrade_type, int level = 1);
}
namespace upgrade_types {
	void get(upgrade_type*&r, BWAPI::UpgradeType game_upgrade_type, int level = 1) {
		r = upgrades::get_upgrade_type(game_upgrade_type, level);
	}
	typedef upgrade_type* upgrade_type_pointer;
	upgrade_type_pointer terran_infantry_weapons_1, terran_infantry_weapons_2, terran_infantry_weapons_3;
	upgrade_type_pointer terran_infantry_armor_1, terran_infantry_armor_2, terran_infantry_armor_3;
	upgrade_type_pointer terran_vehicle_weapons_1, terran_vehicle_weapons_2, terran_vehicle_weapons_3;
	upgrade_type_pointer terran_vehicle_plating_1, terran_vehicle_plating_2, terran_vehicle_plating_3;
	upgrade_type_pointer terran_ship_weapons_1, terran_ship_weapons_2, terran_ship_weapons_3;
	upgrade_type_pointer terran_ship_plating_1, terran_ship_plating_2, terran_ship_plating_3;

	upgrade_type_pointer ion_thrusters;

	void init() {
		get(terran_infantry_weapons_1, BWAPI::UpgradeTypes::Terran_Infantry_Weapons, 1);
		get(terran_infantry_weapons_2, BWAPI::UpgradeTypes::Terran_Infantry_Weapons, 2);
		get(terran_infantry_weapons_3, BWAPI::UpgradeTypes::Terran_Infantry_Weapons, 3);
		get(terran_infantry_armor_1, BWAPI::UpgradeTypes::Terran_Infantry_Armor, 1);
		get(terran_infantry_armor_2, BWAPI::UpgradeTypes::Terran_Infantry_Armor, 2);
		get(terran_infantry_armor_3, BWAPI::UpgradeTypes::Terran_Infantry_Armor, 3);

		get(terran_vehicle_weapons_1, BWAPI::UpgradeTypes::Terran_Vehicle_Weapons, 1);
		get(terran_vehicle_weapons_2, BWAPI::UpgradeTypes::Terran_Vehicle_Weapons, 2);
		get(terran_vehicle_weapons_3, BWAPI::UpgradeTypes::Terran_Vehicle_Weapons, 3);
		get(terran_vehicle_plating_1, BWAPI::UpgradeTypes::Terran_Vehicle_Plating, 1);
		get(terran_vehicle_plating_2, BWAPI::UpgradeTypes::Terran_Vehicle_Plating, 2);
		get(terran_vehicle_plating_3, BWAPI::UpgradeTypes::Terran_Vehicle_Plating, 3);

		get(terran_ship_weapons_1, BWAPI::UpgradeTypes::Terran_Ship_Weapons, 1);
		get(terran_ship_weapons_2, BWAPI::UpgradeTypes::Terran_Ship_Weapons, 2);
		get(terran_ship_weapons_3, BWAPI::UpgradeTypes::Terran_Ship_Weapons, 3);
		get(terran_ship_plating_1, BWAPI::UpgradeTypes::Terran_Ship_Plating, 1);
		get(terran_ship_plating_2, BWAPI::UpgradeTypes::Terran_Ship_Plating, 2);
		get(terran_ship_plating_3, BWAPI::UpgradeTypes::Terran_Ship_Plating, 3);

		get(ion_thrusters, BWAPI::UpgradeTypes::Ion_Thrusters);
		
	}
}

namespace units {
	unit_type*get_unit_type(BWAPI::UnitType game_unit_type);
}

namespace upgrades {
;

a_list<upgrade_type> upgrade_type_container;

a_map<std::tuple<BWAPI::UpgradeType, int>, upgrade_type*> upgrade_type_map;
a_map<BWAPI::TechType, upgrade_type*> tech_type_map;

upgrade_type*get_upgrade_type(BWAPI::UpgradeType game_upgrade_type, int level) {
	upgrade_type*&r = upgrade_type_map[std::make_tuple(game_upgrade_type, level)];
	if (r) return r;
	upgrade_type_container.emplace_back();
	r = &upgrade_type_container.back();
	r->game_upgrade_type = game_upgrade_type;

	r->minerals_cost = game_upgrade_type.mineralPrice() + game_upgrade_type.mineralPriceFactor()*(level - 1);
	r->gas_cost = game_upgrade_type.gasPrice() + game_upgrade_type.gasPriceFactor()*(level - 1);
	r->builder_type = units::get_unit_type(game_upgrade_type.whatUpgrades());
	r->build_time = game_upgrade_type.upgradeTime() + game_upgrade_type.upgradeTimeFactor()*(level - 1);
	unit_type*req = units::get_unit_type(game_upgrade_type.whatsRequired(level));
	if (r->builder_type) r->required_units.push_back(r->builder_type);
	if (req) r->required_units.push_back(req);
	r->name = game_upgrade_type.getName().c_str();
	if (game_upgrade_type.maxRepeats() > 1) r->name += format(" level %d", level);
	r->level = level;
	r->prev = level == 1 ? nullptr : get_upgrade_type(game_upgrade_type, level - 1);
	for (auto&v : game_upgrade_type.whatUses()) {
		r->what_uses.push_back(units::get_unit_type(v));
	}
	return r;

}

void init() {

	upgrade_types::init();

}

}
