/*
Shooting Game

Controls
Movement: Directional keys
Shots: Z key

Image Material
HamCorossam
http://homepage2.nifty.com/hamcorossam/

Sound Effects
The Matchmakers
http://osabisi.sakura.ne.jp/m2/

Dot Picture Tools
EDGE
http://takabosoft.com/edge
*/

#define _USE_MATH_DEFINES
#include "DxLib.h"
#include <math.h>

//WINDOW SIZE
#define MIN_X 32
#define MIN_Y 16
#define MAX_X 416
#define MAX_Y 464

//Screen center coordinates
#define CENTER_X ((MIN_X + MAX_X)/2)
#define CENTER_Y ((MIN_Y + MAX_Y)/2)

//Angle Calculation  radians->degrees
#define OMEGA( t ) (t * M_PI / 180)

//Maximum number of enemies
#define MAX_ENEMY 50

//Maximum number of bullets in screen
#define MAX_BULLET 2000

//Maximum number of the shooter
#define MAX_PLAYER_SHOT 100

//Maximum number of the lives up abilily
#define MAX_LIVES_INCREASE 1

//Maximum number of power that will exists on the screen at one time
#define MAX_POWER_UP_ITEMS 5

//Maximum time that the powerful bullet ability can stay active. set to 300 frames which should be 5s in this 60fps game
#define MAX_ACTIVE_TIME_POWERFUL_BULLET 300

//Bullet type
#define NORMAL 1//Circle
#define LASER 2//Laser

//Enemy Action Patterns
#define STOP 0
#define STRAIGHT 1
#define CIRCLE 2
#define SIDETOSIDE 3

// Bullet Pattern
#define BULLET_LINEAR 1
#define BULLET_GOLDEN_PI 2

//Maximum number of effects
#define MAX_EFFECT MAX_BULLET

//Color
#define WHITE GetColor(255,255,255)//White
#define RED GetColor(255,0,0) // Red
#define GREEN GetColor(0,255,0) //green

#define LEVEL_UP_SCORE 1500 //Level up for each of this score.(100 points for 1 enemy)

#define BORN1 20 // Frequency of Enemy Appearance
#define BASESCORETONEVERLETBOSSSPAWN 1000 //set anything above 10 should be okay, this is to not let the if statement in MakeEnemy() spawns boss right immidietly after the game start because of the modulo "%" condition is met
#define BOSSLEVEL1 3000 // score at which level 1 boss spawn
#define BOSSLEVEL2 2*BOSSLEVEL1 // score at which level 2 boss spawn
#define BOSSLEVEL3 3*BOSSLEVEL1 // score at which level 3 boss spawn
#define BOSSFREQ 3000 // the frequency at which the boss will spawn, the smaller the more freq, the bigger the less freq

#define SCORE_BOSS 2000
#define SCORE_ENEMY 100


int t;//time

int enemy_img;//Enemy Image
int boss_img;// Boss Image
int bullet_img1,bullet_img2;//Images of Enemy Bullets
int shot_img;//Images of Shooter Bullets
int board_img;//Image of the frame
int back_img;//Background Image
int life_img; // life increase aibility's picture
int effect_img[17];//Effects Images
int powerful_bullet_img; // powerful bullet sign
int score;

int shot_snd;//Sound of the shooter firing
int bullet_snd;//sound of enemy bullets being fired
int bom_snd1;//Explosion 1
int bom_snd2;//Explosion 2
int up_snd;//power-up sound
int life_increase;//when player interact with the falling life.png object


int targetFPS = 60; //FPS
int timePerFrame = 1000 / 60; // Time cost per frame
int startLoopTime = 0; // Time when the loop start
int endLoopTime = 0; // Time when the loop ends
int timePassed = 0; // start - endtime

bool isPlayerObtainPowerfulBullet = false; //keep track if the player has the powerfull bullet ability in hand
bool isPowerfulBulletActive = false; // check if the powerful bullet ability is up
int powerfulBulletActiveTimer = 0; // Keep track of how long the powerful bullet has been active for

//Shooter
struct Player
{
	int x;//coordinate         
	int y;
	int img;//image
	int hp;//remaining lives
	double range;//collision detection radius
	bool isDamage;//true if under fire
};
struct Player player;

//Shooter Bullet
struct PlayerShot
{
	double x;//coordinate  
	double y;
	double angle;
	double speed;
	double range;//collision detection radius
	int img;//image
	int power;//power of the shot
	bool isExist;//true if present, false if not present
};
struct PlayerShot shot[MAX_PLAYER_SHOT];

//Enemy
struct Enemy
{
	double x;//X coordinate 
	double y;//Y coordinate
	double x0;//initial X coordinate  
	double y0;//initial Y coordinate
	double angle;
	double speed;
	double range;//collision detection radius
	int img;//image
	int hp;//Enemy strength
	int action;//Type of enemy action
	bool isExist;//true if present, false if not present
	bool isBoss;//if Boss, true
	int t;//time

};
struct Enemy enemy[MAX_ENEMY];
bool isBossExist = false;

//Enemy Bullet
struct Bullet
{

	double x;//coordinate  
	double y;
	double angle;
	double speed;
	double range;//collision detection radius
	bool isExist;//true if present, false if not present
	int img;//image
	int pattern; // Stores the bullet pattern (linear or goldenPi)
	

	//for special type of pattern
	double originX;
	double originY;
	int creationTime;
};
struct Bullet bullet[MAX_BULLET];//Bullets

//Effect
struct Effect
{
	int x;//ç¿ïW
	int y;
	int img[20];//âÊëú
	int max_img;//âÊëúÇÃç≈ëÂêî
	int t;//åoâﬂéûä‘
	bool isExist;//ë∂ç›ÇµÇΩÇÁtrueÅAÇ¢Ç»Ç©Ç¡ÇΩÇÁfalse
};
struct Effect effect[MAX_EFFECT];

// Increase Player's lives
struct livesIncrease
{
	double x;
	double y;
	double fallSpeed;
	double range;
	int img;
	bool isExist;
};
struct livesIncrease lifeUp[MAX_LIVES_INCREASE];

// items that would give player special abilities
struct powerUpItem
{
	double x;
	double y;
	double fallSpeed;
	double range;
	int img;
	bool isExist;
	int type; // this will determine what type of ability that the item will give to player
};
struct powerUpItem powerUp[MAX_POWER_UP_ITEMS];

void initEnemy(int i)
{
	enemy[i].x = 0;
	enemy[i].y = 0;
	enemy[i].x0 = 0;
	enemy[i].y0 = 0;
	enemy[i].range = 0;
	enemy[i].hp = 0;
	enemy[i].isExist = false;
	enemy[i].isBoss = false;
	enemy[i].t = 0;
}

//init bullet info
void InitBullet(int i)
{
	bullet[i].x = 0;
	bullet[i].y = 0;
	bullet[i].angle = 0;
	bullet[i].speed = 0;
	bullet[i].range = 0;
	bullet[i].isExist = false;
	bullet[i].originX = 0;
	bullet[i].originY = 0;
	bullet[i].creationTime = 0;
}

//init power up items
void InitPowerUp(int i)
{
	powerUp[i].x = 0;
	powerUp[i].y = 0;
	powerUp[i].fallSpeed = 0.0;
	powerUp[i].range = 0.0;
	powerUp[i].img = 0;
	powerUp[i].isExist = false;
	powerUp[i].type = 0;
}

//Initialization
void Init()
{
	int i;
	t = 0;//Time initialization
	score = 0;
	//shooter initial position
	player.x = CENTER_X;
	player.y = CENTER_Y * 1.5 ;

	player.hp = 5;//Start with 5 remaining units

	player.range = 3;//Boundary radius

	player.isDamage = false;

	//Enemy initialization
	for(i = 0; i < MAX_ENEMY; i++)
	{
		initEnemy(i);
	}

	//Bullet initialization
	//
	//Effects initialization
	for(i = 0; i < MAX_BULLET; i++)
	{
		InitBullet(i);

		effect[i].isExist = false;
		effect[i].x = 0;
		effect[i].y = 0;
		effect[i].max_img = 0;
	}

	for (i = 0; i < MAX_LIVES_INCREASE; i++)
	{
		lifeUp[i].x = 0;
		lifeUp[i].y = 0;
		lifeUp[i].fallSpeed = 0;
		lifeUp[i].range = 0;
		lifeUp[i].isExist = false;
	}

}

// Function to read in image and sound files
void LoadData()
{
	//image
	player.img = LoadGraph("fighter.png");
	enemy_img = LoadGraph("smallenemy.png");
	boss_img = LoadGraph("boss.png");
	bullet_img1 = LoadGraph("bullet1.png");
	bullet_img2 = LoadGraph("bullet2.png");
	shot_img = LoadGraph("shot.png");
	board_img = LoadGraph("board.png");
	back_img = LoadGraph("back.png");
	LoadDivGraph("effect.png",17,8,3,64,64,effect_img);
	life_img = LoadGraph("life.png");
	powerful_bullet_img = LoadGraph("powerful_bullet_item_sign.png");

	//sound
	shot_snd = LoadSoundMem("push07.wav");
	bullet_snd = LoadSoundMem("close09.wav");
	bom_snd1 = LoadSoundMem("bom01.wav");
	bom_snd2 = LoadSoundMem("bom10.wav");
	up_snd = LoadSoundMem("power00.wav");
	life_increase = LoadSoundMem("increase_life.wav");

	// Volume setting
	ChangeVolumeSoundMem( 255*0.1, shot_snd ) ;
	ChangeVolumeSoundMem( 255*0.5, bullet_snd ) ;

}

//Effects Generation Functions
void MakeEffect(int x, int y, int max)
{
	int i,j; 

	//Examine the effect in use
	for( i = 0; i < MAX_EFFECT; i++)
	{
		if( !effect[i].isExist )
			break;
	}

	if ( i == MAX_EFFECT )//Not a single one available.
		return;

	effect[i].isExist = true;
	effect[i].x = x;
	effect[i].y = y;
	effect[i].t = 0;
	effect[i].max_img = max;

	for( j = 0; j < max; j++)
	{
		effect[i].img[j] = effect_img[j];
	}

}

//Generation of shooter shot
void MakeShot(double speed, double angle, int power, double range)
{
	int i;

	//Examine the shot being fired.
	for(i = 0; i < MAX_PLAYER_SHOT; i++)
	{
		if( !shot[i].isExist)
			break;
	}
	if( i == MAX_PLAYER_SHOT)
		return;

	shot[i].isExist = true;

	shot[i].x = player.x;
	shot[i].y = player.y;

	shot[i].speed = speed;
	shot[i].angle = angle;
	shot[i].power = power;
	shot[i].range = range;

	shot[i].img = shot_img;

	PlaySoundMem( shot_snd , DX_PLAYTYPE_BACK ) ;//sound of the shot

}

//Fires shooter shots in multiple directions
void MakeWayShot(double speed, int power, double range, int way, double wide_angle, double main_angle)
{
	int i; 

	double w_angle;
	
	if (way == 1)
	{
		MakeShot(speed, main_angle, power, range);
		return;
	}
	
	for( i = 0; i < way; i++) 
	{
		if( wide_angle == OMEGA(360))
			w_angle = main_angle + i * wide_angle / way;//Launch angle
		else
			w_angle = main_angle + i * wide_angle / ( way - 1 ) - wide_angle / 2;//Launch angle

		MakeShot(speed,w_angle,power,range);
	}
}

//powerful bullet ability activater
void CheckIfCanActivatePowerfulBulletAbility()
{
	if (isPlayerObtainPowerfulBullet && !isPowerfulBulletActive)
	{
		isPlayerObtainPowerfulBullet = false;
		isPowerfulBulletActive = true;
		powerfulBulletActiveTimer = t;
	}
}

//Movement of the shooter
void ActionPlayer()
{
	const int fire = 4;
	const int speed = 4;

	double s_speed = 8;
	double s_angle = OMEGA( -90 );
	double range = 16;

	int power = 1;

	int way = 5;


	if( player.isDamage )//doesn't move while being hit by a bullet
		return;

	//Move with the directional keys
	if( CheckHitKey(KEY_INPUT_LEFT) )
		player.x -= speed;
	if( CheckHitKey(KEY_INPUT_RIGHT) )
		player.x += speed;
	if( CheckHitKey(KEY_INPUT_UP) )
		player.y -= speed;
	if( CheckHitKey(KEY_INPUT_DOWN) )
		player.y += speed;
	
	//Restrictions on movement
	if( player.x < MIN_X )
		player.x = MIN_X;
	if( player.x > MAX_X )
		player.x = MAX_X;
	if( player.y < MIN_Y )
		player.y = MIN_Y;
	if( player.y > MAX_Y)
		player.y = MAX_Y;
	
	
	// if player press 'space' to use the Powerful Bullet Special Ability
	//but first check if the player has the ability but hasn't activated yet. If so we will activate it
	if (CheckHitKey(KEY_INPUT_SPACE))
		CheckIfCanActivatePowerfulBulletAbility();
	
	if (CheckHitKey(KEY_INPUT_SPACE) && t % fire == 0 && isPowerfulBulletActive)
	{
		MakeShot(s_speed, s_angle, power * 10, range);

		//if the activation time runs out, we take back the ability from the player
		int abilityLife = t - powerfulBulletActiveTimer;
		if (abilityLife > MAX_ACTIVE_TIME_POWERFUL_BULLET)
		{
			isPowerfulBulletActive = false;
			powerfulBulletActiveTimer = 0;
		}
		return;
	}
	
	// make player's character shoot when playing the 'z' Key
	if( CheckHitKey(KEY_INPUT_Z)  && t % fire == 0 )
	{
		//+1 direction for each certain number of enemies defeated (up to 6 directions)
		way = score > LEVEL_UP_SCORE * 5 ? way : score / LEVEL_UP_SCORE + 1;
		MakeWayShot(s_speed,power,range,way,OMEGA( (way - 1) * 20 ),s_angle);
	}
}

//Movement of the shooter bullets
void MoveShot()
{
	int i;

	double x,y,angle,speed;

	// Examine the shot being fired
	for(i = 0; i < MAX_PLAYER_SHOT; i++)
	{
		if( !shot[i].isExist )
			continue;

		x = shot[i].x;
		y = shot[i].y;

		speed = shot[i].speed;
		angle = shot[i].angle;

		x += speed * cos( angle );
		y += speed * sin( angle );

		// If the bullet goes off the screen, the bullet is eliminated.
		if( x < MIN_X || x > MAX_X || y < MIN_Y || y > MAX_Y)
			shot[i].isExist = false;

		shot[i].x = x;
		shot[i].y = y;

	}
}

//create new life from an (x,y) coordinate and drops it slowly from that coordinate
void MakeLifeItem(double x, double y)
{
	int i = 0;
	int ObjectFallSpeed = 1;

	for (i = 0; i < MAX_LIVES_INCREASE; i++)
	{
		if (!lifeUp[i].isExist)
			break;
	}

	if (i == MAX_LIVES_INCREASE)
		return;

	lifeUp[i].isExist = true;

	lifeUp[i].x = x;
	lifeUp[i].y = y;
	lifeUp[i].fallSpeed = ObjectFallSpeed;
	lifeUp[i].range = 15;
	lifeUp[i].img = life_img;
}

// create a new Special Ability at (x,y) coordinate and drops it slowly from that coordinate
void MakePowerUpItem(double x, double y) //type: 1 (powerful bullet), type: 2 ...
{
	int i = 0;
	int ObjectFallSpeed = 1;

	for (i = 0; i < MAX_POWER_UP_ITEMS; i++)
	{
		if (!powerUp[i].isExist)
			break;
	}

	if (i == MAX_POWER_UP_ITEMS)
		return;

	powerUp[i].isExist = true;

	powerUp[i].x = x;
	powerUp[i].y = y;
	powerUp[i].fallSpeed = ObjectFallSpeed;
	powerUp[i].range = 15;
	powerUp[i].type = 1; //change this later on if you want more special types of abilities
	if (powerUp[i].type == 1)
		powerUp[i].img = powerful_bullet_img;
}

////Hit detection process for PLAYER shot
void JudgePlayerShotIfHitEnemy()
{
	int i,j;
	double x,y;


	// Examine the enemy present
	for(i = 0; i < MAX_ENEMY; i++)
	{

		if( !enemy[i].isExist )
			continue;

		// Examine all shots being fired
		for(j = 0; j < MAX_PLAYER_SHOT; j++)
		{
			if( !shot[j].isExist )
				continue;

			x = shot[j].x - enemy[i].x;
			y = shot[j].y - enemy[i].y;

			// If a shot hits the enemy
			if( hypot (x,y) < enemy[i].range + shot[j].range )
			{
				shot[j].isExist = false;

				enemy[i].hp -= shot[j].power;//Reduce enemy strength

				//Eliminate enemies when they run out of energy.
				if(enemy[i].hp < 0)
				{
					MakeEffect( enemy[i].x, enemy[i].y, 17);//Explosion Effects

					// 1 in twenty chances of special ability spawning after killing an enemy
					if (GetRand(3) == 0)
						MakePowerUpItem(enemy[i].x, enemy[i].y);

					PlaySoundMem( bom_snd1 , DX_PLAYTYPE_BACK ) ;//sound of an explosion
					if (enemy[i].isBoss)
					{
						//spwan player life increase ability
						MakeLifeItem(enemy[i].x, enemy[i].y);

						score += SCORE_BOSS;
						isBossExist = false;
					}
					else
						score += SCORE_ENEMY;
					initEnemy(i);
					if( score % LEVEL_UP_SCORE == 0 && score > 0 && score < LEVEL_UP_SCORE * 6)
						PlaySoundMem( up_snd , DX_PLAYTYPE_BACK ) ;//Power up sound
				}
			}

		}
	}
}

//Display for shooter
void DrawPlayer()
{
	int i;

	//Display for shooter
	DrawRotaGraphF( (float)player.x, (float)player.y, 1.0, 0, player.img, TRUE ) ;

	//Display for shooter shots
	for(i = 0; i < MAX_PLAYER_SHOT; i++)
	{
		if( shot[i].isExist )
			DrawRotaGraphF( (float)shot[i].x, (float)shot[i].y, 1.0, shot[i].angle, shot[i].img, TRUE ) ;
	}
}

//Shooter Aiming Angle
double TargetAnglePlayer(double x, double y)
{	
	return atan2( player.y - y, player.x - x);	
}

//Generating a barrage of bullets
//Direction bullet (x,y: launch point, speed: velocity, angle: angle)
void MakeBullet(double x, double y, double speed, double angle, double range, int img)
{
	int i; 

	for( i = 0; i < MAX_BULLET; i++)
	{
		//If bullet[i] is not used, go to parameter setting
		if( !bullet[i].isExist )
			break;
	}
	if ( i == MAX_BULLET )//Not a single one available.
		return;

	bullet[i].isExist = true;

	//Coordinates of launch point
	bullet[i].x = x;
	bullet[i].y = y;

	bullet[i].angle = angle;//Firing angle
	bullet[i].speed = speed;//Speed

	bullet[i].range = range;

	bullet[i].img = img;//Image

	PlaySoundMem( bullet_snd , DX_PLAYTYPE_BACK ) ; //sound of the shot

}

//way bullet(way:which direction to hit, angle:angle of fan shape, main_angle:which direction the fan shape faces)
void MakeWayBullet(double x, double y, double speed, int way, double wide_angle ,double main_angle, double range,int img)
{
	int i; 

	double w_angle;

	for( i = 0; i < way; i++)
	{
		if( wide_angle == OMEGA(360))
			w_angle = main_angle + i * wide_angle / way;//Firing angle
		else
			w_angle = main_angle + i * wide_angle / ( way - 1 ) - wide_angle / 2;//Firing angle

		MakeBullet(x,y,speed,w_angle,range,img);
	}

}

// make boss's golden pi pattern's bullets
void MakeGoldenPiBullet(double x, double y, double range, double W, int img)
{
	int i;

	for (i = 0; i < MAX_BULLET; i++)
	{
		if (!bullet[i].isExist)
		{
			break;
		}
	}
	if (i == MAX_BULLET) // max TOTAL NUMBER of bullet is reached, won't spawn the new ones
		return;

	bullet[i].angle = W; //omega
	bullet[i].speed = 1.5;
	bullet[i].range = range;
	bullet[i].isExist = true;
	bullet[i].img = img;
	bullet[i].pattern = BULLET_GOLDEN_PI;
	
	
	bullet[i].originX = x;
	bullet[i].originY = y;
	bullet[i].creationTime = t;
}


//check if a bullet is outside the window, if so delete
void CheckIfBulletIsStillInsideTheWindowArea(double x, double y, int i)
{
	if (x < MIN_X || x > MAX_X || y < MIN_Y || y > MAX_Y)
	{
		//reset everything back
		InitBullet(i);
	}

}

//Movement of Bullets
void MoveBullet()
{
	double x, y;
	double angle;
	int i;

	//Examine all bullets being fired.
	for(i = 0; i < MAX_BULLET; i++)
	{
		if( !bullet[i].isExist )
			continue;

		if (bullet[i].pattern == BULLET_GOLDEN_PI)
		{
			// we are sure that this bullet[i] is the one for golden pi

			//preping function
			//r(t):
			int Rmax = 90 ;
			int bulletAge = t - bullet[i].creationTime;
			double Rt = Rmax / (1 + (10 * pow(M_E, -0.07 * bulletAge)));

			//x(t) and y(t)
			bullet[i].x = Rt * cos(bullet[i].angle * OMEGA(bulletAge)) + bullet[i].originX;
			bullet[i].y = Rt * sin(bullet[i].angle * OMEGA(bulletAge)) + bullet[i].originY;

			CheckIfBulletIsStillInsideTheWindowArea(bullet[i].x, bullet[i].y, i);

			// end the code for the golden pi bullet here, do not want it to be executed again down below,
			//which are for linear bullet patterns only
			continue;
		}

		x = bullet[i].x;
		y = bullet[i].y;

		angle = bullet[i].angle;

		//Advance by a speed in the angle angle direction
		x += bullet[i].speed * cos( angle );
		y += bullet[i].speed * sin( angle );

		CheckIfBulletIsStillInsideTheWindowArea(x, y, i);

		bullet[i].x = x;
		bullet[i].y = y;
	}

}

void JudgeBullet()
{
	int i;
	double x,y;

	//Examine all bullets being fired.
	for(i = 0; i < MAX_BULLET; i++)
	{
		if( !bullet[i].isExist )
			continue;

		x = bullet[i].x - player.x;
		y = bullet[i].y - player.y;

		//If the player got hit
		if( hypot (x,y) < player.range + bullet[i].range && !player.isDamage)
		{
			MakeEffect(player.x, player.y, 17);
			InitBullet(i);
			player.isDamage = true;

			player.hp--;

			PlaySoundMem( bom_snd2, DX_PLAYTYPE_BACK ) ;//sound of an explosion
		}
	}
}

//Eliminate all enemy bullets
void EraseBullet()
{
	int i;

	for(i = 0; i < MAX_BULLET; i++)
		InitBullet(i);
}

//Appearance of the Enemy
void MakeEnemy()
{
	int i;
	//small enemy Go straight to random direction on the bottom side
	if( t % BORN1 == 0 && !isBossExist )
	{
		for(i = 0;i < MAX_ENEMY; i++)
		{
			if( !enemy[i].isExist )
				break;
		}

		if( i == MAX_ENEMY )
			return ;

		enemy[i].isExist = true;

		//Appears at a random position at the top
		enemy[i].x0 = GetRand(MAX_X - MIN_X) + MIN_X;
		enemy[i].y0 = MIN_Y;
		enemy[i].x = enemy[i].x0;
		enemy[i].y = enemy[i].y0; 
		enemy[i].hp = 1;

		enemy[i].angle = OMEGA( GetRand(180) );//Angles from 0Åã to 180Åã.
		enemy[i].speed = GetRand(6) + 2;//Speeds from 2 to 8

		enemy[i].range = 15;//The size of the hit

		enemy[i].action = STRAIGHT;//Go straight to a fixed angle

		enemy[i].img = enemy_img;
	}

	// Boss spawns every BOSSFREQ score earned
	if( score % BOSSFREQ == 0 && t > 0 && !isBossExist && score > BASESCORETONEVERLETBOSSSPAWN) //the last condition is to not let the boss spawn right after the game just started
	{
		for(i = 0;i < MAX_ENEMY; i++)
		{
			if( !enemy[i].isExist )
				break;
		}

		if( i == MAX_ENEMY )
			return ;
		
		// we are sure that this enemy[i] is going to be the boss---

		// setting hp accoring to each level
		if (score == BOSSLEVEL1)
			enemy[i].hp = 200;
		else if (score == BOSSLEVEL2)
			enemy[i].hp = 400;
		else if (score == BOSSLEVEL3)
			enemy[i].hp = 600;
		else						//highest level boss?
			enemy[i].hp = 800;

		//boss innit
		isBossExist = true;
		enemy[i].isExist = true;
		enemy[i].isBoss = true;

		enemy[i].x0 = CENTER_X;
		enemy[i].y0 = MAX_Y / 4;
		enemy[i].x = enemy[i].x0;
		enemy[i].y = enemy[i].y0;

		

		enemy[i].angle = OMEGA( 90 );//pointing down
		enemy[i].speed = 0;

		enemy[i].range = 120;

		enemy[i].action = SIDETOSIDE;//CIRCLE;//coordinate fixation

		enemy[i].img = boss_img;
	}
	

}


// All enemies movement controller
void EnemyMovementController(int i)
{
	switch (enemy[i].action)
	{
	case STOP://DO nothing
		break;

	case STRAIGHT://Go straight in a fixed direction
	{
		enemy[i].x += enemy[i].speed * cos(enemy[i].angle);
		enemy[i].y += enemy[i].speed * sin(enemy[i].angle);
		break;
	}

	case CIRCLE://Go straight in a fixed direction
	{
		const int r = 100;
		enemy[i].x = r * cos(OMEGA(enemy[i].t)) + enemy[i].x0;
		enemy[i].y = r * sin(OMEGA(enemy[i].t)) + enemy[i].y0;
		break;
	}

	case SIDETOSIDE:
	{
		const int moveAmplitute = 10; // amplitute of the side to side motion
		enemy[i].x = moveAmplitute * cos(OMEGA(enemy[i].t)) + enemy[i].x0;
		enemy[i].y = enemy[i].y;
		break;
	}
	}
}


//control boss's fire and its patter according to each level of boss
void BossFiringActionController(double enemyX, double enemyY) //boss level dictates its firing pattern, and the other arguement are passed straight from "ActionEnemy()"
{
	double x, y;
	int i;
	int bossLevel;
	
	// boss coordinate
	x = enemyX;
	y = enemyY;

	// Calculating boss level using "score"
	bossLevel = (score) / 3000;

	switch (bossLevel)
	{
		case 1:
			MakeWayBullet(x + 110, y, 1.0, 2, OMEGA(50), OMEGA(90) + OMEGA(30) * sin(OMEGA(t)), 3, bullet_img2);
			MakeWayBullet(x - 110, y, 1.0, 2, OMEGA(50), OMEGA(90) + OMEGA(30) * sin(OMEGA(t)), 3, bullet_img2);
			MakeGoldenPiBullet(x, y, 3, 0.5, bullet_img1);
			break;

		case 2:
			MakeWayBullet(x + 110, y, 1, 2, OMEGA(50), OMEGA(90) + OMEGA(30) * sin(OMEGA(t)), 3, bullet_img2);
			MakeWayBullet(x - 110, y, 1, 2, OMEGA(50), OMEGA(90) + OMEGA(30) * sin(OMEGA(t)), 3, bullet_img2);
			MakeWayBullet(x, y, 1.0, 2, OMEGA(5), TargetAnglePlayer(x, y), 8, bullet_img1);
			break;

		case 3:
			MakeWayBullet(x + 110, y, 1, 2, OMEGA(50), OMEGA(90) + OMEGA(30) * sin(OMEGA(t)), 3, bullet_img2);
			MakeWayBullet(x - 110, y, 1, 2, OMEGA(50), OMEGA(90) + OMEGA(30) * sin(OMEGA(t)), 3, bullet_img2);
			MakeWayBullet(x, y, 1.0, 2, OMEGA(5), TargetAnglePlayer(x, y), 8, bullet_img1);
			break;

		default:
			MakeWayBullet(x + 110, y, 1, 2, OMEGA(50), OMEGA(90) + OMEGA(30) * sin(OMEGA(t)), 3, bullet_img2);
			MakeWayBullet(x - 110, y, 1, 2, OMEGA(50), OMEGA(90) + OMEGA(30) * sin(OMEGA(t)), 3, bullet_img2);
			MakeWayBullet(x, y, 1.0, 2, OMEGA(5), TargetAnglePlayer(x, y), 8, bullet_img1);
			break;
	}
}

// Enemy Actions
void ActionEnemy()
{
	int i;

	int fire;

	double speed;
	//	double angle;

	int way;
	//	double main_angle;

	double range;

	int x,y;

	for(i = 0; i < MAX_ENEMY; i++){

		if( !enemy[i].isExist )//If there is an enemy, move on to the next.
			continue;

		EnemyMovementController(i); //control all enemies movement
		
		x = enemy[i].x;
		y = enemy[i].y;

		//If the enemy goes off the screen, the enemy is eliminated.
		if (x < MIN_X || x > MAX_X || y < MIN_Y || y > MAX_Y)
		{
			enemy[i].isExist = false;
			enemy[i].t = 0;
			continue;
		}
		enemy[i].t++;

		//enemy fire contoller
		if (enemy[i].isBoss)
		{
			switch (enemy[i].action)
			{
			case STOP: case CIRCLE: case SIDETOSIDE:
				fire = 40;

				speed = 2.0;
				way = 5;
				range = 8;
				//Fires a bullet every fire loop
				if (t % fire == 0)
				{
					BossFiringActionController(x, y);
				}
				break;
			}
		}

		switch (enemy[i].action)
		{
		case STRAIGHT:
			const int LEVEL_MAX_SCORE = 100000;
			if (score > LEVEL_MAX_SCORE)
				fire = 20;
			else
				fire = (LEVEL_MAX_SCORE - score) * 80 / LEVEL_MAX_SCORE + 20;
			x = enemy[i].x;
			y = enemy[i].y;
			speed = 2.0;
			way = 3;
			range = 8;
			//Fires a bullet every fire loop
			if (t % fire == 0)
			{
				switch (GetRand(1))//Fire a barrage of either
				{
				case 0:
					MakeWayBullet(x, y, speed, way, OMEGA(30), TargetAnglePlayer(x, y), range, bullet_img1);
					break;

				case 1:
					MakeWayBullet(x, y, speed, 2 * way, OMEGA(360), OMEGA(GetRand(360)), range, bullet_img1);
					break;
				}
			}
			break;
		}
	}
}

// falling Objects' movement controller
void FallObjectsMovementHandler()
{
	double x, y;
	int i;

	for (i = 0; i < MAX_LIVES_INCREASE; i++)
	{
		if (!lifeUp[i].isExist)
			continue;

		x = lifeUp[i].x;
		y = lifeUp[i].y;

		x = x;
		y += lifeUp[i].fallSpeed;

		if (x < MIN_X || x > MAX_X || y < MIN_Y || y > MAX_Y)
			lifeUp[i].isExist = false;

		lifeUp[i].x = x;
		lifeUp[i].y = y;
	}

	for (i = 0; i < MAX_POWER_UP_ITEMS; i++)
	{
		if (!powerUp[i].isExist)
			continue;

		x = powerUp[i].x;
		y = powerUp[i].y;

		x = x;
		y += powerUp[i].fallSpeed;

		if (x < MIN_X || x > MAX_X || y < MIN_Y || y > MAX_Y)
			InitPowerUp(i);

		powerUp[i].x = x;
		powerUp[i].y = y;
	}
}

//judge if the player took the falling object (ability)
void JudgeFallItemCollision()
{
	double x, y;
	int i;

	//check for player interaction with lifeUp special item
	for (i = 0; i < MAX_LIVES_INCREASE; i++)
	{
		if (!lifeUp[i].isExist)
			continue;

		x = lifeUp[i].x - player.x;
		y = lifeUp[i].y - player.y;

		if (hypot(x, y) < player.range + lifeUp[i].range && player.hp > 0)
		{
			lifeUp[i].isExist = false;

			player.hp++;
			PlaySoundMem(life_increase, DX_PLAYTYPE_BACK); //play when player got lives_increase
		}
	}

	//check for player interaction with Special Power Up item
	for (i = 0; i < MAX_POWER_UP_ITEMS; i++)
	{
		if (!powerUp[i].isExist)
			continue;

		x = powerUp[i].x - player.x;
		y = powerUp[i].y - player.y;

		if (hypot(x, y) < player.range + powerUp[i].range && player.hp > 0)
		{
			InitPowerUp(i);
			isPlayerObtainPowerfulBullet = true;

			PlaySoundMem(life_increase, DX_PLAYTYPE_BACK); //play when player got the ability
		}

	}
}

void DrawFallObject()
{
	double x, y, angle;
	int img;

	int i;

	//check all lives object that exists
	for (i = 0; i < MAX_LIVES_INCREASE; i++)
	{
		if (!lifeUp[i].isExist)
			continue;

		x = lifeUp[i].x;
		y = lifeUp[i].y;

		angle = 0;

		img = lifeUp[i].img;

		// Distplay of the falling obj
		DrawRotaGraphF((float)x, (float)y, 1.0, angle, img, TRUE);
	}

	//check all falling items that exist
	for (i = 0; i < MAX_POWER_UP_ITEMS; i++)
	{
		if (!powerUp[i].isExist)
			continue;

		x = powerUp[i].x;
		y = powerUp[i].y;

		angle = 0;

		img = powerUp[i].img;

		// Distplay of the falling obj
		DrawRotaGraphF((float)x, (float)y, 1.0, angle, img, TRUE);
	}
}

//Display of Bullets
void DrawBullet()
{
	double x,y,angle;
	int img;

	int i;

	//Examine all bullets being fired
	for(i = 0; i < MAX_BULLET; i++)
	{
		if( !bullet[i].isExist )
			continue;

		x = bullet[i].x ;
		y = bullet[i].y ;

		angle = bullet[i].angle ;

		img = bullet[i].img ;

		//Bullet Display
		DrawRotaGraphF( (float)x, (float)y, 1.0, angle, img, TRUE ) ;

	}
}

//Enemy Display
void DrawEnemy()
{
	int i;

	for( i = 0; i < MAX_ENEMY; i++ )
	{
		if( !enemy[i].isExist )//If there is an enemy, move on to the next.
			continue;

		DrawRotaGraph( enemy[i].x, enemy[i].y, 1.0, enemy[i].angle, enemy[i].img, TRUE ) ;

	}
}

//Displaying Effects
void DrawEffect()
{
	int i;
	int x,y;

	int ft;

	//Examine all effects during firing
	for(i = 0; i < MAX_EFFECT; i++)
	{
		if( !effect[i].isExist )
			continue;

		x = effect[i].x ;
		y = effect[i].y ;
		ft = effect[i].t ;

		DrawRotaGraphF( (float)x, (float)y, 1.0, 0, effect[i].img[ft], TRUE ) ;

		effect[i].t++ ;//Advancing Effects Time

		//Turn it off after displaying everything.
		if(effect[i].t >= effect[i].max_img)
		{
			effect[i].isExist = false;

			if(effect[i].x == player.x && effect[i].y == player.y)//In case of shooter's bullet
			{
				EraseBullet();//Eliminate all enemy bullets
				player.isDamage = false;//No longer under fire.
			}
		}
	}
}

//Background Display
void DrawBack()
{
	int time;

	time = 8 * ( t % ( ( MAX_Y - MIN_Y ) / 8 ) );

	//background scrolling display
	DrawGraph( MIN_X, MIN_Y + time, back_img, false );

	if( time > 0 )
		DrawGraph( MIN_X, 2 * MIN_Y + time - MAX_Y, back_img, false );
}

//Display of scores, etc.
void DrawSystem()
{
	int fontHandle = CreateFontToHandle("Arial", 12, 6); 

	DrawGraph( 0, 0, board_img,true);
	DrawFormatString(540,102,WHITE,"%d",score); // display score

	//display the player's remaining lives
	if (player.hp > 2)
		DrawFormatString(548,152,WHITE,"%d",player.hp);
	else
		DrawFormatString(548, 152, RED, "%d", player.hp);
	
	// display powerful bullet's ability status
	if (isPlayerObtainPowerfulBullet)
		DrawFormatStringToHandle(450, 202, GREEN, fontHandle, "Powerful Bullet: READY");
	else
		DrawFormatStringToHandle(450, 202, RED, fontHandle, "Powerful Bullet: not ready");

	DeleteFontToHandle(fontHandle);
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow )
{
	// Change title
	SetMainWindowText( "Shooting" ) ;
	// Change to window mode
	ChangeWindowMode( TRUE ) ;
	// DX library initialization process
	if (DxLib_Init() == -1)
		return -1;	// Terminate immediately if an error occurs
	
	// Set the screen to the back of the screen to draw on
	SetDrawScreen(DX_SCREEN_BACK);

	while(1)
	{	
		LoadData();
		Init();

		DrawString(0,330,"Press Space Bar",WHITE);//Press spacebar to start
		ScreenFlip();
		while(!CheckHitKey(KEY_INPUT_SPACE))
		{
			if ( ProcessMessage() == -1 )
			{
				DxLib_End();// Exit process for DX library use
				return 0;
			}		//Main loop Esc key to exit
		}
		while( !CheckHitKey(KEY_INPUT_ESCAPE) &&  player.hp > 0)
		{

			startLoopTime = GetNowCount();

			if ( ProcessMessage() == -1 )
			{
				DxLib_End();// DX library termination process
				return 0;
			}

			ClearDrawScreen();

			ActionPlayer();

			MoveShot();

			MakeEnemy();

			ActionEnemy();

			MoveBullet();

			FallObjectsMovementHandler();

			JudgePlayerShotIfHitEnemy();
			 
			JudgeBullet();

			JudgeFallItemCollision();

			DrawBack();

			DrawEnemy();

			DrawPlayer();

			DrawEffect();

			DrawFallObject();

			DrawBullet();

			DrawSystem();

			t++;//advance time

			// Reflects the contents of the back screen on the front screen
			ScreenFlip();

			// maintain fps
			endLoopTime = GetNowCount();
			timePassed = endLoopTime - startLoopTime;
			if (timePassed < timePerFrame)
				WaitTimer(timePerFrame - timePassed);

		}

		DrawString(0,300,"GAME OVER",WHITE);

		ScreenFlip();
	}
	DxLib_End();// DX library termination process
	return 0;
}
