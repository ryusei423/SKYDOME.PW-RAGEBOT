#pragma once
#include"../interfaces/interface.h"
#include "../Entity/entity.h"
#include"../hooks/hooks.h"

#include"Autowall.h"
#include"autowall_qo0.h"
#include"../PlayerLog/PlayerLog.h"
class Aim_info {
public:
	Aim_info() {

	}

	Aim_info(C_BasePlayer* player) {
		this->aim_ent = player;
		this->valid = true;
	}

	bool valid;
	C_BasePlayer* aim_ent;

};


class AimPoint {
public:
	enum AimPointType
	{
		//代表扫描点来自一个回溯记录
		FromRecord,

		//代表扫描点来自最新的动画
		//我们要优先扫描最新的记录，此外我们不会计算回溯记录的多点 因为那会摧毁fps
		FromNewest

	};
	AimPoint() = default;
	AimPoint(Vector point,int hitbox,int damage, AimPointType type) {
		this->point = point;
		this->hitbox = hitbox;
		this->damage = damage;
		this->PointType = type;

	}

	lag_record_t* Record;//如果扫描点来自一个回溯记录 我们要储存那个记录以便计算命中率
	C_BasePlayer* player;
	AimPointType PointType;

	Vector point;
	int hitbox;
	int damage;
	int health;//这个扫描点的目标的血量

	bool shoting = false;
	bool is_safe;
	float simeTime;
	int need_backtrack;
};

class RagebotShotInfo {
public:
	RagebotShotInfo(std::string targetname,int target_entid,int damage,int hitbox,int backtracktick,int pointtype,bool targetshot,int hitgroup) {
		this->targetname = targetname;
		this->target_entid = target_entid;
		this->hitbox = hitbox;
		this->Damage = damage;
		this->backtracktick = backtracktick;
		this->pointtype = pointtype;
		this->targetshot = targetshot;
		this->hitgroup = hitgroup;
	}
	std::string targetname;
	int target_entid;
	int Damage;
	int hitbox;
	int hitgroup;
	int backtracktick;
	int pointtype;
	bool targetshot;

};


class Ragebot {
	public:

		void on_crmo(CUserCmd* pcmd);

		bool HitChance(C_BaseEntity* pEnt, C_BaseCombatWeapon* pWeapon, Vector Angle, int chance);
		bool CanShot(C_BaseCombatWeapon* weapon);
		bool CheckPlayer(C_BasePlayer* player);

		float EsimplifyGetHitchanc(C_BaseCombatWeapon* weapon);
		bool ShouldShot(int i, bool backtack);
		bool ShouldMuitPoint(int i);
		void autostop(CUserCmd* ucmd);

		void get_target();

		C_BasePlayer* GetBestTarget();

		
		std::vector<Aim_info>noobs;


		//呃呃 又要推倒之前做的一点东西了 下面是新函数 上面的基本都已经废弃了
		bool TestRecords(std::deque<lag_record_t>& records, C_BasePlayer* player);//此函数在2022/7/22添加 用途是以最少的性能测试这个人是否可以被回溯，而不遗漏有用的东西
		void GetCanShotLagRecord();
		std::vector<lag_record_t>Records;
		std::vector<AimPoint>Aimpoints;
		
		void GetPoint();
		void CalcSafePoint(); 
		std::vector<AimPoint>safe_points;
		void GetPlayerMultipoint(C_BasePlayer* player);
		bool GetRecorodPoint(C_BasePlayer* player, lag_record_t* record, bool istest, Vector test_pos);
	

		
		AimPoint GetBestPoint();
		AimPoint GetBestRecordPoint();
		AimPoint GetShotingRecordPoint();

		//调试用
		void DrawCapsule(C_BasePlayer* player, matrix3x4_t* bones, Color clr, bool ignorez = true, float duration = 5.f);

		//全局baim标志以及计算 计算函数是基于noobs的
		bool Shouldbaim[65];
		
		Vector m_vecOrigin = {0,0,0};
		Vector m_vecOrigin_extrapolation;
		float oring_move_per_tick_ = 0;
		void MainTest();
		bool valid_target[65];
		bool has_valid;
		void CalcShouldbaim();
		std::vector<AimPoint> FindAimpointByPlayer(int entindex);

		//https://www.unknowncheats.me/forum/counterstrike-global-offensive/468825-doubletap-shot-lag-compensation.html
		int MinBacktrackTick = 0;

		int rage_last_shot;
};

inline Ragebot c_rage;