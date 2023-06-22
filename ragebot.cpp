#include"ragebot.h"


                                                         

//          __             __                                    
//    _____/ /____  ______/ /___  ____ ___  ___    ____ _      __
//   / ___/ //_/ / / / __  / __ \/ __ `__ \/ _ \  / __ \ | /| / /
//  (__  ) ,< / /_/ / /_/ / /_/ / / / / / /  __/ / /_/ / |/ |/ / 
// /____/_/|_|\__, /\__,_/\____/_/ /_/ /_/\___(_) .___/|__/|__/  
//           /____/                            /_/               
// Kiana1337 Ryusei423




//检查是否为有效的玩家实体
//后面可能会加个射击队友 或者直接用是否为敌人来检测 可能出现在v1或者v2的更新中
bool Ragebot::CheckPlayer(C_BasePlayer* player) {
	
	if (!player)return false;
	if (player->IsDormant() || 
		!player->IsAlive() ||
		!player->isEnemy() ||
		player->GetClientClass()->m_ClassID!= CCSPlayer ||
		player == g_Cheatdata.LocalPlayer || 
		player->m_vecOrigin() == Vector(0, 0, 0)||
		player->GetImmunity()||
		g_PlayerLog.get_log(player->EntIndex()).record.empty())//ragebot现在开始完全基于PlayerLog
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
	-极大地改进了左轮手枪上的ragebot.
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
		auto time = weapon->m_flPostponeFireReadyTime();//改成>=应该更好 aa那边不知道会不会被影响
		if (!time || time >= g_Utilities::get_servertime() /*|| Misc::warmuptick < 14*/) {
			g_Cheatdata.Revolver_tryshoting = false;
			return false;
		}
			
	}

	//检查子弹
	if (weapon->m_iClip1() <= 0)
		return false;
	//玩家的下一次攻击
	if(g_Cheatdata.LocalPlayer->NextAttack() > g_Utilities::get_servertime())
		return false;
	//武器的下一次攻击
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
	//如果命中率等于0直接返回true
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

	//考虑我们已经开启了无后座
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
			valid_target[i.aim_ent->EntIndex()] = false;//让后面的扫描跳过这个目标 我们会有更多的fps
		};


	}





}


int FixTickcout(float sim) {
	//加上他妈的客户端内插他妈的他妈的他妈的他妈的他妈的他妈的他妈的他妈的他妈的他妈的他妈的他妈的他妈的他妈的他妈的他妈的
	return TIME_TO_TICKS(sim + g_LagComp.get_lerptime());
}

/*
	众所周知ragebot的最新记录扫描是直接读取records[0]的，那它是本tick中的最新记录也很正常
	或许要了解它实际上正不正常 是帧阶段的问题
	问题不在这里


*/
void Ragebot::on_crmo(CUserCmd* pcmd)
{

	//重置信息
	noobs.clear();
	Aimpoints.clear();
	safe_points.clear();

	C_BaseCombatWeapon* weapon = (C_BaseCombatWeapon*)g_interfaces::g_ClientEntityList->GetClientEntity(hooks::g_Local_player->GetActiveWeaponIndex());if (!weapon)return;
	CCSWeaponInfo* weapon_info = weapon->GetInfo();if (!weapon || !weapon_info)return;

	//不能射击时依然储存扫描点 我们还有别的事情要处理
	/*if (!CanShot(weapon))
		return;*/
	if (!weapon || weapon->IsGrenade() || weapon->m_iItemDefinitionIndex() == WEAPON_HEALTHSHOT || weapon->m_iItemDefinitionIndex() == WEAPON_C4){
		return;
	}
	
	//获取所有可能为目标的实体
	get_target();

	if (noobs.empty())
		return;
	if (tickbase.isShifting)
		return;
	
	//获取将要交火的实体
	//get_will_shot();

	//检查有效性
	//Checkwillshot();

	has_valid = false;
	MinBacktrackTick = g_config.rage_doubletap_key.value ? tickbase.doubletapCharge > 10 ? tickbase.doubletapCharge - 10 : 0 : 0;

	Vector move_dir;
	float oring_move_per_tick = (m_vecOrigin  - g_Cheatdata.LocalPlayer->m_vecOrigin()).Length();
	if (oring_move_per_tick > 10.f || oring_move_per_tick < -10.f){
		//我们得到了一个狗屎,地图已经更改或者玩家已经重生
		m_vecOrigin = g_Cheatdata.LocalPlayer->m_vecOrigin();
		if (CanShot(weapon)){
			goto StartScann;
		}
	}
	
	Math::AngleVectors(Vector(0.f, pcmd->viewangles.yaw, 0.f), &move_dir);
	m_vecOrigin_extrapolation = g_EnginePrediction.getunpredEyepos() + (move_dir * (oring_move_per_tick * 3));
	MainTest();
	m_vecOrigin = g_Cheatdata.LocalPlayer->m_vecOrigin();
	//如果我们马上就要射击
	if (has_valid && g_config.rage_autostop_auto_between_shots) {
		if (weapon_info->WeaponType == WEAPONTYPE_KNIFE /*|| weapon->m_iItemDefinitionIndex() == WEAPON_ZEUS*/)
			return;
		

		if (!CanShot(weapon)){
			//不要过早地开始急停,不然我们会在射击中一直处于急停的状态
			//此数值可以考虑交由用户处理，过低会让提前急停的效果变弱，过高又和完全的射击中急停无异
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

	//获取回溯扫描点
	if (g_config.rage_backtrack)GetCanShotLagRecord();

	//我们要禁用插值以保证最新记录的准确性(现在还没)
	//快进到2022/1/4:已经转为从PlayerLog中获取骨骼矩阵。插值已经被恢复
	for (int i = 0; i < noobs.size(); i++)
	{
		GetPlayerMultipoint(noobs[i].aim_ent);
	}

	if(Aimpoints.empty()){
		return;
	}
	

	/*
		2022/1/4
		现在的结构大概是:

		获取所有回溯扫描点
		获取所有最新扫描点
		
		从扫描点列表中查找所有最新扫描点
		如果查找失败则查找回溯扫描点

		这对性能很不友好...对ragebot的结构优化已经进入计划


	
	
	*/
	CalcSafePoint();
	CalcShouldbaim();
	AimPoint BestPoint;
	
	if (g_config.rage_prefer_shooting){
		BestPoint = GetShotingRecordPoint();
		//如果point是000，那退出此段 继续进行常规扫描
		if (BestPoint.point != Vector{ 0,0,0 }){
			goto endpointselect;
		}
	}

	BestPoint = GetBestPoint();
	if (BestPoint.point == Vector{ 0,0,0 }) {
		//如果从最新扫描点中查找失败 我们将开始查找回溯扫描点
		if(g_config.rage_backtrack)BestPoint = GetBestRecordPoint();
	}

	//如果全部失败则证明没有目标
	if (BestPoint.point == Vector{0,0,0})return;

	endpointselect:;
	//g_interfaces::g_IVDebugOverlay->AddBoxOverlay(BestPoint.point, Vector(-2, -2, -2), Vector(2, 2, 2), QAngle(0, 0, 0), /*26, 189, 230,*/221,34,221, 127, 0.5);
	QAngle AimAngle = Math::CalcAngle(/*g_Cheatdata.LocalPlayer->GetEyePos()*/g_EnginePrediction.getunpredEyepos(), BestPoint.point);

	
	
	/*
		Update 2021/10/8

		-适配最新的游戏版本
		-改进的DTHC
		-改进的自动停止
		-改进的射击回溯
		-修复了回溯命中率上关于恢复骨骼的错误
	
		只在命中率不满足时运行autostop是可行的，重点是如果第一次dt攻击和第二次dt攻击的时间差距过大 就不应该使用dt命中率
		我们需要一个"理想dt时间"如果超出时间则把下一次攻击为dt的flag设为0
		此值应该比我们储存的模拟tick要高一点
	*/

	/*
		10.11说明
		自动左轮会造成命中率计算错误而不急停。
		为防止rage出现奇怪的bug而我不知道 这点将在释放后被修复
		释放之前将不会再有大的rage更改 除非出现影响严重的bug	
	
	*/

	//如果连狙+开镜 我们直接写死命中率 18
	//或者把正常命中率除以5
	//实际上是个荒唐且无用的想法

	//应用dthc
	int hc = g_config.rage_hitchance;
	if (tickbase.DoubleTapAttack) {
		hc = g_config.rage_doubletap_hc;
		tickbase.DoubleTapAttack = false;
	}

	//储存当前骨骼并且为命中率扫描恢复骨缓存
	//(当在射击回溯的时候)
	lag_record_t backup(BestPoint.player);
		BestPoint.Record->recoverBone(BestPoint.player);

		bool hcOk = HitChance(BestPoint.player, weapon, g_Utilities::QangleVector(AimAngle),/*g_config.rage_hitchance*/hc);
		//提前计算命中率并且在同一tick中进行射击与自动停止 这应该能让我们获得更快的射击速度
		//并且只在命中率不满足时进行自动停止 可以让dt工作更正常
		if (!hcOk /*&& weapon->can_shot()*/ && weapon->m_iItemDefinitionIndex() != WEAPON_ZEUS)autostop(pcmd);
		
		//r8上有一些命中率计算错误 我们需要在r8上应用自动停止
		//实际上不用
		//if (weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && weapon->can_shot())autostop(pcmd);

	//恢复旧骨骼
	backup.recoverBone(BestPoint.player);


	if (BestPoint.player && hcOk /*&&*/ /*weapon->can_shot()*//*CanShot(weapon)*//*在完全解决预测问题之前 强制射击*/) {
		pcmd->viewangles = AimAngle;
		if (!g_config.rage_silent)g_interfaces::g_EngineClient->SetViewAngles(&AimAngle);

		//if (BestPoint.PointType == AimPoint::AimPointType::FromRecord)
		//{
		//	//恢复模拟时间来获得延迟补偿
		//	///*g_interfaces::g_GlobalVar->curtime*/pcmd->tick_count = /*BestPoint.simeTime*/FixTickcout(BestPoint.simeTime);
		//	BestPoint.player->set_abs_origin(BestRecord.m_vecOrigin);
		//	
		//}

		//完全无用 问题不是出在这里
		/*auto tickcountFix = calcBacktrack(BestPoint.player,AimAngle);
		if (tickcountFix)
		{
			pcmd->tick_count = tickcountFix;
		}else*/
		//g_Utilities::print(std::string("origin:tickcout:") + std::to_string(pcmd->tick_count));
		pcmd->tick_count = FixTickcout(BestPoint.Record->m_flSimulationTime);
		//g_Utilities::print(std::string("new:tickcout:") + std::to_string(pcmd->tick_count));
		//DrawCapsule(BestPoint.player, BestPoint.Record->matrix, Color(238, 17, 238, 255));
		//测试用的 释放版本要注释掉
		//if (pcmd->buttons & IN_ATTACK && BestPoint.PointType == AimPoint::AimPointType::FromRecord && g_Cheatdata.m_DrawBacktrackAimbotBone)
		//{
		//	DrawCapsule(BestPoint.player, BestPoint.Record->matrix,Color(238,17,238,255));
		//	//memcpy(&g_Cheatdata.ragebot_LagcompTest, BestPoint.Record->matrix, sizeof(matrix3x4_t) * 128);
		//	//g_Cheatdata.ragebot_test_ent_index = BestPoint.player->EntIndex();
		//}
		

		//auto debugTmp = FixTickcout(BestPoint.simeTime);
		if (g_config.rage_autoshot) {
			pcmd->buttons |= IN_ATTACK;
			//或许这个检查应该在别的地方 比如说整个快进过程都不运行aimbot
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

		//应该只在积攒tick到达14的时候设为false
		//释放后再更新 我不想让任何东西影响rage
		//

		//如果doingAutoRevolver设为false就代表我们的ragebot正在尝试在左轮上射击
		//所以AA的判断逻辑是doingAutoRevolver == false和左轮可以开火
		g_Cheatdata.Revolver_tryshoting = true;
	}


	return;	

}
void Ragebot::autostop(CUserCmd* ucmd)
{
	//lastfaststop = g_EnginePrediction.getunpredTime();
	//如果在空中并且没有开启空中急停 我们不执行
	if (!(g_Cheatdata.LocalPlayer->m_fFlags() & FL_ONGROUND) && !g_config.rage_autostop_in_air){
		return;
	}

	//取消跳跃按钮 来做空中急停
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
		//计算获取我们正在移动的方向
		Math::VectorAngles(g_Cheatdata.LocalPlayer->m_vecVelocity(), angle);

		// get our current speed of travel.
		//获取速度
		float speed = g_Cheatdata.LocalPlayer->m_vecVelocity().Length()/*450.f*/;

		// fix direction by factoring in where we are looking.
		// 考虑我们正在看的地方和正在移动的方向不一致
		angle.y = ucmd->viewangles.yaw - angle.y;

		// convert corrected angle back to a direction.
		// 将校正后的角度转换回方向数据(原翻译)
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
	//重新初始化baim标识
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
			//首先设置为true，然后再开始计算需要关闭的情况
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
	
	-修复了回溯扫描(是的,我的错,之前它从来没有工作过lol).


*/
bool Ragebot::GetRecorodPoint(C_BasePlayer* player, lag_record_t* record, bool istest = false, Vector test_pos = {0,0,0}) {
	//if (!g_PlayerLog.IsValid(record->m_flSimulationTime))return false;
	
	//由于dt回溯的问题 我们需要扫描原地回溯 
	//if (record->m_vecOrigin == player->Get_vec_origin())return false;

	
	//恢复记录中的数据 开始扫描伤害
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
			//如果这是一次测试
			//即使这是一个回溯记录 我们也考虑正常hitbox
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

		//测试中不需要中心点
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
	完全扫描回溯正在造成一些fps问题 一些优化措施是必须的
	我想到的解决方法是对回溯记录进行测试 比如说我们测试三个地方 前百分之25处 前百分之50处 前百分之100处
	测试数量可以根据fps决定 包括是否进行测试

	这是一个TODO 我要先完成参数系统 这个选择的唯一原因是我现在没有那么多的用户
	我打算写一个带有武器图标的现代化指示器 把一些代码直接写在cpp中可能也不错 我害怕的是它会让代码过于臃肿
	我的喜好是代码乱一点无所谓，但是不能臃肿
	我们甚至可以直接将自带的指示器写在lua里 哈哈
*/
void Ragebot::GetCanShotLagRecord()
{
	//过滤记录
	//g_PlayerLog.FilterRecords();
	for (int i = 1; i < g_interfaces::g_EngineClient->GetMaxClients(); i++)
	{
		//g_PlayerLog.FilterRecords();
		auto curplayer = static_cast<C_BasePlayer*>(g_interfaces::g_ClientEntityList->GetClientEntity(i));
		auto& curlog = g_PlayerLog.get_log(i);
		auto& currecord = curlog.record;
		if (!CheckPlayer(curplayer))continue;
		if (g_resolver.datas[curplayer->EntIndex()].breaklc)continue;//不要回溯LC目标
		
		//g_Cheatdata.hc_testpoint1 = curlog.record.begin()->m_vecOrigin;
		//备份当前最新的数据
		lag_record_t oldRecord(curplayer);
		if (!/*TestRecords(currecord, curplayer)*/valid_target[i]) {
			oldRecord.recoverBone(curplayer);
			continue;
		}
		//开始扫描回溯矩阵
		//找到一个最近回溯之后此变量被设为true，然后我们只寻找射击记录
		bool findRecord = false;
		//注释掉的是反向迭代 用来测试回溯稳定性
		for (auto i = 0u; i < currecord.size(); i++/*int i = currecord.size()-1; i >= 0; i--*/) {
			//备份旧的信息
			//lag_record_t oldRecord(curplayer);
			//g_Cheatdata.hc_testpoint1 = i->m_vecOrigin;
			if(currecord[i].valid && currecord[i].m_bonefix){
				int bt = TIME_TO_TICKS(curplayer->m_flSimulationTime() - currecord[i].m_flSimulationTime);
				if (bt < MinBacktrackTick){
					continue;
				}
				if (!findRecord && GetRecorodPoint(curplayer, &currecord[i])) {
					//如果已经找到并且储存了一组扫描点 我们退出这个人的扫描进程
					//break;
					findRecord = true;
					continue;//只在一个记录上运行一次扫描
				}
				if (currecord[i].m_Shoting){
					GetRecorodPoint(curplayer, &currecord[i]);
				}
			}


		}
		
		//恢复备份的记录
		oldRecord.recoverBone(curplayer);
		
	}


}

//基本是ShouldShot的副本，我懒得更改那个函数
bool Ragebot::ShouldMuitPoint(int i)
{
	switch (i)
	{
			//头
		case HITBOX_HEAD: {
			if (g_config.rage_hitbox_multpoint[0])return true; else return false;
		}break;

			//脖子
		case HITBOX_NECK: {
			if (g_config.rage_hitbox_multpoint[1])return true; else return false;
		}break;

			//胸
		case HITBOX_CHEST: {
			if (g_config.rage_hitbox_multpoint[2])return true; else return false;
		}break;
		case HITBOX_LOWER_CHEST: {
			if (g_config.rage_hitbox_multpoint[2])return true; else return false;
		}break;
		case HITBOX_UPPER_CHEST: {
			if (g_config.rage_hitbox_multpoint[2])return true; else return false;
		}break;

			//胃
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


			//大腿
		case HITBOX_RIGHT_THIGH: {
			if (g_config.rage_hitbox_multpoint[5])return true; else return false;
		}break;
		case HITBOX_LEFT_THIGH: {
			if (g_config.rage_hitbox_multpoint[5])return true; else return false;
		}break;
			//小腿
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
		//头
		case HITBOX_HEAD:{
			if (backtack ? g_config.rage_backtrack_hitbox[0] : g_config.rage_hitbox[0])return true;else return false;
		}break;
		
		//脖子
		case HITBOX_NECK: {
			if (backtack ? g_config.rage_backtrack_hitbox[1] : g_config.rage_hitbox[1])return true;else return false;
		}break;

		//胸
		case HITBOX_CHEST: {
			if (backtack ? g_config.rage_backtrack_hitbox[2] : g_config.rage_hitbox[2])return true;else return false;
		}break;
		case HITBOX_LOWER_CHEST: {
			if (backtack ? g_config.rage_backtrack_hitbox[2] : g_config.rage_hitbox[2])return true;else return false;
		}break;
		case HITBOX_UPPER_CHEST: {
			if (backtack ? g_config.rage_backtrack_hitbox[2] : g_config.rage_hitbox[2])return true;else return false;
		}break;

		//胃
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


		//大腿
		case HITBOX_RIGHT_THIGH: {
			if (backtack ? g_config.rage_backtrack_hitbox[5] : g_config.rage_hitbox[5])return true;else return false;
		}break;
		case HITBOX_LEFT_THIGH: {
			if (backtack ? g_config.rage_backtrack_hitbox[5] : g_config.rage_hitbox[5])return true;else return false;
		}break;
		//小腿
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


//获取多点之后直接push进aimpoint
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
	//最新的一个记录尚未被构建骨骼
	//把接下来的事情交给回溯扫描
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
			我突然意识到ragebot从缓存中获取的矩阵也是不准确的,也是未经修复的CSGO客户端矩阵
			它存在客户端动画错误,与服务器的误差严重

			我还是比较庆幸我意识到了这一点的,不然我的ragebot就只能使用一个和服务器不同步的狗屎矩阵

			动画修复目前的主要问题是无论我是否设置absyaw 动画都有一个错误的absyaw 就好像我的设置无效
			如果absyaw不是在动画更新中被游戏构建的，那我就必须设置absyaw 因为游戏的其他动画代码已经完全停止工作
			我应该检查我的动画代码开始运行之前absyaw是否被设置
		
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
					//关于baim选项
					if (Shouldbaim[curpoint.player->EntIndex()] && curpoint.hitbox == HITBOX_HEAD) {
						continue;
					}
					curdamage = curpoint.damage;
					rtPort = curpoint;
				}

			}

		}
		//最低血量
		else if (g_config.rage_Target_selection == 1) {
			for (int i = 0; i < safe_points.size(); i++)
			{
				const auto& curpoint = safe_points[i];
				if (curpoint.PointType == AimPoint::AimPointType::FromRecord)continue;
				if (curpoint.health < curhea)
				{
					//关于baim选项
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
			//如果不是force 下去进行正常扫描
			if (g_config.rage_safe_point == 2) {
				return rtPort;
			}

			//重新初始化数据
			curdamage = 0;
			curhea = 1000;
			rtPort.point = { 0,0,0 };
				
		}
	
	
	}

	


	//最高伤害
	if (g_config.rage_Target_selection == 0)
	{
		for (int i = 0; i < Aimpoints.size(); i++)
		{
			const auto& curpoint = Aimpoints[i];
			if (curpoint.PointType == AimPoint::AimPointType::FromRecord)continue;
			if (curpoint.damage > curdamage)
			{
				//关于baim选项
				if (Shouldbaim[curpoint.player->EntIndex()] && curpoint.hitbox == HITBOX_HEAD){
					continue;
				}
				curdamage = curpoint.damage;
				rtPort = curpoint;
			}

		}
		
	}
	//最低血量
	else if (g_config.rage_Target_selection == 1) {
		for (int i = 0; i < Aimpoints.size(); i++)
		{
			const auto& curpoint = Aimpoints[i];
			if (curpoint.PointType == AimPoint::AimPointType::FromRecord)continue;
			if (curpoint.health < curhea)
			{
				//关于baim选项
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
	//只用来处理未找到记录的情况 我们要告诉主函数我们没找到任何东西
	AimPoint rtPort;
	rtPort.point = Vector(0, 0, 0);
	
	for (int i = 0; i < Aimpoints.size(); i++) {
		if (Aimpoints[i].shoting) {
			//返回最新最可靠的一个 此外这样也能让性能变得更好
			return Aimpoints[i];
		}

	}

	return rtPort;
}

//我们应该针对射击记录进行优化，此外就是fps问题
//我想这个问题不是只对我发生，我们需要在不错过高伤害记录的情况下尽量优化 现在的架构其实就有这个问题 只要找到扫描点就退出 这有可能会让我们错过很多记录
//需要思考一下 今天没这个心情
//自己活在这世上真的太困难了

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
			//关于baim选项
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