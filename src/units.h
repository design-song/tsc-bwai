

struct player_t {
	BWAPI::Player game_player;
	bool is_enemy;
};


struct unit_type {
	BWAPI::UnitType game_unit_type;
	a_string name;
	// right, down, left, up
	std::array<int,4> dimensions;
	int dimension_right() {return dimensions[0];}
	int dimension_down() {return dimensions[1];}
	int dimension_left() {return dimensions[2];}
	int dimension_up() {return dimensions[3];}
	bool is_worker;
	bool is_minerals;
	bool is_gas;
	bool requires_creep, requires_pylon;
	int race;
	int tile_width, tile_height;
	double minerals_cost, gas_cost;
	int build_time;
	bool is_building;
	bool is_addon;
	bool is_resource_depot;
	bool is_refinery;
	unit_type*builder_type;
	a_vector<unit_type*> builds;
	bool build_near_me;
	a_vector<unit_type*> required_units, required_for;
	bool produces_land_units;
	bool needs_neighboring_free_space;
	double required_supply;
	double provided_supply;
	bool is_flyer;
	enum { size_none, size_small, size_medium, size_large };
	int size;
};

struct weapon_stats {
	BWAPI::WeaponType game_weapon_type;
	player_t*player;

	double damage;
	int cooldown;
	double min_range;
	double max_range;
	bool targets_air;
	bool targets_ground;
	enum { damage_type_none, damage_type_concussive, damage_type_normal, damage_type_explosive };
	int damage_type;
};

struct unit_stats {
	unit_type*type;
	player_t*player;

	double max_speed;
	double acceleration;
	double stop_distance;

	double energy;
	double shields;
	double hp;
	double armor;

	double sight_range;

	weapon_stats*ground_weapon;
	int ground_weapon_hits;
	weapon_stats*air_weapon;
	int air_weapon_hits;

};

struct unit_building {
	unit*u;
	bool is_lifted;
	xy walk_squares_occupied_pos;
	a_vector<grid::walk_square*> walk_squares_occupied;
	a_vector<grid::sixtyfour_square*> sixtyfour_squares_occupied;
	a_vector<grid::build_square*> build_squares_occupied;
	xy build_pos;
};

struct unit;
namespace units {
	a_vector<unit*> unit_containers[13];
}

a_vector<unit*>&all_units_ever = units::unit_containers[0];
a_vector<unit*>&live_units = units::unit_containers[1];
a_vector<unit*>&visible_units = units::unit_containers[2];
a_vector<unit*>&invisible_units = units::unit_containers[3];
a_vector<unit*>&visible_buildings = units::unit_containers[4];
a_vector<unit*>&resource_units = units::unit_containers[5];

a_vector<unit*>&my_units = units::unit_containers[6];
a_vector<unit*>&my_workers = units::unit_containers[7];
a_vector<unit*>&my_buildings = units::unit_containers[8];
a_vector<unit*>&my_resource_depots = units::unit_containers[9];

a_vector<unit*>&enemy_units = units::unit_containers[10];
a_vector<unit*>&visible_enemy_units = units::unit_containers[11];
a_vector<unit*>&enemy_buildings = units::unit_containers[12];

a_unordered_map<unit_type*,a_vector<unit*>> my_units_of_type;

struct unit_controller;
struct unit {
	BWAPI::Unit game_unit;
	player_t*owner;
	unit_type*type;
	unit_stats*stats;
	unit_building*building;
	unit_controller*controller;

	bool visible, dead;
	int last_seen, creation_frame;
	xy pos;
	bool gone;
	double speed, hspeed, vspeed;
	int resources;
	BWAPI::Order game_order;
	bool is_carrying_minerals_or_gas;
	bool is_being_constructed;
	bool is_completed;
	bool is_morphing;
	int remaining_build_time;
	int remaining_train_time;
	int remaining_research_time;
	int remaining_upgrade_time;
	int remaining_whatever_time;
	unit*build_unit;
	a_vector<unit_type*> train_queue;
	unit*addon;
	bool cloaked, detected;
	bool invincible;

	double energy;
	double shields;
	double hp;
	int weapon_cooldown;

	unit*target;
	unit*order_target;

	std::array<size_t,std::extent<decltype(units::unit_containers)>::value> container_indexes;
};

namespace units {
	unit_type*get_unit_type(unit_type*&,BWAPI::UnitType);
}
namespace unit_types {
	void get(unit_type*&rv,BWAPI::UnitType game_unit_type) {
		units::get_unit_type(rv,game_unit_type);
	}
	typedef unit_type*unit_type_pointer;
	unit_type_pointer unknown;
	unit_type_pointer cc, supply_depot, barracks, factory, science_facility, nuclear_silo, bunker, refinery, machine_shop;
	unit_type_pointer missile_turret;
	unit_type_pointer scv, marine, vulture, siege_tank_tank_mode, siege_tank_siege_mode;
	unit_type_pointer medic, ghost, battlecruiser;
	unit_type_pointer nexus, pylon, gateway, photon_cannon, robotics_facility;
	unit_type_pointer probe;
	unit_type_pointer hatchery, lair, hive, creep_colony, sunken_colony, spore_colony, nydus_canal, spawning_pool, evolution_chamber;
	unit_type_pointer drone, overlord, zergling;
	unit_type_pointer vespene_geyser;
	std::reference_wrapper<unit_type_pointer> units_that_produce_land_units[] = {
		cc, barracks, factory,
		nexus, gateway, robotics_facility,
		hatchery, lair, hive
	};
	void init() {
		get(unknown, BWAPI::UnitTypes::Unknown);

		get(cc, BWAPI::UnitTypes::Terran_Command_Center);
		get(supply_depot, BWAPI::UnitTypes::Terran_Supply_Depot);
		get(barracks, BWAPI::UnitTypes::Terran_Barracks);
		get(factory, BWAPI::UnitTypes::Terran_Factory);
		get(science_facility, BWAPI::UnitTypes::Terran_Science_Facility);
		get(nuclear_silo, BWAPI::UnitTypes::Terran_Nuclear_Silo);
		get(bunker, BWAPI::UnitTypes::Terran_Bunker);
		get(refinery, BWAPI::UnitTypes::Terran_Refinery);
		get(machine_shop, BWAPI::UnitTypes::Terran_Machine_Shop);
		get(missile_turret, BWAPI::UnitTypes::Terran_Missile_Turret);

		get(scv, BWAPI::UnitTypes::Terran_SCV);
		get(marine, BWAPI::UnitTypes::Terran_Marine);
		get(medic, BWAPI::UnitTypes::Terran_Medic);
		get(ghost, BWAPI::UnitTypes::Terran_Ghost);
		get(vulture, BWAPI::UnitTypes::Terran_Vulture);
		get(siege_tank_tank_mode, BWAPI::UnitTypes::Terran_Siege_Tank_Tank_Mode);
		get(siege_tank_siege_mode, BWAPI::UnitTypes::Terran_Siege_Tank_Siege_Mode);
		get(battlecruiser, BWAPI::UnitTypes::Terran_Battlecruiser);

		get(nexus, BWAPI::UnitTypes::Protoss_Nexus);
		get(pylon, BWAPI::UnitTypes::Protoss_Pylon);
		get(gateway, BWAPI::UnitTypes::Protoss_Gateway);
		get(photon_cannon, BWAPI::UnitTypes::Protoss_Photon_Cannon);
		get(robotics_facility, BWAPI::UnitTypes::Protoss_Robotics_Facility);

		get(probe, BWAPI::UnitTypes::Protoss_Probe);

		get(hatchery, BWAPI::UnitTypes::Zerg_Hatchery);
		get(lair, BWAPI::UnitTypes::Zerg_Lair);
		get(hive, BWAPI::UnitTypes::Zerg_Hive);
		get(creep_colony, BWAPI::UnitTypes::Zerg_Creep_Colony);
		get(sunken_colony, BWAPI::UnitTypes::Zerg_Sunken_Colony);
		get(spore_colony, BWAPI::UnitTypes::Zerg_Spore_Colony);
		get(nydus_canal, BWAPI::UnitTypes::Zerg_Nydus_Canal);
		get(spawning_pool, BWAPI::UnitTypes::Zerg_Spawning_Pool);
		get(evolution_chamber, BWAPI::UnitTypes::Zerg_Evolution_Chamber);

		get(drone, BWAPI::UnitTypes::Zerg_Drone);
		get(overlord, BWAPI::UnitTypes::Zerg_Overlord);
		get(zergling, BWAPI::UnitTypes::Zerg_Zergling);

		get(vespene_geyser, BWAPI::UnitTypes::Resource_Vespene_Geyser);
	}
}

struct unit_controller;
unit_controller*get_unit_controller(unit*);
namespace units {
;

static const size_t npos = ~(size_t)0;

player_t*my_player;
player_t*opponent_player;

void update_unit_container(unit*u,a_vector<unit*>&cont,bool contain) {
	size_t idx = &cont - &units::unit_containers[0];
	size_t&u_idx = u->container_indexes[idx];
	if (contain && u_idx==npos) {
		u_idx = cont.size();
		cont.push_back(u);
	}
	if (!contain && u_idx!=npos) {
		if (u_idx!=cont.size()-1) {
			cont[cont.size()-1]->container_indexes[idx] = u_idx;
			std::swap(cont[cont.size()-1],cont[u_idx]);
		}
		cont.pop_back();
		u_idx = npos;
	}
}


a_deque<unit> unit_container;
a_deque<unit_building> unit_building_container;

a_deque<player_t> player_container;
a_unordered_map<BWAPI::Player,player_t*> player_map;

player_t*get_player(BWAPI::Player game_player) {
	player_t*&p = player_map[game_player];
	if (!p) {
		player_container.emplace_back();
		p = &player_container.back();
		p->game_player = game_player;
		p->is_enemy = game_player->isEnemy(game->self());
	}
	return p;
}

void update_unit_owner(unit*u) {
	u->owner = get_player(u->game_unit->getPlayer());
}

a_deque<unit_type> unit_type_container;
a_unordered_map<int,unit_type*> unit_type_map;

unit_type*get_unit_type(unit_type*&rv,BWAPI::UnitType game_unit_type);
unit_type*get_unit_type(BWAPI::UnitType game_unit_type) {
	unit_type*r;
	get_unit_type(r,game_unit_type);
	return r;
}
unit_type*new_unit_type(BWAPI::UnitType game_unit_type,unit_type*ut) {
	ut->game_unit_type = game_unit_type;
	ut->name = game_unit_type.getName().c_str();
	ut->dimensions[0] = game_unit_type.dimensionRight();
	ut->dimensions[1] = game_unit_type.dimensionDown();
	ut->dimensions[2] = game_unit_type.dimensionLeft();
	ut->dimensions[3] = game_unit_type.dimensionUp();
	ut->is_worker = game_unit_type.isWorker();
	ut->is_minerals = game_unit_type.isMineralField();
	ut->is_gas = game_unit_type==BWAPI::UnitTypes::Resource_Vespene_Geyser || game_unit_type==BWAPI::UnitTypes::Terran_Refinery || game_unit_type==BWAPI::UnitTypes::Protoss_Assimilator || game_unit_type==BWAPI::UnitTypes::Zerg_Extractor;
	ut->requires_creep = game_unit_type.requiresCreep();
	ut->requires_pylon = game_unit_type.requiresPsi();
	ut->race = game_unit_type.getRace()==BWAPI::Races::Terran ? race_terran : game_unit_type.getRace()==BWAPI::Races::Protoss ? race_protoss : game_unit_type.getRace()==BWAPI::Races::Zerg ? race_zerg : race_unknown;
	ut->tile_width = game_unit_type.tileWidth();
	ut->tile_height = game_unit_type.tileHeight();
	ut->minerals_cost = (double)game_unit_type.mineralPrice();
	ut->gas_cost = (double)game_unit_type.gasPrice();
	ut->build_time = game_unit_type.buildTime();
	ut->is_building = game_unit_type.isBuilding();
	ut->is_addon = game_unit_type.isAddon();
	ut->is_resource_depot = game_unit_type.isResourceDepot();
	ut->is_refinery = game_unit_type.isRefinery();
	ut->builder_type = get_unit_type(game_unit_type.whatBuilds().first);
	ut->builder_type->builds.push_back(ut);
	ut->build_near_me = ut->is_resource_depot || ut==unit_types::supply_depot || ut==unit_types::pylon || ut==unit_types::creep_colony || ut==unit_types::sunken_colony || ut==unit_types::spore_colony;
	for (auto&v : game_unit_type.requiredUnits()) {
		unit_type*t = get_unit_type(v.first);
		t->required_for.push_back(ut);
		ut->required_units.push_back(t);
		if (v.second!=1) xcept("%s requires %d %s\n",ut->name,v.second,t->name);
	}
	ut->produces_land_units = false;
	for (auto&v : unit_types::units_that_produce_land_units) {
		if (v==ut) ut->produces_land_units = true;
	}
	if (ut->produces_land_units) log("%s produces land units!\n",ut->name);
	ut->needs_neighboring_free_space = ut->is_building && (ut->produces_land_units || ut==unit_types::nydus_canal || ut==unit_types::bunker);
	ut->required_supply = (double)game_unit_type.supplyRequired() / 2.0;
	ut->provided_supply = (double)game_unit_type.supplyProvided() / 2.0;
	ut->is_flyer = game_unit_type.isFlyer();
	switch (game_unit_type.size().getID()) {
	case BWAPI::UnitSizeTypes::Enum::Independent:
	case BWAPI::UnitSizeTypes::Enum::None:
	case BWAPI::UnitSizeTypes::Enum::Unknown:
		ut->size = unit_type::size_none;
		break;
	case BWAPI::UnitSizeTypes::Enum::Small:
		ut->size = unit_type::size_small;
		break;
	case BWAPI::UnitSizeTypes::Enum::Medium:
		ut->size = unit_type::size_medium;
		break;
	case BWAPI::UnitSizeTypes::Enum::Large:
		ut->size = unit_type::size_large;
		break;
	default:
		xcept("unknown size %s for unit %s", game_unit_type.size().getName(), game_unit_type.getName());
	}
	return ut;
}
unit_type*get_unit_type(unit_type*&rv,BWAPI::UnitType game_unit_type) {
	unit_type*&ut = unit_type_map[game_unit_type.getID()];
	if (ut) return rv=ut;
	unit_type_container.emplace_back();
	rv = ut = &unit_type_container.back();
	return new_unit_type(game_unit_type,ut);
}

void update_unit_type(unit*u) {
	unit_type*ut = get_unit_type(u->game_unit->getType());
	if (ut==u->type) return;
	u->type = ut;
	if (ut->is_building) {
		if (!u->building) {
			unit_building_container.emplace_back();
			u->building = &unit_building_container.back();
			u->building->u = u;
		}
	} else u->building = 0;
}

a_deque<weapon_stats> weapon_stats_container;
a_map<std::tuple<BWAPI::WeaponType, player_t*>, weapon_stats*> weapon_stats_map;

void update_weapon_stats(weapon_stats*st) {

	auto&gw = st->game_weapon_type;
	auto&gp = st->player->game_player;

	st->damage = gp->damage(gw);
	st->cooldown = gw.damageCooldown(); // FIXME: adrenal glands
	st->min_range = gw.minRange();
	st->max_range = gp->weaponMaxRange(gw);
	st->targets_air = gw.targetsAir();
	st->targets_ground = gw.targetsGround();
	switch (gw.damageType().getID()) {
	case BWAPI::DamageTypes::Enum::Unknown:
		st->damage_type = weapon_stats::damage_type_none;
		break;
	case BWAPI::DamageTypes::Enum::Concussive:
		st->damage_type = weapon_stats::damage_type_concussive;
		break;
	case BWAPI::DamageTypes::Enum::Normal:
		st->damage_type = weapon_stats::damage_type_normal;
		break;
	case BWAPI::DamageTypes::Enum::Explosive:
		st->damage_type = weapon_stats::damage_type_explosive;
		break;
	default:
		xcept("unknown damage type %s for weapon %s\n", gw.damageType().getName(), gw.getName());
	}

}

weapon_stats*get_weapon_stats(BWAPI::WeaponType type, player_t*player) {
	weapon_stats*&st = weapon_stats_map[std::make_tuple(type, player)];
	if (st) return st;
	weapon_stats_container.emplace_back();
	st = &weapon_stats_container.back();
	st->game_weapon_type = type;
	st->player = player;
	update_weapon_stats(st);
	return st;
};

a_deque<unit_stats> unit_stats_container;
a_map<std::tuple<unit_type*,player_t*>,unit_stats*> unit_stats_map;

void update_stats(unit_stats*st) {

	auto&gp = st->player->game_player;
	auto&gu = st->type->game_unit_type;

	st->max_speed = std::ceil(gp->topSpeed(gu)*256) / 256.0;
	st->max_speed += 20.0/256.0; // Verify this: some(all?) speeds in BWAPI seem to be too low by this amount.
	int acc = gu.acceleration();
	st->acceleration = (double)acc / 256.0;
	if (st->acceleration==0 || acc==1) st->acceleration = st->max_speed;
	st->stop_distance = (double)gu.haltDistance()/256.0;

	st->energy = (double)gp->maxEnergy(gu);
	st->shields = (double)gu.maxShields();
	st->hp = (double)gu.maxHitPoints();
	st->armor = (double)gp->armor(gu);

	st->sight_range = (double)gp->sightRange(gu);

	st->ground_weapon_hits = gu.maxGroundHits();
	if (st->ground_weapon_hits == 0 && st->ground_weapon) {
		log("warning: %s.maxGroundHits() returned 0 (setting to 1)\n", st->type->name);
		st->ground_weapon_hits = 1;
	}
	st->air_weapon_hits = gu.maxAirHits();
	if (st->air_weapon_hits == 0 && st->air_weapon) {
		log("warning: %s.maxAirHits() returned 0 (setting to 1)\n", st->type->name);
		st->air_weapon_hits = 1;
	}
	if (st->ground_weapon) update_weapon_stats(st->ground_weapon);
	if (st->air_weapon) update_weapon_stats(st->air_weapon);
}

void update_all_stats() {
	for (auto&v : unit_stats_container) update_stats(&v);
}

unit_stats*get_unit_stats(unit_type*type,player_t*player) {
	unit_stats*&st = unit_stats_map[std::make_tuple(type,player)];
	if (st) return st;
	unit_stats_container.emplace_back();
	st = &unit_stats_container.back();
	st->type = type;
	st->player = player;
	st->ground_weapon = nullptr;
	st->air_weapon = nullptr;
	auto gw = st->type->game_unit_type.groundWeapon();
	if (gw != BWAPI::WeaponTypes::None) st->ground_weapon = get_weapon_stats(gw, player);
	auto aw = st->type->game_unit_type.airWeapon();
	if (aw != BWAPI::WeaponTypes::None) st->air_weapon = get_weapon_stats(aw, player);
	update_stats(st);
	return st;
}

unit*get_unit(BWAPI::Unit game_unit);
void update_unit_stuff(unit*u) {
	auto position = u->game_unit->getPosition();
	if (u->game_unit->getID() < 0) xcept("(update) %s->getId() is %d\n", u->type->name, u->game_unit->getID());
	if (u->visible != u->game_unit->isVisible()) xcept("visible mismatch in unit %s (%d vs %d)", u->type->name, u->visible, u->game_unit->isVisible());
	if ((size_t)position.x >= (size_t)grid::map_width || (size_t)position.y >= (size_t)grid::map_height) xcept("unit %s is outside map", u->type->name);
	u->pos.x = position.x;
	u->pos.y = position.y;
	u->hspeed = u->game_unit->getVelocityX();
	u->vspeed = u->game_unit->getVelocityY();
	u->speed = xy_typed<double>(u->hspeed,u->vspeed).length();
	u->resources = u->game_unit->getResources();
	u->game_order = u->game_unit->getOrder();
	u->is_carrying_minerals_or_gas = u->game_unit->isCarryingMinerals() || u->game_unit->isCarryingGas();
	u->is_being_constructed = u->game_unit->isBeingConstructed();
	u->is_completed = u->game_unit->isCompleted();
	u->is_morphing = u->game_unit->isMorphing();
	u->remaining_build_time = u->game_unit->getRemainingBuildTime();
	u->remaining_train_time = u->game_unit->getRemainingTrainTime();
	u->remaining_research_time = u->game_unit->getRemainingResearchTime();
	u->remaining_upgrade_time = u->game_unit->getRemainingUpgradeTime();
	u->remaining_whatever_time = std::max(u->remaining_build_time,std::max(u->remaining_train_time,std::max(u->remaining_research_time,u->remaining_upgrade_time)));
	u->build_unit = u->game_unit->getBuildUnit() ? (unit*)u->game_unit->getBuildUnit()->getClientInfo() : nullptr;
	u->train_queue.clear();
	for (auto&v : u->game_unit->getTrainingQueue()) {
		u->train_queue.push_back(get_unit_type(v));
	}
	u->addon = u->game_unit->getAddon() ? get_unit(u->game_unit->getAddon()) : nullptr;

	u->cloaked = u->game_unit->isCloaked();
	u->detected = u->game_unit->isDetected();
	u->invincible = u->game_unit->isInvincible();

	u->energy = u->game_unit->getEnergy();
	u->shields = u->game_unit->getShields();
	u->hp = u->game_unit->getHitPoints();
	u->weapon_cooldown = std::max(u->game_unit->getGroundWeaponCooldown(), u->game_unit->getAirWeaponCooldown());

	auto targetunit = [&](BWAPI::Unit gu) -> unit* {
		if (!gu) return nullptr;
		if (!gu->isVisible()) return nullptr;
		if (gu->getID() < 0) xcept("(target) %s->getId() is %d\n", gu->getType().getName(), gu->getID());
		return get_unit(gu);
	};
	u->target = targetunit(u->game_unit->getTarget());
	u->order_target = targetunit(u->game_unit->getOrderTarget());

	unit_building*b = u->building;
	if (b) {
		b->is_lifted = u->game_unit->isLifted();
		auto tile_pos = u->game_unit->getTilePosition();
		if ((size_t)tile_pos.x >= (size_t)grid::build_grid_width || (size_t)tile_pos.y >= (size_t)grid::build_grid_height) xcept("unit %s has tile pos outside map", u->type->name);
		if ((size_t)tile_pos.x + u->type->tile_width >= (size_t)grid::build_grid_width || (size_t)tile_pos.y + u->type->tile_height >= (size_t)grid::build_grid_height) xcept("unit %s has tile pos outside map", u->type->name);
		b->build_pos.x = tile_pos.x*32;
		b->build_pos.y = tile_pos.y*32;
	}
}

unit*new_unit(BWAPI::Unit game_unit) {
	unit_container.emplace_back();
	unit*u = &unit_container.back();

	u->game_unit = game_unit;
	u->owner = 0;
	u->type = 0;
	u->stats = 0;
	u->building = 0;
	u->controller = 0;

	u->visible = false;
	u->dead = false;
	u->creation_frame = current_frame;
	u->last_seen = current_frame;
	u->gone = false;

	//update_unit_stuff(u);

	u->container_indexes.fill(npos);

	update_unit_owner(u);
	update_unit_type(u);
	u->stats = get_unit_stats(u->type,u->owner);

	u->visible = game_unit->isVisible();
	update_unit_stuff(u);

	return u;
}

unit*get_unit(BWAPI::Unit game_unit) {
	unit*u = (unit*)game_unit->getClientInfo();
	if (!u) {
		u = new_unit(game_unit);
		game_unit->setClientInfo(u);
	}
	return u;
}

struct event_t {
	enum {t_show, t_hide, t_create, t_morph, t_destroy, t_renegade, t_complete, t_refresh};
	int t;
	BWAPI::Unit game_unit;
	event_t(int t,BWAPI::Unit game_unit) : t(t), game_unit(game_unit) {}
};
a_deque<event_t> events;

void on_unit_show(BWAPI::Unit game_unit) {
	events.emplace_back(event_t::t_show,game_unit);
}
void on_unit_hide(BWAPI::Unit game_unit) {
	events.emplace_back(event_t::t_hide,game_unit);
}
void on_unit_create(BWAPI::Unit game_unit) {
	events.emplace_back(event_t::t_create,game_unit);
}
void on_unit_morph(BWAPI::Unit game_unit) {
	events.emplace_back(event_t::t_morph,game_unit);
}
void on_unit_destroy(BWAPI::Unit game_unit) {
	events.emplace_back(event_t::t_destroy,game_unit);
}
void on_unit_renegade(BWAPI::Unit game_unit) {
	events.emplace_back(event_t::t_renegade,game_unit);
}
void on_unit_complete(BWAPI::Unit game_unit) {
	events.emplace_back(event_t::t_complete,game_unit);
}

void update_buildings_walk_squares_task() {

	a_vector<unit_building*> registered_buildings;
	while (true) {

		for (size_t i=0;i<registered_buildings.size();++i) {
			unit_building*b = registered_buildings[i];
			unit*u = b->u;
			if (b->u->building!=b || b->is_lifted || u->pos!=b->walk_squares_occupied_pos || u->gone || u->dead) {
				//++pathing::update_spaces;
				log("unregistered %s, because %s, %d squares\n",u->type->name,b->u->building!=b ? "unit changed" : b->is_lifted ? "lifted" : u->pos!=b->walk_squares_occupied_pos ? "moved" : u->gone ? "gone" : u->dead ? "dead" : "?",b->walk_squares_occupied.size());
				for (auto*ws_p : b->walk_squares_occupied) {
					find_and_erase(ws_p->buildings, u);
					//if (ws_p->building==u) ws_p->building = 0;
				}
				b->walk_squares_occupied.clear();
				for (auto*sfs_p : b->sixtyfour_squares_occupied) {
					//while (!sfs_p->distance_map_used.empty()) pathing::invalidate_distance_map(sfs_p->distance_map_used.front());
				}
				b->sixtyfour_squares_occupied.clear();
				for (auto*bs_p : b->build_squares_occupied) {
					if (bs_p->building==u) bs_p->building = 0;
				}
				b->build_squares_occupied.clear();
				xy upper_left = u->pos - xy(u->type->dimension_left(), u->type->dimension_up());
				xy bottom_right = u->pos + xy(u->type->dimension_right(), u->type->dimension_down());
				square_pathing::invalidate_area(upper_left, bottom_right);
				if (i!=registered_buildings.size()-1) std::swap(registered_buildings[registered_buildings.size()-1],registered_buildings[i]);
				registered_buildings.pop_back();
				--i;
			} else {
				for (auto*ws_p : b->walk_squares_occupied) {
					//if (ws_p->building!=u) ws_p->building = u;
				}
				for (auto*bs_p : b->build_squares_occupied) {
					if (bs_p->building!=u) bs_p->building = u;
				}
			}
		}

		for (unit*u : visible_buildings) {
			unit_building*b = u->building;
			if (b->is_lifted) continue;

			if (b->walk_squares_occupied.empty()) {
				//++pathing::update_spaces;
				xy upper_left = u->pos - xy(u->type->dimension_left(),u->type->dimension_up());
				xy bottom_right = u->pos + xy(u->type->dimension_right(),u->type->dimension_down());
				for (int y=upper_left.y&-8;y<=bottom_right.y;y+=8) {
					if ((size_t)y >= (size_t)grid::map_height) continue;
					for (int x=upper_left.x&-8;x<=bottom_right.x;x+=8) {
						if ((size_t)x >= (size_t)grid::map_width) continue;
						auto&ws = grid::get_walk_square(xy(x,y));
						b->walk_squares_occupied.push_back(&ws);
						//ws.building = u;
						ws.buildings.push_back(u);
					}
				}
				for (int y=upper_left.y&-64 - 64;y<=bottom_right.y+64;y+=64) {
					if ((size_t)y >= (size_t)grid::map_height) continue;
					for (int x=upper_left.x&-64 - 64;x<=bottom_right.x+64;x+=64) {
						if ((size_t)x >= (size_t)grid::map_width) continue;
						auto&sfs = grid::get_sixtyfour_square(xy(x,y));
						b->sixtyfour_squares_occupied.push_back(&sfs);
						//while (!sfs.distance_map_used.empty()) pathing::invalidate_distance_map(sfs.distance_map_used.front());
					}
				}
				for (int y=0;y<u->type->tile_height;++y) {
					for (int x=0;x<u->type->tile_width;++x) {
						auto&bs = grid::get_build_square(u->building->build_pos + xy(x*32,y*32));
						b->build_squares_occupied.push_back(&bs);
						bs.building = u;
					}
				}
				square_pathing::invalidate_area(upper_left, bottom_right);
				registered_buildings.push_back(b);
				b->walk_squares_occupied_pos = u->pos;
				log("registered %s, %d squares\n",u->type->name,b->walk_squares_occupied.size());
			} else {
				bool found = false;
				for (auto*ptr : registered_buildings) {
					if (ptr == b) found = true;
				}
				if (!found) DebugBreak();
			}
		}

		multitasking::sleep(4);
	}

}

a_vector<std::function<void(unit*)>> on_create_callbacks, on_morph_callbacks;

void update_units_task() {

	int last_update_stats = 0;
	int last_refresh = 0;

	a_vector<unit*> created_units, morphed_units;

	while (true) {
		my_player = get_player(game->self());
		opponent_player = get_player(game->neutral());
		for (auto&v : game->getPlayers()) {
			auto*p = get_player(v);
			if (p->is_enemy) opponent_player = p;
		}

		grid::update_build_grid();

		created_units.clear();
		morphed_units.clear();
		while (!events.empty()) {
			auto e = events.front();
			events.pop_front();
			unit*u = get_unit(e.game_unit);
			// These events are delayed, so e.g. a unit is not necessarily visible even though we are processing a show event.
			u->visible = u->game_unit->isVisible();
			u->visible &= u->game_unit->exists(); // BWAPI bug: destroyed units are visible
			switch (e.t) {
			case event_t::t_show:
				log("show %s\n",u->type->name);
				if (u->visible) update_unit_owner(u);
				if (u->visible) update_unit_type(u);
				update_stats(u->stats);
				break;
			case event_t::t_hide:
				log("hide %s\n",u->type->name);
				// the unit is still visible, but will be invisible the next frame
				if (u->visible) update_unit_stuff(u);
				u->visible = false;
				break;
			case event_t::t_create:
				created_units.push_back(u);
				break;
			case event_t::t_morph:
				if (u->visible) update_unit_type(u);
				morphed_units.push_back(u);
				break;
			case event_t::t_destroy:
				log("destroy %s\n", u->type->name);
				u->visible = false;
				u->dead = true;
				break;
			case event_t::t_renegade:
				if (u->visible) update_unit_owner(u);
				break;
// 			case event_t::t_complete:
// 				log("%s completed\n",u->type->name);
// 				u->is_completed = true;
// 				break;
			}
			//log("event %d - %s visible ? %d\n", e.t, u->type->name, u->visible);

			update_unit_container(u,all_units_ever,true);
			update_unit_container(u,live_units,!u->dead);
			update_unit_container(u,visible_units,u->visible);
			update_unit_container(u,invisible_units,!u->dead && !u->visible);
			update_unit_container(u,visible_buildings,u->visible && u->building);
			update_unit_container(u,resource_units,!u->dead && !u->gone && (u->type->is_minerals || u->type->is_gas));

			update_unit_container(u,my_units,u->visible && u->owner==my_player);
			//update_unit_container(u,my_workers,u->visible && u->owner==my_player && u->type->is_worker && u->is_completed && !u->is_morphing);
			update_unit_container(u,my_buildings,u->visible && u->owner==my_player && u->building);
			update_unit_container(u,my_resource_depots,u->visible && u->owner==my_player && u->type->is_resource_depot);

			update_unit_container(u, enemy_units, !u->dead && u->owner->is_enemy);
			update_unit_container(u, visible_enemy_units, u->visible && u->owner->is_enemy);
			update_unit_container(u, enemy_buildings, !u->dead && !u->gone && u->building && u->owner->is_enemy);

			my_units_of_type.clear();
			for (unit*u : my_units) {
				my_units_of_type[u->type].push_back(u);
			}

			if (u->owner==my_player && !u->controller) get_unit_controller(u);
		}

// 		if (current_frame - last_refresh >= 90) {
// 			for (unit*u : live_units) {
// 				events.emplace_back(event_t::t_refresh, u->game_unit);
// 			}
// 			last_refresh = current_frame;
// 		}

		for (unit*u : visible_units) {
			u->last_seen = current_frame;
			update_unit_stuff(u);

			if (u->gone) {
				log("%s is no longer gone\n",u->type->name);
				u->gone = false;
			}
		}

		for (unit*u : all_units_ever) {
			update_unit_container(u,my_workers,u->visible && u->owner==my_player && u->type->is_worker && u->is_completed && !u->is_morphing);

			if (u->game_unit->getID() < 0) {
				xcept("unit %p has invalid id %d\n", u->game_unit->getID());
			}
		}

		for (unit*u : invisible_units) {
			if (!u->gone) {
				if (grid::is_visible(u->pos)) {
					log("%s is gone\n",u->type->name);
					u->gone = true;
				}
			}
		}

		if (current_frame-last_update_stats>=15*5) {
			update_all_stats();
		}

		for (unit*u : created_units) {
			for (auto&f : on_create_callbacks) f(u);
		}
		for (unit*u : morphed_units) {
			for (auto&f : on_morph_callbacks) f(u);
		}

		multitasking::sleep(1);
	}

}

void render() {

// 	for (unit*u : my_buildings) {
// 		game->drawBoxMap(u->building->pos.x, u->building->pos.y, u->building->pos.x + 32, u->building->pos.y + 32, BWAPI::Colors::White);
// 	}

}

void init() {

	my_player = get_player(game->self());
	unit_types::init();

	multitasking::spawn(0,update_units_task,"update units");
	multitasking::spawn(update_buildings_walk_squares_task,"update buildings walk squares");

	render::add(render);

// 	log("supply depot armor: %d\n", my_player->game_player->armor(BWAPI::UnitTypes::Terran_Supply_Depot));
// 
// 	for (auto&v : BWAPI::UnitTypes::allUnitTypes()) {
// 		//unit_type*ut = get_unit_type(v);
// 		
// 		unit*u;
// 		//log("%s : %s %s\n", v.getName(), v.groundWeapon().damageType().getName(), v.airWeapon().damageType().getName());
// 		log("%s : %s %s\n", v.getName(), v.groundWeapon().getName(), v.airWeapon().getName());
// 		log("%s : %d %d\n", v.getName(), v.groundWeapon().damageAmount(), v.airWeapon().damageAmount());
// 		log("%s : %d %d\n", v.getName(), v.maxGroundHits(), v.maxAirHits());
// 		if (v.groundWeapon() != BWAPI::WeaponTypes::None && v.maxGroundHits() == 0) log("error\n");
// 		if (v.airWeapon() != BWAPI::WeaponTypes::None && v.maxAirHits() == 0) log("error\n");
// 	}
// 
// 	xcept("stop");

}


}



