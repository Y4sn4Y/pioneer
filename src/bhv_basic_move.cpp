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

#include "bhv_basic_move.h"

#include "bhv_basic_tackle.h"

#include <rcsc/action/basic_actions.h>
#include <rcsc/action/body_go_to_point.h>
#include <rcsc/action/body_intercept.h>

#include <rcsc/player/player_agent.h>
#include <rcsc/player/debug_client.h>
#include <rcsc/player/intercept_table.h>

#include <rcsc/common/logger.h>
#include <rcsc/common/server_param.h>

#include <vector>
#include <cstdio>

using namespace rcsc;

/*-------------------------------------------------------------------*/
/*!

 */
bool
Bhv_BasicMove::execute( PlayerAgent * agent )
{
    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicMove" );
   /*const WorldModel & wm = agent->world();
   Vector2D v=new_position(wm,wm.self().unum());
   Body_GoToPoint(v,1,100).execute(agent);
   return true;*/

    //-----------------------------------------------
    // tackle
    if ( Bhv_BasicTackle( 0.8, 80.0 ).execute( agent ) )
    {
        return true;
    }

    const WorldModel & wm = agent->world();
    /*--------------------------------------------------------*/
    // chase ball
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();

    if ( ! wm.existKickableTeammate()
         && ( self_min <= 3
              || ( self_min <= mate_min
                   && self_min < opp_min + 3 )
              )
         )
    {
        dlog.addText( Logger::TEAM,
                      __FILE__": intercept" );
        Body_Intercept().execute( agent );

        return true;
    }
   if (wm.self().unum()>5)
   {
       if(press(agent))
       {
           return true;
       }
   }
    const Vector2D target_point = getPosition( wm, wm.self().unum() );
    const double dash_power = get_normal_dash_power( wm );

    double dist_thr = wm.ball().distFromSelf() * 0.1;
    if ( dist_thr < 1.0 ) dist_thr = 1.0;

    dlog.addText( Logger::TEAM,
                  __FILE__": Bhv_BasicMove target=(%.1f %.1f) dist_thr=%.2f",
                  target_point.x, target_point.y,
                  dist_thr );

    agent->debugClient().addMessage( "BasicMove%.0f", dash_power );
    agent->debugClient().setTarget( target_point );
    agent->debugClient().addCircle( target_point, dist_thr );

    if ( ! Body_GoToPoint( target_point, dist_thr, dash_power
                           ).execute( agent ) )
    {
        Body_TurnToBall().execute( agent );
    }

    return true;
}

rcsc::Vector2D Bhv_BasicMove::getPosition(const rcsc::WorldModel & wm, int self_unum){
    int ball_step = 0;
    if ( wm.gameMode().type() == GameMode::PlayOn
         || wm.gameMode().type() == GameMode::GoalKick_ )
    {
        ball_step = std::min( 1000, wm.interceptTable()->teammateReachCycle() );
        ball_step = std::min( ball_step, wm.interceptTable()->opponentReachCycle() );
        ball_step = std::min( ball_step, wm.interceptTable()->selfReachCycle() );
    }

    Vector2D ball_pos = wm.ball().inertiaPoint( ball_step );

    dlog.addText( Logger::TEAM,
                  __FILE__": HOME POSITION: ball pos=(%.1f %.1f) step=%d",
                  ball_pos.x, ball_pos.y,
                  ball_step );

    std::vector<Vector2D> positions(12);
    double min_x_rectengle[12]={0,-52,-52,-52,-52,-52,-30,-30,-30,0,0,0};
    double max_x_rectengle[12]={0,-48,-10,-10,-10,-10,15,15,15,50,50,50};
    double min_y_rectengle[12]={0,-2,-20,-10,-30,10,-20,-30, 0,-20,-30, 0};
    double max_y_rectengle[12]={0,+2, 10, 20,-10,30, 20, 0,30, 20, 0, 30};

    for(int i=1; i<=11; i++){
          double xx_rectengle = max_x_rectengle[i] - min_x_rectengle[i];
          double yy_rectengle = max_y_rectengle[i] - min_y_rectengle[i];
          double x_ball = ball_pos.x + 52.5;
          x_ball /= 105.5;
          double y_ball = ball_pos.y + 34;
          y_ball /= 68.0;
          double x_pos = xx_rectengle * x_ball + min_x_rectengle[i];
          double y_pos = yy_rectengle * y_ball + min_y_rectengle[i];
          positions[i] = Vector2D(x_pos,y_pos);
    }

    if ( ServerParam::i().useOffside() )
    {
        double max_x = wm.offsideLineX();
        if ( ServerParam::i().kickoffOffside()
             && ( wm.gameMode().type() == GameMode::BeforeKickOff
                  || wm.gameMode().type() == GameMode::AfterGoal_ ) )
        {
            max_x = 0.0;
        }
        else
        {
            int mate_step = wm.interceptTable()->teammateReachCycle();
            if ( mate_step < 50 )
            {
                Vector2D trap_pos = wm.ball().inertiaPoint( mate_step );
                if ( trap_pos.x > max_x ) max_x = trap_pos.x;
            }

            max_x -= 1.0;
        }

        for ( int unum = 1; unum <= 11; ++unum )
        {
            if ( positions[unum].x > max_x )
            {
                dlog.addText( Logger::TEAM,
                              "____ %d offside. home_pos_x %.2f -> %.2f",
                              unum,
                              positions[unum].x, max_x );
                positions[unum].x = max_x;
            }
        }
    }
    return positions.at(self_unum);
}

double Bhv_BasicMove::get_normal_dash_power( const WorldModel & wm )
{
    static bool s_recover_mode = false;

    if ( wm.self().staminaModel().capacityIsEmpty() )
    {
        return std::min( ServerParam::i().maxDashPower(),
                         wm.self().stamina() + wm.self().playerType().extraStamina() );
    }

    // check recover
    if ( wm.self().staminaModel().capacityIsEmpty() )
    {
        s_recover_mode = false;
    }
    else if ( wm.self().stamina() < ServerParam::i().staminaMax() * 0.5 )
    {
        s_recover_mode = true;
    }
    else if ( wm.self().stamina() > ServerParam::i().staminaMax() * 0.7 )
    {
        s_recover_mode = false;
    }

    /*--------------------------------------------------------*/
    double dash_power = ServerParam::i().maxDashPower();
    const double my_inc
        = wm.self().playerType().staminaIncMax()
        * wm.self().recovery();

    if ( wm.ourDefenseLineX() > wm.self().pos().x
         && wm.ball().pos().x < wm.ourDefenseLineX() + 20.0 )
    {
    }
    else if ( s_recover_mode )
    {
    }
    else if ( wm.existKickableTeammate()
              && wm.ball().distFromSelf() < 20.0 )
    {
    }
    else if ( wm.self().pos().x > wm.offsideLineX() )
    {
    }
    else
    {
        dash_power = std::min( my_inc * 1.7,
                               ServerParam::i().maxDashPower() );
    }

    return dash_power;
}
bool Bhv_BasicMove::press(rcsc::PlayerAgent * agent)
{
    const WorldModel & wm = agent->world();
    const PlayerObject *op = wm.getOpponentNearestToSelf(5,true);
    double dist_op = wm.getDistOpponentNearestToSelf(5,true);
    double dist_self_ball = wm.ball().distFromSelf();
    Vector2D opp_ball = wm.ball().pos();
    Vector2D op_pos = op->pos();
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    double dist_tm = wm.getDistTeammateNearestTo(op_pos,5);
    if( dist_op>15 || self_min<opp_min || mate_min<opp_min || dist_tm<3 || dist_self_ball>30)
    {
        return false;
    }
    else if(dist_self_ball<=dist_op)
    {
        Body_Intercept().execute(agent);
        return true;
    }
    else if(!Body_GoToPoint(op_pos,1,get_normal_dash_power(wm)).execute(agent))
    {
        Body_TurnToBall().execute(agent);
    }
    return true;
}
/*bool Bhv_BasicMove::press(rcsc::PlayerAgent * agent)
{
    const WorldModel & wm = agent->world();
    const PlayerObject *op = wm.getOpponentNearestToSelf(5,true);
    double dist_op = wm.getDistOpponentNearestToSelf(5,true);
    Vector2D op_pos = op->pos();
    const int self_min = wm.interceptTable()->selfReachCycle();
    const int mate_min = wm.interceptTable()->teammateReachCycle();
    const int opp_min = wm.interceptTable()->opponentReachCycle();
    double dist_tm = wm.getDistTeammateNearestTo(op_pos,5);
    if( dist_op>15 || self_min<opp_min || mate_min<opp_min || dist_tm<3)
    {
        return false;
    }
    else if(!Body_GoToPoint(op_pos,1,get_normal_dash_power(wm)).execute(agent))
    {
        Body_TurnToBall().execute(agent);
    }
    return true;
}*/

int ball_sqr(Vector2D ball_pos)
{
    double ball_x = ball_pos.x , ball_y = ball_pos.y;
    int number;
    int x=0, y=0;
    for (double yi = -34 ; yi < ball_y ; yi = yi + 8.75)
    {
    ++y;
    }
    for (double xi = -52.5 ; xi < ball_x ; xi = xi + 8.5)
    {
        ++x;
    }
    --x;
    number = x*8 + y;
    return number;
}
Vector2D sqr_center(int ball_location)
{
    int number = ball_location;
    double center_x , center_y;
    int x,y;
    Vector2D center;
    x = number / 8 + 1;
    y = number % 8;
    center_x = -52.5 + (x * 8.75) - 4.37;
    center_y = -34 + (y * 8.5) - 4.25;
    center.x = center_x;
    center.y = center_y;
    return center; 
}

int closest_tm_num (const rcsc::WorldModel& wm)
{
    Vector2D ball_pos = wm.ball().pos();
    const AbstractPlayerObject * tm = wm.ourPlayer(2);
    Vector2D tm_pos = tm->pos();
    int closest_tm = 2;
    double closest_tm_dist = tm_pos.dist(ball_pos);
    for (int u = 3;u < 12;++u)
    {
        const AbstractPlayerObject * tm = wm.ourPlayer(u);
        tm_pos = tm -> pos();
        if (tm_pos.dist(ball_pos) < closest_tm_dist)
        {
            closest_tm = u;
            closest_tm_dist = tm_pos.dist(ball_pos);
        }
    }
    return closest_tm;
}

Vector2D target_pos(int sqr_num,int unum,const rcsc::WorldModel& wm)
{
    const PlayerObject *op = wm.getOpponentNearestToSelf(5,true);
    const AbstractPlayerObject * me = wm.ourPlayer(unum);
    Vector2D my_pos = me->pos();
    Vector2D target = wm.ball().pos();
    Vector2D ball_pos = wm.ball().pos();
    Vector2D op_pos = op->pos();
    double dist_op = wm.getDistOpponentNearestToSelf(5);
    int closest_tm = closest_tm_num(wm);
    
    if (unum == closest_tm) 
    {
        target = ball_pos;
    }
    else if (unum == 2 || unum == 3)
    {
        if (sqr_num < 32)
        {
            target = ball_pos;
        }
        else {
            target = sqr_center(sqr_num);
        }
    }
    else 
    {
        if (dist_op < 30 && unum != closest_tm)
        {
            target = op_pos;
            if (my_pos.x < 0)
            {
                target.x = target.x - 2;
            }
            else{
                target.x = target.x + 2;
            }
        }
        else
        {
            target = sqr_center(sqr_num);
        }
    }
    return target;
}

rcsc::Vector2D Bhv_BasicMove::new_position(const rcsc::WorldModel& wm, int unum)
{
    Vector2D ball_pos = wm.ball().pos();
    int number = ball_sqr (ball_pos);
    int pos[97][7]=
    {
        {0,0,0,0,0,0}, //0
        {0,0,18,20,1,6},//1
        {0,0,11,20,2,14},//2
        {0,0,3,12,11,14},//3
        {0,0,4,5,12,13},//4
        {0,0,12,5,12,13},//5
        {0,0,21,6,11,13},//6
        {0,0,12,22,12,14},//7
        {0,0,12,8,36,8},//8
        {0,0,18,12,9,14},//9
        {0,0,11,20,10,29},//10
        {0,0,11,20,10,29},//11
        {0,0,4,5,10,29,27},//12
        {0,0,4,5,19,29,27},//13
        {0,0,12,14,19,29},//14
        {0,0,13,22,29,15},//15
        {0,0,14,22,29,16},//16
        {0,0,25,10,17,22,28}, //17
        {0,0,25,9,18,22,28}, //18
        {0,0,26,10,19,22,28}, //19
        {0,0,12,13,10,22,28}, //20
        {0,0,12,13,18,22,28}, //21
        {0,0,20,21,18,22,28}, //22
        {0,0,20,20,22,23,29}, //23
        {0,0,20,23,18,24,29}, //24
        {0,0,18,20,25,22,27}, //25
        {0,0,25,19,26,21,28}, //26
        {0,0,27,20,18,22,36}, //27
        {0,0,28,21,19,23,37}, //28
        {0,0,20,29,18,22,36}, //29
        {0,0,21,30,19,23,28}, //30
        {0,0,20,22,19,31,29}, //31
        {0,0,22,23,26,32,29}, //32
        {0,0,19,20,26,22,28}, //33
        {0,0,19,21,26,30,36}, //34
        {0,0,19,21,26,30,37}, //35
        {0,0,20,21,26,30,36}, //36
        {0,0,20,21,27,30,37}, //37
        {0,0,20,22,27,31,36}, //38
        {0,0,20,22,26,31,29}, //39
        {0,0,20,22,26,31,37}, //40
        {0,0,19,21,26,30,36}, //41
        {0,0,19,21,26,31,36}, //42
        {0,0,19,21,26,31,37}, //43
        {0,0,20,21,26,30,37}, //44
        {0,0,20,22,26,31,45}, //45
        {0,0,20,22,26,31,37}, //46
        {0,0,20,22,26,31,37}, //47
        {0,0,20,22,34,31,37}, //48
        {0,0,12,13,27,29,36}, //49
        {0,0,20,21,27,30,36}, //50
        {0,0,11,13,18,22,28}, //51
        {0,0,20,21,26,30,28}, //52
        {0,0,21,22,27,23,27}, //53
        {0,0,21,22,28,31,29}, //54
        {0,0,20,21,27,22,29}, //55
        {0,0,21,22,27,31,29}, //56
        {0,0,20,22,27,30,28}, //57
        {0,0,19,21,26,30,36}, //58
        {0,0,19,21,27,30,36}, //59
        {0,0,18,20,27,30,36}, //60
        {0,0,20,22,27,30,37}, //61
        {0,0,20,22,27,31,37}, //62
        {0,0,21,22,28,31,30}, //63
        {0,0,21,22,28,31,38}, //64
        {0,0,19,21,26,30,36}, //65
        {0,0,26,28,34,39,37}, //66
        {0,0,28,29,35,39,37}, //67
        {0,0,28,29,35,38,37}, //68
        {0,0,28,30,35,39,37}, //69
        {0,0,28,30,35,39,45}, //70
        {0,0,28,30,35,39,45}, //71
        {0,0,28,30,35,39,45}, //73
        {0,0,27,29,34,38,44}, //73
        {0,0,27,29,34,38,44}, //74
        {0,0,27,29,34,38,44}, //75
        {0,0,28,30,35,38,45}, //76
        {0,0,28,30,35,39,45}, //77
        {0,0,28,30,35,39,45}, //78
        {0,0,28,30,35,39,46}, //79
        {0,0,28,30,35,39,46}, //80
        {0,0,28,30,35,38,44}, //81
        {0,0,35,37,42,46,52}, //82
        {0,0,36,38,43,47,52}, //83
        {0,0,36,38,43,46,53}, //84
        {0,0,36,38,43,47,53}, //85
        {0,0,36,38,50,47,53}, //86
        {0,0,36,38,43,47,52}, //87
        {0,0,36,38,43,47,53}, //88
        {0,0,35,38,42,47,53}, //89
        {0,0,35,37,42,46,52}, //90
        {0,0,35,37,42,46,52}, //91
        {0,0,35,36,42,46,52}, //92
        {0,0,36,38,42,47,53}, //93
        {0,0,36,38,43,47,53}, //94
        {0,0,36,38,43,47,53}, //95
        {0,0,36,38,43,47,53}, //96
    };
    return target_pos(pos[number][unum],unum,wm);
}
