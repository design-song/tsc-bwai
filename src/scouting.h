
namespace scouting {
;

double comsat_supply = 60.0;

struct scout {
	unit*scout_unit = nullptr;
	enum { type_default, type_scout_proxy };
	int type = type_default;

	refcounted_ptr<resource_spots::spot> dst_s;
	int dst_s_seen = 0;
	bool scout_resources = false;
	xy scout_location;
	a_deque<xy> path;
	int timer = 0;

	xy left, right, origin;
	xy a, b;

	void process();
};

a_unordered_map<resource_spots::spot*, int> last_scouted;

void scout::process() {

// 	if (scout_unit){
// 		if (scout_unit->controller->action != unit_controller::action_scout) scout_unit = nullptr;
// 		else if (scout_unit->dead) scout_unit = nullptr;
// 	}
	if (scout_unit){
		if (scout_unit->dead) scout_unit = nullptr;
	}

	if (!scout_unit) return;

	if (type == type_scout_proxy) {

		if (current_frame >= timer || diag_distance(scout_unit->pos - scout_location) <= 32 * 4) {
			timer = current_frame + 30;
			if (scout_location==xy()) scout_location = scout_unit->pos;
			xy dst;
			combat::find_nearby_entirely_walkable_tiles(scout_unit->pos, [&](xy pos) {
				if (diag_distance(pos - scout_location) >= 32 * 30) return false;
				if (dst != xy()) return false;
				auto&bs = grid::get_build_square(pos);
				if (bs.building) return false;
				if (current_frame - bs.last_seen >= 15 * 60 * 3) dst = pos;
				return true;
			}, 32 * 40);
			if (dst != xy()) {
				scout_unit->controller->action = unit_controller::action_scout;
				scout_unit->controller->go_to = dst;
			} else {
				scout_unit->controller->action = unit_controller::action_idle;
				scout_unit = nullptr;
			}
		}

		return;
	}

	if (current_used_total_supply < 30) {
		bool found_them = test_pred(enemy_buildings, [&](unit*e) {
			return e->type->is_resource_depot;
		});
		if (found_them && combat::my_closest_base != xy() && combat::op_closest_base != xy()) {
			if (timer <= current_frame || (current_frame-timer<=20 && diag_distance(scout_unit->pos - scout_unit->controller->go_to) <= 32 * 2)) {
				timer = current_frame + 15 * 2;
				path = combat::find_bigstep_path(scout_unit->type, combat::my_closest_base, combat::op_closest_base, square_pathing::pathing_map_index::no_enemy_buildings);
				if (path.size() > 8) {
					xy a = path[path.size() / 3];
					xy b = path[path.size() / 3 + 4];
					xy rel = b - a;
					double ang = std::atan2(rel.y, rel.x);
					ang += PI / 2;
					xy origin = path[path.size() / 3 + 2];
					auto look = [&](double mult) {
						xy pos = origin;
						for (double dist = 0; dist < 32 * 20; dist += 32) {
							xy r = origin;
							r.x += (int)(std::cos(ang)*dist*mult);
							r.y += (int)(std::sin(ang)*dist*mult);
							if (unit_pathing_distance(scout_unit->type, origin, r) >= 32 * 40) break;
							pos = r;
						}
						return square_pathing::get_nearest_node_pos(scout_unit->type, pos);
					};
					xy left = look(-1);
					xy right = look(1);
					this->a = a;
					this->b = b;
					this->origin = origin;
					this->left = left;
					this->right = right;
					scout_unit->controller->action = unit_controller::action_scout;
					if (scout_unit->controller->go_to != left && scout_unit->controller->go_to != right) {
						scout_unit->controller->go_to = left;
					} else {
						if (diag_distance(scout_unit->pos - scout_unit->controller->go_to) <= 32 * 2) {
							if (scout_unit->controller->go_to == left) scout_unit->controller->go_to = right;
							else scout_unit->controller->go_to = left;
						}
					}
				}
			}
			return;
		}
	}

	if (!dst_s) {
		dst_s = get_best_score_p(resource_spots::spots, [&](resource_spots::spot*s) {
			double d;
			if (!enemy_buildings.empty()) {
				d = get_best_score_value(enemy_buildings, [&](unit*u) {
					return unit_pathing_distance(scout_unit->type, u->pos, s->cc_build_pos);
				});
			} else {
				d = get_best_score_value(start_locations, [&](xy pos) {
					return unit_pathing_distance(scout_unit->type, pos, s->cc_build_pos);
				});
			}
			if (current_frame - last_scouted[s] <= 15 * 4) d += 4000;
			auto&bs = grid::get_build_square(s->cc_build_pos);
			int age = current_frame - bs.last_seen;
			//d /= std::max(age, 1);
			d -= age * 2;
			d += unit_pathing_distance(scout_unit, s->cc_build_pos) / 10000;
			// todo: find safe path, pathing for flying, better stuff...
			if (!scout_unit->is_flying) {
				for (auto*n : square_pathing::find_path(square_pathing::get_pathing_map(scout_unit->type), scout_unit->pos, s->pos)) {
					for (unit*e : enemy_units) {
						weapon_stats*w = scout_unit->is_flying ? e->stats->air_weapon : e->stats->ground_weapon;
						if (!w) continue;
						double d = diag_distance(n->pos - e->pos);
						if (d <= w->max_range * 2) d += 100;
					}
				}
			}
			return d;
		}, std::numeric_limits<double>::infinity());
		if (dst_s) {
			dst_s_seen = grid::get_build_square(dst_s->cc_build_pos).last_seen;
			scout_resources = false;
		}
	}

	if (!dst_s) return;

	last_scouted[&*dst_s] = current_frame;

	if (dst_s_seen != -1) {
		if (!square_pathing::unit_can_reach(scout_unit, scout_unit->pos, dst_s->cc_build_pos, square_pathing::pathing_map_index::no_enemy_buildings)) {
			dst_s = nullptr;
		} else {
			scout_unit->controller->action = unit_controller::action_scout;
			scout_unit->controller->go_to = dst_s->cc_build_pos;
			if (!scout_resources) {
				if (grid::is_visible(dst_s->cc_build_pos, 4, 3)) scout_resources = true;
			} else {
				auto*r = get_best_score_p(dst_s->resources, [&](const resource_spots::resource_t*r) {
					double age = current_frame - r->u->last_seen;
					if (age <= 15 * 10) return std::numeric_limits<double>::infinity();
					return -age;
				}, std::numeric_limits<double>::infinity());
				if (!r) dst_s_seen = -1;
				else scout_unit->controller->go_to = r->u->pos;
			}
		}
	} else {
		dst_s = nullptr;
	}

}

xy scan_best_pos;
double scan_best_score;
void scan() {

	double scan_energy_cost = bwapi_tech_type_energy_cost(BWAPI::TechTypes::Scanner_Sweep);
	int scans_available = 0;
	int comsats = 0;
	for (unit*u : my_units_of_type[unit_types::comsat_station]) {
		++comsats;
		if (u->energy < scan_energy_cost) continue;
		scans_available += (int)(u->energy / scan_energy_cost);
	}

	a_map<xy, double> values;

	for (unit*e : enemy_units) {
		if (e->gone) continue;
		if (!e->cloaked || e->detected) continue;
		int in_range_count = 0;
		for (unit*u : my_units) {
			if (u->type->is_non_usable) continue;
			weapon_stats*w = e->is_flying ? u->stats->air_weapon : u->stats->ground_weapon;
			if (!w) continue;
			if (diag_distance(e->pos - u->pos) <= w->max_range) ++in_range_count;
		}
		if (in_range_count >= 3) values[e->pos] += 100000;
	}
	for (unit*e : enemy_units) {
		if (e->gone) continue;
		if (current_frame < e->scan_me_until) values[e->pos] += 20000;
	}
	if (scans_available > 2) {
		for (auto&s : resource_spots::spots) {
			int t = (current_frame - grid::get_build_square(s.pos).last_seen) - 15 * 60 * 10;
			if (t < 0) continue;
			values[s.pos] += t;
		}
	}
	for (unit*u : my_workers) {
		if (u->controller->action != unit_controller::action_build) continue;
		if (u->controller->fail_build_count >= 10) {
			values[u->pos] += 10000;
		}
	}
	if (buildpred::op_unverified_minerals_gathered>3000) {
		for (unit*e : enemy_units) {
			if (e->visible || e->gone) continue;
			if (e->type->is_building) continue;
			values[e->pos] += e->minerals_value + e->gas_value;
		}
	}
	int follow_worker_count = 0;
	for (unit*e : enemy_units) {
		if (e->visible || e->gone) continue;
		if (!e->type->is_worker) continue;
		if (combat::op_base.test(grid::build_square_index(e->pos))) continue;
		if (++follow_worker_count >= 4) break;
		auto*s = get_best_score_p(resource_spots::spots, [&](resource_spots::spot*s) {
			if (grid::is_visible(s->pos)) return std::numeric_limits<double>::infinity();
			return unit_pathing_distance(e, s->pos);
		}, std::numeric_limits<double>::infinity());
		if (s) values[s->pos] += 3000;
	}
	for (size_t idx : combat::op_base) {
		xy pos((idx % (size_t)grid::build_grid_width) * 32, (idx / (size_t)grid::build_grid_width) * 32);
		int last_seen = grid::get_build_square(pos).last_seen;
		int age = current_frame - last_seen;
		if (age <= 15 * 60 * 4) continue;
		if (build_spot_finder::is_buildable_at(unit_types::barracks, pos)) {
			values[pos] += age / 200;
		}
	}

	for (auto&v : buildpred::opponent_states) {
		auto&st = std::get<1>(v);
		for (auto&ri : st.resource_info) {
			values[ri.first->u->pos] += ri.second.gathered;
		}
		for (auto&v : st.bases) {
			values[v.s->pos] += 20;
			if (!v.verified) values[v.s->cc_build_pos] += 10000;
		}
	}
	auto*scan_st = units::get_unit_stats(unit_types::spell_scanner_sweep, players::my_player);
	double best_score = 0.0;
	xy best_pos;
	a_vector<unit*> detectors;
	for (unit*u : my_units) {
		if (u->type->is_detector) detectors.push_back(u);
	}
	for (auto&v : values) {
		double s = 0.0;
		for (auto&v2 : values) {
			bool is_revealed = false;
			for (unit*u : detectors) {
				if ((u->pos - v2.first).length() <= u->stats->sight_range) {
					is_revealed = true;
					break;
				}
			}
			if (is_revealed) continue;
			if (&v2 == &v || (v.first - v2.first).length() <= scan_st->sight_range) s += v2.second;
		}
		if (s > best_score) {
			best_score = s;
			best_pos = v.first;
		}
	}
	scan_best_pos = best_pos;
	scan_best_score = best_score;
	bool use_scan = false;
	use_scan |= best_score >= 3000.0 && scans_available > comsats * 2;
	use_scan |= best_score >= 6000.0 && scans_available > comsats;
	use_scan |= best_score >= 10000.0 && scans_available > 0;
	if (use_scan) {
		unit*u = get_best_score(my_units_of_type[unit_types::comsat_station], [&](unit*u) {
			return -u->energy;
		});
		if (u) {
			u->game_unit->useTech(BWAPI::TechTypes::Scanner_Sweep, BWAPI::Position(best_pos.x, best_pos.y));
		}
	}
}

a_vector<scout> all_scouts;

scout&add_scout(unit*u) {
	scout sc;
	sc.scout_unit = u;
	all_scouts.push_back(sc);
	return all_scouts.back();
}
void rm_scout(unit*u) {
	for (auto&v : all_scouts) {
		if (v.scout_unit == u) {
			if (u && u->controller->action == unit_controller::action_scout) u->controller->action = unit_controller::action_idle;
			v.scout_unit = nullptr;
		}
	}
}

int last_scout = 0;
int last_vulture_scout = 0;
int last_proxy_scout = 0;

void process_scouts() {

	if (all_scouts.empty()) {
		if (last_scout == 0 || current_frame - last_scout >= 15 * 60 * 3 || current_used_total_supply >= 100) {
			if (my_workers.size() < 8) return;
			unit*scout_unit = nullptr;
			if (my_completed_units_of_type[unit_types::vulture].size() >= 15 && current_frame - last_vulture_scout >= 15 * 60 * 2) {
				last_vulture_scout = current_frame;
				scout_unit = get_best_score(my_completed_units_of_type[unit_types::vulture], [&](unit*u) {
					return u->last_attacked;
				}, std::numeric_limits<double>::infinity());
			} else {
				scout_unit = get_best_score(my_workers, [&](unit*u) {
					if (u->controller->action != unit_controller::action_gather) return std::numeric_limits<double>::infinity();
					return 0.0;
				}, std::numeric_limits<double>::infinity());
			}
			if (scout_unit) last_scout = current_frame;
			if (scout_unit) add_scout(scout_unit);
		}
	}
	if (current_frame <= 15 * 60 * 8 && my_completed_units_of_type[unit_types::marine].empty()) {
		if (current_frame - last_proxy_scout >= 15 * 60 * 3) {
			last_proxy_scout = current_frame;
			unit*scout_unit = get_best_score(my_workers, [&](unit*u) {
				if (u->controller->action != unit_controller::action_gather) return std::numeric_limits<double>::infinity();
				return 0.0;
			}, std::numeric_limits<double>::infinity());
			if (scout_unit) add_scout(scout_unit).type = scout::type_scout_proxy;
		}
	}

	for (auto i = all_scouts.begin(); i != all_scouts.end();) {
		if (i->scout_unit == nullptr) i = all_scouts.erase(i);
		else {
			i->process();
			++i;
		}
	}

}

void scouting_task() {

	int last_scan = 0;

	while (true) {

		multitasking::sleep(4);

		process_scouts();

		if (current_used_total_supply >= comsat_supply || (!my_completed_units_of_type[unit_types::academy].empty() && current_used_total_supply >= 40)) {
			if (!my_units_of_type[unit_types::cc].empty()) {
				if (my_units_of_type[unit_types::academy].empty()) {
					build::add_build_sum(0, unit_types::academy, 1);
				} else {
					bool is_building_nuclear_silo = false;
					for (auto&t : build::build_tasks) {
						if (t.type->unit == unit_types::nuclear_silo) {
							is_building_nuclear_silo = true;
							break;
						}
					}
					if (!is_building_nuclear_silo) {
						for (unit*cc : my_units_of_type[unit_types::cc]) {
							if (cc->addon) continue;
							if (cc->building->is_lifted) continue;
							//if (cc->game_unit->canBuildAddon(unit_types::comsat_station->game_unit_type)) {
							if (true) {
								if (my_units_of_type[unit_types::refinery].empty()) {
									build::add_build_sum(0, unit_types::refinery, 1);
								} else build::add_build_sum(0, unit_types::comsat_station, 1);
								break;
							}
						}
					} else {
						for (auto&t : build::build_tasks) {
							if (t.type->unit == unit_types::comsat_station) {
								build::cancel_build_task(&t);
								break;
							}
						}
					}
				}
			}
		}

		if (current_frame - last_scan >= 15 * 5) {
			last_scan = current_frame;
			scan();
		}
		
	}
}

void render() {

	for (auto&v : all_scouts) {
		if (v.scout_unit) {
			if (v.dst_s) game->drawLineMap(v.scout_unit->pos.x, v.scout_unit->pos.y, v.dst_s->pos.x, v.dst_s->pos.y, BWAPI::Colors::Blue);
		}

		if (!v.path.empty()) {
			xy lp;
			for (xy p : v.path) {
				if (lp != xy()) {
					game->drawLineMap(lp.x, lp.y, p.x, p.y, BWAPI::Colors::Blue);
				}
				lp = p;
			}
		}
		game->drawLineMap(v.a.x, v.a.y, v.b.x, v.b.y, BWAPI::Colors::Red);
		game->drawCircleMap(v.origin.x, v.origin.y, 24, BWAPI::Colors::Green);
		game->drawLineMap(v.left.x, v.left.y, v.right.x, v.right.y, BWAPI::Colors::Green);
	}

	auto*scan_st = units::get_unit_stats(unit_types::spell_scanner_sweep, players::my_player);
	game->drawCircleMap(scan_best_pos.x, scan_best_pos.y, (int)scan_st->sight_range, BWAPI::Colors::Blue);
	game->drawTextMap(scan_best_pos.x, scan_best_pos.y, "\x0e%g", scan_best_score);

}

void init() {

	multitasking::spawn(scouting_task, "scouting");

	render::add(render);

}

}

