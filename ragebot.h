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
		//����ɨ�������һ�����ݼ�¼
		FromRecord,

		//����ɨ����������µĶ���
		//����Ҫ����ɨ�����µļ�¼���������ǲ��������ݼ�¼�Ķ�� ��Ϊ�ǻ�ݻ�fps
		FromNewest

	};
	AimPoint() = default;
	AimPoint(Vector point,int hitbox,int damage, AimPointType type) {
		this->point = point;
		this->hitbox = hitbox;
		this->damage = damage;
		this->PointType = type;

	}

	lag_record_t* Record;//���ɨ�������һ�����ݼ�¼ ����Ҫ�����Ǹ���¼�Ա����������
	C_BasePlayer* player;
	AimPointType PointType;

	Vector point;
	int hitbox;
	int damage;
	int health;//���ɨ����Ŀ���Ѫ��

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


		//���� ��Ҫ�Ƶ�֮ǰ����һ�㶫���� �������º��� ����Ļ������Ѿ�������
		bool TestRecords(std::deque<lag_record_t>& records, C_BasePlayer* player);//�˺�����2022/7/22��� ��;�������ٵ����ܲ���������Ƿ���Ա����ݣ�������©���õĶ���
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

		//������
		void DrawCapsule(C_BasePlayer* player, matrix3x4_t* bones, Color clr, bool ignorez = true, float duration = 5.f);

		//ȫ��baim��־�Լ����� ���㺯���ǻ���noobs��
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