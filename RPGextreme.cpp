#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <unordered_set>
#include <map>
#include <tuple>

enum class ACCESSORY_LIST {
	HR, ///> 전투에서 승리할 때마다 체력을 3 회복한다. 체력은 최대 체력 수치까지만 회복된다.
	RE, ///> 주인공이 사망했을 때 소멸하며, 주인공을 최대 체력으로 부활시켜 준 뒤, 주인공을 첫 시작 위치로 돌려보낸다. 
	// 레벨이나 장비 등의 다른 정보는 변함이 없다. 전투 중이던 몬스터가 있다면 해당 몬스터의 체력도 최대치로 회복된다. 
	// 소멸한 뒤에 다시 이 장신구를 얻는다면 또 착용한다.
	CO, ///> 모든 전투에서, 첫 번째 공격에서 주인공의 공격력(무기 합산)이 두 배로 적용된다. 
	// 즉, 모든 첫 공격에서 몬스터에게 max(1, 공격력×2 – 몬스터의 방어력)만큼의 데미지를 입힌다.
	EX, ///> 얻는 경험치가 1.2배가 된다. 소수점 아래는 버린다.
	DX, ///> 가시 함정에 입는 데미지가 1로 고정되며, 
	// Courage 장신구와 함께 착용할 경우, Courage의 공격력 효과가 두 배로 적용되는 대신 세 배로 적용된다.
	HU, ///> 보스 몬스터와 전투에 돌입하는 순간 체력을 최대치까지 회복하고, 보스 몬스터의 첫 공격에 0의 데미지를 입는다.
	CU ///>  아무 능력이 없으며, 그냥 장신구 한 자리를 차지한다.
};
// std::ostream& operator<<(std::ostream& os, const MAP_CONTENTS& contents)
std::ostream& operator<<(std::ostream& os, const ACCESSORY_LIST& ornament) {
	switch (ornament)
	{
	case ACCESSORY_LIST::HR:
		os << "HR";
		break;
	case ACCESSORY_LIST::RE:
		os << "RE";
		break;
	case ACCESSORY_LIST::CO:
		os << "CO";
		break;
	case ACCESSORY_LIST::EX:
		os << "EX";
		break;
	case ACCESSORY_LIST::DX:
		os << "DX";
		break;
	case ACCESSORY_LIST::HU:
		os << "HU";
		break;
	case ACCESSORY_LIST::CU:
		os << "CU";
		break;
	default:
		os << "Undefined Ornament";
		break;
	}

	return os;
}

class Character {
public:
	auto get_name() const { return name; }
	virtual int get_health() const { return health; }
	virtual int get_attack() const { return attack; }
	virtual int get_defense() const { return defense; }
	auto set_name(const std::string& name) -> void { this->name = name; }
	auto set_health(const int health) -> void { this->health = health; }
	auto set_attack(const int attack) -> void { this->attack = attack; }
	auto set_defense(const int defense) -> void { this->defense = defense; }
	// damage만큼의 체력을 뺌. 이 damage는 방어력과 상관 없이 제외됨
	auto get_damaged_health(const int damage) {
		set_health(get_health() -  damage);
	}

	virtual void health_up(const int heal) {
		set_health(get_health() + heal);
	}

	// 상대방을 공격함. 이때 입힌 damage는 max(1, 내 공격력 - 상대방 방어력)
	void attack_opponent(Character& opponent) {
		opponent.get_damaged_health(std::max(1, this->get_attack() - opponent.get_defense()));
	}

	auto get_exp() const { return experience; }
	auto is_alive() const { return get_health() > 0; }
protected:
	std::string name;
	int health;
	int attack;
	int defense;
	int experience;
};

class Monster : public Character {
public:
	Monster() {
		this->name = "";
		this->health = 0;
		this->attack = 0;
		this->defense = 0;
		this->experience = 0;
		this->max_health = 0;
	}

	Monster(const std::string name, const int health, const int attack, const int defense, const int experience) {
		this->name = name;
		this->health = health;
		this->attack = attack;
		this->defense = defense;
		this->experience = experience;
		this->max_health = health;
	}

	Monster(const Monster& monster) 
		: Monster(monster.name, 
			monster.health, 
			monster.attack,
			monster.defense, 
			monster.experience) {}

	auto get_max_health() const { return max_health; }

	using Character::attack_opponent;
	/// <summary>
	/// opponent에게 damage만큼의 피해를 가함
	/// 이때 가한 데미지 - opponent의 방어력 만큼 데미지가 차감됨
	/// 단 음(-)의 데미지를 가할 수 없으므로, 음의 수라면 1의 데미지가 됨
	/// 즉 공격력 < 상대방 방어력이라면 1만큼 데미지가 들어감
	/// </summary>
	/// <param name="opponent">공격하는 상대방</param>
	/// <param name="damage">가하는 데미지</param>
	void attack_opponent(Character& opponent, const int damage) {
		opponent.set_health(opponent.get_health() - std::max(1, damage - opponent.get_defense()));
	}
private:
	int max_health;
};

class Weapon {
public:
	Weapon() :attack(0) {}
	Weapon(const int attack) : attack(attack) {}
	auto get_attack() const { return attack; }
private:
	int attack;
};

class Armour {
public:
	Armour() : defence(0) {}
	Armour(const int defence) : defence(defence) {}
	auto get_defence() const { return defence; }

private:
	int defence;
};

class Player : public Character {
public:
	Player() : Player(20, 2, 2, 1) {} // 기본 생성자
	Player(const int health, const int attack, const int defense, const int level) {
		this->health = health;
		this->attack = attack;
		this->defense = defense;
		max_health = this->health;
		this->level = level;
		threshold_to_levelUp = 5 * level;
		max_accessory = 4;
		accessories.reserve(max_accessory);
	}

	using Character::attack_opponent;
	/// <summary>
	/// opponent에게 damage만큼의 피해를 가함
	/// 이때 가한 데미지 - opponent의 방어력 만큼 데미지가 차감됨
	/// 단 음(-)의 데미지를 가할 수 없으므로, 음의 수라면 1의 데미지가 됨
	/// 즉 공격력 < 상대방 방어력이라면 1만큼 데미지가 들어감
	/// </summary>
	/// <param name="opponent">공격하는 상대방</param>
	/// <param name="damage">가하는 데미지, 0이 될 수도 있음</param>
	void attack_opponent(Character& opponent, const int damage) {
		opponent.set_health(opponent.get_health() - std::max(1, damage - opponent.get_defense()));
	}


	auto getMaxHealth() const { return max_health; }
	auto getMaxAccessoryNumber() const { return max_accessory; }
	const auto& getAccessories() const { return accessories; }
	auto get_current_accessory_number() const { return accessories.size(); }
	
	void health_up(const int heal) override {
		set_health(std::min(getMaxHealth(), get_health() + heal));
	}

	void levelUp() {
		level += 1;
		threshold_to_levelUp = 5 * level;
		max_health += 5;
		attack += 2;
		defense += 2;
		experience = 0;

		set_health(max_health);
	}

	auto exprience_up(const int exp) {
		this->experience += exp;
		if (this->experience >= threshold_to_levelUp) {
			levelUp();
		}
	}

	auto setArmour(const Armour& armour) { this->armour = std::make_unique<Armour>(armour);	}
	auto setWeapon(const Weapon& weapon) { this->weapon = std::make_unique<Weapon>(weapon);	}

	int get_defense() const override { 
		if (armour.get() == nullptr) return defense;
		return defense + armour->get_defence(); 
	}
	int get_attack() const override { 
		if (weapon.get() == nullptr) return attack;
		return attack + weapon->get_attack();	
	}

	auto get_pure_attack() const { return attack; }
	auto get_pure_defense() const {	return defense;	}

	auto get_weapon_attack() const {
		if (weapon.get() == nullptr) return 0;
		return weapon->get_attack(); 
	}
	auto get_armour_defense() const { 
		if (armour.get() == nullptr) return 0;
		return armour->get_defence();	
	}

	auto setAccessory(const ACCESSORY_LIST& gear) {
		if (accessories.size() >= max_accessory) { return; }
		accessories.emplace(gear);
	}
	auto setAccessory(const ACCESSORY_LIST&& gear) {
		if (accessories.size() >= max_accessory) { return; }
		accessories.emplace(gear);
	}

	auto isAccessoryExist(const ACCESSORY_LIST& gear) {
		if (accessories.find(gear) == accessories.end()) { 
			return false; 
		}
		else return true;
	}

	auto popAccessory(const ACCESSORY_LIST& gear) {
		accessories.erase(gear);
	}

	auto printPlayerInfo() const {
		std::cout << "LV : " << level << std::endl;
		int hp = get_health() >= 0 ? get_health() : 0;  ///> HP가 음수일 때는 0으로 표시
		std::cout << "HP : " << hp << '/' << getMaxHealth() << std::endl;
		std::cout << "ATT : " << get_pure_attack() << '+' << get_weapon_attack() << std::endl;
		std::cout << "DEF : " << get_pure_defense() << '+' << get_armour_defense() << std::endl;
		std::cout << "EXP : " << get_exp() << '/' << threshold_to_levelUp << std::endl;
		/*std::cout << "ORNANMENT : ";
		for (const auto& orn : accessories) {
			std::cout << orn << " ";
		}
		std::cout << std::endl;*/
	}
private:
	int max_health; ///> 최대 체력
	int level; ///> 현재 레벨
	int threshold_to_levelUp; ///> 레벨업까지 필요한 기준점
	// threshold_to_levelUp = 5 * level

	int max_accessory = 4;

	std::unique_ptr<Armour> armour;
	std::unique_ptr<Weapon> weapon;
	std::unordered_set<ACCESSORY_LIST> accessories;
};



enum class MAP_CONTENTS : char {
	EMPTY = '.',
	WALL = '#',
	BOX = 'B',
	TRAP = '^',
	MONSTER = '&',
	BOSS_MONSTER = 'M',
	PLAYER = '@'
};

enum class ITEMTYPE : char {
	WEAPON = 'W',
	ARMOUR = 'A',
	ORNAMENT = 'O' ///> == accessory
};

std::ostream& operator<<(std::ostream& os, const MAP_CONTENTS& contents) {
	os << static_cast<char>(contents);
	return os;
}

// Only for pairs of std::hash-able types for simplicity.
// You can of course template this struct to allow other hash functions
struct pair_hash {
	template <class T1, class T2>
	std::size_t operator () (const std::pair<T1, T2>& p) const {
		auto h1 = std::hash<T1>{}(p.first);
		auto h2 = std::hash<T2>{}(p.second);

		// Mainly for demonstration purposes, i.e. works but is overly simple
		// In the real world, use sth. like boost.hash_combine
		return h1 ^ h2;
	}
};

using Position = std::pair<int, int>;
using WorldMap = std::vector< std::vector<MAP_CONTENTS> >;
template <class T>
using HashMap = std::unordered_map< Position, T, pair_hash>;


class World {
public:
	void createWorld(const std::vector<std::string>& inputMap) {
		world.reserve(inputMap.size() + 2);
		for (auto& row : world) {
			world.reserve(row.size() + 2);
		}
		std::vector<MAP_CONTENTS> padding(inputMap[0].size() + 2, MAP_CONTENTS::WALL);
		world.emplace_back(padding); // padding
		for (const auto& str : inputMap) {
			std::vector<MAP_CONTENTS> temp;
			temp.emplace_back(MAP_CONTENTS::WALL); // padding
			for (const auto& c : str) {
				temp.emplace_back(static_cast<MAP_CONTENTS>(c));
			}
			temp.emplace_back(MAP_CONTENTS::WALL); // padding
			world.emplace_back(temp);
		}
		world.emplace_back(padding); // padding
	}

	auto setPosition(const int row, const int col, const MAP_CONTENTS content) {
		world[row][col] = content;
	}

	// world.setMonsterInfo(row, col, name, health, atk, defense, exp);
	void setMonsterInfo(
		const int row, 
		const int col, 
		const std::string name,
		const int health,
		const int attack, 
		const int defense, 
		const int exp) {
		Monster monster(name, health, attack, defense, exp);
		monsterInfo.emplace(std::move(std::make_pair(row, col)), monster);
	}

	auto setWeaponInfo(const int row, const int col, const int atk) {
		weaponInfo.emplace(std::move(std::make_pair(row, col)), std::move(Weapon(atk)));
	}

	auto setArmourInfo(const int row, const int col, const int defense) {
		armourInfo.emplace(std::move(std::make_pair(row, col)), std::move(Armour(defense)));
	}

	auto setAccessoryInfo(const int row, const int col, const ACCESSORY_LIST accessory) {
		accessoryInfo.emplace(std::move(std::make_pair(row, col)), accessory);
	}

	auto getContents(const int X, const int Y) { return world[X][Y]; }

	auto getArmourInfo(const int row, const int col) {
		return armourInfo[std::make_pair(row, col)];
	}

	auto getMonsterInfo(const int row, const int col) {
		return monsterInfo[std::make_pair(row, col)];
	}

	auto getWeaponInfo(const int row, const int col) {
		return weaponInfo[std::make_pair(row, col)];
	}

	auto getAccessoryInfo(const int row, const int col) {
		return accessoryInfo[std::make_pair(row, col)];
	}

	// return number of monsters, including boss 'M'
	std::tuple<int, int> getWorldInfo() const {
		int monsterSum = 0;
		const int bossMonster = 1; ///> boss monster의 개수는 1

		int boxSum = 0;

		for (const auto& col : world) {
			monsterSum += std::count(col.begin(), col.end(), MAP_CONTENTS::MONSTER);
			boxSum += std::count(col.begin(), col.end(), MAP_CONTENTS::BOX);
		}

		return { monsterSum + bossMonster, boxSum };
	}

	auto getPlayerPosition() const {
		for (std::size_t i = 0; i < world.size(); i++) {
			for (std::size_t j = 0; j < world[i].size(); j++) {
				if (world[i][j] == MAP_CONTENTS::PLAYER) {
					return std::make_pair(i, j);
				}
			}
		}
	}

	auto getPositionInfo(const int X, const int Y) {
		return world[X][Y];
	}

	auto printWorld() const {
		for (std::size_t i = 1; i < world.size() - 1; i++) {
			for (std::size_t j = 1; j < world[i].size() - 1; j++) {
				std::cout << world[i][j];
			}
			std::cout << std::endl;
		}
	}

	auto getItemType(const int X, const int Y) {
		auto pos = std::make_pair(X, Y);
		
		if (armourInfo.find(pos) != armourInfo.end()) return ITEMTYPE::ARMOUR;
		else if (weaponInfo.find(pos) != weaponInfo.end()) return ITEMTYPE::WEAPON;
		else if (accessoryInfo.find(pos) != accessoryInfo.end()) return ITEMTYPE::ORNAMENT;
		else throw("ITEM DOES NOT EXISTs");
	}
private:
	std::vector< std::vector<MAP_CONTENTS> > world; ///> 맵, 전장
	HashMap<Monster> monsterInfo; ///> 몬스터 정보
	HashMap<Weapon> weaponInfo; ///> 무기 정보
	HashMap<Armour> armourInfo; ///> 방어구 정보
	HashMap<ACCESSORY_LIST> accessoryInfo; ///> 악세서리 정보
	std::pair<int, int> playerPosition; ///> player의 포지션
};

enum class MOVE_DIRECTION : char
{
	LEFT = 'L', 
	RIGHT = 'R',
	UP = 'U',
	DOWN = 'D'
};

enum class TURN_RESULT {
	PLAYER_DIED, ///> 플레이어 사망
	PLAYER_ALIVE, ///> 플레이어 생존, 계속 진행
	BOSS_DIED, ///> 보스 사망, 게임 우승
};

class RPGExtreme {
public:
	auto inputWorld() {
		int N = 0; int M = 0;
		std::cin >> N >> M;
		std::vector<std::string> worldInput;
		for (int i = 0; i < N; i++) {
			std::string temp = "";
			std::cin >> temp;
			worldInput.emplace_back(temp);
		}

		world.createWorld(worldInput);
		auto [monster, box] = world.getWorldInfo();
		monsterCount = monster;
		boxCount = box;
		playerPosition = world.getPlayerPosition();
		initialPlayerPosition.first = playerPosition.first;
		initialPlayerPosition.second = playerPosition.second;
	}

	auto inputMonsterInfo() {
		int row = 0; int col = 0;
		std::string name = "";
		int atk = 0;
		int defense = 0;
		int health = 0;
		int exp = 0;
		for (int i = 0; i < monsterCount; i++) {
			std::cin >> row >> col >> name >> atk >> defense >> health >> exp;
			world.setMonsterInfo(row, col, name, health, atk, defense, exp);
		}
	}

	auto inputItemInfo() {
		int row = 0; int col = 0;
		char type = 0;
		for (int i = 0; i < boxCount; i++) {
			std::cin >> row >> col >> type;
			int figure = 0;
			std::string	ornaType = "";
			switch (type) {
			case static_cast<char>(ITEMTYPE::WEAPON):
				std::cin >> figure;
				world.setWeaponInfo(row, col, figure);
				break;
			case static_cast<char>(ITEMTYPE::ARMOUR):
				std::cin >> figure;
				world.setArmourInfo(row, col, figure);
				break;
			case static_cast<char>(ITEMTYPE::ORNAMENT):
				std::cin >> ornaType;
				world.setAccessoryInfo(row, col, ORNAMENT_TYPE[ornaType]);
			default:
				break;
			}
		}		
	}

	auto movePlayer(const MOVE_DIRECTION direction) {
		MAP_CONTENTS result = MAP_CONTENTS::PLAYER;
		switch (direction) {
		case MOVE_DIRECTION::LEFT:
			result = moveLeft();
			break;
		case MOVE_DIRECTION::RIGHT:
			result = moveRight();
			break;
		case MOVE_DIRECTION::UP:
			result = moveUp();
			break;
		case MOVE_DIRECTION::DOWN:
			result = moveDown();
			break;
		default:
			break;
		}
		return result;
	}

	/// <summary>
	/// Fight with opponent
	/// </summary>
	/// <param name="monster">opponent that fight with player</param>
	/// <param name="courage">multiply the player's attack skill once</param>
	void fight(Monster& monster, const int courage) {
		player.attack_opponent(monster, courage * player.get_attack());
		// 만약 플레이어의 첫 공격에도 몬스터가 살아있다면,
		// 서로 일합식 주고 받음
		while (monster.is_alive() && player.is_alive()) {
			monster.attack_opponent(player);
			if (!player.is_alive()) break;
			player.attack_opponent(monster);
		}
	}
	/// <summary>
	/// Fight with opponent, First Attack by boss equals monsterAttack(only once)
	/// then next equal as original boss attack
	/// </summary>
	/// <param name="monster">opponent that fight with player</param>
	/// <param name="courage">multiply the player's attack skill once</param>
	/// <param name="monsterAttack">equals first boss attack</param>
	void fight(Monster& monster, const int courage, const int monsterAttack) {
		player.attack_opponent(monster, courage * player.get_attack());
		// 만약 플레이어의 첫 공격에도 몬스터가 살아있다면 몬스터가 공격함
		if (monsterAttack != 0 && monster.is_alive()) {
			// monsterAttack == 0 이면 몬스터는 공격하지 않고 턴을 넘김
			monster.attack_opponent(player);
		}
		// 보스의 첫 공격에도 플레이어가 살아있다면 서로 한 합 식 주고 받음
		while (player.is_alive() && monster.is_alive()) {
			player.attack_opponent(monster);
			if (!monster.is_alive()) break;
			monster.attack_opponent(player);
		}
	}

	/*
	* // player 승리, monster 사망 시
			if (player.is_alive()) {
				if (player.isAccessoryExist(ACCESSORY_LIST::EX)) {
					// EX 착용 시 EXP 계수 1.2배
					player.exprience_up(1.2 * opponentMonster.get_exp());
				}
				else {
					player.exprience_up(opponentMonster.get_exp());
				}

				if (player.isAccessoryExist(ACCESSORY_LIST::HR)) {
					// HR 착용 시 체력 3씩 회복
					player.health_up(HR_regeneration);
				}
			}
	*/
	void playerWin(const Monster& monster) {
		if (player.isAccessoryExist(ACCESSORY_LIST::EX)) {
			// EX 착용 시 EXP 계수 1.2배
			player.exprience_up(1.2 * monster.get_exp());
		}
		else {
			player.exprience_up(monster.get_exp());
		}
		if (player.isAccessoryExist(ACCESSORY_LIST::HR)) {
			// HR 착용 시 승리 후 체력 3씩 회복
			player.health_up(HR_regeneration);
		}
	}

	auto action(const MAP_CONTENTS result) {
		auto xPos = playerPosition.first;
		auto yPos = playerPosition.second;
		auto turnResult = TURN_RESULT::PLAYER_ALIVE;
		std::string killedBy = "";

		switch (result) {
		case MAP_CONTENTS::TRAP:
			// 가시 함정 -> 방어력에 상관 없이 체력을 trapDamage만큼 깎임
			// 이때 DX가 있다면 1만 깎인다
			if (player.isAccessoryExist(ACCESSORY_LIST::DX)) {
				player.get_damaged_health(1);
			}
			else {
				player.get_damaged_health(getTrapDamage());
			}

			// player 사망 시
			if (!player.is_alive()) {
				if (player.isAccessoryExist(ACCESSORY_LIST::RE)) {
					reincarnation();
				}
				else {
					turnResult = TURN_RESULT::PLAYER_DIED;
					killedBy = "SPIKE TRAP";
				}
			}
			break;
		case MAP_CONTENTS::BOX:
		{
			ITEMTYPE type;
			type = world.getItemType(xPos, yPos);

			switch (type)
			{
			case ITEMTYPE::WEAPON:
			{
				auto weapon = world.getWeaponInfo(xPos, yPos);
				player.setWeapon(weapon);
				break;
			}
			case ITEMTYPE::ARMOUR:
			{
				auto armour = world.getArmourInfo(xPos, yPos);
				player.setArmour(armour);
				break;
			}
			case ITEMTYPE::ORNAMENT:
			{
				auto ornament = world.getAccessoryInfo(xPos, yPos);
				player.setAccessory(ornament);
				break;
			}
			default:
				break;
			}
			break;
		}
			// 몬스터와 전투 시작
		case MAP_CONTENTS::MONSTER: 
		{
			auto opponentMonster = world.getMonsterInfo(xPos, yPos);
			auto courageAttack = 1;
			if (player.isAccessoryExist(ACCESSORY_LIST::CO)) {
				if (player.isAccessoryExist(ACCESSORY_LIST::DX)) courageAttack = 3;
				else courageAttack = 2;
			}
			fight(opponentMonster, courageAttack);

			// player 승리, monster 사망 시
			if (player.is_alive()) {
				playerWin(opponentMonster);
			}
			// player 사망
			else {
				if (player.isAccessoryExist(ACCESSORY_LIST::RE)) {
					reincarnation(opponentMonster);
				}
				else {
					turnResult = TURN_RESULT::PLAYER_DIED;
					killedBy = opponentMonster.get_name();
				}
			}
			break;
		}
		case MAP_CONTENTS::BOSS_MONSTER:
		{
			auto bossMonster = world.getMonsterInfo(xPos, yPos);
			auto courageAttack = 1;
			auto bossAttack = bossMonster.get_attack();
			if (player.isAccessoryExist(ACCESSORY_LIST::CO)) {
				if (player.isAccessoryExist(ACCESSORY_LIST::DX)) courageAttack = 3;
				else courageAttack = 2;
			}
			if (player.isAccessoryExist(ACCESSORY_LIST::HU)) {
				// HU 아이템 장착 시 maxHealth로 체력을 회복하고, 첫 공격을 0으로 만든다.
				player.set_health(player.getMaxHealth());
				bossAttack = 0;
			}

			fight(bossMonster, courageAttack, bossAttack);
			// player 승리, monster 사망 시
			if (player.is_alive()) {
				playerWin(bossMonster);
				turnResult = TURN_RESULT::BOSS_DIED;
			}
			// player 사망 시
			else {
				if (player.isAccessoryExist(ACCESSORY_LIST::RE)) {
					reincarnation(bossMonster, MAP_CONTENTS::BOSS_MONSTER);
				}
				else {
					turnResult = TURN_RESULT::PLAYER_DIED;
					killedBy = bossMonster.get_name();
				}
			}
			break;
		}
		// 기타 나머지는 모두 break 처리
		default:
			break;
		}

		return std::make_tuple(turnResult, killedBy);
	}

	auto playGame(const std::string& behavior) {
		int passedTurn = 0;
		TURN_RESULT turnResult = TURN_RESULT::PLAYER_ALIVE;
		std::string killedBy = "";
		for (const auto dir : behavior) {
			passedTurn++;
			const auto direction = static_cast<MOVE_DIRECTION>(dir);
			auto contents = movePlayer(direction);
			auto [t_result, killed_by] = action(contents);
			turnResult = t_result; killedBy = killed_by;
			if (turnResult == TURN_RESULT::PLAYER_DIED) {
				break;
			}
			if (turnResult == TURN_RESULT::BOSS_DIED) {
				break;
			}
		}
		if (turnResult != TURN_RESULT::PLAYER_DIED) {
			world.setPosition(playerPosition.first, playerPosition.second, MAP_CONTENTS::PLAYER);
		}
		world.printWorld();
		std::cout << "Passed Turns : " << passedTurn << std::endl;

		if (player.get_health() < 0) {
			player.set_health(0);
		}
		player.printPlayerInfo();
		switch (turnResult)
		{
		case TURN_RESULT::PLAYER_DIED:
			std::cout << "YOU HAVE BEEN KILLED BY " << killedBy << ".." << std::endl;
			break;
		case TURN_RESULT::PLAYER_ALIVE:
			std::cout << "Press any key to continue." << std::endl;
			break;
		case TURN_RESULT::BOSS_DIED:
			std::cout << "YOU WIN!" << std::endl;
			break;
		default:
			break;
		}
	}


	auto printWorld() const {
		world.printWorld();
	}

	auto printPlayerInfo() const {
		player.printPlayerInfo();
	}
private:
	World world;
	Player player;
	int monsterCount = 0;
	int boxCount = 0;
	Position playerPosition;
	Position initialPlayerPosition;
	int HR_regeneration = 3;
	int trapDamage = 5;
	void setTrapDamage(const int damage) { trapDamage = damage; }
	int getTrapDamage() const { return trapDamage; }
	
	std::unordered_map<std::string, ACCESSORY_LIST> ORNAMENT_TYPE = {
		{"HR", ACCESSORY_LIST::HR},
		{"RE", ACCESSORY_LIST::RE},
		{"CO", ACCESSORY_LIST::CO},
		{"EX", ACCESSORY_LIST::EX},
		{"DX", ACCESSORY_LIST::DX},
		{"HU", ACCESSORY_LIST::HU},
		{"CU", ACCESSORY_LIST::CU},
	};

	/// <summary>
	/// 플레이어 포지션을 이동시킴
	/// 이때 만약 #(벽)에 막힐 경우 현재 위치의 contents를 반환함
	/// 단 반환하는 정보는 둘 중 하나임 - TRAP or EMPTY
	/// </summary>
	/// <returns>이동시킨 후의 위치 정보. 이때 만약 #(벽)에 막힐 경우 현재 위치 contents 반환한다.</returns>
	MAP_CONTENTS moveLeft() {
		int rowForward = playerPosition.first;
		int colForward = playerPosition.second - 1;
		if (world.getPositionInfo(playerPosition.first, playerPosition.second) != MAP_CONTENTS::TRAP) {
			world.setPosition(playerPosition.first, playerPosition.second, MAP_CONTENTS::EMPTY);
		}
		if (world.getPositionInfo(rowForward, colForward) == MAP_CONTENTS::WALL) { 
			return world.getPositionInfo(playerPosition.first, playerPosition.second);
		}

		playerPosition.first = rowForward;
		playerPosition.second = colForward;
		return world.getPositionInfo(playerPosition.first, playerPosition.second);
	}
	MAP_CONTENTS moveRight() {
		int rowForward = playerPosition.first;
		int colForward = playerPosition.second + 1;
		if (world.getPositionInfo(playerPosition.first, playerPosition.second) != MAP_CONTENTS::TRAP) {
			world.setPosition(playerPosition.first, playerPosition.second, MAP_CONTENTS::EMPTY);
		}
		if (world.getPositionInfo(rowForward, colForward) == MAP_CONTENTS::WALL) {
			return world.getPositionInfo(playerPosition.first, playerPosition.second);
		}

		playerPosition.first = rowForward;
		playerPosition.second = colForward;
		return world.getPositionInfo(playerPosition.first, playerPosition.second);
	}
	MAP_CONTENTS moveUp() {
		int rowForward = playerPosition.first - 1;
		int colForward = playerPosition.second;
		if (world.getPositionInfo(playerPosition.first, playerPosition.second) != MAP_CONTENTS::TRAP) {
			world.setPosition(playerPosition.first, playerPosition.second, MAP_CONTENTS::EMPTY);
		}
		if (world.getPositionInfo(rowForward, colForward) == MAP_CONTENTS::WALL) {
			return world.getPositionInfo(playerPosition.first, playerPosition.second);
		}

		playerPosition.first = rowForward;
		playerPosition.second = colForward;
		return world.getPositionInfo(playerPosition.first, playerPosition.second);
	}
	MAP_CONTENTS moveDown() {
		int rowForward = playerPosition.first + 1;
		int colForward = playerPosition.second;
		if (world.getPositionInfo(playerPosition.first, playerPosition.second) != MAP_CONTENTS::TRAP) {
			world.setPosition(playerPosition.first, playerPosition.second, MAP_CONTENTS::EMPTY);
		}
		if (world.getPositionInfo(rowForward, colForward) == MAP_CONTENTS::WALL) {
			return world.getPositionInfo(playerPosition.first, playerPosition.second);
		}
		
		playerPosition.first = rowForward;
		playerPosition.second = colForward;
		return world.getPositionInfo(playerPosition.first, playerPosition.second);
	}

	void reincarnation() {
		playerPosition.first = initialPlayerPosition.first;
		playerPosition.second = initialPlayerPosition.second;
		player.popAccessory(ACCESSORY_LIST::RE);
		player.set_health(player.getMaxHealth());
	}

	void reincarnation(Monster& monster, const MAP_CONTENTS contents = MAP_CONTENTS::MONSTER) {
		const int currentXpos = playerPosition.first;
		const int currentYpos = playerPosition.second;

		reincarnation();

		monster.set_health(monster.get_max_health());
		// move 과정에서 해당 monster 포지션이 EMPTY(.)로 변했으므로, 이를 다시 몬스터로 바꿔준다.
		world.setPosition(currentXpos, currentYpos, contents);
	}
};

int main() {
	std::cin.tie(nullptr);
	std::iostream::sync_with_stdio(false);
	RPGExtreme game;
	std::string actions = "";
	game.inputWorld();
	std::cin >> actions;
	game.inputMonsterInfo();
	game.inputItemInfo();
	game.playGame(actions);

	return 0;
}