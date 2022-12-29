#include <SFML/Graphics.hpp>
#include <time.h>
#include <list>
using namespace sf;

const int W = 1200;
const int H = 800;

float DEGTORAD = 0.017453f;

class Animation
{

public:
	float Frame, speed;
	Sprite sprite;
	std::vector<IntRect> frames;

	Animation(){}

	Animation(Texture &t, int x, int y, int w, int h, int count, float Speed)
	{
		Frame = 0;
		speed = Speed;

		for (int i = 0; i < count; i++)
			frames.push_back(IntRect(x + i * w, y, w, h));

		sprite.setTexture(t);
		sprite.setOrigin(w / 2, h / 2);
		sprite.setTextureRect(frames[0]);
	}

	void update()
	{
		Frame += speed;
		int n = frames.size();
		if (Frame >= n)
			Frame -= n;
		if (n > 0)
			sprite.setTextureRect(frames[int(Frame)]);
	}

	bool isEnd()
	{
		return Frame + speed >= frames.size();
	}

};

class Entity
{

public:
	float x, y, dx, dy, R, angle;
	bool life;
	std::string name;
	Animation animation;

	Entity()
	{
		life = 1;
	}

	void settings(Animation &a, int X, int Y, float Angle = 0, int radius = 1)
	{
		animation = a;
		x = X;
		y = Y;
		angle = Angle;
		R = radius;
	}

	virtual void update() {};

	void draw(RenderWindow &app)
	{
		animation.sprite.setPosition(x, y);
		animation.sprite.setRotation(angle + 90);
		app.draw(animation.sprite);

		CircleShape circle(R);
		circle.setFillColor(Color(255, 0, 0, 170));
		circle.setPosition(x, y);
		circle.setOrigin(R, R);
	}

	virtual ~Entity() {};

};

class Asteroid : public Entity
{

public:
	Asteroid()
	{
		dx = rand() % 8 - 4;
		dy = rand() % 8 - 4;
		name = "Asteroid";
	}

	void update()
	{
		x += dx;
		y += dy;

		if (x > W)
			x = 0;
		if (x < 0)
			x = W;
		if (y > H)
			y = 0;
		if (y < 0)
			y = H;
	}

};

class Bullet : public Entity
{

public:
	Bullet()
	{
		name = "Bullet";
	}

	void update()
	{
		dx = cos(angle*DEGTORAD) * 6;
		dy = sin(angle*DEGTORAD) * 6;
		angle += rand() % 7 - 3;
		x += dx;
		y += dy;

		if (x > W || x < 0 || y > H || y < 0)
			life = 0;
	}

};

class Player : public Entity
{

public:
	bool thrust;

	Player()
	{
		name = "Player";
	}

	void update()
	{
		if (thrust)
		{
			dx += cos(angle * DEGTORAD) * 0.2;
			dy += sin(angle * DEGTORAD) * 0.2;
		}
		else
		{
			dx *= 0.99;
			dy *= 0.99;
		}

		int maxSpeed = 10;
		float speed = sqrt(dx * dx + dy * dy);
		if (speed > maxSpeed)
		{
			dx *= maxSpeed / speed;
			dy *= maxSpeed / speed;
		}

		x += dx;
		y += dy;

		if (x > W)
			x = 0;
		if (x < 0)
			x = W;
		if (y > H)
			y = 0;
		if (y < 0)
			y = H;
	}

};

bool isCollide(Entity *a, Entity *b)
{
	return (b->x - a->x)*(b->x - a->x) +
		(b->y - a->y)*(b->y - a->y) <
		(a->R + b->R)*(a->R + b->R);
}

int main()
{
	srand(time(0));

	RenderWindow app(VideoMode(W, H), "Asteroids");
	app.setFramerateLimit(60);

	Texture t1, t2, t3, t4, t5, t6, t7;
	t1.loadFromFile("Images/spaceship.png");
	t2.loadFromFile("Images/background.jpg");
	t3.loadFromFile("Images/explosions/type_C.png");
	t4.loadFromFile("Images/rock.png");
	t5.loadFromFile("Images/rock_small.png");
	t6.loadFromFile("Images/fire_red.png");
	t7.loadFromFile("Images/explosions/type_B.png");

	t1.setSmooth(true);
	t2.setSmooth(true);

	Sprite background(t2);

	Animation sExplosion(t3, 0, 0, 256, 256, 48, 0.5);
	Animation sRock(t4, 0, 0, 64, 64, 16, 0.2);
	Animation sRock_small(t5, 0, 0, 64, 64, 16, 0.2);
	Animation sBullet(t6, 0, 0, 32, 64, 16, 0.8);
	Animation sPlayer(t1, 40, 0, 40, 40, 1, 0);
	Animation sPlayer_go(t1, 40, 40, 40, 45, 1, 0);
	Animation sExplosion_ship(t7, 0, 0, 192, 192, 64, 0.5);

	std::list<Entity*> entities;

	for (int i = 0; i < 8; i++)
	{
		Asteroid * asteroid = new Asteroid();
		asteroid->settings(sRock, rand() % W, rand() % H, rand() % 360, 25);
		entities.push_back(asteroid);
	}

	Player *player = new Player();
	player->settings(sPlayer, W / 2, H / 2, 0, 20);
	entities.push_back(player);

	while (app.isOpen())
	{
		Event event;
		while (app.pollEvent(event))
		{
			if (event.type == Event::Closed)
				app.close();

			if(event.type == Event::KeyPressed)
				if (event.key.code == Keyboard::Space)
				{
					Bullet *bullet = new Bullet();
					bullet->settings(sBullet, player->x, player->y, player->angle, 10);
					entities.push_back(bullet);
				}
		}

		if (Keyboard::isKeyPressed(Keyboard::Right)) 
			player->angle += 3;
		if (Keyboard::isKeyPressed(Keyboard::Left)) 
			player->angle -= 3;
		if (Keyboard::isKeyPressed(Keyboard::Up))
			player->thrust = true;
		else
			player->thrust = false;

		for(auto a:entities)
			for (auto b : entities)
			{
				if(a->name == "Asteroid" && b->name == "Bullet")
					if (isCollide(a, b))
					{
						a->life = false;
						b->life = false;

						Entity *e = new Entity();
						e->settings(sExplosion, a->x, a->y);
						e->name = "Explosion";
						entities.push_back(e);

						for (int i = 0; i < 2; i++)
						{
							if (a->R == 15)
								continue;
							Entity *e = new Asteroid();
							e->settings(sRock_small, a->x, a->y, rand() % 360, 15);
							entities.push_back(e);
						}

					}

				if(a->name == "Player" && b->name == "Asteroid")
					if (isCollide(a, b))
					{
						b->life = false;

						Entity *e = new Entity();
						e->settings(sExplosion_ship, a->x, a->y);
						e->name = "Explosion";
						entities.push_back(e);

						player->settings(sPlayer, W / 2, H / 2, 0, 20);
						player->dx = 0;
						player->dy = 0;
					}

			}

		if (player->thrust)
			player->animation = sPlayer_go;
		else
			player->animation = sPlayer;

		for (auto e : entities)
			if (e->name == "Explosion")
				if (e->animation.isEnd())
					e->life = 0;

		if (rand() % 150 == 0)
		{
			Asteroid *asteroid = new Asteroid();
			asteroid->settings(sRock, 0, rand() % H, rand() % 360, 25);
			entities.push_back(asteroid);
		}

		for (auto it = entities.begin(); it != entities.end();)
		{
			Entity *e = *it;

			e->update();
			e->animation.update();

			if (e->life == false)
			{
				it = entities.erase(it);
				delete e;
			}
			else
				it++;
		}

		app.draw(background);
		for (auto i : entities)
			i->draw(app);
		app.display();
	}

	return 0;

}
