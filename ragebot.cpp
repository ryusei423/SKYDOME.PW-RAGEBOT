#include"ragebot.h"


                                                         

//          __             __                                    
//    _____/ /____  ______/ /___  ____ ___  ___    ____ _      __
//   / ___/ //_/ / / / __  / __ \/ __ `__ \/ _ \  / __ \ | /| / /
//  (__  ) ,< / /_/ / /_/ / /_/ / / / / / /  __/ / /_/ / |/ |/ / 
// /____/_/|_|\__, /\__,_/\____/_/ /_/ /_/\___(_) .___/|__/|__/  
//           /____/                            /_/               
// Kiana1337 Ryusei423




//����Ƿ�Ϊ��Ч�����ʵ��
//������ܻ�Ӹ�������� ����ֱ�����Ƿ�Ϊ��������� ���ܳ�����v1����v2�ĸ�����
bool Ragebot::CheckPlayer(C_BasePlayer* player) {
	
	if (!player)return false;
	if (player->IsDormant() || 
		!player->IsAlive() ||
		!player->isEnemy() ||
		player->GetClientClass()->m_ClassID!= CCSPlayer ||
		player == g_Cheatdata.LocalPlayer || 
		player->m_vecOrigin() == Vector(0, 0, 0)||
		player->GetImmunity()||
		g_PlayerLog.get_log(player->EntIndex()).record.empty())//ragebot���ڿ�ʼ��ȫ����PlayerLog
	{
		return false;
	}

	return true;

}
//C_BasePlayer* get_target() {
//
//	for (int i = 1;i < g_interfaces::g_EngineClient->GetMaxClients();i++) {
//
//		C_BasePlayer* aim_ent = (C_BasePlayer*)g_interfaces::g_ClientEntityList->GetClientEntity(i);
//		if (check_player(aim_ent)) {
//
//
//			return aim_ent;
//		}
//		else continue;
//
//	}
//	return NULL;
//}
int HitboxToHitgroup(Hitboxes hitbox) {
	switch (hitbox)
	{
	case Hitboxes::HITBOX_HEAD: return HITGROUP_HEAD;
	case Hitboxes::HITBOX_NECK: return HITGROUP_NECK;
	case Hitboxes::HITBOX_PELVIS:
	case Hitboxes::HITBOX_STOMACH:
		return HITGROUP_STOMACH;
	case Hitboxes::HITBOX_LOWER_CHEST:
	case Hitboxes::HITBOX_CHEST:
	case Hitboxes::HITBOX_UPPER_CHEST:
		return HITGROUP_CHEST;
	case Hitboxes::HITBOX_RIGHT_CALF:
	case Hitboxes::HITBOX_RIGHT_THIGH:
	case Hitboxes::HITBOX_RIGHT_FOOT:
		return HITGROUP_RIGHTLEG;
	case Hitboxes::HITBOX_LEFT_CALF:
	case Hitboxes::HITBOX_LEFT_THIGH:
	case Hitboxes::HITBOX_LEFT_FOOT:
		return HITGROUP_LEFTLEG;
	case Hitboxes::HITBOX_RIGHT_UPPER_ARM:
	case Hitboxes::HITBOX_RIGHT_FOREARM:
	case Hitboxes::HITBOX_RIGHT_HAND:
		return HITGROUP_RIGHTARM;
	case Hitboxes::HITBOX_LEFT_UPPER_ARM:
	case Hitboxes::HITBOX_LEFT_FOREARM:
	case Hitboxes::HITBOX_LEFT_HAND:
		return HITGROUP_LEFTARM;

	default: return HITGROUP_GENERIC;
	}
}

C_BasePlayer* Ragebot::GetBestTarget() {

	
	C_BasePlayer* rt = NULL;
	int hea = noobs[0].aim_ent->m_iHealth();
	for (int i = 0; i < noobs.size();i++)
	{
		if (noobs[i].aim_ent->m_iHealth() <= hea) {
		
			hea = noobs[i].aim_ent->m_iHealth();
			rt = noobs[i].aim_ent;
		}
	}
	return rt;


}

/*
	Update 2021/11/13
	-����ظĽ���������ǹ�ϵ�ragebot.
*/


bool Ragebot::CanShot(C_BaseCombatWeapon* weapon) {
	if (!g_Cheatdata.LocalPlayer || !g_Cheatdata.LocalPlayer->IsAlive())
		return false;
	if (g_Cheatdata.ucmd->command_number == tickbase.LastShift+1){
		return true;
	}
	auto info = weapon->GetInfo();

	if (info->WeaponType == WEAPONTYPE_KNIFE && weapon->m_iItemDefinitionIndex() != WEAPON_ZEUS)
		return false;

	if (weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER) {
		auto time = weapon->m_flPostponeFireReadyTime();//�ĳ�>=Ӧ�ø��� aa�Ǳ߲�֪���᲻�ᱻӰ��
		if (!time || time >= g_Utilities::get_servertime() /*|| Misc::warmuptick < 14*/) {
			g_Cheatdata.Revolver_tryshoting = false;
			return false;
		}
			
	}

	//����ӵ�
	if (weapon->m_iClip1() <= 0)
		return false;
	//��ҵ���һ�ι���
	if(g_Cheatdata.LocalPlayer->NextAttack() > g_Utilities::get_servertime())
		return false;
	//��������һ�ι���
	if (weapon->m_flNextPrimaryAttack() > g_Utilities::get_servertime())
		return false;

	return true;
}


void Ragebot::get_target() {

	for (int i = 1;i < g_interfaces::g_EngineClient->GetMaxClients();i++) {

		C_BasePlayer* aim_ent = (C_BasePlayer*)g_interfaces::g_ClientEntityList->GetClientEntity(i);
		if (CheckPlayer(aim_ent)) {
			Aim_info tmp;
			tmp.aim_ent = aim_ent;

			this->noobs.emplace_back(tmp);
			valid_target[aim_ent->EntIndex()] = true;
		}
		else continue;

	}



}





bool Ragebot::HitChance(C_BaseEntity* pEnt, C_BaseCombatWeapon* pWeapon, Vector Angle,int chance)
{
	//��������ʵ���0ֱ�ӷ���true
	if (chance == 0 )
		return true;

	auto unpreaeyepos = g_EnginePrediction.getunpredEyepos();
	pWeapon->UpdateWeaponAccuracy();
	//const auto backupvel = g_Cheatdata.LocalPlayer->GetVelocity();
	//const auto backupabsvel = g_Cheatdata.LocalPlayer->GetABSVelocity();
	//g_Cheatdata.LocalPlayer->GetABSVelocity() = g_Cheatdata.LocalPlayer->GetVelocity() = g_EnginePrediction.getunpredVel();
	///*pWeapon->UpdateWeaponAccuracy();*/
	//g_Cheatdata.LocalPlayer->GetVelocity() = backupvel;
	//g_Cheatdata.LocalPlayer->GetABSVelocity() = backupabsvel;

	float Seeds = 256.f;

	//���������Ѿ��������޺���
	//Angle -= (g_Cheatdata.LocalPlayer->get_aimpuch() * g_interfaces::g_cvar_sys->FindVar("weapon_recoil_scale")->Getfloat());

	Vector forward, right, up;

	Math::AngleVectors(g_Utilities::QangleVector(Angle), forward, right, up);

	int Hits = 0, neededHits = (Seeds * (chance / 100.f));

	float weapSpread = pWeapon->GetSpread(), weapInaccuracy = pWeapon->GetInaccuracy();

	for (int i = 0; i < Seeds; i++)
	{
		//

		Math::RandSeed((i & 255) + 1);
		const auto b = Math::RandFloat(0.f, 2.0f * M_PI);
		const auto c = Math::RandFloat(0.0f, 1.0f);
		const auto d = Math::RandFloat(0.f, 2.0f * M_PI);

		float Inaccuracy = c * weapInaccuracy;
		float Spread = c * weapSpread;

		const Vector v_spread((cos(b) * Spread) + (cos(d) * Inaccuracy), (sin(b) * Spread) + (sin(d) * Inaccuracy), 0);

		//Vector spreadView((cos(Math::RandFloat(0.f, 2.f * M_PI)) * Inaccuracy) + (cos(Math::RandFloat(0.f, 2.f * M_PI)) * Spread), (sin(Math::RandFloat(0.f, 2.f * M_PI)) * Inaccuracy) + (sin(Math::RandFloat(0.f, 2.f * M_PI)) * Spread), 0), direction;
		//direction = Vector(forward.x + (spreadView.x * right.x) + (spreadView.y * up.x), forward.y + (spreadView.x * right.y) + (spreadView.y * up.y), forward.z + (spreadView.x * right.z) + (spreadView.y * up.z)).Normalized();

		Vector direction/* = Vector(forward.x + (v_spread.x * right.x) + (v_spread.y * up.x), forward.y + (v_spread.x * right.y) + (v_spread.y * up.y), forward.z + (v_spread.x * right.z) + (v_spread.y * up.z)).Normalized()*/;

		direction.x = forward.x + (right.x * v_spread.x) + (up.x * v_spread.y);
		direction.y = forward.y + (right.y * v_spread.x) + (up.y * v_spread.y);
		direction.z = forward.z + (right.z * v_spread.x) + (up.z * v_spread.y);
		direction.Normalized();
		//Vector total_spread = (forward + right * spreadView.x + up * spreadView.y);
		//total_spread.NormalizeInPlace();

		Vector viewanglesSpread, viewForward;
		Math::VectorAngles(direction, up,viewanglesSpread);
		//Math::VectorNormalize1(viewanglesSpread);
		//viewanglesSpread.NormalizeInPlace();

		

		Math::AngleVectors(viewanglesSpread, &viewForward);
		Math::VectorNormalize1(viewForward);
		viewForward.NormalizeInPlace();
		viewForward = /*g_Cheatdata.LocalPlayer->GetEyePos()*/unpreaeyepos + (viewForward * pWeapon->GetInfo()->flRange);

		CGameTrace Trace;
		Ray_t tmp;
		tmp.Init(/*g_Cheatdata.LocalPlayer->GetEyePos()*/unpreaeyepos, viewForward);

		g_Cheatdata.hc_testpoint0 = /*g_Cheatdata.LocalPlayer->GetEyePos()*/unpreaeyepos;
		g_Cheatdata.hc_testpoint1 = viewForward;

		g_interfaces::g_EngineTrace->ClipRayToEntity(tmp, mask_shot | contents_grate, pEnt, &Trace);

		if (Trace.hit_entity == pEnt)
			Hits++;

		if (((Hits / Seeds) * 100.f) >= chance)
			return true;

		if ((Seeds - i + Hits) < neededHits)
			return false;
	}

	return false;
}


float Ragebot::EsimplifyGetHitchanc(C_BaseCombatWeapon* weapon) {
	if (!hooks::g_Local_player)return 0;

	float inaccuracy = weapon->GetInaccuracy();
	if (inaccuracy == 0) inaccuracy = 0.01;
	inaccuracy = 1 / inaccuracy;
	return inaccuracy;

}

void Ragebot::MainTest(){
	for (auto& i : noobs){
		auto& log = g_PlayerLog.get_log(i.aim_ent->EntIndex());
		if (TestRecords(log.record, i.aim_ent)) {
			has_valid = true;
		}
		else{
			valid_target[i.aim_ent->EntIndex()] = false;//�ú����ɨ���������Ŀ�� ���ǻ��и����fps
		};


	}





}


int FixTickcout(float sim) {
	//��������Ŀͻ����ڲ���������������������������������������������������������������������������������
	return TIME_TO_TICKS(sim + g_LagComp.get_lerptime());
}

/*
	������֪ragebot�����¼�¼ɨ����ֱ�Ӷ�ȡrecords[0]�ģ������Ǳ�tick�е����¼�¼Ҳ������
	����Ҫ�˽���ʵ������������ ��֡�׶ε�����
	���ⲻ������


*/
void Ragebot::on_crmo(CUserCmd* pcmd)
{

	//������Ϣ
	noobs.clear();
	Aimpoints.clear();
	safe_points.clear();

	C_BaseCombatWeapon* weapon = (C_BaseCombatWeapon*)g_interfaces::g_ClientEntityList->GetClientEntity(hooks::g_Local_player->GetActiveWeaponIndex());if (!weapon)return;
	CCSWeaponInfo* weapon_info = weapon->GetInfo();if (!weapon || !weapon_info)return;

	//�������ʱ��Ȼ����ɨ��� ���ǻ��б������Ҫ����
	/*if (!CanShot(weapon))
		return;*/
	if (!weapon || weapon->IsGrenade() || weapon->m_iItemDefinitionIndex() == WEAPON_HEALTHSHOT || weapon->m_iItemDefinitionIndex() == WEAPON_C4){
		return;
	}
	
	//��ȡ���п���ΪĿ���ʵ��
	get_target();

	if (noobs.empty())
		return;
	if (tickbase.isShifting)
		return;
	
	//��ȡ��Ҫ�����ʵ��
	//get_will_shot();

	//�����Ч��
	//Checkwillshot();

	has_valid = false;
	MinBacktrackTick = g_config.rage_doubletap_key.value ? tickbase.doubletapCharge > 10 ? tickbase.doubletapCharge - 10 : 0 : 0;

	Vector move_dir;
	float oring_move_per_tick = (m_vecOrigin  - g_Cheatdata.LocalPlayer->m_vecOrigin()).Length();
	if (oring_move_per_tick > 10.f || oring_move_per_tick < -10.f){
		//���ǵõ���һ����ʺ,��ͼ�Ѿ����Ļ�������Ѿ�����
		m_vecOrigin = g_Cheatdata.LocalPlayer->m_vecOrigin();
		if (CanShot(weapon)){
			goto StartScann;
		}
	}
	
	Math::AngleVectors(Vector(0.f, pcmd->viewangles.yaw, 0.f), &move_dir);
	m_vecOrigin_extrapolation = g_EnginePrediction.getunpredEyepos() + (move_dir * (oring_move_per_tick * 3));
	MainTest();
	m_vecOrigin = g_Cheatdata.LocalPlayer->m_vecOrigin();
	//����������Ͼ�Ҫ���
	if (has_valid && g_config.rage_autostop_auto_between_shots) {
		if (weapon_info->WeaponType == WEAPONTYPE_KNIFE /*|| weapon->m_iItemDefinitionIndex() == WEAPON_ZEUS*/)
			return;
		

		if (!CanShot(weapon)){
			//��Ҫ����ؿ�ʼ��ͣ,��Ȼ���ǻ��������һֱ���ڼ�ͣ��״̬
			//����ֵ���Կ��ǽ����û��������ͻ�����ǰ��ͣ��Ч�������������ֺ���ȫ������м�ͣ����
			if (TIME_TO_TICKS(weapon->m_flNextPrimaryAttack() - g_Utilities::get_servertime()) <= 10) {
				autostop(pcmd);
				//pcmd->buttons |= IN_SPEED;
			}

			return;
		}
		//autostop(pcmd);
		
		//pcmd->buttons |= IN_SPEED;
	}

	if (!has_valid || !CanShot(weapon)){
		return;
	}

	StartScann:;

	//��ȡ����ɨ���
	if (g_config.rage_backtrack)GetCanShotLagRecord();

	//����Ҫ���ò�ֵ�Ա�֤���¼�¼��׼ȷ��(���ڻ�û)
	//�����2022/1/4:�Ѿ�תΪ��PlayerLog�л�ȡ�������󡣲�ֵ�Ѿ����ָ�
	for (int i = 0; i < noobs.size(); i++)
	{
		GetPlayerMultipoint(noobs[i].aim_ent);
	}

	if(Aimpoints.empty()){
		return;
	}
	

	/*
		2022/1/4
		���ڵĽṹ�����:

		��ȡ���л���ɨ���
		��ȡ��������ɨ���
		
		��ɨ����б��в�����������ɨ���
		�������ʧ������һ���ɨ���

		������ܺܲ��Ѻ�...��ragebot�Ľṹ�Ż��Ѿ�����ƻ�


	
	
	*/
	CalcSafePoint();
	CalcShouldbaim();
	AimPoint BestPoint;
	
	if (g_config.rage_prefer_shooting){
		BestPoint = GetShotingRecordPoint();
		//���point��000�����˳��˶� �������г���ɨ��
		if (BestPoint.point != Vector{ 0,0,0 }){
			goto endpointselect;
		}
	}

	BestPoint = GetBestPoint();
	if (BestPoint.point == Vector{ 0,0,0 }) {
		//���������ɨ����в���ʧ�� ���ǽ���ʼ���һ���ɨ���
		if(g_config.rage_backtrack)BestPoint = GetBestRecordPoint();
	}

	//���ȫ��ʧ����֤��û��Ŀ��
	if (BestPoint.point == Vector{0,0,0})return;

	endpointselect:;
	//g_interfaces::g_IVDebugOverlay->AddBoxOverlay(BestPoint.point, Vector(-2, -2, -2), Vector(2, 2, 2), QAngle(0, 0, 0), /*26, 189, 230,*/221,34,221, 127, 0.5);
	QAngle AimAngle = Math::CalcAngle(/*g_Cheatdata.LocalPlayer->GetEyePos()*/g_EnginePrediction.getunpredEyepos(), BestPoint.point);

	
	
	/*
		Update 2021/10/8

		-�������µ���Ϸ�汾
		-�Ľ���DTHC
		-�Ľ����Զ�ֹͣ
		-�Ľ����������
		-�޸��˻����������Ϲ��ڻָ������Ĵ���
	
		ֻ�������ʲ�����ʱ����autostop�ǿ��еģ��ص��������һ��dt�����͵ڶ���dt������ʱ������� �Ͳ�Ӧ��ʹ��dt������
		������Ҫһ��"����dtʱ��"�������ʱ�������һ�ι���Ϊdt��flag��Ϊ0
		��ֵӦ�ñ����Ǵ����ģ��tickҪ��һ��
	*/

	/*
		10.11˵��
		�Զ����ֻ���������ʼ�����������ͣ��
		Ϊ��ֹrage������ֵ�bug���Ҳ�֪�� ��㽫���ͷź��޸�
		�ͷ�֮ǰ���������д��rage���� ���ǳ���Ӱ�����ص�bug	
	
	*/

	//�������+���� ����ֱ��д�������� 18
	//���߰����������ʳ���5
	//ʵ�����Ǹ����������õ��뷨

	//Ӧ��dthc
	int hc = g_config.rage_hitchance;
	if (tickbase.DoubleTapAttack) {
		hc = g_config.rage_doubletap_hc;
		tickbase.DoubleTapAttack = false;
	}

	//���浱ǰ��������Ϊ������ɨ��ָ��ǻ���
	//(����������ݵ�ʱ��)
	lag_record_t backup(BestPoint.player);
		BestPoint.Record->recoverBone(BestPoint.player);

		bool hcOk = HitChance(BestPoint.player, weapon, g_Utilities::QangleVector(AimAngle),/*g_config.rage_hitchance*/hc);
		//��ǰ���������ʲ�����ͬһtick�н���������Զ�ֹͣ ��Ӧ���������ǻ�ø��������ٶ�
		//����ֻ�������ʲ�����ʱ�����Զ�ֹͣ ������dt����������
		if (!hcOk /*&& weapon->can_shot()*/ && weapon->m_iItemDefinitionIndex() != WEAPON_ZEUS)autostop(pcmd);
		
		//r8����һЩ�����ʼ������ ������Ҫ��r8��Ӧ���Զ�ֹͣ
		//ʵ���ϲ���
		//if (weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && weapon->can_shot())autostop(pcmd);

	//�ָ��ɹ���
	backup.recoverBone(BestPoint.player);


	if (BestPoint.player && hcOk /*&&*/ /*weapon->can_shot()*//*CanShot(weapon)*//*����ȫ���Ԥ������֮ǰ ǿ�����*/) {
		pcmd->viewangles = AimAngle;
		if (!g_config.rage_silent)g_interfaces::g_EngineClient->SetViewAngles(&AimAngle);

		//if (BestPoint.PointType == AimPoint::AimPointType::FromRecord)
		//{
		//	//�ָ�ģ��ʱ��������ӳٲ���
		//	///*g_interfaces::g_GlobalVar->curtime*/pcmd->tick_count = /*BestPoint.simeTime*/FixTickcout(BestPoint.simeTime);
		//	BestPoint.player->set_abs_origin(BestRecord.m_vecOrigin);
		//	
		//}

		//��ȫ���� ���ⲻ�ǳ�������
		/*auto tickcountFix = calcBacktrack(BestPoint.player,AimAngle);
		if (tickcountFix)
		{
			pcmd->tick_count = tickcountFix;
		}else*/
		//g_Utilities::print(std::string("origin:tickcout:") + std::to_string(pcmd->tick_count));
		pcmd->tick_count = FixTickcout(BestPoint.Record->m_flSimulationTime);
		//g_Utilities::print(std::string("new:tickcout:") + std::to_string(pcmd->tick_count));
		//DrawCapsule(BestPoint.player, BestPoint.Record->matrix, Color(238, 17, 238, 255));
		//�����õ� �ͷŰ汾Ҫע�͵�
		//if (pcmd->buttons & IN_ATTACK && BestPoint.PointType == AimPoint::AimPointType::FromRecord && g_Cheatdata.m_DrawBacktrackAimbotBone)
		//{
		//	DrawCapsule(BestPoint.player, BestPoint.Record->matrix,Color(238,17,238,255));
		//	//memcpy(&g_Cheatdata.ragebot_LagcompTest, BestPoint.Record->matrix, sizeof(matrix3x4_t) * 128);
		//	//g_Cheatdata.ragebot_test_ent_index = BestPoint.player->EntIndex();
		//}
		

		//auto debugTmp = FixTickcout(BestPoint.simeTime);
		if (g_config.rage_autoshot) {
			pcmd->buttons |= IN_ATTACK;
			//����������Ӧ���ڱ�ĵط� ����˵����������̶�������aimbot
			if (!tickbase.isShifting) {
				rage_last_shot = pcmd->tick_count;
				LuaCallback::CALL_CALLBACK_ragebotshot(RagebotShotInfo("", BestPoint.player->EntIndex(), BestPoint.damage, BestPoint.hitbox, TIME_TO_TICKS(BestPoint.player->m_flSimulationTime() - BestPoint.simeTime/* - g_interfaces::g_GlobalVar->curtime*/), BestPoint.PointType,BestPoint.shoting, HitboxToHitgroup((Hitboxes)BestPoint.hitbox)));
				g_ShotManager.resetall();
				g_ShotManager.postshot(ShotItem(g_interfaces::g_GlobalVar->tickcount, BestPoint.point, BestPoint.player->EntIndex(), BestPoint.hitbox, BestPoint.damage, TIME_TO_TICKS(BestPoint.player->m_flSimulationTime() - BestPoint.simeTime),BestPoint.Record));
				if (g_config.vision_showimpacts)g_interfaces::g_IVDebugOverlay->AddBoxOverlay(BestPoint.point, Vector(-2, -2, -2), Vector(2, 2, 2), QAngle(0, 0, 0), g_config.vision_showimpacts_local.r(), g_config.vision_showimpacts_local.g(), g_config.vision_showimpacts_local.b(), g_config.vision_showimpacts_local.a(), 3);
			}
		}
		
		if (pcmd->buttons & IN_ATTACK){
			//DrawCapsule(BestPoint.player, BestPoint.Record->matrix, Color(230,26,230,255));
			g_Cheatdata.sendPacket = true;
		}

		Autopeek.doBack = true;

		//Ӧ��ֻ�ڻ���tick����14��ʱ����Ϊfalse
		//�ͷź��ٸ��� �Ҳ������κζ���Ӱ��rage
		//

		//���doingAutoRevolver��Ϊfalse�ʹ������ǵ�ragebot���ڳ��������������
		//����AA���ж��߼���doingAutoRevolver == false�����ֿ��Կ���
		g_Cheatdata.Revolver_tryshoting = true;
	}


	return;	

}
void Ragebot::autostop(CUserCmd* ucmd)
{
	//lastfaststop = g_EnginePrediction.getunpredTime();
	//����ڿ��в���û�п������м�ͣ ���ǲ�ִ��
	if (!(g_Cheatdata.LocalPlayer->m_fFlags() & FL_ONGROUND) && !g_config.rage_autostop_in_air){
		return;
	}

	//ȡ����Ծ��ť �������м�ͣ
	ucmd->buttons &= ~IN_JUMP;

	C_BaseCombatWeapon* weapon = (C_BaseCombatWeapon*)g_interfaces::g_ClientEntityList->GetClientEntity(hooks::g_Local_player->GetActiveWeaponIndex()); if (!weapon)return;
	CCSWeaponInfo* weapon_info = weapon->GetInfo(); if (!weapon || !weapon_info)return;
	/*if (g_EnginePrediction.getunpredVel().Length() < weapon_info->flMaxPlayerSpeed){

	}*/

	auto get_standing_accuracy = [&]() -> const float
	{
		const auto max_speed = (g_Cheatdata.LocalPlayer->m_bIsScoped() ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed);
		return max_speed / 3.f;
	};

	if (g_Cheatdata.LocalPlayer->m_vecVelocity().Length() <= get_standing_accuracy()) return;;

	if (g_config.rage_autostop_full) {
		/*hooks::g_Local_player->set_moveType(2);

		hooks::g_Local_player->SetVelocity(Vector(0,0,0));*/
		//ucmd->buttons |= IN_SPEED;
		
		const auto velocity = /*hooks::g_Local_player->GetVelocity()*/g_EnginePrediction.getunpredVel();
		const auto speed = velocity.Length();
		if (speed > 15.f) {
			Vector dir;
			Math::VectorAngles(velocity,dir);
			QAngle Direction = g_Utilities::QangleVector(dir);
			dir.y = ucmd->viewangles.yaw - dir.y;
			
			Vector new_move;
			Math::AngleVectors(dir, &new_move);
			const auto max = max(std::fabs(ucmd->forwardmove), std::fabs(ucmd->sidemove));
			const auto mult = 450.f / max;
			new_move *= -mult;

			ucmd->forwardmove = new_move.x;
			ucmd->sidemove = new_move.y;


		}
		else
		{
			ucmd->forwardmove = 0.f;
			ucmd->sidemove = 0.f;
		}
		
		

	}


	if (g_config.rage_autostop_duck) {
		ucmd->buttons |= IN_DUCK;
		
	}


	if (g_config.rage_autostop_ls) {
		/*ucmd->buttons |= IN_SPEED;
		Misc::slowwalk(ucmd,g_config.rage_autostop_ls_speed);*/

		Vector angle;
		//�����ȡ���������ƶ��ķ���
		Math::VectorAngles(g_Cheatdata.LocalPlayer->m_vecVelocity(), angle);

		// get our current speed of travel.
		//��ȡ�ٶ�
		float speed = g_Cheatdata.LocalPlayer->m_vecVelocity().Length()/*450.f*/;

		// fix direction by factoring in where we are looking.
		// �����������ڿ��ĵط��������ƶ��ķ���һ��
		angle.y = ucmd->viewangles.yaw - angle.y;

		// convert corrected angle back to a direction.
		// ��У����ĽǶ�ת���ط�������(ԭ����)
		Vector direction;
		Math::AngleVectors(angle, &direction);

		Vector stop = direction * -speed;

		if (speed > 13.f) {
			ucmd->forwardmove = stop.x;
			ucmd->sidemove = stop.y;
		}
		else {
			ucmd->forwardmove = 0.f;
			ucmd->sidemove = 0.f;
		}


	}






}


std::vector<AimPoint> Ragebot::FindAimpointByPlayer(int entindex) {
	std::vector<AimPoint> ret;
	for (auto& point : Aimpoints){
		if (point.player->EntIndex() == entindex)
		{
			ret.push_back(point);
		}
	}

	return ret;
}


void Ragebot::CalcShouldbaim()
{
	//���³�ʼ��baim��ʶ
	for (size_t i = 0; i <= 64; i++){
		Shouldbaim[i] = false;
	}

	for (auto noob : noobs){
		auto index = noob.aim_ent->EntIndex();

		if (g_config.rage_forcebaim){
			Shouldbaim[index] = true;
			continue;
		}

		if (g_config.rage_priority_baim){
			//��������Ϊtrue��Ȼ���ٿ�ʼ������Ҫ�رյ����
			Shouldbaim[index] = true;

			if (g_config.rage_cancle_biam_if_you_low_hp && g_Cheatdata.LocalPlayer->m_iHealth() <= 50){
				Shouldbaim[index] = false;
			}
			if (g_config.rage_cancel_biam_if_enemy_moveing && noob.aim_ent->m_vecVelocity().Length() >= 200.f)
			{
				Shouldbaim[index] = false;
			}
			if (g_config.rage_cancle_biam_if_only_head){
				bool onlyhead = true;
				for (auto& point : FindAimpointByPlayer(index)){
					if (point.hitbox != HITBOX_HEAD)
					{
						onlyhead = false;
					}
				}

				Shouldbaim[index] = !onlyhead;
			}
			

		}

		

	}

}

/*
	Update 2021/10/14
	
	-�޸��˻���ɨ��(�ǵ�,�ҵĴ�,֮ǰ������û�й�����lol).


*/
bool Ragebot::GetRecorodPoint(C_BasePlayer* player, lag_record_t* record, bool istest = false, Vector test_pos = {0,0,0}) {
	//if (!g_PlayerLog.IsValid(record->m_flSimulationTime))return false;
	
	//����dt���ݵ����� ������Ҫɨ��ԭ�ػ��� 
	//if (record->m_vecOrigin == player->Get_vec_origin())return false;

	
	//�ָ���¼�е����� ��ʼɨ���˺�
	record->recoverBone(player);

	bool rt = false;
	Vector origin = istest ? test_pos : g_EnginePrediction.getunpredEyepos();

	const auto model = player->GetModel();
	if (!model) return false;
	const auto studio_hdr = g_interfaces::g_mdlinfo->GetStudiomodel(model);
	if (!studio_hdr) return false;
	const auto set = studio_hdr->GetHitboxSet(player->m_nHitboxSet());

	for (auto i = 0; i < HITBOX_MAX; i++){
		if (istest){
			//�������һ�β���
			//��ʹ����һ�����ݼ�¼ ����Ҳ��������hitbox
			if (!ShouldShot(i, true) && !ShouldShot(i, false)){
				continue;
			}
		}else if (!ShouldShot(i,true))continue;

		auto hitbox = set->GetHitbox(i);
		if (!hitbox)
			continue;

		auto& mat = record->matrix[hitbox->bone];
		const auto mod = hitbox->m_flRadius != -1.f ? hitbox->m_flRadius : 0.f;

		Vector max;
		Vector min;
		Math::VectorTransform(hitbox->bbmax + mod, mat, max);
		Math::VectorTransform(hitbox->bbmin - mod, mat, min);
		

		const auto center = (min + max) * 0.5f;
		const auto cur_angles = Math::CalcAngle(center, origin);

		Vector curAngles = { cur_angles.pitch,cur_angles.yaw,cur_angles.roll };

		Vector forward;
		Math::AngleVectors(curAngles, &forward);

		auto rs = hitbox->m_flRadius * (g_config.rage_multipoint / 100.f);

		if (rs < 0.2)
			continue;

		const auto right = forward.Cross(Vector(0, 0, 1)) * rs;
		const auto left = Vector(-right.x, -right.y, right.z);
		const auto top = Vector(0, 0, 1) * rs;

		//Vector center
		Vector rightPoint = center + right;
		Vector leftPoint = center + left;
		Vector topPoint = center + top;

		std::vector<AimPoint> points;
		AimPoint point_center, point_left, point_right, point_top;

		//�����в���Ҫ���ĵ�
		if (!istest || (istest && !ShouldMuitPoint(i))){
			point_center.health = player->m_iHealth();
			point_center.damage = /*g_Autowall.GetDamagez(center, player)*/g_CAutoWall.GetDamage(g_Cheatdata.LocalPlayer,center);
			point_center.point = center;
			point_center.PointType = AimPoint::AimPointType::FromRecord;
			point_center.player = player;
			point_center.Record = record;
			point_center.hitbox = i;
			point_center.shoting = record->m_Shoting;
			points.emplace_back(point_center);
		}

		if (ShouldMuitPoint(i))
		{
			point_right.health = player->m_iHealth();
			point_right.damage = /*g_Autowall.ConstomGetDMG(origin,rightPoint, player)*/g_CAutoWall.GetDamageConstom(g_Cheatdata.LocalPlayer, origin, rightPoint);
			point_right.point = rightPoint;
			point_right.PointType = AimPoint::AimPointType::FromRecord;
			point_right.player = player;
			point_right.Record = record;
			point_right.hitbox = i;
			point_right.shoting = record->m_Shoting;

			point_left.health = player->m_iHealth();
			point_left.damage = /*g_Autowall.ConstomGetDMG(origin, leftPoint, player)*/g_CAutoWall.GetDamageConstom(g_Cheatdata.LocalPlayer, origin, leftPoint);
			point_left.point = leftPoint;
			point_left.PointType = AimPoint::AimPointType::FromRecord;
			point_left.player = player;
			point_left.Record = record;
			point_left.hitbox = i;
			point_left.shoting = record->m_Shoting;

			point_top.health = player->m_iHealth();
			point_top.damage = /*g_Autowall.ConstomGetDMG(origin, topPoint, player)*/g_CAutoWall.GetDamageConstom(g_Cheatdata.LocalPlayer, origin, topPoint);
			point_top.point = topPoint;
			point_top.PointType = AimPoint::AimPointType::FromRecord;
			point_top.player = player;
			point_top.Record = record;
			point_top.hitbox = i;
			point_top.shoting = record->m_Shoting;

			points.emplace_back(point_left);
			points.emplace_back(point_right);
			points.emplace_back(point_top);
		}

		

		for (int i = 0; i < points.size(); i++)
		{
			if ((points[i].damage >= g_config.rage_damage) || (points[i].damage >= points[i].health))
			{
				points[i].simeTime = record->m_flSimulationTime;
				if (!istest)c_rage.Aimpoints.push_back(points[i]);
				rt = true;
			}

		}
	}

	
	return rt;
}

bool Ragebot::TestRecords(std::deque<lag_record_t>& records,C_BasePlayer* player){
	int valid = 0;
	
	lag_record_t backup(player);
	for (int i = 0; i < records.size();i++) {
		if (records[i].valid){
			valid = i;
		}
	}
	std::vector<int> testpoints;
	testpoints.emplace_back(valid / 2);
	testpoints.emplace_back(valid / 4);
	testpoints.emplace_back(valid);
	testpoints.emplace_back(valid - valid / 4);
	
	bool rt = false;
	for (auto i : testpoints){
		if (GetRecorodPoint(player, &records[i],true,m_vecOrigin_extrapolation)){
			rt = true;
		}
	}
	backup.recoverBone(player);
	return rt;
}

/*
	��ȫɨ������������һЩfps���� һЩ�Ż���ʩ�Ǳ����
	���뵽�Ľ�������ǶԻ��ݼ�¼���в��� ����˵���ǲ��������ط� ǰ�ٷ�֮25�� ǰ�ٷ�֮50�� ǰ�ٷ�֮100��
	�����������Ը���fps���� �����Ƿ���в���

	����һ��TODO ��Ҫ����ɲ���ϵͳ ���ѡ���Ψһԭ����������û����ô����û�
	�Ҵ���дһ����������ͼ����ִ���ָʾ�� ��һЩ����ֱ��д��cpp�п���Ҳ���� �Һ��µ��������ô������ӷ��
	�ҵ�ϲ���Ǵ�����һ������ν�����ǲ���ӷ��
	������������ֱ�ӽ��Դ���ָʾ��д��lua�� ����
*/
void Ragebot::GetCanShotLagRecord()
{
	//���˼�¼
	//g_PlayerLog.FilterRecords();
	for (int i = 1; i < g_interfaces::g_EngineClient->GetMaxClients(); i++)
	{
		//g_PlayerLog.FilterRecords();
		auto curplayer = static_cast<C_BasePlayer*>(g_interfaces::g_ClientEntityList->GetClientEntity(i));
		auto& curlog = g_PlayerLog.get_log(i);
		auto& currecord = curlog.record;
		if (!CheckPlayer(curplayer))continue;
		if (g_resolver.datas[curplayer->EntIndex()].breaklc)continue;//��Ҫ����LCĿ��
		
		//g_Cheatdata.hc_testpoint1 = curlog.record.begin()->m_vecOrigin;
		//���ݵ�ǰ���µ�����
		lag_record_t oldRecord(curplayer);
		if (!/*TestRecords(currecord, curplayer)*/valid_target[i]) {
			oldRecord.recoverBone(curplayer);
			continue;
		}
		//��ʼɨ����ݾ���
		//�ҵ�һ���������֮��˱�������Ϊtrue��Ȼ������ֻѰ�������¼
		bool findRecord = false;
		//ע�͵����Ƿ������ �������Ի����ȶ���
		for (auto i = 0u; i < currecord.size(); i++/*int i = currecord.size()-1; i >= 0; i--*/) {
			//���ݾɵ���Ϣ
			//lag_record_t oldRecord(curplayer);
			//g_Cheatdata.hc_testpoint1 = i->m_vecOrigin;
			if(currecord[i].valid && currecord[i].m_bonefix){
				int bt = TIME_TO_TICKS(curplayer->m_flSimulationTime() - currecord[i].m_flSimulationTime);
				if (bt < MinBacktrackTick){
					continue;
				}
				if (!findRecord && GetRecorodPoint(curplayer, &currecord[i])) {
					//����Ѿ��ҵ����Ҵ�����һ��ɨ��� �����˳�����˵�ɨ�����
					//break;
					findRecord = true;
					continue;//ֻ��һ����¼������һ��ɨ��
				}
				if (currecord[i].m_Shoting){
					GetRecorodPoint(curplayer, &currecord[i]);
				}
			}


		}
		
		//�ָ����ݵļ�¼
		oldRecord.recoverBone(curplayer);
		
	}


}

//������ShouldShot�ĸ����������ø����Ǹ�����
bool Ragebot::ShouldMuitPoint(int i)
{
	switch (i)
	{
			//ͷ
		case HITBOX_HEAD: {
			if (g_config.rage_hitbox_multpoint[0])return true; else return false;
		}break;

			//����
		case HITBOX_NECK: {
			if (g_config.rage_hitbox_multpoint[1])return true; else return false;
		}break;

			//��
		case HITBOX_CHEST: {
			if (g_config.rage_hitbox_multpoint[2])return true; else return false;
		}break;
		case HITBOX_LOWER_CHEST: {
			if (g_config.rage_hitbox_multpoint[2])return true; else return false;
		}break;
		case HITBOX_UPPER_CHEST: {
			if (g_config.rage_hitbox_multpoint[2])return true; else return false;
		}break;

			//θ
		case HITBOX_STOMACH: {
			if (g_config.rage_hitbox_multpoint[3])return true; else return false;
		}break;

		case HITBOX_RIGHT_UPPER_ARM: {
			if (g_config.rage_hitbox_multpoint[4])return true; else return false;
		}break;
		case HITBOX_LEFT_UPPER_ARM: {
			if (g_config.rage_hitbox_multpoint[4])return true; else return false;
		}break;
		case HITBOX_RIGHT_FOREARM: {
			if (g_config.rage_hitbox_multpoint[4])return true; else return false;
		}break;
		case HITBOX_LEFT_FOREARM: {
			if (g_config.rage_hitbox_multpoint[4])return true; else return false;
		}break;


			//����
		case HITBOX_RIGHT_THIGH: {
			if (g_config.rage_hitbox_multpoint[5])return true; else return false;
		}break;
		case HITBOX_LEFT_THIGH: {
			if (g_config.rage_hitbox_multpoint[5])return true; else return false;
		}break;
			//С��
		case HITBOX_RIGHT_CALF: {
			if (g_config.rage_hitbox_multpoint[5])return true; else return false;
		}break;
		case HITBOX_LEFT_CALF: {
			if (g_config.rage_hitbox_multpoint[5])return true; else return false;
		}break;

		case HITBOX_RIGHT_FOOT: {
			if (g_config.rage_hitbox_multpoint[6])return true; else return false;
		}break;
		case HITBOX_LEFT_FOOT: {
			if (g_config.rage_hitbox_multpoint[6])return true; else return false;
		}break;



	}

	return false;
}

bool Ragebot::ShouldShot(int i,bool backtack) {
	switch (i)
	{
		//ͷ
		case HITBOX_HEAD:{
			if (backtack ? g_config.rage_backtrack_hitbox[0] : g_config.rage_hitbox[0])return true;else return false;
		}break;
		
		//����
		case HITBOX_NECK: {
			if (backtack ? g_config.rage_backtrack_hitbox[1] : g_config.rage_hitbox[1])return true;else return false;
		}break;

		//��
		case HITBOX_CHEST: {
			if (backtack ? g_config.rage_backtrack_hitbox[2] : g_config.rage_hitbox[2])return true;else return false;
		}break;
		case HITBOX_LOWER_CHEST: {
			if (backtack ? g_config.rage_backtrack_hitbox[2] : g_config.rage_hitbox[2])return true;else return false;
		}break;
		case HITBOX_UPPER_CHEST: {
			if (backtack ? g_config.rage_backtrack_hitbox[2] : g_config.rage_hitbox[2])return true;else return false;
		}break;

		//θ
		case HITBOX_STOMACH: {
			if (backtack ? g_config.rage_backtrack_hitbox[3] : g_config.rage_hitbox[3])return true;else return false;
		}break;

		case HITBOX_RIGHT_UPPER_ARM: {
			if (backtack ? g_config.rage_backtrack_hitbox[4] : g_config.rage_hitbox[4])return true;else return false;
		}break;
		case HITBOX_LEFT_UPPER_ARM: {
			if (backtack ? g_config.rage_backtrack_hitbox[4] : g_config.rage_hitbox[4])return true;else return false;
		}break;
		case HITBOX_RIGHT_FOREARM: {
			if (backtack ? g_config.rage_backtrack_hitbox[4] : g_config.rage_hitbox[4])return true;else return false;
		}break;
		case HITBOX_LEFT_FOREARM: {
			if (backtack ? g_config.rage_backtrack_hitbox[4] : g_config.rage_hitbox[4])return true;else return false;
		}break;


		//����
		case HITBOX_RIGHT_THIGH: {
			if (backtack ? g_config.rage_backtrack_hitbox[5] : g_config.rage_hitbox[5])return true;else return false;
		}break;
		case HITBOX_LEFT_THIGH: {
			if (backtack ? g_config.rage_backtrack_hitbox[5] : g_config.rage_hitbox[5])return true;else return false;
		}break;
		//С��
		case HITBOX_RIGHT_CALF: {
			if (backtack ? g_config.rage_backtrack_hitbox[5] : g_config.rage_hitbox[5])return true;else return false;
		}break;
		case HITBOX_LEFT_CALF: {
			if (backtack ? g_config.rage_backtrack_hitbox[5] : g_config.rage_hitbox[5])return true;else return false;
		}break;

		case HITBOX_RIGHT_FOOT: {
			if (backtack ? g_config.rage_backtrack_hitbox[6] : g_config.rage_hitbox[6])return true;else return false;
		}break;
		case HITBOX_LEFT_FOOT: {
			if (backtack ? g_config.rage_backtrack_hitbox[6] : g_config.rage_hitbox[6])return true;else return false;
		}break;



	}

	return false;
}


//��ȡ���֮��ֱ��push��aimpoint
void Ragebot::GetPlayerMultipoint(C_BasePlayer* player) {

	const auto model = player->GetModel();
	if (!model)
		return;
	const auto studio_hdr = g_interfaces::g_mdlinfo->GetStudiomodel(model);
	if (!studio_hdr)
		return;
	mstudiohitboxset_t* set = studio_hdr->GetHitboxSet(player->m_nHitboxSet());

	auto& log = g_PlayerLog.get_log(player->EntIndex());
	auto& newrecord = /*log.getnewrecord()*//*log.record[0]*/log.record.front();
	//���µ�һ����¼��δ����������
	//�ѽ����������齻������ɨ��
	if (!newrecord.m_bonefix){
		return;
	}

	int need_backtrack = TIME_TO_TICKS(player->m_flSimulationTime() - newrecord.m_flSimulationTime);
	if (need_backtrack < MinBacktrackTick && !g_resolver.datas[player->EntIndex()].breaklc){
		return;
	}

	matrix3x4_t* bone = reinterpret_cast<matrix3x4_t*>(&newrecord.matrix);

	//if(g_Cheatdata.m_DrawAimbotBone)c_rage.DrawCapsule(player, bone, Color(255, 204, 0, 255), 0.2);
	lag_record_t* backup = new lag_record_t(player);
	newrecord.recoverBone(player);
	for (auto i = 0; i < HITBOX_MAX; i++)
	{
		//if (!(i == HITBOX_HEAD || i == HITBOX_PELVIS || i == HITBOX_STOMACH || i == HITBOX_CHEST))continue;

		auto hitbox = set->GetHitbox(i);
		if (!hitbox)
			continue;

		if (!ShouldShot(i, false))continue;
		/*
			2021/10/20
			��ͻȻ��ʶ��ragebot�ӻ����л�ȡ�ľ���Ҳ�ǲ�׼ȷ��,Ҳ��δ���޸���CSGO�ͻ��˾���
			�����ڿͻ��˶�������,����������������

			�һ��ǱȽ���������ʶ������һ���,��Ȼ�ҵ�ragebot��ֻ��ʹ��һ���ͷ�������ͬ���Ĺ�ʺ����

			�����޸�Ŀǰ����Ҫ�������������Ƿ�����absyaw ��������һ�������absyaw �ͺ����ҵ�������Ч
			���absyaw�����ڶ��������б���Ϸ�����ģ����Ҿͱ�������absyaw ��Ϊ��Ϸ���������������Ѿ���ȫֹͣ����
			��Ӧ�ü���ҵĶ������뿪ʼ����֮ǰabsyaw�Ƿ�����
		
		*/
		auto& mat = /*player->GetBoneCache().m_pCachedBones*/bone[hitbox->bone];
		const auto mod = hitbox->m_flRadius != -1.f ? hitbox->m_flRadius : 0.f;

		Vector max;
		Vector min;
		Math::VectorTransform(hitbox->bbmax + mod, mat, max);
		Math::VectorTransform(hitbox->bbmin - mod, mat, min);

		const auto center = (min + max) * 0.5f;
		const auto cur_angles = Math::CalcAngle(center, /*g_Cheatdata.LocalPlayer->GetEyePos()*/g_EnginePrediction.getunpredEyepos());

		Vector curAngles = { cur_angles.pitch,cur_angles.yaw,cur_angles.roll };

		Vector forward;
		Math::AngleVectors(curAngles, &forward);

		auto rs = hitbox->m_flRadius * (g_config.rage_multipoint / 100.f);

		if (rs < 0.2)
			continue;

		const auto right = forward.Cross(Vector(0, 0, 1)) * rs;
		const auto left = Vector(-right.x, -right.y, right.z);
		const auto top = Vector(0, 0, 1) * rs;

		//Vector center
		Vector rightPoint = center + right;
		Vector leftPoint = center + left;
		Vector topPoint = center + top;

		std::vector<AimPoint> points;
		AimPoint point_center, point_left, point_right, point_top;

		point_center.damage = /*g_Autowall.GetDamagez(center, player)*/g_CAutoWall.GetDamage(g_Cheatdata.LocalPlayer,center);
		point_center.point = center;
		point_center.PointType = AimPoint::AimPointType::FromNewest;
		point_center.health = player->m_iHealth();
		point_center.player = player;
		point_center.hitbox = i;
		points.emplace_back(point_center);
		if (ShouldMuitPoint(i)) {
			point_right.damage = /*g_Autowall.GetDamagez(rightPoint, player)*/g_CAutoWall.GetDamage(g_Cheatdata.LocalPlayer, rightPoint);
			point_right.point = rightPoint;
			point_right.PointType = AimPoint::AimPointType::FromNewest;
			point_right.health = player->m_iHealth();
			point_right.player = player;
			point_right.hitbox = i;

			point_left.damage = /*g_Autowall.GetDamagez(leftPoint, player)*/g_CAutoWall.GetDamage(g_Cheatdata.LocalPlayer, leftPoint);
			point_left.point = leftPoint;
			point_left.PointType = AimPoint::AimPointType::FromNewest;
			point_left.health = player->m_iHealth();
			point_left.player = player;
			point_left.hitbox = i;

			point_top.damage = /*g_Autowall.GetDamagez(topPoint, player)*/g_CAutoWall.GetDamage(g_Cheatdata.LocalPlayer, topPoint);
			point_top.point = topPoint;
			point_top.PointType = AimPoint::AimPointType::FromNewest;
			point_top.health = player->m_iHealth();
			point_top.player = player;
			point_top.hitbox = i;
		
			points.emplace_back(point_right);
			points.emplace_back(point_left);
			points.emplace_back(point_top);
		}
		
		for (int i = 0; i < points.size(); i++)
		{
			if ((points[i].damage >= g_config.rage_damage) || (points[i].damage >= points[i].health))
			{
				points[i].simeTime = player->m_flSimulationTime();
				points[i].Record = &log.record[0];
				c_rage.Aimpoints.push_back(points[i]);
			}

		}
	}
	backup->recoverBone(player);
	delete backup;

}

void Ragebot::GetPoint() {
	

}
void Ragebot::CalcSafePoint(){

	
	for (auto& point : Aimpoints){
		lag_record_t backup(point.player);
		point.Record->recoverBone_backup(point.player);
		if (g_Autowall.GetDamagez(point.point, point.player)) {
			point.is_safe = true;
			safe_points.emplace_back(point);
		};


		backup.recoverBone(point.player);
	}
}
AimPoint Ragebot::GetBestPoint(){
	
	int curdamage = 0;
	int curhea = 1000;
	AimPoint rtPort;
	rtPort.point = { 0,0,0 };


	if (g_config.rage_safe_point) {
	
	
		if (g_config.rage_Target_selection == 0)
		{
			for (int i = 0; i < safe_points.size(); i++)
			{
				const auto& curpoint = safe_points[i];
				if (curpoint.PointType == AimPoint::AimPointType::FromRecord)continue;
				if (curpoint.damage > curdamage)
				{
					//����baimѡ��
					if (Shouldbaim[curpoint.player->EntIndex()] && curpoint.hitbox == HITBOX_HEAD) {
						continue;
					}
					curdamage = curpoint.damage;
					rtPort = curpoint;
				}

			}

		}
		//���Ѫ��
		else if (g_config.rage_Target_selection == 1) {
			for (int i = 0; i < safe_points.size(); i++)
			{
				const auto& curpoint = safe_points[i];
				if (curpoint.PointType == AimPoint::AimPointType::FromRecord)continue;
				if (curpoint.health < curhea)
				{
					//����baimѡ��
					if (Shouldbaim[curpoint.player->EntIndex()] && curpoint.hitbox == HITBOX_HEAD) {
						continue;
					}
					curhea = curpoint.health;
					rtPort = curpoint;
				}

			}


		}
		if (rtPort.point != Vector( 0,0,0)){
			return rtPort;
		}
		else {
			//�������force ��ȥ��������ɨ��
			if (g_config.rage_safe_point == 2) {
				return rtPort;
			}

			//���³�ʼ������
			curdamage = 0;
			curhea = 1000;
			rtPort.point = { 0,0,0 };
				
		}
	
	
	}

	


	//����˺�
	if (g_config.rage_Target_selection == 0)
	{
		for (int i = 0; i < Aimpoints.size(); i++)
		{
			const auto& curpoint = Aimpoints[i];
			if (curpoint.PointType == AimPoint::AimPointType::FromRecord)continue;
			if (curpoint.damage > curdamage)
			{
				//����baimѡ��
				if (Shouldbaim[curpoint.player->EntIndex()] && curpoint.hitbox == HITBOX_HEAD){
					continue;
				}
				curdamage = curpoint.damage;
				rtPort = curpoint;
			}

		}
		
	}
	//���Ѫ��
	else if (g_config.rage_Target_selection == 1) {
		for (int i = 0; i < Aimpoints.size(); i++)
		{
			const auto& curpoint = Aimpoints[i];
			if (curpoint.PointType == AimPoint::AimPointType::FromRecord)continue;
			if (curpoint.health < curhea)
			{
				//����baimѡ��
				if (Shouldbaim[curpoint.player->EntIndex()] && curpoint.hitbox == HITBOX_HEAD) {
					continue;
				}
				curhea = curpoint.health;
				rtPort = curpoint;
			}

		}


	}


	return rtPort;
}



AimPoint Ragebot::GetShotingRecordPoint(){
	//ֻ��������δ�ҵ���¼����� ����Ҫ��������������û�ҵ��κζ���
	AimPoint rtPort;
	rtPort.point = Vector(0, 0, 0);
	
	for (int i = 0; i < Aimpoints.size(); i++) {
		if (Aimpoints[i].shoting) {
			//����������ɿ���һ�� ��������Ҳ�������ܱ�ø���
			return Aimpoints[i];
		}

	}

	return rtPort;
}

//����Ӧ����������¼�����Ż����������fps����
//����������ⲻ��ֻ���ҷ�����������Ҫ�ڲ�������˺���¼������¾����Ż� ���ڵļܹ���ʵ����������� ֻҪ�ҵ�ɨ�����˳� ���п��ܻ������Ǵ���ܶ��¼
//��Ҫ˼��һ�� ����û�������
//�Լ��������������̫������

AimPoint Ragebot::GetBestRecordPoint() {

	int curdamage = 0;
	AimPoint rtPort;
	rtPort.point = Vector(0,0,0);

	for (int i = 0; i < Aimpoints.size(); i++)
	{
		const auto curpoint = Aimpoints[i];
		if (curpoint.PointType == AimPoint::AimPointType::FromNewest)continue;

		if (curpoint.damage > curdamage)
		{
			//����baimѡ��
			if (Shouldbaim[curpoint.player->EntIndex()] && curpoint.hitbox == HITBOX_HEAD) {
				if (g_config.rage_cancel_biam_if_enemy_shooting && curpoint.shoting) {
					goto rtpoint;
				}continue;
			}
			rtpoint:;
			curdamage = curpoint.damage;
			rtPort = curpoint;
		}

	}
	return rtPort;


}



void Ragebot::DrawCapsule(C_BasePlayer* player, matrix3x4_t* bones, Color clr, bool ignorez,float duration) {
	studiohdr_t* pStudioModel = g_interfaces::g_mdlinfo->GetStudiomodel(player->GetModel());
	if (!pStudioModel)
		return;
	mstudiohitboxset_t* pHitboxSet = pStudioModel->GetHitboxSet(0);
	if (!pHitboxSet)
		return;

	for (int i = 0; i < pHitboxSet->numhitboxes; i++)
	{
		mstudiobbox_t* pHitbox = pHitboxSet->GetHitbox(i);
		if (!pHitbox)
			continue;

		Vector vMin, vMax;
		Math::VectorTransform(pHitbox->bbmin, bones[pHitbox->bone], vMin);
		Math::VectorTransform(pHitbox->bbmax, bones[pHitbox->bone], vMax);

		int r = 255,
			g = 255,
			b = 255,
			a = 255;

		if (pHitbox->m_flRadius != -1)
			g_interfaces::g_IVDebugOverlay->AddCapsuleOverlay(vMin, vMax, pHitbox->m_flRadius, clr.r(), clr.g(), clr.b(), clr.a(), duration, 0, ignorez);
	}
}