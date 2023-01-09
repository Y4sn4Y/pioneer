// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

//Student Soccer 2D Simulation Base , STDAGENT2D
//Simplified the Agent2D Base for HighSchool Students.
//Technical Committee of Soccer 2D Simulation League, IranOpen
//Nader Zare
//Mostafa Sayahi
//Pooria Kaviani
/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bhv_basic_offensive_kick.h"

#include <rcsc/action/body_hold_ball.h>
#include <rcsc/action/body_smart_kick.h>
#include <rcsc/action/body_stop_ball.h>
#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

#include <rcsc/geom/sector_2d.h>

#include <vector>
#include <boost/core/swap.hpp>
#include <sys/swap.h>
#include <boost/core/swap.hpp>
using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!

 */
bool
Bhv_BasicOffensiveKick::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicOffensiveKick" );

    const WorldModel & wm = agent->world();
    
   
    const PlayerPtrCont & opps = wm.opponentsFromSelf();
    const PlayerObject * nearest_opp
        = ( opps.empty()
            ? static_cast< PlayerObject * >( 0 )
            : opps.front() );
    const double nearest_opp_dist = ( nearest_opp
                                      ? nearest_opp->distFromSelf()
                                      : 1000.0 );
//    const Vector2D nearest_opp_pos = ( nearest_opp
//                                       ? nearest_opp->pos()
//                                       : Vector2D( -1000.0, 0.0 ) );

    if(nearest_opp_dist < 10){
    	if(pass(agent))
    		return true;
    }



    if(dribble(agent)){
    	return true;
    }

    if ( nearest_opp_dist > 2.5 )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": hold" );
        agent->debugClient().addMessage( "OffKickHold" );
        Body_HoldBall().execute( agent );
        return true;
    }
    clearball(agent);
    return true;
    if( wm.gameMode().type()!=GameMode::PlayOn && safe(agent , best_target ,    3)) 
    {
        Body_SmartKick(best_target ,3 , 2.9 , 1).execute(agent);
    }
    else if(safe(agent , best_target , 3 ))
    {
        Body_SmartKick(best_target ,3 , 2.9 , 1).execute(agent);
    }
}

bool Bhv_BasicOffensiveKick::shoot( rcsc::PlayerAgent * agent ){
	const WorldModel & wm = agent->world();
	Vector2D ball_pos = wm.ball().pos();
	Vector2D center_goal = Vector2D(52.5,0);
	if(ball_pos.dist(center_goal) > 15)
            return false;
    Vector2D goal[26];
    double DistFromGoal[26];
    int goal_num=0;
    for (double i=-6;i<=6;i=i+0.5){
        goal[goal_num]= Vector2D(52.5,i);
        DistFromGoal[goal_num] = goal[goal_num].dist(ball_pos);
        Sector2D goal_triangle = Sector2D (ball_pos,1,ball_pos.dist(goal[goal_num]),(goal[goal_num] - ball_pos).th()-20,(goal[goal_num] - ball_pos).th()+20);
        if (wm.existOpponentIn(goal_triangle,5,true)){
            goal[goal_num]=Vector2D(0,0);
        }
        ++goal_num;
    }
    double swaping;
    Vector2D swap_vec;
    for (int i=0;i<25;++i)
    {
        for (int j=i+1;j<26;++j)
        {
            if (DistFromGoal[i] > DistFromGoal[j])
            {
                swaping = DistFromGoal[i];
                DistFromGoal[i]=DistFromGoal[j];
                DistFromGoal[j]=swaping;
                swap_vec = goal[i];
                goal[i]=goal[j];
                goal[j]=swap_vec;
            }
        }
    }
    int best_shoot=-1;
    for (int i=25;i<0;--i)
    {
        if (goal[i] != Vector2D(0,0))
        {
            best_shoot=i;
        }
    }
    if (best_shoot != -1)
    {
        return false;
    }
    else {
        Body_SmartKick(goal[best_shoot],3,2.7,2).execute(agent);
        return true;
    }
	Vector2D left_goal = Vector2D(52.5,6);ljvku
	Vector2D right_goal = Vector2D(52.5,-6);
    Sector2D left_triangle =Sector2D (ball_pos,1,ball_pos.dist(left_goal),(left_goal - ball_pos).th()-20,(left_goal - ball_pos).th()+20);
    Sector2D right_triangle = Sector2D (ball_pos,1,ball_pos.dist(right_goal),(right_goal - ball_pos).th()-20,(right_goal - ball_pos).th()+20);
    if (!wm.existOpponentIn(shoot, 5 , true){
            continue;
        }
	if(left_goal.dist(ball_pos) < right_goal.dist(ball_pos) && ! wm.existOpponentIn(left_triangle,5,true)){
        Body_SmartKick(left_goal,2.7,2.7,2).execute(agent);
        return true;
        
	}else if (! wm.existOpponentIn(right_triangle,5,true)){
        Body_SmartKick(right_goal,2.7,2.7,2).execute(agent);
        return true;
	}
	
}

bool Bhv_BasicOffensiveKick::pass(PlayerAgent * agent){
	const WorldModel & wm = agent->world();
	std::vector<Vector2D> targets;
	Vector2D ball_pos = wm.ball().pos();
	for(int u = 1;u<=11;u++){
		const AbstractPlayerObject * tm = wm.ourPlayer(u);
		if(tm->unum() == wm.self().unum() )
			continue;
		Vector2D tm_pos = tm->pos();
        Vector2D new_self = wm.self().pos();
		if(tm->pos().dist(ball_pos) > new_self.x - 1)
			continue;
		Sector2D pass = Sector2D(ball_pos,1,tm_pos.dist(ball_pos)+3,(tm_pos - ball_pos).th() - 15,(tm_pos - ball_pos).th() + 15);
		if(!wm.existOpponentIn(pass,5,true)){
			targets.push_back(tm_pos);
		}
	}
	if(targets.size() == 0)
		return false;
	Vector2D best_target = targets[0];
    for(unsigned int i=1;i<targets.size();i++){
		if(targets[i].x > best_target.x)
			best_target = targets[i];
	}
	
	if(wm.gameMode().type()!= GameMode::PlayOn && safe(agent,best_target,3))
        Body_SmartKick(best_target,3,2.7,1).execute(agent);
	else if(safe(agent,best_target,3))
        Body_SmartKick(best_target,3,2.7,2).execute(agent);
	return true;
}

bool Bhv_BasicOffensiveKick::dribble(PlayerAgent * agent){
	const WorldModel & wm = agent->world();
	Vector2D ball_pos = wm.ball().pos();
	double dribble_angle = (Vector2D(52.5,0) - ball_pos).th().degree();
	Sector2D dribble_sector = Sector2D(ball_pos,0,3,dribble_angle - 15,dribble_angle+15);
	if(!wm.existOpponentIn(dribble_sector,5,true)){
		Vector2D target = Vector2D::polar2vector(3,dribble_angle) + ball_pos;
		if(Body_SmartKick(target,3,2.7,2).execute(agent)){
			return true;
		}
	}
	return false;
}

bool Bhv_BasicOffensiveKick::clearball(PlayerAgent * agent){
    const WorldModel & wm = agent->world();
    Vector2D ball_pos = wm.ball().pos();
    Vector2D target = Vector2D(52.5,0);
    if(ball_pos.x < 0){
        if(ball_pos.x > -25){
            if(ball_pos.dist(Vector2D(0,-34)) < ball_pos.dist(Vector2D(0,+34))){
                target = Vector2D(0,-34);
            }else{
                target = Vector2D(0,+34);
            }
        }else{
            if(ball_pos.absY() < 10 && ball_pos.x < -10){
                if(ball_pos.y > 0){
                    target = Vector2D(-52,20);
                }else{
                    target = Vector2D(-52,-20);
                }
            }else{
                if(ball_pos.y > 0){
                    target = Vector2D(ball_pos.x,34);
                }else{
                    target = Vector2D(ball_pos.x,-34);
                }
            }
        }
    }
    if(Body_SmartKick(target,305,2.7,3).execute(agent)){
        return true;
    }
    Body_StopBall().execute(agent);
    return true;
}

int Bhv_BasicOffensiveKick::score_dist(rcsc::PlayerAgent* agent, rcsc::PlayerObject* tm)
{
    const WorldModel & wm = agent->world();
    Vector2D tm_pos = tm->pos();
    Vector2D center_goal(52.5,0);
    double dist = tm_pos.dist(center_goal);
    int score;
    if(dist<10)
    {
        score=10;
    }
    else if(dist<20)
    {
        score=9;
    }
    else if(dist<30)
    {
        score=8;
    }
    else if(dist<40)
    {
        score=7;
    }
    else if(dist<50)
    {
        score=6;
    }
    else if(dist<60)
    {
        score=5;
    }
    else if(dist<70)
    {
        score=4;
    }
    else if(dist<80)
    {
        score=3;
    }
    else if(dist<90)
    {
        score=2;
    }
    else
    {
        score=1;
    }
    return score;
}

int Bhv_BasicOffensiveKick::sum_score(rcsc::PlayerAgent* agent, rcsc::PlayerObject* tm)
{
    return score_dist(agent,tm) + score_sector(agent,tm)+score_safe(agent,tm);
}

bool Bhv_BasicOffensiveKick::pass_test(rcsc::PlayerAgent* agent)
{
    const WorldModel &wm = agent->world();
    PlayerPtrCont tm = wm.teammatesFromSelf();
    PlayerObject *t;
    PlayerObject *best_player = tm[0];
    int max_score = sum_score(agent,best_player);
    for(int i=1 ; i<tm.size() ; i++)
    {
        t=tm[i];
       // if(t->unum()==1)
       // {
          //  continue;
       // }
        if(sum_score(agent,t) > max_score)
        {
            max_score=sum_score(agent,t);
            best_player=t;
        }
    }
    
    
    std::vector<Vector2D> targets;
	Vector2D ball_pos = wm.ball().pos();
	for(int u = 1;u<=11;u++){
		const AbstractPlayerObject * tm = wm.ourPlayer(u);
		if(tm->unum() == wm.self().unum() )
			continue;
		Vector2D tm_pos = tm->pos();
		if(tm->pos().dist(ball_pos) > 30)
			continue;
		Sector2D pass = Sector2D(ball_pos,1,tm_pos.dist(ball_pos)+3,(tm_pos - ball_pos).th() - 15,(tm_pos - ball_pos).th() + 15);
		if(!wm.existOpponentIn(pass,5,true)){
			targets.push_back(tm_pos);
		}
	}
	if(targets.size() == 0)
		return false;
	Vector2D best_target = targets[0];
    for(unsigned int i=1;i<targets.size();i++){
		if(targets[i].x > best_target.x)
			best_target = targets[i];
	}
    
    
    if(Body_SmartKick(best_player->pos(),2.7,2.7,2).execute(agent))
    {
        return true;
    }
    return false;
}

int Bhv_BasicOffensiveKick::score_sector(rcsc::PlayerAgent* agent, rcsc::PlayerObject* tm)
{
    const WorldModel &wm = agent->world();
    Vector2D self_pos = wm.self().pos();
    Vector2D tm_pos = tm->pos();
    double dist = self_pos.dist(tm_pos);
    AngleDeg ang_self_tm = (tm_pos - self_pos).th();
    Sector2D pass = Sector2D(self_pos,1,dist,ang_self_tm-15,ang_self_tm+15);
    if(wm.existOpponentIn(pass,5,true))
    {
        return -100;
    }
    else
    {
        return 10;
    }
    
}

int Bhv_BasicOffensiveKick::score_safe(rcsc::PlayerAgent* agent, rcsc::PlayerObject* tm)
{
    const WorldModel & wm = agent->world();
    Vector2D ball_pos = wm.ball().pos();
    Vector2D next_pos;
    Vector2D tm_pos = tm->pos();
    AngleDeg ang_self_tm = (ball_pos-tm_pos).th();
    double dist_self_tm = ball_pos.dist(tm_pos);
    double dist_op;
    for(double i=1 ; i<=dist_self_tm ; i+=2)
    {
        next_pos = Vector2D::polar2vector(i,ang_self_tm)+ball_pos;
        dist_op = wm.getDistOpponentNearestTo(next_pos,5);
        if(dist_op<i/2)
        {
            return -100;
        }
    }
    return 10;
}

bool Bhv_BasicOffensiveKick::safe_pos(rcsc::PlayerAgent* agent, rcsc::Vector2D pos)
{
    const WorldModel & wm = agent->world();
    Vector2D ball_pos = wm.ball().pos();
    Vector2D next_pos;
    AngleDeg ang_self_pos = (ball_pos-pos).th();
    double dist_self_pos = ball_pos.dist(pos);
    double dist_op;
    for(double i=1 ; i<=dist_self_pos ; i+=2)
    {
        next_pos = Vector2D::polar2vector(i,ang_self_pos)+ball_pos;
        dist_op = wm.getDistOpponentNearestTo(next_pos,2);
        if(dist_op<i/2)
        {
            return false;
        }
    }
    return true;
}


bool Bhv_BasicOffensiveKick::shoot_test(rcsc::PlayerAgent* agent)
{
    const WorldModel & wm = agent -> world();
    Vector2D pos_goal;
    for(int i=-6;i<=6;i++)
    {
        pos_goal.assign(52,i);
        if(safe_pos(agent,pos_goal)==true)
        {
            if(Body_SmartKick(pos_goal,2.8,2.8,2).execute(agent))
            {
                return true;
            }
        }
    }
    if(dribble(agent)){
    	return true;
    }
    return false;
}


bool Bhv_BasicOffensiveKick::safe(rcsc::PlayerAgent* agent, rcsc::Vector2D mate_pos, double mate_dist)
{
    const WorldModel & wm=agent -> world();
    Vector2D ball_pos=wm.ball().pos();
    AngleDeg mate_angle = (mate_pos-ball_pos).th();
    Vector2D velo = Vector2D :: polar2vector(mate_dist,mate_angle);
    bool result = true;
    double pos_num=1;
    Vector2D next_pos = inertia_n_step_point(ball_pos,velo,pos_num,ServerParam::i().ballDecay());
    while (mate_pos.dist(next_pos) > 1 && pos_num<30)
    {
        next_pos=inertia_n_step_point(ball_pos,velo,pos_num,ServerParam::i().ballDecay());
        double dist_near_opp=wm.getDistOpponentNearestTo(next_pos,5);
        if (dist_near_opp<1)
        {
            result = false;
        }
        agent -> debugClient().addCircle(next_pos,1);
        ++pos_num;
    }
    return result;
}
